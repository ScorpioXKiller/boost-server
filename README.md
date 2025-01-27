# File Server Manager

## Overview

This repository contains a **C++-based server application** that interacts with a **Python client** to perform various file operations such as **listing**, **backing up**, **restoring**, and **deleting files**. The server listens for incoming connections, processes client requests according to a custom protocol, and performs the requested file operations.

## Features

- **Custom Protocol Implementation**: Defines and uses custom protocols for communication between the client and server.
- **Handle Client Requests**: Processes various client requests including saving files, deleting files, listing files, and restoring files.
- **Response Generation**: Sends appropriate responses back to the client with status codes and payloads as needed.
- **Asynchronous Networking**: Utilizes **Boost.Asio** for asynchronous network operations to handle multiple client connections efficiently.
- **Comprehensive Logging**: Provides utility functions for logging and debugging, including hex dumps of data.

## File Structure

- **`main.cpp`**: The main program that initializes and runs the server.
- **`Server.h` / `Server.cpp`**: Implements the `Server` class, handling incoming client connections and request processing.
- **`Request.h`**: Defines the `Request` struct, representing a client's request after parsing the protocol.
- **`Response.h`**: Defines the `Response` struct, representing the server's response to a client request.
- **`protocols.h`**: Defines the `Command` and `ServerStatus` enums used in the protocol communication.
- **`utility.h` / `utility.cpp`**: Provides utility functions for data serialization/deserialization, logging, and debugging.

## Usage

### Prerequisites

- **C++ Compiler**: A compiler that supports **C++17** (e.g., GCC 7.2 or later, Clang 5.0 or later).
- **Boost Libraries**: **Boost.Asio** library is required for networking. Install Boost before building the server.

#### Installing Boost (if not already installed)

- **Ubuntu/Debian**: