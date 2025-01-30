/**
 * @file main.cpp
 * @brief Entry point for the server application.
 * @details This file contains the main function that initializes and runs the server.
 *
 * @version 1.0
 * @date 24/01/2025
 * @id 342725405
 * @author Dmitriy Gorodov
 */

#include "Server.hpp"
#include <iostream>

constexpr unsigned short PORT = 8080;

/**
 * @brief The main function that initializes and runs the server.
 * @return 0 on successful execution, non-zero on error.
 */
int main()
{
	try
	{
		boost::asio::io_context io_context;
		Server server(io_context, PORT);
		std::cout << "Listening on port " << PORT << "...\n";
		io_context.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
	return 0;
}