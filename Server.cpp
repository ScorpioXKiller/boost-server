#include "Server.h"
#include "ClientSession.h"
#include <iostream>

using boost::asio::ip::tcp;

Server::Server(boost::asio::io_context& io_context, unsigned short port)
    : io_context_(io_context),
    acceptor_(io_context_, tcp::endpoint(tcp::v4(), port))
{
    start_accept();
}

void Server::start_accept()
{
    auto socket = make_shared<tcp::socket>(io_context_);
    acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code& error)
        {
            handle_accept(socket, error);
        });
}

void Server::handle_accept(shared_ptr<tcp::socket> socket, const boost::system::error_code& error)
{
    if (!error)
    {
        cout << "\n[Server] Accepted new client connection.\n";
        auto session = make_shared<ClientSession>(move(socket));
        session->start();
    }
    else
    {
        cerr << "[Server] Accept error: " << error.message() << "\n";
    }
    start_accept();
}