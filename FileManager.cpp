/**
 * @file FileManager.h
 * @brief FileManager class implementation.
 * @details This class handles creating directories, saving, reading, deleting, and listing files.
 * 
 * @version 1.0
 * @author Dmitriy Gorodov
 * @id 342725405
 * @date 24/01/2025
 */

#include "FileManager.h"

#include <boost/asio.hpp>
#include <filesystem>
#include <fstream>
#include <system_error>
#include <random>
#include <regex>

FileManager::FileManager(const std::string& root_folder)
    : root_folder_(root_folder){}

void FileManager::create_root_directory() const
{
	std::filesystem::create_directories(root_folder_);
}

void FileManager::create_user_directory(const uint32_t user_id) const
{
    const std::string user_path = user_folder_path(user_id);
    std::filesystem::create_directories(user_path);
}

bool FileManager::save_file(const uint32_t user_id, const std::string& filename,
    const std::vector<unsigned char>& data) const
{
    const std::string file_path = user_folder_path(user_id) + filename;
    std::ofstream ofs(file_path, std::ios::binary);
    if (!ofs.is_open())
    {
        return false;
    }
    ofs.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    return true;
}

bool FileManager::read_file(const uint32_t user_id, const std::string& filename,
                            std::vector<unsigned char>& out_data) const
{
    const std::string file_path = user_folder_path(user_id) + filename;
    std::ifstream ifs(file_path, std::ios::binary | std::ios::ate);
    if (!ifs.is_open())
    {
        return false;
    }

    const auto file_size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    out_data.resize(file_size);
    ifs.read(reinterpret_cast<char*>(out_data.data()), file_size);
    return true;
}

bool FileManager::delete_file(const uint32_t user_id, const std::string& filename) const
{
    const std::string file_path = user_folder_path(user_id) + filename;
    std::error_code ec;
    const bool removed = std::filesystem::remove(file_path, ec);

	if (ec)
	{
		throw std::filesystem::filesystem_error("Error while deleting the file", ec);
	}

	return removed;
}

std::vector<std::string> FileManager::list_user_files(const uint32_t user_id) const
{
	std::vector<std::string> files;
    boost::system::error_code ec;

    const std::string user_path = user_folder_path(user_id);


    for (auto& p : std::filesystem::directory_iterator(user_path, ec))
    {
        if (is_regular_file(p.path(), ec))
        {
			const std::string filename = p.path().filename().string();

			// Exclude the list file itself from the list of files
            if (regex_match(filename, RANDOM_FILENAME_PATTERN))
            {
				continue;
            }

            files.push_back(filename);
        }
    }

	if (ec)
	{
		throw std::filesystem::filesystem_error("Error listing files", ec);
	}

    return files;
}

std::string FileManager::write_file_list(const uint32_t user_id, const std::vector<std::string>& files) const
{
	// Remove any existing list file for this user (if any) before creating a new one
	const std::string user_path = user_folder_path(user_id);

	for (auto& p : std::filesystem::directory_iterator(user_path))
	{
		if (is_regular_file(p.path()))
		{
			if (const std::string filename = p.path().filename().string(); regex_match(filename, RANDOM_FILENAME_PATTERN))
			{
				std::filesystem::remove(p.path());
			}
		}
	}

	std::string random_txt = generate_random_filename();
    const std::string list_file_path = user_folder_path(user_id) + random_txt;

	std::ofstream ofs(list_file_path, std::ios::binary);
    for (auto& f : files)
    {
        ofs << f << "\n";
    }
    return random_txt;
}

std::string FileManager::user_folder_path(const uint32_t user_id) const
{
    return root_folder_ + std::to_string(user_id) + "/";
}

std::string FileManager::generate_random_filename()
{
    static constexpr  char chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<size_t> dist(0, sizeof(chars) - 2);

    std::string result;
    result.reserve(32);

    for (int i = 0; i < 32; ++i) {
        result.push_back(chars[dist(rng)]);
    }

    return result;
}