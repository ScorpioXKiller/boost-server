/**
 * @file ClientSession.h
 * @brief ClientSession class implementation.
 * @details This class manages the lifecycle of a client session, including reading requests, processing commands, and sending responses.
 *
 * @version 1.0
 * @author Dmitriy Gorodov
 * @id 342725405
 * @date 24/01/2025
 */

#include "ClientSession.h"
#include "utility.h"
#include "protocols.h"
#include "ProtocolParcer.h"
#include "FileManager.h"
#include "Request.h"
#include "Response.h"

#include <iostream>
#include <filesystem>
#include <thread>

ClientSession::ClientSession(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
    : socket_(std::move(socket))
{
}

void ClientSession::start()
{
    auto self = shared_from_this();
    std::thread([self]() {
        self->handle_client_requests();
        }).detach();
}

void ClientSession::handle_client_requests() const
{
    try
    {
        // Create a protocol parser on this socket 
        ProtocolParcer parser(socket_);

		// Prepare a response object
        Response response;

		// Loop to handle multiple requests from the same client until the client disconnects
		while (true)
        {
			if (!this->socket_->is_open())
			{
				std::cout << "Client disconnected.\n";
				break;
			}

            // Read the client's request (blocking call)
			boost::system::error_code ec;
            auto [user_id, version, op_code, filename, file_data] = parser.read_request(ec);

            // Check for client disconnection
            if (ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset)
            {
                std::cout << "Client disconnected.\n";
                break;
            }

			if (ec)
			{
				send_error_response(response, "Server Error: Error reading request: " + ec.message());
				break;
			}

			// Basic check for path traversal
            if (filename.find("..") != std::string::npos)
            {
                send_error_response(response,"Server Error: Invalid filename (possible path traversal).");
                return;
            }

            // Setup a FileManager (responsible for file operations)
            const FileManager file_manager(STORAGE_FOLDER);

            // Create the root directory if it doesn't exist
            try
            {
                file_manager.create_root_directory();
            }
            catch (const std::filesystem::filesystem_error& error)
            {
				send_error_response(response, "Server Error: Cannot create root directory: " + std::string(error.what()));
				return;
            }

            // Create a user directory if it doesn't exist
            try
            {
                file_manager.create_user_directory(user_id);
            }
            catch (const std::filesystem::filesystem_error& error)
            {
				send_error_response(response, "Server Error: Cannot create user directory: " + std::string(error.what()));
                return;
            }


			// Remove any preceding slash/backslash from the filename
            if (const auto last_slash = filename.find_last_of("/\\"); last_slash != std::string::npos)
            {
                filename = filename.substr(last_slash + 1);
            }

            // Switch on op_code
            switch (op_code)
            {
            case Command::SAVE_FILE:
            {
                try
                {
                    if (const bool success = file_manager.save_file(user_id, filename, file_data); !success)
                    {
                        send_error_response(response, "Server Error: Error while the saving file: " + filename);
                    }
                    else
                    {
                        response.filename = filename;
                        response.status = ServerStatus::SUCCESS_NO_PAYLOAD; // 212
						parser.write_response(response);
                    }
                    break;
                }
				catch (const std::filesystem::filesystem_error& error)
				{
					response.status = ServerStatus::ERR_GENERAL;
					send_error_response(response, "Server error: " + std::string(error.what()));
					break;
				}
				catch (...)
				{
					response.status = ServerStatus::ERR_GENERAL;
					send_error_response(response, "Server Error: Error processing SAVE_FILE request.");
                    break;
				}
            }

            case Command::RESTORE_FILES:
            {
                try
                {
	                std::vector<unsigned char> data;
                    response.filename = filename;

                    if (const bool found = file_manager.read_file(user_id, filename, data); !found)
                    {
                        response.status = ServerStatus::ERR_FILE_NOT_FOUND;
                        send_error_response(response, "Error restoring file: file not found for this user.");
                    }
                    else
                    {
                        response.status = ServerStatus::SUCCESS_FOUND;
                        response.payload = std::move(data);
						parser.write_response(response);
                    }
                    break;
                }
				catch (const std::filesystem::filesystem_error& error)
				{
					response.status = ServerStatus::ERR_GENERAL;
					send_error_response(response, "Server error: " + std::string(error.what()));
					break;
				}
				catch (...)
				{
					response.status = ServerStatus::ERR_GENERAL;
					send_error_response(response, "Server Error: Error processing RESTORE_FILES request.");
                    break;
				}
            }

            case Command::DELETE_FILE:
            {
                try
                {
                    response.filename = filename;

                    if (const bool removed = file_manager.delete_file(user_id, filename); !removed)
                    {
						response.status = ServerStatus::ERR_FILE_NOT_FOUND;
						send_error_response(response,"Error deleting file: file not found for this user.");
						break;
                    }

                    response.status = ServerStatus::SUCCESS_NO_PAYLOAD;
					parser.write_response(response);
                    break;
				}
                catch (const std::filesystem::filesystem_error& error)
                {
					response.status = ServerStatus::ERR_GENERAL;
                    send_error_response(response, "Server error: " + std::string(error.what()));
                    break;
                }
                catch (...)
                {
                    response.status = ServerStatus::ERR_GENERAL;
					send_error_response(response, "Server Error: Error processing DELETE_FILE request.");
                    break;
                }
            }

            case Command::LIST_FILES:
            {
                try
                {
	                if (const auto files = file_manager.list_user_files(user_id); files.empty())
                    {
						response.status = ServerStatus::ERR_NO_FILES;
						send_error_response(response, "Error listing files: no files found for this user.");
                        break;
                    }
                    else
                    {
                        // create a random txt file containing the list
                        std::string txt_file = file_manager.write_file_list(user_id, files);

                        // read that file into 'payload'
                        std::vector<unsigned char> data;
                        if (const bool ok = file_manager.read_file(user_id, txt_file, data); !ok)
                        {
							response.status = ServerStatus::ERR_GENERAL;
							send_error_response(response, "Server Error: Error while reading file list.");
                            break;
                        }
                        
                        response.status = ServerStatus::SUCCESS_FILE_LIST;
                    	response.filename = txt_file;
                    	response.payload = std::move(data);
						parser.write_response(response);
						break;
                    }
                }
				catch (const std::filesystem::filesystem_error& error)
				{
					response.status = ServerStatus::ERR_GENERAL;
					send_error_response(response, "Server Error: " + std::string(error.what()));
					break;
				}
				catch (...)
				{
					response.status = ServerStatus::ERR_GENERAL;
					send_error_response(response, "Server Error: Error processing LIST_FILES request.");
					break;
				}
            }

            default:
				send_error_response(response, "Server error: the operation [" + std::to_string(static_cast<int>(op_code)) + "] is not supported.");
                continue;
            }

			// Clear the response for the next iteration
			response = Response();

        }
    }
    catch (const std::exception& e)
    {
		Response response;
        std::cerr << "[Server][Thread " << std::this_thread::get_id()
            << "] Exception in handle_client_requests(): " << e.what() << "\n";

		send_error_response(response, "Fatal server error: " + std::string(e.what()));
    }
}

void ClientSession::send_error_response(const Response& response, const std::string& message) const
{
	std::cerr << " => returning error " << static_cast<uint16_t>(response.status) << "\n";

    try {
        ProtocolParcer parser(socket_);
		parser.write_response(response);
    }
    catch (...) {
	    std::cerr << "[Server][Thread " << std::this_thread::get_id()
			<< "] Error sending error response\n";
    }
}