/**
 * @file utility.h
 * @brief This file implements utility functions.
 * @details It contains utility functions for reading and writing integers in little-endian format.
 *
 * @version 1.0
 * @author Dmitriy Gorodov
 * @id 342725405
 * @date 24/01/2025
 */

#include "utility.h"

uint16_t read_uint_16_le(const unsigned char* data, const short start, const short end) {
    return data[start] |
        static_cast<uint16_t>((data[end] << 8));
}

uint32_t read_uint_32_le(const unsigned char* data) {
    return (static_cast<uint32_t>(data[0])) |
        ((static_cast<uint32_t>(data[1]) << 8)) |
        ((static_cast<uint32_t>(data[2]) << 16)) |
        ((static_cast<uint32_t>(data[3]) << 24));
}

void write_uint8(std::vector<unsigned char>& buffer, const uint8_t value)
{
    buffer.push_back(value);
}

void write_uint16_le(std::vector<unsigned char>& buffer, const uint16_t value)
{
    buffer.push_back(static_cast<unsigned char>(value & 0xFF));
    buffer.push_back(static_cast<unsigned char>((value >> 8) & 0xFF));
}

void write_uint32_le(std::vector<unsigned char>& buffer, const uint32_t value)
{
    buffer.push_back(static_cast<unsigned char>(value & 0xFF));
    buffer.push_back(static_cast<unsigned char>((value >> 8) & 0xFF));
    buffer.push_back(static_cast<unsigned char>((value >> 16) & 0xFF));
    buffer.push_back(static_cast<unsigned char>((value >> 24) & 0xFF));
}