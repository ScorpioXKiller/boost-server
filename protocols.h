#pragma once

#include <cstdint>
#include <string>

using namespace std;

enum class Command : uint8_t {
	SAVE_FILE = 100,
	DELETE_FILE = 201,
	LIST_FILES = 202,
	RESTORE_FILES = 200
};

enum class ServerStatus : uint16_t {
	SUCCESS_FOUND = 210, // "File found/restored" or "File saved" with payload
	SUCCESS_FILE_LIST = 211, // "List of files" returned
	SUCCESS_NO_PAYLOAD = 212, // "Backup or delete success" (no payload)
	ERR_FILE_NOT_FOUND = 1001,
	ERR_NO_FILES = 1002,
	ERR_GENERAL = 1003
};

uint16_t read_uint_16_le(unsigned char* data, short start = 0, short end = 1);
uint32_t read_uint_32_le(unsigned char* data);

void write_uint_16_le(char* data, uint16_t value);
void write_uint_32_le(char* data, uint32_t value);