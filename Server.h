#pragma once

#include <boost/asio.hpp>
#include <memory>

using namespace std;

class Server
{
public:
    Server(boost::asio::io_context& io_context, unsigned short port);
    void start_accept();

private:
    void handle_accept(shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code& error);

    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
};