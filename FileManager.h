/**
 * @file FileManager.h
 * @brief FileManager class definition.
 * @details This header file contains the FileManager class definition for managing file-related operations, such as saving, reading, deleting, and listing files.
 *
 * @version 1.0
 * @date 24/01/2025
 * @autor Dmitriy Gorodov
 * @id 342725405
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <regex>

const std::regex RANDOM_FILENAME_PATTERN("^[A-Za-z0-9]{32}$");

/**
 * @class FileManager
 * @brief Manages all file-related operations: creating directories, saving, reading, deleting, listing, etc.
 */
class FileManager {
public:
    /**
     * @brief Constructs a FileManager with a given root folder.
     * @param root_folder The root folder for file operations.
     */
    explicit FileManager(const std::string& root_folder);

    /**
     * @brief Creates the root directory if it doesn't exist.
     */
	void create_root_directory() const;


    /**
     * @brief Ensures the user's subfolder exists.
     * @param user_id The user ID for the directory.
     */
    void create_user_directory(uint32_t user_id) const;

    /**
     * @brief Saves file data to the user's directory.
     * @param user_id The user ID.
     * @param filename The filename to save.
     * @param data The file data to save.
     * @return True on success; false on error.
     */
    bool save_file(uint32_t user_id, const std::string& filename,
        const std::vector<unsigned char>& data) const;

    /**
     * @brief Reads file data into 'out_data'.
     * @param user_id The user ID.
     * @param filename The filename to read.
     * @param out_data The vector to store the file data.
     * @return True if the file exists; false otherwise.
     */
    bool read_file(uint32_t user_id, const std::string& filename,
                   std::vector<unsigned char>& out_data) const;

    /**
     * @brief Deletes a file.
     * @param user_id The user ID.
     * @param filename The filename to delete.
     * @return True if the file was removed; false if not found or error.
     */
    bool delete_file(uint32_t user_id, const std::string& filename) const;

    /**
     * @brief Lists files in the user's folder.
     * @param user_id The user ID.
     * @return The list of filenames.
     */
    std::vector<std::string> list_user_files(const uint32_t user_id) const;

    /**
     * @brief Writes a text file listing all 'files'.
     * @param user_id The user ID.
     * @param files The list of filenames to write.
     * @return The name of the newly created text file (a 32-char random name).
     */
    std::string write_file_list(uint32_t user_id, const std::vector<std::string>& files) const;

    /**
     * @brief Generates a random filename (32 alphanumeric characters).
     * @return The generated random filename.
     */
    static std::string generate_random_filename();

private:
    /**
	 * @brief The root folder for file operations.
     */
    std::string root_folder_;

    /**
     * @brief Helper function to get the user's folder path.
     * @param user_id The user ID.
     * @return The user's folder path.
     */
    std::string user_folder_path(uint32_t user_id) const;
};