/*
 *
 * Copyright 2017 Asylo authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "asylo/util/logging.h"
#include "asylo/platform/posix/sockets/socket_client.h"
#include "asylo/platform/posix/sockets/socket_server.h"
#include "asylo/platform/posix/sockets/socket_test.pb.h"
#include "asylo/platform/posix/sockets/socket_test_transmit.h"
#include "asylo/test/util/enclave_test_application.h"

namespace asylo {

class Inet6SocketTest : public EnclaveTestCase {
 public:
  Status Run(const EnclaveInput &input, EnclaveOutput *output) override {
    if (!input.HasExtension(socket_test_input)) {
      return Status(error::GoogleError::INVALID_ARGUMENT,
                    "Missing inet6 socket_test_input");
    }
    SocketTestInput test_input = input.GetExtension(socket_test_input);
    if (!test_input.has_action() || !test_input.has_server_port()) {
      return Status(error::GoogleError::INVALID_ARGUMENT,
                    "inet6 socket_test_input is incomplete");
    }
    SocketTestInput::SocketAction enc_command = test_input.action();
    int server_port = test_input.server_port();

    if (enc_command == SocketTestInput::INITSERVER) {
      SocketTestOutput *test_output = nullptr;
      if (output) {
        test_output = output->MutableExtension(socket_test_output);
      }
      return EncSetupServer(server_port, test_output);
    } else if (enc_command == SocketTestInput::RUNSERVER) {
      return EncRunServer();
    } else if (enc_command == SocketTestInput::RUNCLIENT) {
      return EncRunClient(server_port);
    } else {
      return Status(error::GoogleError::INVALID_ARGUMENT,
                    "Unrecognized command for inet6 socket test");
    }
  }

 private:
  // Runs INET6 socket server inside enclave.
  Status EncSetupServer(int enc_server_port, SocketTestOutput *output) {
    if (!enc_socket_server_.ServerSetup(enc_server_port).ok()) {
      return Status(error::GoogleError::INTERNAL, "Server setup failed");
    }
    if (output) {
      output->set_server_port(enc_socket_server_.GetPort());
    }
    return Status::OkStatus();
  }

  // Runs INET6 socket server inside enclave.
  Status EncRunServer() {
    if (!enc_socket_server_.ServerAccept().ok()) {
      return Status(error::GoogleError::INTERNAL, "Server accept failed");
    }
    if (!ServerTransmit(&enc_socket_server_).ok()) {
      return Status(error::GoogleError::INTERNAL, "Server transmit failed");
    }
    return Status::OkStatus();
  }

  // Runs INET6 socket client inside enclave.
  Status EncRunClient(int app_server_port) {
    SocketClient enc_socket_client;
    if (!enc_socket_client.ClientSetup(kLocalIpv6AddrStr, app_server_port)
             .ok()) {
      return Status(error::GoogleError::INTERNAL, "Client setup failed");
    }
    if (!ClientTransmit(&enc_socket_client).ok()) {
      return Status(error::GoogleError::INTERNAL, "Client transmit failed");
    }
    return Status::OkStatus();
  }

  SocketServer enc_socket_server_;
};
TrustedApplication *BuildTrustedApplication() { return new Inet6SocketTest; }

}  // namespace asylo