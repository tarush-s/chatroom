#pragma once

#include "../networking/socket.h"

#include <string>

namespace client {
    
class Client {
public: 
    explicit Client(Socket socket, std::string username);
    
    void SendMessage(const std::string& message);
    std::string ReceiveMessage();

    const std::string& GetUserName() const;
    [[nodiscard]] int GetFd() const;
private: 
    std::string username_;
    Socket socket_;
};  

} // end of namespace 