#include "protocols.h"

#include <cstdint>
#include <random>
#include <string>

using namespace std;

uint16_t read_uint_16_le(unsigned char* data, short start, short end) {
    return (static_cast<unsigned char>(data[start])) |
        static_cast<uint16_t>((static_cast<unsigned char>(data[end]) << 8));
}

uint32_t read_uint_32_le(unsigned char* data) {
    return (static_cast<uint32_t>(static_cast<unsigned char>(data[0]))) |
        ((static_cast<uint32_t>(static_cast<unsigned char>(data[1])) << 8)) |
        ((static_cast<uint32_t>(static_cast<unsigned char>(data[2])) << 16)) |
        ((static_cast<uint32_t>(static_cast<unsigned char>(data[3])) << 24));
}

void write_uint_16_le(char* data, uint16_t value) {
    data[0] = static_cast<char>(value & 0xFF);
    data[1] = static_cast<char>((value >> 8) & 0xFF);
}

void write_uint_32_le(char* data, uint32_t value) {
    data[0] = static_cast<char>(value & 0xFF);
    data[1] = static_cast<char>((value >> 8) & 0xFF);
    data[2] = static_cast<char>((value >> 16) & 0xFF);
    data[3] = static_cast<char>((value >> 24) & 0xFF);
}