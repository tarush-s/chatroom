#include "client.h"

#include <arpa/inet.h>   
#include <stdexcept>

namespace client {

static constexpr uint32_t kMaxMessageSize = 1u << 20;

Client::Client(Socket socket, std::string username) : socket_(std::move(socket)), username_(username) {}
    
void Client::SendMessage(const std::string& message) {
    if (message.size() > kMaxMessageSize) {
        throw std::runtime_error("Message exceeds maximum allowed size");
    }

    uint32_t len_network = htonl(static_cast<uint32_t>(message.size()));
    if (!socket_.SendAll(&len_network, sizeof len_network)) {
        throw std::runtime_error("Failed to send message length");
    }
    if (!socket_.SendAll(message.data(), message.size())) {
        throw std::runtime_error("Failed to send message body");
    }
}

std::string Client::ReceiveMessage() {
    uint32_t len_network = 0;
    if (!socket_.ReceiveAll(&len_network, sizeof len_network)) {
        throw std::runtime_error("Connection closed while reading message length");
    }

    uint32_t len = ntohl(len_network);
    if (len > kMaxMessageSize) {
        throw std::runtime_error("Advertised message length exceeds maximum allowed size");
    }

    std::string message(len, '\0');
    if (len > 0 && !socket_.ReceiveAll(message.data(), len)) {
        throw std::runtime_error("Connection closed while reading message body");
    }
    return message;
}

const std::string& Client::GetUserName() const {
    return username_;
}

[[nodiscard]] int Client::GetFd() const {
    return socket_.GetFd();
}

} // end of namespace 