/*
 * Copyright (c) 2018 IOTA Stiftung
 * https://github.com/iotaledger/rpchub
 *
 * Refer to the LICENSE file for licensing information
 */

#include "hub/commands/helper.h"

#include <sstream>
#include <string>

#include "proto/hub.pb.h"

namespace hub {
namespace cmd {
std::string errorToString(const hub::rpc::ErrorCode& e) {
  std::ostringstream ss;
  hub::rpc::Error error;

  error.set_code(e);
  error.SerializeToOstream(&ss);

  return ss.str();
}

bool isAddressValid(std::string_view sv) {
  bool is_valid = true;
  switch (sv[80]) {
    case '9':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
      break;
    default:
      is_valid = false;
  }
  return is_valid;
}

std::string boolToString(bool isTrue) { return isTrue ? "true" : "false"; }
bool stringToBool(std::string isTrueStr) {
  return isTrueStr.compare("true") ? true : false;
}
}  // namespace cmd
}  // namespace hub
