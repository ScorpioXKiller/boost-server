/**
 * @file Server.h
 * @brief Server class implementation.
 * @details This class manages the server operations, including accepting client connections and handling them.
 *
 * @version 1.0
 * @author Dmitriy Gorodov
 * @id 342725405
 * @date 24/01/2025
 */

#include "Server.h"
#include "ClientSession.h"
#include <iostream>

using boost::asio::ip::tcp;

Server::Server(boost::asio::io_context& io_context, const unsigned short port)
    : io_context_(io_context),
    acceptor_(io_context_, tcp::endpoint(tcp::v4(), port))
{
    start_accept();
}

void Server::start_accept()
{
    auto socket = std::make_shared<tcp::socket>(io_context_);
    acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code& error)
        {
            handle_accept(socket, error);
        });
}

void Server::handle_accept(std::shared_ptr<tcp::socket> socket, const boost::system::error_code& error)
{
    if (!error)
    {
	    std::cout << "Accepted connection from: " << socket->remote_endpoint() << "\n";
        const auto session = std::make_shared<ClientSession>(std::move(socket));
        session->start();
    }
    else
    {
	    std::cerr << "Error accepting connection: " << error.message() << "\n";
    }
    start_accept();
}