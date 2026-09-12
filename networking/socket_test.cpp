#include "socket.h"

#include <gtest/gtest.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>


TEST(SocketTest, CreatesValidSocket) {
    Socket socket;

    EXPECT_TRUE(socket.IsValid());
    EXPECT_NE(socket.GetFd(), -1);
}


TEST(SocketTest, SocketCanBind) {
    Socket socket;

    EndPoint endpoint{
        "127.0.0.1",
        0
    };

    EXPECT_NO_THROW(socket.Bind(endpoint));
}


TEST(SocketTest, SocketCanListen) {
    Socket socket;

    EndPoint endpoint{
        "127.0.0.1",
        0
    };

    ASSERT_NO_THROW(socket.Bind(endpoint));
    EXPECT_NO_THROW(socket.Listen(10));
}


TEST(SocketTest, ClientCanConnectToServer) {
    Socket server;

    EndPoint server_endpoint{
        "127.0.0.1",
        0
    };

    ASSERT_NO_THROW(server.Bind(server_endpoint));
    ASSERT_NO_THROW(server.Listen(10));

    sockaddr_in addr{};
    socklen_t addr_len = sizeof(addr);

    ASSERT_EQ(
        ::getsockname(
            server.GetFd(),
            reinterpret_cast<sockaddr*>(&addr),
            &addr_len
        ),
        0
    );

    uint16_t port = ntohs(addr.sin_port);

    Socket client;

    EndPoint client_endpoint{
        "127.0.0.1",
        port
    };

    EXPECT_NO_THROW(client.Connect(client_endpoint));
}


TEST(SocketTest, AcceptCreatesValidClientSocket) {
    Socket server;

    EndPoint server_endpoint{
        "127.0.0.1",
        0
    };

    ASSERT_NO_THROW(server.Bind(server_endpoint));
    ASSERT_NO_THROW(server.Listen(10));

    sockaddr_in addr{};
    socklen_t addr_len = sizeof(addr);

    ASSERT_EQ(
        ::getsockname(
            server.GetFd(),
            reinterpret_cast<sockaddr*>(&addr),
            &addr_len
        ),
        0
    );

    uint16_t port = ntohs(addr.sin_port);

    Socket client;

    EndPoint client_endpoint{
        "127.0.0.1",
        port
    };

    ASSERT_NO_THROW(client.Connect(client_endpoint));

    Socket accepted;

    // We need Accept() to block until the client connects.
    ASSERT_NO_THROW(accepted = server.Accept());

    EXPECT_TRUE(accepted.IsValid());
    EXPECT_NE(accepted.GetFd(), -1);

    EXPECT_NE(
        accepted.GetFd(),
        server.GetFd()
    );
}


TEST(SocketTest, ClientCanSendAndServerCanReceive) {
    Socket server;

    EndPoint server_endpoint{
        "127.0.0.1",
        0
    };

    ASSERT_NO_THROW(server.Bind(server_endpoint));
    ASSERT_NO_THROW(server.Listen(10));

    sockaddr_in addr{};
    socklen_t addr_len = sizeof(addr);

    ASSERT_EQ(
        ::getsockname(
            server.GetFd(),
            reinterpret_cast<sockaddr*>(&addr),
            &addr_len
        ),
        0
    );

    uint16_t port = ntohs(addr.sin_port);

    Socket client;

    EndPoint client_endpoint{
        "127.0.0.1",
        port
    };

    ASSERT_NO_THROW(client.Connect(client_endpoint));

    Socket accepted = server.Accept();

    std::string message = "Hello server";

    ssize_t bytes_sent = client.Send(
        client.GetFd(),
        message
    );

    ASSERT_EQ(
        bytes_sent,
        static_cast<ssize_t>(message.size())
    );

    std::string received;

    ssize_t bytes_received = accepted.Receive(
        accepted.GetFd(),
        received
    );

    ASSERT_EQ(
        bytes_received,
        static_cast<ssize_t>(message.size())
    );

    EXPECT_EQ(received, message);
}


TEST(SocketTest, ServerCanSendAndClientCanReceive) {
    Socket server;

    EndPoint server_endpoint{
        "127.0.0.1",
        0
    };

    ASSERT_NO_THROW(server.Bind(server_endpoint));
    ASSERT_NO_THROW(server.Listen(10));

    sockaddr_in addr{};
    socklen_t addr_len = sizeof(addr);

    ASSERT_EQ(
        ::getsockname(
            server.GetFd(),
            reinterpret_cast<sockaddr*>(&addr),
            &addr_len
        ),
        0
    );

    uint16_t port = ntohs(addr.sin_port);

    Socket client;

    EndPoint client_endpoint{
        "127.0.0.1",
        port
    };

    ASSERT_NO_THROW(client.Connect(client_endpoint));

    Socket accepted = server.Accept();

    std::string message = "Hello client";

    ssize_t bytes_sent = accepted.Send(
        accepted.GetFd(),
        message
    );

    ASSERT_EQ(
        bytes_sent,
        static_cast<ssize_t>(message.size())
    );

    std::string received;

    ssize_t bytes_received = client.Receive(
        client.GetFd(),
        received
    );

    ASSERT_EQ(
        bytes_received,
        static_cast<ssize_t>(message.size())
    );

    EXPECT_EQ(received, message);
}


TEST(SocketTest, ReceiveReturnsZeroWhenPeerClosesConnection) {
    Socket server;

    EndPoint server_endpoint{
        "127.0.0.1",
        0
    };

    ASSERT_NO_THROW(server.Bind(server_endpoint));
    ASSERT_NO_THROW(server.Listen(10));

    sockaddr_in addr{};
    socklen_t addr_len = sizeof(addr);

    ASSERT_EQ(
        ::getsockname(
            server.GetFd(),
            reinterpret_cast<sockaddr*>(&addr),
            &addr_len
        ),
        0
    );

    uint16_t port = ntohs(addr.sin_port);

    Socket client;

    EndPoint client_endpoint{
        "127.0.0.1",
        port
    };

    ASSERT_NO_THROW(client.Connect(client_endpoint));

    Socket accepted = server.Accept();

    // Client closes its connection.
    ::close(client.GetFd());

    std::string received;

    ssize_t result = accepted.Receive(
        accepted.GetFd(),
        received
    );

    EXPECT_EQ(result, 0);
}