#pragma once

#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;

class Session : public enable_shared_from_this<Session> {
public:
	explicit Session(boost::asio::ip::tcp::socket socket);
	void start();

private:
	void do_response();
	void do_write_response(const string& response, bool close_after = false);
	void handle_backup_request();

private:
	boost::asio::ip::tcp::socket socket_;
	vector<char> buffer_;
};