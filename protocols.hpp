/**
 * @file protocols.hpp
 * @brief Defines the Command and ServerStatus enums for protocol operations.
 * @details This file contains the definitions of the Command and ServerStatus enums used for client-server communication.
 *
 * @version 1.0
 * @author Dmitriy Gorodov
 * @id 342725405
 * @date 24/01/2025
 */

#pragma once

#include <cstdint>

 /**
  * @enum Command
  * @brief Represents the various commands that can be sent by the client.
  */
enum class Command : uint8_t {
	SAVE_FILE = 100,      ///< Command to save a file.
	DELETE_FILE = 201,    ///< Command to delete a file.
	LIST_FILES = 202,     ///< Command to list all files.
	RESTORE_FILES = 200   ///< Command to restore a file.
};

/**
 * @enum ServerStatus
 * @brief Represents the various statuses that can be returned by the server.
 */
enum class ServerStatus : uint16_t {
    SUCCESS_FOUND = 210,       ///< Status indicating the file was found and returned.
    SUCCESS_FILE_LIST = 211,   ///< Status indicating the file list was returned.
    SUCCESS_NO_PAYLOAD = 212,  ///< Status indicating the operation was successful with no payload.
    ERR_FILE_NOT_FOUND = 1001, ///< Error status indicating the file was not found.
    ERR_NO_FILES = 1002,       ///< Error status indicating there are no files for the specific client.
    ERR_GENERAL = 1003         ///< General error status indicating an error occurred with the server.
};