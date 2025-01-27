/**
 * @file Request.h
 * @brief Defines the Request struct for client requests.
 * @details This file contains the definition of the Request struct used for client requests to the server.
 *
 * @version 1.0
 * @author Dmitriy Gorodov
 * @id 342725405
 * @date 24/01/2025
 */

#pragma once

#include "protocols.h"

#include <string>
#include <vector>

 /**
  * @struct Request
  * @brief Represents a single request from the client after parsing the protocol.
  */
struct Request {
    uint32_t user_id = 0;                       ///< The user ID associated with the request.
    uint8_t version = 0;                        ///< The protocol version of the request.
    Command op_code = static_cast<Command>(0);  ///< The operation code indicating the type of request.
    std::string filename;                       ///< The filename sent by the client (possibly empty for some operations).
    std::vector<unsigned char> file_data;       ///< The file data for SAVE_FILE operations (not used for DELETE/RESTORE/LIST).
};
