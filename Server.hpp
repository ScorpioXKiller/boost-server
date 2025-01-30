/**
 * @file Server.hpp
 * @brief Server class definition.
 * @details This header file contains the Server class definition for managing client connections and handling them asynchronously.
 *
 * @version 1.0
 * @author Dmitriy Gorodov
 * @id 342725405
 * @date 24/01/2025
 */

#pragma once

#include <boost/asio.hpp>

/**
 * @class Server
 * @brief Manages the server operations, including accepting client connections and handling them.
 */
class Server
{
public:
    /**
     * @brief Constructs a Server with a given io_context and port.
     * @param io_context The io_context for asynchronous operations.
     * @param port The port on which the server listens for connections.
     */
    Server(boost::asio::io_context& io_context, unsigned short port);

    /**
     * @brief Starts accepting client connections.
     */
	void start_accept();

private:
    /**
     * @brief Handles the acceptance of a new client connection.
     * @param socket The socket for the new client connection.
     * @param error The error code to set if an error occurs.
     */
    void handle_accept(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code& error);

	/**
    * @brief The io_context for asynchronous operations.
    */
    boost::asio::io_context& io_context_;

	/**
    * @brief The acceptor for client connections.
    */
    boost::asio::ip::tcp::acceptor acceptor_;
};