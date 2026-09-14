#pragma once 

#include "../utility/smart_fd.h"
#include "endpoint.h"

#include <cstddef>
#include <cstdint>

class Socket {
public:
    Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept = default;
    Socket& operator=(Socket&& other) noexcept = default;

    void Bind(const EndPoint& endpoint);
    void Listen(const int backlog);

    Socket Accept();
    void Connect(const EndPoint& endpoint);

    bool SendAll(const void* data, std::size_t len);
    bool ReceiveAll(void* buffer, std::size_t len);
    
    ssize_t Send(const void* data, std::size_t len);
    ssize_t Receive(void* buffer, std::size_t len);

    [[nodiscard]] int GetFd() const;
    [[nodiscard]] bool IsValid() const;

private: 
    explicit Socket(int fd);
    SmartFd fd_;
};
