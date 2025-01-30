/**
 * @file ProtocolParcer.cpp
 * @brief ProtocolParcer class implementation.
 * @details This file contains the implementation of the ProtocolParcer class, which reads requests from and writes responses to the client following a specific protocol.
 *
 * @version 1.0
 * @author Dmitriy Gorodov
 * @id 342725405
 * @date 24/01/2025
 */

#include "ProtocolParcer.hpp"
#include "utility.hpp"

#include <iostream>

ProtocolParcer::ProtocolParcer(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
    : socket_(std::move(socket)){}

Request ProtocolParcer::read_request(boost::system::error_code& ec) const
{
    Request request;
	ec.clear();

    // Read the main 8-byte header
    unsigned char header[REQUEST_HEADER_SIZE];
    if (!read_exact(header, REQUEST_HEADER_SIZE, ec))
    {
		if (!ec)
		{
            ec = boost::asio::error::operation_aborted;
		}

		throw std::runtime_error("Error reading header");
    }

	// Parse the header
    request.user_id = read_uint_32_le(header);       // [0..3] user_id
    request.version = header[4];                    // [4]    version
    request.op_code = static_cast<Command>(header[5]); // [5] op_code

    // Read filename
    if (const uint16_t name_len = read_uint_16_le(header, 6, 7); name_len > 0)
    {
	    std::vector<unsigned char> name_buf(name_len, 0);
        if (!read_exact(name_buf.data(), name_len, ec))
        {
			if (!ec)
			{
				ec = boost::asio::error::operation_aborted;
			}

			throw std::runtime_error("Error reading filename");
        }
        request.filename.assign(reinterpret_cast<const char*>(name_buf.data()), name_len);
    }

    // If this is SAVE_FILE (op_code=100), read 4 more bytes => file_size, then read file_data
    if (request.op_code == Command::SAVE_FILE)
    {
        unsigned char size_buf[PAYLOAD_FILE_SIZE];
        if (!read_exact(size_buf, PAYLOAD_FILE_SIZE, ec))
        {
            if (!ec)
            {
				ec = boost::asio::error::operation_aborted;
            }

			throw std::runtime_error("Error reading file_size");
        }
        const uint32_t file_size = read_uint_32_le(size_buf);

        // Now read 'file_size' bytes in chunks
        request.file_data.resize(file_size);
        size_t total_read = 0;
        while (total_read < file_size)
        {
            const size_t to_read = std::min(static_cast<size_t>(MAX_BUFFER_SIZE), file_size - total_read);

            if (!read_exact(request.file_data.data() + total_read, to_read, ec))
            {
                if (!ec)
                {
                    ec = boost::asio::error::operation_aborted;
                }

                throw std::runtime_error("Error reading file data");
            }
			total_read += to_read;
        }
    }

    return request;
}

void ProtocolParcer::write_response(const Response& resp) const
{
	std::vector<unsigned char> buffer;

    write_uint8(buffer, resp.version);

    const uint16_t status = static_cast<uint16_t>(resp.status);
    write_uint16_le(buffer, status);

    const uint16_t name_len = static_cast<uint16_t>(resp.filename.size());
    write_uint16_le(buffer, name_len);

    buffer.insert(buffer.end(), resp.filename.begin(), resp.filename.end());

    if (status == 210 || status == 211)
    {
        const uint32_t p_size = static_cast<uint32_t>(resp.payload.size());
        write_uint32_le(buffer, p_size);

        buffer.insert(buffer.end(), resp.payload.begin(), resp.payload.end());
    }

    if (!write_exact(buffer.data(), buffer.size()))
    {
        std::cerr << "Error sending response\n";
    }
}

bool ProtocolParcer::read_exact(void* buffer, const size_t size, boost::system::error_code& ec) const
{
    size_t total_read = 0;
    auto* out = static_cast<unsigned char*>(buffer);

    while (total_read < size)
    {
        boost::system::error_code error;
        const size_t chunk = socket_->read_some(boost::asio::buffer(out + total_read, size - total_read), ec);
        if (error)
        {
			ec = error;
            return false;
        }
        total_read += chunk;
    }
    return true;
}

bool ProtocolParcer::write_exact(const void* buffer, const std::size_t size) const
{
    size_t total_written = 0;
    auto* in = static_cast<const unsigned char*>(buffer);

    while (total_written < size)
    {
        boost::system::error_code ec;
        const size_t chunk = socket_->write_some(boost::asio::buffer(in + total_written, size - total_written), ec);
        if (ec)
        {
            return false;
        }
        total_written += chunk;
    }
    return true;
}