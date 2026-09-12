#pragma once 

#include <unistd.h>
#include <utility>

struct FdCloser {
    void close(int fd) const {
        if (fd != -1) {
            ::close(fd);
        }
    }
};

template <typename Closer> 
class UniqueFd{
public:
    UniqueFd() = default;
    explicit UniqueFd(int fd) : fd_(fd) {}

    UniqueFd(const UniqueFd&) = delete;
    UniqueFd& operator=(const UniqueFd&) = delete;

    UniqueFd(UniqueFd&& other) noexcept : 
            fd_(std::exchange(other.fd_, kInvalidFd)) {}

    UniqueFd& operator=(UniqueFd&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        Reset(std::exchange(other.fd_, kInvalidFd));
        return *this;
    }

    void Reset(int fd = kInvalidFd) {
        if (fd_ != kInvalidFd){
            Closer closer;
            closer.close(fd_);
        }

        fd_ = fd;
    }

    [[nodiscard]] int Get() const {
        return fd_;
    }

    [[nodiscard]] bool IsValid() const {
        return fd_ != kInvalidFd;
    }

    ~UniqueFd() {
        Reset();
    }

private:
    static constexpr int kInvalidFd = -1;
    int fd_ {kInvalidFd};
};

using SmartFd = UniqueFd<FdCloser>;