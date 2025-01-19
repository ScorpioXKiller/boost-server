// ClientSession.h
#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <string>

class ClientSession : public std::enable_shared_from_this<ClientSession>
{
public:
    explicit ClientSession(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void start();

private:
    void handle_read();

    std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
};
