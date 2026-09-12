#include "socket.h"

#include <arpa/inet.h> 
#include <netinet/in.h>
#include <sys/socket.h>

#include <stdexcept>

Socket::Socket() : fd_(::socket(AF_INET, SOCK_STREAM, 0)) {
    if(!fd_.IsValid()) {
        throw std::runtime_error("Could not create a socket");
    }
}

Socket::Socket(int fd) : fd_(fd) {}

void Socket::Bind(const EndPoint& endpoint) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(endpoint.port);

    if (::inet_pton(AF_INET, endpoint.ip_address.c_str(), &addr.sin_addr) != 1) {
        throw std::runtime_error("Invalid Ip address");
    }

    if (::bind(fd_.Get(), reinterpret_cast<sockaddr*>(&addr), sizeof addr) == -1) {
        throw std::runtime_error("Could not bind");
    }
}

void Socket::Listen(const int backlog) {
    if (::listen(fd_.Get(), backlog) == -1) {
        throw std::runtime_error("Unalbe to listen");
    }
}

Socket Socket::Accept() {
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof client_addr;

    int fd = ::accept(fd_.Get(), reinterpret_cast<sockaddr*>(&client_addr), &client_len); 
    if (fd == -1) {
        throw std::runtime_error("Could not create accept socket");
    }

    return Socket(fd);
}

void Socket::Connect(const EndPoint& endpoint) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(endpoint.port);

    if (::inet_pton(AF_INET, endpoint.ip_address.c_str(), &addr.sin_addr) != 1) {
        throw std::runtime_error("Invalid Ip address");
    }

    if (::connect(fd_.Get(), reinterpret_cast<sockaddr*>(&addr), sizeof addr) == -1) {
        throw std::runtime_error("Could not connect");
    }
}

ssize_t Socket::Send(int fd, const std::string& message) {
    return ::send(fd, message.data(), message.size(), 0);
}

ssize_t Socket::Receive(int fd, std::string& message) {
    message.resize(1024);
    ssize_t bytes_received = recv(fd, message.data(), message.size(), 0);
    if (bytes_received > 0) {
        message.resize(static_cast<std::size_t>(bytes_received));
    }

    return bytes_received;
}

int Socket::GetFd() const {
    return fd_.Get();
}

bool Socket::IsValid() const {
    return fd_.IsValid();
}