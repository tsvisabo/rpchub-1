/*
 * Copyright (c) 2018 IOTA Stiftung
 * https://github.com/iotaledger/rpchub
 *
 * Refer to the LICENSE file for licensing information
 */

#include "hub/commands/user_withdraw_cancel.h"

#include <cstdint>

#include <sqlpp11/connection.h>
#include <sqlpp11/functions.h>
#include <sqlpp11/select.h>

#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "common/stats/session.h"
#include "hub/db/db.h"
#include "hub/db/helper.h"
#include "schema/schema.h"

#include "hub/commands/factory.h"
#include "hub/commands/helper.h"

namespace hub {
namespace cmd {

static CommandFactoryRegistrator<UserWithdrawCancel> registrator;

boost::property_tree::ptree UserWithdrawCancel::doProcess(
    const boost::property_tree::ptree& request) noexcept {
  boost::property_tree::ptree tree;
  UserWithdrawCancelRequest req;
  UserWithdrawCancelReply rep;
  auto maybeUuid = request.get_optional<std::string>("uuid");
  if (maybeUuid) {
    req.uuid = maybeUuid.value();
  }

  auto status = doProcess(&req, &rep);

  if (status != common::cmd::OK) {
    tree.add("error", common::cmd::errorToStringMap.at(status));
  } else {
    tree.add("success", rep.success ? "true" : "false");
  }

  return tree;
}

common::cmd::Error UserWithdrawCancel::doProcess(
    const UserWithdrawCancelRequest* request,
    UserWithdrawCancelReply* response) noexcept {
  auto& connection = db::DBManager::get().connection();
  auto transaction = connection.transaction();

  nonstd::optional<common::cmd::Error> errorCode;
  bool success = false;

  try {
    boost::uuids::uuid uuid = boost::uuids::string_generator()(request->uuid);

    auto result = connection.cancelWithdrawal(boost::uuids::to_string(uuid));

    success = result != 0;

    if (success) {
      auto withdrawalInfo =
          connection.getWithdrawalInfoFromUUID(boost::uuids::to_string(uuid));

      // Add user account balance entry
      connection.createUserAccountBalanceEntry(
          withdrawalInfo.userId, withdrawalInfo.amount,
          db::UserAccountBalanceReason::WITHDRAWAL_CANCEL, withdrawalInfo.id);

      transaction->commit();

      LOG(INFO) << "Withdrawal: " << request->uuid
                << " cancelled successfully.";
    } else {
      transaction->rollback();
      LOG(ERROR)
          << "Withdrawal: " << request->uuid
          << " can not be cancelled. (either it had been swept or it has "
             "already been cancelled)";

      errorCode = common::cmd::WITHDRAWAL_CAN_NOT_BE_CANCELLED;
    }
  } catch (const std::exception& ex) {
    LOG(ERROR) << session() << " Commit failed: " << ex.what();

    try {
      transaction->rollback();
    } catch (const sqlpp::exception& ex) {
      LOG(ERROR) << session() << " Rollback failed: " << ex.what();
    }

    errorCode = common::cmd::UNKNOWN_ERROR;
  }

  if (errorCode) {
    return errorCode.value();
  }

  response->success = success;

  return common::cmd::OK;
}

}  // namespace cmd
}  // namespace hub
