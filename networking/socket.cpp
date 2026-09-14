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

ssize_t  Socket::Send(const void* data, std::size_t len){
    ssize_t sent;
    do {
        sent = ::send(fd_.Get(), data, len, MSG_NOSIGNAL);
    } while (sent < 0 && errno == EINTR);

    return sent;
}

ssize_t Socket::Receive(void* buffer, std::size_t len) {
    ssize_t received;
    do {
        received = ::recv(fd_.Get(), buffer, len, 0);
    } while (received < 0 && errno == EINTR);

    return received;
}

bool Socket::SendAll(const void* data, std::size_t len) {
    const char* ptr = static_cast<const char*>(data);
    std::size_t total_sent = 0;
    
    while(total_sent < len ) {
        ssize_t sent = Send(ptr + total_sent, len - total_sent);
        if (sent < 0) {
            return false;
        }

        total_sent += static_cast<std::size_t>(sent);
    }
    
    return true;
}

bool Socket::ReceiveAll(void* buffer, std::size_t len) {
    char* ptr = static_cast<char*>(buffer);
    std::size_t total_received = 0;
    
    while(total_received < len ) {
        ssize_t received = Receive(ptr + total_received, len - total_received);
        if (received <= 0) {
            return false;
        }

        total_received += static_cast<std::size_t>(received);
    }
    
    return true;
}

int Socket::GetFd() const {
    return fd_.Get();
}

bool Socket::IsValid() const {
    return fd_.IsValid();
}