/**
 * @file ClientSession.h
 * @brief ClientSession class definition.
 * @details This header file contains the ClientSession class definition for handling client requests.
 * @version 1.0
 * @author Dmitriy Gorodov
 * @id 342725405
 * @date 24/01/2025
 */

#pragma once
#include "Response.h"

#include <boost/asio.hpp>
#include <string>

constexpr unsigned short SERVER_VERSION = 1;
const std::string STORAGE_FOLDER = "c:/backupsvr/";

/**
 * @class ClientSession
 * @brief Manages a single client session, handling requests and sending responses.
 */
class ClientSession : public std::enable_shared_from_this<ClientSession>
{
public:
    /**
     * @brief Constructs a ClientSession with a given socket.
     * @param socket The socket for communication with the client.
     */
    explicit ClientSession(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

    /**
     * @brief Starts the client session in a new thread.
     */
    void start();

private:
    /**
     * @brief Handles client requests in a loop until the client disconnects.
     * @details This method performs the following steps:
     * 1. Creates a protocol parser on the socket.
     * 2. Enters a loop to handle multiple requests from the same client until the client disconnects.
     * 3. Reads the client's request and checks for disconnection or errors.
     * 4. Performs a basic check for path traversal in the filename.
     * 5. Sets up a FileManager for file operations.
     * 6. Creates the root directory and user directory if they do not exist.
     * 7. Processes the request based on the operation code (op_code):
     *    - SAVE_FILE: Saves the file to the user's directory.
     *    - RESTORE_FILES: Restores the file from the user's directory.
     *    - DELETE_FILE: Deletes the file from the user's directory.
     *    - LIST_FILES: Lists all files in the user's directory.
     * 8. Sends the appropriate response to the client.
     * 9. Handles any exceptions that occur during request processing.
     */
	void handle_client_requests() const;

    /**
     * @brief Sends an error response to the client.
     * @param response The response object containing the error status.
     * @param message The error message to log.
     */
	void send_error_response(const Response& response, const std::string& message) const;

	/**
	 * @brief The socket for communication with the client.
	 */
    std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
};