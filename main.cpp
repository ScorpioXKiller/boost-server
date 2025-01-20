/*#include <boost/asio.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <thread>

#ifdef _WIN32
  #include <direct.h>
  #include <cerrno>
#else
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <cerrno>
#endif // DEBUG

using namespace boost::asio::ip;
using namespace boost;
using namespace std;

const string STORAGE_FOLDER = "c:/backups/";

static bool create_directory_if_not_exists(const std::string& dir)
{
#ifdef _WIN32
	// _mkdir returns 0 if successful, -1 if an error occurred.
	// If the directory already exists, it sets errno to EEXIST.
	if (_mkdir(dir.c_str()) == 0 || errno == EEXIST)
		return true;
	else
		return false;
#else
	// On POSIX systems, mkdir returns 0 if successful, or -1 on error.
	// We also check if directory already exists (errno == EEXIST).
	struct stat st;
	// stat(...) returns 0 on success; it fills st with info about 'dir'.
	// If it doesn't exist, we attempt to create it.
	if (stat(dir.c_str(), &st) != 0)
	{
		// Directory does not exist, so try to create it
		if (mkdir(dir.c_str(), 0755) == 0)
			return true;
		else
			return false;
	}
	else
	{
		// Directory already exists, or 'dir' is something else
		// Check if it's actually a directory
		if ((st.st_mode & S_IFDIR) != 0)
			return true;  // It's a directory
		else
			return false; // It's not a directory
	}
#endif
}

static void handle_client(tcp::socket *socket)
{
	try
	{
        system::error_code error;
        char filename_buffer[512] = {};

        size_t filename_length = socket->read_some(
			asio::buffer(filename_buffer, 511), 
            error
        );

		if (error)
		{
			cerr << "[Server][Thread " << this_thread::get_id() << "] Error reading filename: " << error.message() << "\n";
			delete socket;
			return;
		}

		string request(filename_buffer, filename_length);
		const string prefix = "BACKUP:";
		size_t colon_pos = request.find(':');

		if (colon_pos == string::npos)
		{
			cerr << "[Server][Thread " << this_thread::get_id() << "] Invalid request received (no colon): " << request << "\n";
			delete socket;
			return;
		}

		if (request.compare(0, prefix.size(), prefix) != 0)
		{
			cerr << "[Server][Thread " << this_thread::get_id() << "] Invalid request received (prefix mismatch): " << request << "\n";
			delete socket;
			return;
		}

		string remainder = request.substr(prefix.size());

		size_t second_colon_pos = remainder.find(':');

		if (second_colon_pos == string::npos)
		{
			cerr << "[Server][Thread " << this_thread::get_id() << "] Invalid request received (missing second colon): " << request << "\n";
			delete socket;
			return;
		}

		string user_id = remainder.substr(0, second_colon_pos);
		string filename = remainder.substr(second_colon_pos + 1);

		while (!filename.empty() &&
			(filename[filename.size() - 1] == '\n' ||
				filename[filename.size() - 1] == '\r' ||
				filename[filename.size() - 1] == ' '))
		{
			filename.erase(filename.size() - 1);
		}

		while (!user_id.empty() &&
			(user_id[user_id.size() - 1] == '\n' ||
				user_id[user_id.size() - 1] == '\r' ||
				user_id[user_id.size() - 1] == ' '))
		{
			user_id.erase(user_id.size() - 1);
		}

		cout << "[Server][Thread " << this_thread::get_id() << "] user_id: " << user_id << ", filename: " << filename << "\n";

		// remove the folder path from the filename
		size_t last_slash = filename.find_last_of("/\\");

		if (last_slash != string::npos)
		{
			filename = filename.substr(last_slash + 1);
		}

		cout << "[Server][Thread " << this_thread::get_id() << "] The client: " << user_id << " wants to back up the file: " << filename << "\n";

		string ready_message = "READY";
		write(*socket, asio::buffer(ready_message), error);

		if (!create_directory_if_not_exists(STORAGE_FOLDER))
		{
			std::cerr << "[Server][Thread "
				<< this_thread::get_id()
				<< "] Failed to create or access folder: " << STORAGE_FOLDER << "\n";
			delete socket;
			return;
		}

		string user_folder = STORAGE_FOLDER + user_id + "/";
		if (!create_directory_if_not_exists(user_folder))
		{
			std::cerr << "[Server][Thread "
				<< this_thread::get_id()
				<< "] Failed to create or access folder: " << user_folder << "\n";
			delete socket;
			return;
		}

		string full_path = user_folder + filename;
		ofstream output_file(full_path.c_str(), ios::binary);
		if (!output_file.is_open())
		{
			cerr << "[Server][Thread " << this_thread::get_id() << "] Failed to open file for writing: " << full_path << "\n";
			delete socket;
			return;
		}

		char data_buffer[1024];
		while (true)
		{
			memset(data_buffer, 0, 1024);
            size_t length = socket->read_some(asio::buffer(data_buffer), error);

			if (error == asio::error::eof)
			{
                cout << "[Server][Thread "
                    << this_thread::get_id()
                    << "] Client finished sending file data (EOF)."
                    << "\n";
				break;
			}

			else if (error)
			{
				cerr << "[Server][Thread " << this_thread::get_id() << "] Error while receiving file data: " << error.message() << "\n";
				break;
			}

			output_file.write(data_buffer, static_cast<streamsize>(length));
		}

		output_file.close();
		cout << "[Server][Thread " << this_thread::get_id() << "] File successfully saved as: " << user_folder << "\n";
	}

	catch (std::exception& e)
	{
		cerr << "[Server][Thread " << this_thread::get_id() << "] Exception: " << e.what() << "\n";
	}

	delete socket;
}

int main()
{
	try
	{
		asio::io_context io_service;
		tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 1234));

		cout << "[Server] Listening on port 1234..." << "\n";

		while (true)
		{
			// Dynamically allocate a new socket for each client
			auto new_socket = new tcp::socket(io_service);

			// Accept the incoming connection
			acceptor.accept(*new_socket);
			std::cout << "[Server] Accepted a new client connection." << "\n";

			// Spawn a new thread to handle the client
			thread t([new_socket] { handle_client(new_socket); });

			// Detach so the thread will clean up on its own
			// (the main thread won't wait for it to finish)
			t.detach();
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "[Server] Exception: " << e.what() << "\n";
	}

	return 0;
}*/

#include <boost/asio.hpp>
#include "Server.h"
#include <iostream>

using namespace std;

int main()
{
	try
	{
		boost::asio::io_context io_context;
		Server server(io_context, 1234);
		cout << "[Server] Listening on port 1234...\n";
		io_context.run();
	}
	catch (const exception& e)
	{
		cerr << "[Server] Exception: " << e.what() << "\n";
	}
	return 0;
}