/**
 * @file Response.hpp
 * @brief Defines the Response struct for server responses.
 * @details This file contains the definition of the Response struct used for server responses to client requests.
 *
 * @version 1.0
 * @author Dmitriy Gorodov
 * @id 342725405
 * @date 24/01/2025
 */

#pragma once

#include "protocols.hpp"

#include <string>
#include <vector>

 /**
  * @struct Response
  * @brief Represents the server's response to a single request.
  */
struct Response {
    uint8_t version = 1;                              ///< Server version.
    ServerStatus status = ServerStatus::ERR_GENERAL;  ///< Status of the response.
    std::string filename;                             ///< The filename returned to the client (may be empty if not relevant).
	std::vector<unsigned char> payload;               ///< The payload if status is 210 (file found) or 211 (list).
};