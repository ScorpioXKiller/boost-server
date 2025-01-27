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

  ```bash
  sudo apt-get install libboost-all-dev
  ```

- **CentOS/Fedora**:
  ```bash 
  sudo yum install boost-devel
  ```

- **Windows**:
  - Download and install Boost from the official website: [Boost Downloads](https://www.boost.org/users/download/)

### Build Instructions

1. **Clone the Repository**:

   ```bash
   git clone https://github.com/yourusername/file-server-manager.git cd file-server-manager 
   ```

2. **Build the Server**:

   - Using a **Makefile** (if provided):

   ```bash
   make
   ```

   - Alternatively, compile using `g++`:

   ```bash
    g++ -std=c++17 -o server main.cpp Server.cpp utility.cpp -lboost_system
   ```

### Running the Server

1. **Configure the Server**:

   - The default port is set to `8080` in `main.cpp`. To change the port, modify the `PORT` constant:

     ```cpp
     constexpr unsigned short PORT = your_desired_port;
     ```

2. **Run the Server**:

   ```bash
   ./server
   ```
   
   - The server will start listening on the specified port and output:

   ```bash
     Server started on port 8080
     ```

3. **Connect with the Client**:

   - Ensure the Python client is configured to connect to the correct IP and port specified by the server.
   - Follow the client's instructions to perform file operations.

## Example

Here is an example of how to use the server alongside the Python client:

1. **Start the Server**:

   ```bash
   ./server
   ```

2. **Prepare the Client**:

   - Update the `server.info` file in the client repository with the server's IP and port:

   ```bash
    127.0.0.1:8080
   ```

3. **Run the Python Client**:

   ```bash
   python client.py
   ```
   - The client will connect to the server and display the available commands.


4. **Perform Operations**:

   - Use the client to send requests to the server for backing up, restoring, listing, or deleting files.

## Documentation

Each file and class in the repository includes comprehensive comments explaining its purpose, parameters, and functionality. Here is a brief overview:

- **`main.cpp`**: Contains the `main` function that initializes the server and starts the `io_context` event loop.
- **`Server` Class** (`Server.h` / `Server.cpp`): Manages client connections, reads incoming data, parses requests, and sends responses.
- **`Request` Struct** (`Request.h`): Represents a parsed client request, including user ID, operation code, filename, and file data.
- **`Response` Struct** (`Response.h`): Represents the server's response, including status codes, filenames, and payloads.
- **`protocols.h`**: Defines enums for `Command` and `ServerStatus` to standardize operation codes and status responses.
- **Utility Functions** (`utility.h` / `utility.cpp`): Includes functions for reading and writing data in little-endian format, hex dumping, and printing request details.

## Dependencies

- **Boost.Asio**: For asynchronous networking operations.
- **C++17 Standard**: Utilizes modern C++ features; ensure your compiler supports C++17.

## Notes

- **Error Handling**: The server includes basic error handling and will output exceptions to `stderr`.
- **Asynchronous Design**: Designed to handle multiple client connections using asynchronous operations without blocking the main thread.

## Author

- **Dmitriy Gorodov**
- **ID**: 342725405
- **Date**: 24/01/2025

## License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.   