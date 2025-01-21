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