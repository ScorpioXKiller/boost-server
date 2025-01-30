/**
 * @file utility.hpp
 * @brief This file defines utility functions.
 * @details It contains utility functions for reading and writing integers in little-endian format.
 *
 * @version 1.0
 * @author Dmitriy Gorodov
 * @id 342725405
 * @date 24/01/2025
 */

#pragma once

#include <boost/asio.hpp>
#include <filesystem>

/**
 * @brief Reads a 16-bit unsigned integer from the given data in little-endian format.
 * @param data The data to read from.
 * @param start The starting byte position (default is 0).
 * @param end The ending byte position (default is 1).
 * @return The 16-bit unsigned integer read from the data.
 */
uint16_t read_uint_16_le(const unsigned char* data, short start = 0, short end = 1);

/**
 * @brief Reads a 32-bit unsigned integer from the given data in little-endian format.
 * @param data The data to read from.
 * @return The 32-bit unsigned integer read from the data.
 */
uint32_t read_uint_32_le(const unsigned char* data);

/**
 * @brief Writes an 8-bit unsigned integer to the given buffer.
 * @param buffer The buffer to write to.
 * @param value The 8-bit unsigned integer to write.
 */
void write_uint8(std::vector<unsigned char>& buffer, uint8_t value);

/**
 * @brief Writes a 16-bit unsigned integer to the given buffer in little-endian format.
 * @param buffer The buffer to write to.
 * @param value The 16-bit unsigned integer to write.
 */
void write_uint16_le(std::vector<unsigned char>& buffer, uint16_t value);

/**
 * @brief Writes a 32-bit unsigned integer to the given buffer in little-endian format.
 * @param buffer The buffer to write to.
 * @param value The 32-bit unsigned integer to write.
 */
void write_uint32_le(std::vector<unsigned char>& buffer, uint32_t value);