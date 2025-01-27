/**
 * @file ProtocolParcer.h
 * @brief ProtocolParcer class definition.
 * @details This header file contains the ProtocolParcer class definition for reading requests from and writing responses to the client, following a specific protocol.
 * @version 1.0
 * @author Dmitriy Gorodov
 * @id 342725405
 * @date 24/01/2025
 */

#pragma once
#include "Request.h"
#include "Response.h"
#include <boost/asio.hpp>

constexpr short MAX_BUFFER_SIZE = 4096; // 4KB 
constexpr short REQUEST_HEADER_SIZE = 8; // 4(user_id) + 1(version) + 1(op_code) + 2(name_len)
constexpr short PAYLOAD_FILE_SIZE = 4; // file_size (4 bytes)

/**
 * @class ProtocolParcer
 * @brief Handles reading requests from and writing responses to the client.
 */
class ProtocolParcer {
public:
    /**
     * @brief Constructs a ProtocolParcer with a given socket.
     * @param socket The socket for communication with the client.
     */
    ProtocolParcer(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

    /**
     * @brief Reads a single request from the client (blocking read).
     * @param ec The error code to set if an error occurs.
     * @return The parsed request.
     * @throws std::runtime_error if an error occurs during reading.
     */
    Request read_request(boost::system::error_code& ec) const;

    /**
     * @brief Writes the given Response to the client (blocking write).
	 * @details The response is written in the following format:
     *   version (1 byte)
	 *   status  (2 bytes, little-endian)
	 *   name_len(2 bytes, little-endian)
     *   filename (name_len bytes)
     *   if status=210 or 211 => 4-byte payload size + payload
     * @param resp The response to write.
     * @throws std::runtime_error if an error occurs during writing.
     */
    void write_response(const Response& resp) const;

private:
	/**
	 * @brief The socket for communication with the client.
	 */
    std::shared_ptr<boost::asio::basic_stream_socket<boost::asio::ip::tcp>> socket_;

    /**
     * @brief Reads exactly the specified number of bytes from the socket.
     * @param buffer The buffer to read into.
     * @param size The number of bytes to read.
     * @param ec The error code to set if an error occurs.
     * @return True if the read was successful, false otherwise.
     */
    bool read_exact(void* buffer, size_t size, boost::system::error_code& ec) const;

    /**
     * @brief Writes exactly the specified number of bytes to the socket.
     * @param buffer The buffer to write from.
     * @param size The number of bytes to write.
     * @return True if the write was successful, false otherwise.
     */
	bool write_exact(const void* buffer, size_t size) const;
};