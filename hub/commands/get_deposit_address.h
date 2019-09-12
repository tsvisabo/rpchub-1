/*
 * Copyright (c) 2018 IOTA Stiftung
 * https://github.com/iotaledger/rpchub
 *
 * Refer to the LICENSE file for licensing information
 */

#ifndef HUB_COMMANDS_GET_DEPOSIT_ADDRESS_H_
#define HUB_COMMANDS_GET_DEPOSIT_ADDRESS_H_

#include <string>

#include "common/commands/command.h"

namespace hub {
namespace rpc {
class GetDepositAddressRequest;
class GetDepositAddressReply;
}  // namespace rpc

namespace cmd {

/// Gets a new deposit address.
/// @param[in] hub::rpc::GetDepositAddressRequest
/// @param[in] hub::rpc::GetDepositAddressReply
class GetDepositAddress
    : public common::Command<hub::rpc::GetDepositAddressRequest,
                             hub::rpc::GetDepositAddressReply> {
 public:
  using Command<hub::rpc::GetDepositAddressRequest,
                hub::rpc::GetDepositAddressReply>::Command;

  static std::shared_ptr<common::ICommand> create() {
    return std::shared_ptr<common::ICommand>(new GetDepositAddress());
  }

  static const std::string name() { return "GetDepositAddress"; }

  common::cmd::Error doProcess(
      const hub::rpc::GetDepositAddressRequest* request,
      hub::rpc::GetDepositAddressReply* response) noexcept override;

  boost::property_tree::ptree doProcess(
      const boost::property_tree::ptree& request) noexcept override;
};
}  // namespace cmd
}  // namespace hub

#endif  // HUB_COMMANDS_GET_DEPOSIT_ADDRESS_H_
