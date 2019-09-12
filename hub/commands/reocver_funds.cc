/*
 * Copyright (c) 2018 IOTA Stiftung
 * https://github.com/iotaledger/rpchub
 *
 * Refer to the LICENSE file for licensing Secretrmation
 */

#include "hub/commands/recover_funds.h"

#include <cstdint>
#include <utility>

#include <sqlpp11/exception.h>

#include "common/crypto/manager.h"
#include "common/crypto/types.h"
#include "common/stats/session.h"
#include "hub/auth/manager.h"
#include "hub/bundle/bundle_utils.h"
#include "hub/commands/factory.h"
#include "hub/commands/helper.h"
#include "hub/db/db.h"
#include "hub/db/helper.h"
#include "hub/db/types.h"
#include "proto/hub.pb.h"
#include "schema/schema.h"

namespace hub {
namespace cmd {

static CommandFactoryRegistrator<RecoverFunds> registrator;

boost::property_tree::ptree RecoverFunds::doProcess(
    const boost::property_tree::ptree& request) noexcept {
  boost::property_tree::ptree tree;
  return tree;
}

common::cmd::Error RecoverFunds::doProcess(
    const hub::rpc::RecoverFundsRequest* request,
    hub::rpc::RecoverFundsReply* response) noexcept {
  auto& connection = db::DBManager::get().connection();
  auto& cryptoProvider = common::crypto::CryptoManager::get().provider();
  auto& authProvider = auth::AuthManager::get().provider();

  try {
    uint64_t userId;

    // Get userId for identifier
    {
      auto maybeUserId = connection.userIdFromIdentifier(request->userid());
      if (!maybeUserId) {
        return common::cmd::USER_DOES_NOT_EXIST;
      }

      userId = maybeUserId.value();
    }

    nonstd::optional<common::crypto::Address> address = {
        common::crypto::Address(request->address())};
    nonstd::optional<common::crypto::Address> payoutAddress;

    if (request->validatechecksum()) {
      payoutAddress =
          std::move(common::crypto::CryptoManager::get()
                        .provider()
                        .verifyAndStripChecksum(request->payoutaddress()));

      if (!payoutAddress.has_value()) {
        return common::cmd::INVALID_CHECKSUM;
      }
    } else {
      payoutAddress = {common::crypto::Address(request->payoutaddress())};
    }

    // 1. Check that address was used before
    auto maybeAddressInfo = connection.getAddressInfo(address.value());
    if (!maybeAddressInfo) {
      return common::cmd::UNKNOWN_ADDRESS;
    }

    if (maybeAddressInfo->userId.compare(request->userid()) != 0) {
      return common::cmd::WRONG_USER_ADDRESS;
    }

    if (!maybeAddressInfo->usedForSweep) {
      return common::cmd::INVALID_ADDRESS;
    }

    // Verify payout address wasn't spent before
    auto res = _api->wereAddressesSpentFrom({payoutAddress.value().str()});
    if (!res.has_value() || res.value().states.empty()) {
      return common::cmd::IOTA_NODE_UNAVAILABLE;
    } else if (res.value().states.front()) {
      return common::cmd::ADDRESS_WAS_SPENT;
    }

    const auto& iriBalances = _api->getBalances({request->address()});
    if (!iriBalances) {
      return common::cmd::ADDRESS_NOT_KNOWN_TO_NODE;
    }

    auto amount = iriBalances.value().at(request->address());

    if (amount == 0) {
      return common::cmd::ADDRESS_BALANCE_ZERO;
    }

    std::vector<hub::db::TransferInput> deposits;

    hub::db::TransferInput ti = {maybeAddressInfo.value().id, userId,
                                 address.value(), maybeAddressInfo.value().uuid,
                                 amount};
    deposits.push_back(std::move(ti));

    std::vector<hub::db::TransferOutput> outputs;

    // Create the "withdrawl"
    outputs.emplace_back(
        hub::db::TransferOutput{-1, amount, {}, payoutAddress.value()});

    auto bundle = hub::bundle_utils::createBundle(deposits, {}, outputs, {});

    connection.createSweep(std::get<0>(bundle), std::get<1>(bundle), 0);

  } catch (const std::exception& ex) {
    return common::cmd::UNKNOWN_ERROR;
  }

  return common::cmd::OK;
}

}  // namespace cmd
}  // namespace hub
