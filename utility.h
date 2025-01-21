#pragma once

#include <boost/asio.hpp>
#include <string>
#include <filesystem>
#include <cstdint>
#include <random>
#include <iomanip>
#include <sstream>
#include <iostream>

#ifdef _WIN32
#include <direct.h>
#include <cerrno>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>
#endif

using namespace std;

bool create_directory_if_not_exists(const std::string& dir);
vector<string> list_files_in_directory(const string& dir);
string generate_random_filename();
bool read_exact(std::shared_ptr<boost::asio::ip::tcp::socket> sock, void* buffer, size_t size, boost::system::error_code& ec);
string hex_dump_limited(const unsigned char* data, size_t size, size_t max_bytes = 16);
void print_request_table(uint32_t user_id, uint8_t client_version, uint8_t op_code, uint16_t name_len, const string& filename, uint32_t file_size, const unsigned char* file_content);

uint16_t read_uint_16_le(unsigned char* data, short start = 0, short end = 1);
uint32_t read_uint_32_le(unsigned char* data);

void write_uint8(vector<unsigned char>& buffer, uint8_t value);
void write_uint16_le(vector<unsigned char>& buffer, uint16_t value);
void write_uint32_le(vector<unsigned char>& buffer, uint32_t value);