#pragma once

#include <boost/asio.hpp>

class Session : public std::enable_shared_from_this<Session> {
public:
	explicit Session(boost::asio::ip::tcp::socket socket);
	void start();

private:
	void do_response();
	void do_write_response(const std::string& response, bool close_after = false);
	void handle_backup_request();

private:
	boost::asio::ip::tcp::socket socket_;
	std::vector<char> buffer_;
};