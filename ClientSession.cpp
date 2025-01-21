#include "ClientSession.h"
#include "protocols.h"

#include "utility.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <filesystem>

using boost::asio::ip::tcp;

const string STORAGE_FOLDER = "c:/backupsvr/";
const short MAX_BUFFER_SIZE = 4096;
const short REQUEST_HEADER_SIZE = 8;
const short PAYLOAD_FILE_SIZE = 4;

ClientSession::ClientSession(shared_ptr<tcp::socket> socket)
    : socket_(move(socket)){}

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

        // --------------------------------------------------------------------
        // 1) Read the main header (8 bytes):
        //    [0..3] user_id   (4 bytes, little-endian)
        //    [4]    version   (1 byte)
        //    [5]    op_code   (1 byte)
        //    [6..7] name_len  (2 bytes, little-endian)
        // --------------------------------------------------------------------
        
        unsigned char header[REQUEST_HEADER_SIZE] = { 0 };

        if (!read_exact(socket_, header, REQUEST_HEADER_SIZE, error))
        {
            cerr << "[Server][Thread " << std::this_thread::get_id()
                << "] Error reading header: " << error.message() << "\n";
            return;
        }

        // Parse the header fields
        // user_id (little-endian 32-bit)

		uint32_t user_id = read_uint_32_le(header);

        // version (1 byte)
        uint8_t version = header[4];

        // op_code (1 byte)
        uint8_t op_code = header[5];

        // name_len (little-endian 16-bit)
		uint16_t name_len = read_uint_16_le(header, 6, 7);

        // --------------------------------------------------------------------
        // 2) Read the filename (name_len bytes)
        // --------------------------------------------------------------------
        
		string filename;
        if (name_len > 0)
        {
            vector<unsigned char> name_buf(name_len, 0);
            if (!read_exact(socket_, name_buf.data(), name_len, error))
            {
                cerr << "[Server][Thread " << this_thread::get_id()
                    << "] Error reading filename: " << error.message() << "\n";
                return;
            }

            filename.assign(reinterpret_cast<const char*>(name_buf.data()), name_len);
        }

        // Debug output
        cout << "[Server][Thread " << this_thread::get_id() << "]"
            << " user_id = " << user_id
            << ", version = " << (int)version
            << ", op_code = " << (int)op_code
            << ", filename = " << filename
            << "\n";

        //--------------------------------------------------------------
        // We'll store the server's response in these local variables,
        // then build a single binary response after we handle the request.
        //--------------------------------------------------------------
        
        uint8_t  server_version = 1;  // e.g., server version
        uint16_t response_status = (uint16_t)ServerStatus::ERR_GENERAL; // default
        string response_filename;    // For the response header
        vector<unsigned char> response_payload;

        //--------------------------------------------------------------
        // 3) Create main folder + user folder if needed
        //--------------------------------------------------------------
        if (!create_directory_if_not_exists(STORAGE_FOLDER))
        {
            cerr << "[Server] Cannot access " << STORAGE_FOLDER << "\n";
            response_status = (uint16_t)ServerStatus::ERR_GENERAL;
            // We'll build the response at the end
        }

        string user_folder = STORAGE_FOLDER + to_string(user_id) + "/";
        create_directory_if_not_exists(user_folder);

        

        //--------------------------------------------------------------
        // 4) Switch on op_code to handle different operations
        //--------------------------------------------------------------

        switch (op_code) {
            
            // ----------------------------------------------------------
            // SAVE (backup) file => op_code = 100
            // ----------------------------------------------------------
            
            case static_cast<uint8_t>(Command::SAVE_FILE):
            {
                // Next 4 bytes => file_size
                unsigned char size_buf[PAYLOAD_FILE_SIZE];
                if (!read_exact(socket_, size_buf, PAYLOAD_FILE_SIZE, error))
                {
                    cerr << "[Server] Error reading file_size: " << error.message() << "\n";
                    response_status = (uint16_t)ServerStatus::ERR_GENERAL;
                    break;
                }
                uint32_t file_size = read_uint_32_le(size_buf);

                // Strip path from filename
                size_t last_slash = filename.find_last_of("/\\");
                if (last_slash != string::npos)
                {
                    filename = filename.substr(last_slash + 1);
                }

				response_filename = filename;

                // Save file
                string file_path = user_folder + filename;
                ofstream ofs(file_path, ios::binary);
                if (!ofs.is_open())
                {
                    cerr << "[Server] Cannot open for writing: " << file_path << "\n";
                    response_status = (uint16_t)ServerStatus::ERR_GENERAL;
                    break;
                }

                uint32_t bytes_received = 0;
                char data_buffer[MAX_BUFFER_SIZE];
                while (bytes_received < file_size)
                {
                    uint32_t remain = file_size - bytes_received;
                    size_t to_read = min<uint32_t>(remain, MAX_BUFFER_SIZE);

                    size_t chunk = socket_->read_some(boost::asio::buffer(data_buffer, to_read), error);
                    if (error)
                    {
                        cerr << "[Server] Error receiving file data: " << error.message() << "\n";
                        response_status = (uint16_t)ServerStatus::ERR_GENERAL;
                        break;
                    }
                    ofs.write(data_buffer, (streamsize)chunk);
                    bytes_received += (uint32_t)chunk;
                }
                ofs.close();

                //--------------------------------------------------------------
                // Print the request as an ASCII table
                // -------------------------------------------------------------

                print_request_table(user_id, version, op_code, name_len, filename, file_size, reinterpret_cast<unsigned char*>(data_buffer));

                // On success
                if (bytes_received == file_size)
                {
                    // => status=212 means "success with no payload"
                    response_status = (uint16_t)ServerStatus::SUCCESS_NO_PAYLOAD; // 212
                }
            }
            break;

            // ----------------------------------------------------------
            // RETRIEVE (restore) file => op_code = 200
            // ----------------------------------------------------------

            case static_cast<uint8_t>(Command::RESTORE_FILES):
            {
                string file_path = user_folder + filename;
                ifstream ifs(file_path, ios::binary | ios::ate);
                if (!ifs.is_open())
                {
                    response_status = (uint16_t)ServerStatus::ERR_FILE_NOT_FOUND;
                    break;
                }

                streampos file_size = ifs.tellg();
                ifs.seekg(0, ios::beg);
                response_payload.resize(static_cast<size_t>(file_size));
                ifs.read(reinterpret_cast<char*>(response_payload.data()), file_size);
                ifs.close();

                response_filename = filename;
                response_status = (uint16_t)ServerStatus::SUCCESS_FOUND;
            }
			break;

            // ----------------------------------------------------------
            // DELETE file => op_code = 201
            // ----------------------------------------------------------

            case static_cast<uint8_t>(Command::DELETE_FILE):
            {
                string file_path = user_folder + filename;
                boost::system::error_code fs_error;
				bool removed = filesystem::remove(file_path, fs_error);
                if (removed && !fs_error)
                {
                    response_status = (uint16_t)ServerStatus::SUCCESS_NO_PAYLOAD;
                }
                else
                {
                    if (!removed && !fs_error) 
                    {
						response_status = (uint16_t)ServerStatus::ERR_FILE_NOT_FOUND;
                    }
                    else
                    {
						response_status = (uint16_t)ServerStatus::ERR_GENERAL;
                    }
                }
            }
            break;

            // ----------------------------------------------------------
            // LIST all user files => op_code = 202
            // ----------------------------------------------------------

            case static_cast<uint8_t>(Command::LIST_FILES):
            {
				auto files = list_files_in_directory(user_folder);
                if (files.empty())
                {
					response_status = (uint16_t)ServerStatus::ERR_NO_FILES;
                    break;
                }

                string txt_file_name = generate_random_filename();
				string list_file_path = user_folder + txt_file_name;

				ofstream ofs(list_file_path);
                if (!ofs.is_open())
                {
					response_status = (uint16_t)ServerStatus::ERR_GENERAL;
					break;
                }

                for (auto& file : files)
                {
					ofs << file << "\n";
                }
                ofs.close();

                ifstream ifs(list_file_path, ios::binary | ios::ate);
				auto file_size = ifs.tellg();
				ifs.seekg(0, ios::beg);
				response_payload.resize(static_cast<size_t>(file_size));
				ifs.read(reinterpret_cast<char*>(response_payload.data()), file_size);
                ifs.close();

                response_status = (uint16_t)ServerStatus::SUCCESS_FILE_LIST;
				response_filename = txt_file_name;
            }
            break;

            default:
				response_status = (uint16_t)ServerStatus::ERR_GENERAL;
                break;
        }

        //--------------------------------------------------------------
        // 5) Build the final binary response
        //
        //    Response:
        //      - version  (1 byte)
        //      - status   (2 bytes, LE)
        //      - name_len (2 bytes, LE)
        //      - filename (name_len bytes)
        //      - if status in [210,211]: size (4 bytes, LE) + payload
        //        else no payload and size
        //--------------------------------------------------------------
        
		vector<unsigned char> response;

        // (1) version (1 byte)
		write_uint8(response, server_version);

        // (2) status (2 bytes, LE)
		write_uint16_le(response, response_status);

        // (3) name_len (2 bytes, LE)
        uint16_t response_name_len = static_cast<uint16_t>(response_filename.size());
		write_uint16_le(response, response_name_len);

        // (4) filename
        response.insert(response.end(), response_filename.begin(), response_filename.end());

        // If status in {210, 211} => add payload
        if (response_status == 210 || response_status == 211)
        {
            // 4-byte payload size
            uint32_t psize = static_cast<uint32_t>(response_payload.size());
			write_uint32_le(response, psize);

            // payload
            response.insert(response.end(), response_payload.begin(), response_payload.end());
        }

		boost::asio::write(*socket_, boost::asio::buffer(response), error);
        if (error)
        {
            std::cerr << "[Server][Thread " << std::this_thread::get_id()
                << "] Error sending response: " << error.message() << "\n";
        }
        else
        {
            std::cout << "[Server][Thread " << std::this_thread::get_id()
                << "] Sent response: status=" << response_status
                << ", filename=" << response_filename
                << ", payload_size=" << response_payload.size() << "\n";
        }

    }
    catch (const exception& e)
    {
        cerr << "[Server][Thread " << this_thread::get_id()
            << "] Exception: " << e.what() << "\n";
    }
}