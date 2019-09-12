/*
 * Copyright (c) 2018 IOTA Stiftung
 * https://github.com/iotaledger/rpchub
 *
 * Refer to the LICENSE file for licensing information
 */

#ifndef HUB_COMMANDS_USER_WITHDRAW_CANCEL_H_
#define HUB_COMMANDS_USER_WITHDRAW_CANCEL_H_

#include <string>

#include "common/commands/command.h"

namespace hub {
namespace rpc {
class UserWithdrawCancelRequest;
class UserWithdrawCancelReply;
}  // namespace rpc

namespace cmd {

/// Cancels a previous withdrawal request if it hasn't been processed yet.
/// @param[in] hub::rpc::UserWithdrawCancelRequest
/// @param[in] hub::rpc::UserWithdrawCancelReply
class UserWithdrawCancel
    : public common::Command<hub::rpc::UserWithdrawCancelRequest,
                             hub::rpc::UserWithdrawCancelReply> {
 public:
  using Command<hub::rpc::UserWithdrawCancelRequest,
                hub::rpc::UserWithdrawCancelReply>::Command;

  static std::shared_ptr<common::ICommand> create() {
    return std::shared_ptr<common::ICommand>(new UserWithdrawCancel());
  }

  static const std::string name() { return "UserWithdrawCancel"; }

  common::cmd::Error doProcess(
      const hub::rpc::UserWithdrawCancelRequest* request,
      hub::rpc::UserWithdrawCancelReply* response) noexcept override;

  boost::property_tree::ptree doProcess(
      const boost::property_tree::ptree& request) noexcept override;
};
}  // namespace cmd
}  // namespace hub

#endif  // HUB_COMMANDS_USER_WITHDRAW_CANCEL_H_
