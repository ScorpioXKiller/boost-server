#include "ClientSession.h"
#include "Utility.h"
#include <iostream>
#include <fstream>
#include <thread>

using boost::asio::ip::tcp;

const std::string STORAGE_FOLDER = "c:/backups/";

ClientSession::ClientSession(std::shared_ptr<tcp::socket> socket)
    : socket_(std::move(socket))
{
}

void ClientSession::start()
{
    auto self = shared_from_this();
    std::thread([self]()
        {
            self->handle_read();
        }).detach();
}

void ClientSession::handle_read()
{
    try
    {
        boost::system::error_code error;
        char filename_buffer[512] = {};

        size_t filename_length = socket_->read_some(
            boost::asio::buffer(filename_buffer, 511),
            error
        );

        if (error)
        {
            std::cerr << "[Server][Thread " << std::this_thread::get_id()
                << "] Error reading filename: " << error.message() << "\n";
            return;
        }

        std::string request(filename_buffer, filename_length);
        const std::string prefix = "BACKUP:";
        size_t colon_pos = request.find(':');

        if (colon_pos == std::string::npos || request.compare(0, prefix.size(), prefix) != 0)
        {
            std::cerr << "[Server][Thread " << std::this_thread::get_id()
                << "] Invalid request received: " << request << "\n";
            return;
        }

        std::string remainder = request.substr(prefix.size());
        size_t second_colon_pos = remainder.find(':');

        if (second_colon_pos == std::string::npos)
        {
            std::cerr << "[Server][Thread " << std::this_thread::get_id()
                << "] Invalid request format: " << request << "\n";
            return;
        }

        std::string user_id = remainder.substr(0, second_colon_pos);
        std::string filename = remainder.substr(second_colon_pos + 1);

        // Trim trailing whitespace and newlines
        user_id.erase(user_id.find_last_not_of(" \n\r") + 1);
        filename.erase(filename.find_last_not_of(" \n\r") + 1);

        std::cout << "[Server][Thread " << std::this_thread::get_id()
            << "] User ID: " << user_id << ", Filename: " << filename << "\n";

        // Extract filename without path
        size_t last_slash = filename.find_last_of("/\\");
        if (last_slash != std::string::npos)
        {
            filename = filename.substr(last_slash + 1);
        }

        std::cout << "[Server][Thread " << std::this_thread::get_id()
            << "] Client " << user_id << " wants to back up file: " << filename << "\n";

        // Send ready message to client
        std::string ready_message = "READY";
        boost::asio::write(*socket_, boost::asio::buffer(ready_message), error);

        // Create necessary directories
        if (!create_directory_if_not_exists(STORAGE_FOLDER))
        {
            std::cerr << "[Server][Thread " << std::this_thread::get_id()
                << "] Failed to access folder: " << STORAGE_FOLDER << "\n";
            return;
        }

        std::string user_folder = STORAGE_FOLDER + user_id + "/";
        if (!create_directory_if_not_exists(user_folder))
        {
            std::cerr << "[Server][Thread " << std::this_thread::get_id()
                << "] Failed to access folder: " << user_folder << "\n";
            return;
        }

        std::string full_path = user_folder + filename;
        std::ofstream output_file(full_path, std::ios::binary);
        if (!output_file.is_open())
        {
            std::cerr << "[Server][Thread " << std::this_thread::get_id()
                << "] Failed to open file: " << full_path << "\n";
            return;
        }

        // Read file data from client
        char data_buffer[1024];
        while (true)
        {
            size_t length = socket_->read_some(boost::asio::buffer(data_buffer), error);

            if (error == boost::asio::error::eof)
            {
                std::cout << "[Server][Thread " << std::this_thread::get_id()
                    << "] Client finished sending data.\n";
                break;
            }
            else if (error)
            {
                std::cerr << "[Server][Thread " << std::this_thread::get_id()
                    << "] Error receiving data: " << error.message() << "\n";
                break;
            }

            output_file.write(data_buffer, static_cast<std::streamsize>(length));
        }

        output_file.close();
        std::cout << "[Server][Thread " << std::this_thread::get_id()
            << "] File saved: " << full_path << "\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "[Server][Thread " << std::this_thread::get_id()
            << "] Exception: " << e.what() << "\n";
    }
}
