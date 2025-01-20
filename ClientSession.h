// ClientSession.h
#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <string>

using namespace std;

class ClientSession : public enable_shared_from_this<ClientSession>
{
public:
    explicit ClientSession(shared_ptr<boost::asio::ip::tcp::socket> socket);
    void start();

private:
    void handle_read();

    shared_ptr<boost::asio::ip::tcp::socket> socket_;
	vector<char> buffer_;
};
