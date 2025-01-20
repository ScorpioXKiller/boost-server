#include "Session.h"
#define BOOST_ASIO_ENABLE_HANDLER_TRACKING

#include <fstream>
#include <iostream>
#include <string>

static const string BACKUP_ROOT = "c:/backupsvr/";

Session::Session(ip::tcp::socket socket) : socket_(std::move(socket)) {}

void Session::start() { do_response(); }

void Session::do_response() {
    auto self(shared_from_this());
	auto buffer = make_shared<vector<char>>(1024);

    async_read(
        socket_, boost::asio::buffer(*buffer),
        [this, self, buffer](boost::system::error_code error,
            std::size_t length) {
                if (!error) {
                    string message(buffer->data(), length);
                    cout << "Received message from client: " << message << "\n";


                    if (message == "Hello from client") {
                    	const auto response = make_shared<string>("Hello, Client. I'm ready to help you.") ;
                        do_write_response(*response);
                    }
                    else if (message == "I want to backup a file") {
                        handle_backup_request();
                    }
                    else if (message == "Yes, I want to save another file") {
                        const auto response = make_shared<string>("Please send the next file.");
                        do_write_response(*response);
                    }
                    else if (message == "No") {
                        const auto response = make_shared<string>("Glad to help.");
                        do_write_response(*response, true); // Disconnect after this
                    }
                    else {
                        const auto response = make_shared<string>("Unknown command.");
                        do_write_response(*response);
                    }
                }
                else {
                    cerr << "Error reading message: " << error.message() << "\n";

					if (error == error::eof || error == error::connection_reset) {
						cout << "Client disconnected." << "\n";
					} else
					{
						cerr << "Unhandled error: " << error.message() << "\n";
					}
                }
        });
}

//void Session::handle_backup_request() {
//    auto self(shared_from_this());
//
//    // Read the file name
//    auto filename_buf = make_shared<boost::asio::streambuf>();
//    async_read_until(socket_, *filename_buf, "\n",
//        [this, self, filename_buf](const boost::system::error_code& error, std::size_t /*length*/) {
//        	if (!error) {
//                istream is(filename_buf.get());
//                string file_name;
//                getline(is, file_name);
//                file_name.erase(file_name.find_last_not_of("\r\n") + 1); // Trim newline
//
//                // Read the file size
//                auto filesize_buf = make_shared<boost::asio::streambuf>();
//                async_read_until(socket_, *filesize_buf, "\n",
//                    [this, self, file_name, filesize_buf](const boost::system::error_code& error, std::size_t /*length*/) {
//                        if (!error) {
//                            istream is(filesize_buf.get());
//                            string filesize_str;
//                            getline(is, filesize_str);
//                            size_t file_size = std::stoul(filesize_str);
//
//                            // Read the file data
//                            auto file_buf = make_shared<std::vector<char>>(file_size);
//                            async_read(socket_, buffer(*file_buf, file_size),
//                                [this, self, file_name, file_buf](boost::system::error_code error, std::size_t /*length*/) {
//                                    if (!error) {
//                                        // Save the received data to a file
//                                        std::ofstream file(BACKUP_ROOT + file_name, ios::binary);
//                                        if (file.is_open()) {
//                                            file.write(file_buf->data(), file_buf->size());
//                                            file.close();
//
//                                            string response = "File uploaded successfully. Would you like to save another file?\n";
//                                            do_write_response(response);
//                                        }
//                                        else {
//                                            cerr << "Unable to open file for writing.\n";
//                                        }
//                                    }
//                                    else {
//                                        cerr << "Error reading file data: " << error.message() << endl;
//                                    }
//                                    do_response(); // Continue reading client messages
//                                });
//                        }
//                        else {
//                            cerr << "Error reading file size: " << error.message() << endl;
//                            do_response(); // Continue reading client messages
//                        }
//                    });
//            }
//            else {
//                cerr << "Error reading file name: " << error.message() << endl;
//                do_response(); // Continue reading client messages
//            }
//        });
//}


void Session::handle_backup_request() {
    auto self(shared_from_this());
    auto streambuf = std::make_shared<boost::asio::streambuf>();

    async_read(socket_, *streambuf,
        [this, self, streambuf](boost::system::error_code error, std::size_t length) {
            if (!error) {
                std::ofstream file(BACKUP_ROOT, std::ios::binary);
                std::istream is(streambuf.get());
                std::vector<char> data(length);
                is.read(data.data(), length);
                file.write(data.data(), length);
                file.close();

                auto response = std::make_shared<std::string>("File uploaded successfully. Could you save another one file?\n");
                do_write_response(*response);
            }
            else {
                std::cerr << "Error uploading file: " << error.message() << std::endl;
            }
        });
}



void Session::do_write_response(const std::string& response, bool close_after) {
    auto self(shared_from_this());
	auto response_ptr = make_shared<string>(response);

    async_write(socket_, buffer(*response_ptr),
        [this, self, response_ptr, close_after](
            boost::system::error_code error, size_t /*length*/) {
                if (error) {
                    cerr << "Error sending response: " << error.message()
                        << endl;
                }
                else {
                    cout << "Response sent: " << *response_ptr << endl;
                }

                if (close_after) {
                    socket_.close(); // Disconnect client
                    cout << "Client disconnected.\n";
                }
        });
}