#include "utility.h"
#include "protocols.h"

bool create_directory_if_not_exists(const std::string& dir)
{
#ifdef _WIN32
    if (_mkdir(dir.c_str()) == 0 || errno == EEXIST)
        return true;
    else
        return false;
#else
    struct stat st;
    if (stat(dir.c_str(), &st) != 0)
    {
        if (mkdir(dir.c_str(), 0755) == 0)
            return true;
        else
            return false;
    }
    else
    {
        if ((st.st_mode & S_IFDIR) != 0)
            return true;
        else
            return false;
    }
#endif
}

bool read_exact(shared_ptr<boost::asio::ip::tcp::socket> sock, void* buffer, size_t size, boost::system::error_code& ec)
{
    size_t total_read = 0;
    while (total_read < size && !ec)
    {
        size_t bytes_read = sock->read_some(boost::asio::buffer((char*)buffer + total_read, size - total_read), ec);
        total_read += bytes_read;
    }
    return !ec || (ec == boost::asio::error::eof && total_read == size);
}

vector<string> list_files_in_directory(const string& dir)
{
    vector<string> files;
	boost::system::error_code ec;
    for (auto& p : filesystem::directory_iterator(dir, ec))
    {
        if (filesystem::is_directory(p.path(), ec))
        {
			files.push_back(p.path().filename().string());
        }
    }

	return files;
}

string generate_random_filename() 
{
    static const char chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    static mt19937 rng(random_device{}());
    static uniform_int_distribution<size_t> dist(0, sizeof(chars) - 2);

    string result;
    result.reserve(32);

    for (int i = 0; i < 32; ++i) {
        result.push_back(chars[dist(rng)]);
    }

    return result;
}

// Return a string of up to `max_bytes` in hex, separated by spaces
// e.g. {0x25, 0x50, 0x44, 0x46} => "25 50 44 46"
string hex_dump_limited(const unsigned char* data, size_t size, size_t max_bytes)
{
    ostringstream oss;
    oss << hex << setfill('0');
    size_t count = (size < max_bytes) ? size : max_bytes;
    for (size_t i = 0; i < count; ++i)
    {
        oss << setw(2) << static_cast<unsigned int>(data[i]);
        if (i + 1 < count) oss << " ";
    }
    if (size > max_bytes) oss << "..."; // indicate truncated
    return oss.str();
}

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

/// Write a single byte (uint8_t)
void write_uint8(vector<unsigned char>& buffer, uint8_t value)
{
    buffer.push_back(value);
}

/// Write a 2-byte little-endian integer (uint16_t)
void write_uint16_le(vector<unsigned char>& buffer, uint16_t value)
{
    buffer.push_back(static_cast<unsigned char>(value & 0xFF));
    buffer.push_back(static_cast<unsigned char>((value >> 8) & 0xFF));
}

/// Write a 4-byte little-endian integer (uint32_t)
void write_uint32_le(vector<unsigned char>& buffer, uint32_t value)
{
    buffer.push_back(static_cast<unsigned char>(value & 0xFF));
    buffer.push_back(static_cast<unsigned char>((value >> 8) & 0xFF));
    buffer.push_back(static_cast<unsigned char>((value >> 16) & 0xFF));
    buffer.push_back(static_cast<unsigned char>((value >> 24) & 0xFF));
}

void print_request_table(
    uint32_t user_id,
    uint8_t client_version,
    uint8_t op_code,
    uint16_t name_len,
    const string& filename,
    uint32_t file_size,
    const unsigned char* file_content)
{
    string operation = "";
    bool has_payload = false;
    size_t offset = sizeof(user_id) + sizeof(client_version) + sizeof(op_code) + sizeof(name_len) + filename.size() + sizeof(file_size) + sizeof(file_content);

    switch (op_code)
    {
    case static_cast<uint8_t>(Command::SAVE_FILE):
        operation = "Request to backup the file:";
        has_payload = true;
        break;

    case static_cast<uint8_t>(Command::RESTORE_FILES):
        operation = "Request to restore the file:";
        has_payload = false;
        break;

    case static_cast<uint8_t>(Command::DELETE_FILE):
        operation = "Request to delete the file:";
        has_payload = false;
        break;

    case static_cast<uint8_t>(Command::LIST_FILES):
        operation = "Request to list all client files:";
        has_payload = true;
        break;

    default:
        operation = "Unknown operation request:";
        break;
    }

    cout << "\n" << operation << "\n";
    string f_name = (filename.empty() ? "None" : filename);
    string f_size = has_payload ? to_string(file_size) : "None";
    string hex_f_preview = hex_dump_limited(file_content, file_size, 8);
    string f_content = has_payload ? hex_f_preview : "None";

    // Helper function to center-align text within a given width
    auto center = [](const string& str, int width) {
        int len = str.length();
        if (len >= width) return str;
        int left_padding = (width - len) / 2;
        int right_padding = width - len - left_padding;
        return string(left_padding, ' ') + str + string(right_padding, ' ');
        };

    // Column widths
    const int offset_width = 12;
    const int user_id_width = 12;
    const int client_version_width = 15;
    const int op_code_width = 10;
    const int name_len_width = 9;
    const int filename_width = 15;
    const int size_width = 12;
    const int content_width = 65;

    // Print the header portion
    cout
        << "--------------------------------------------------------------------------------------------\n"
        << "            |" << center("offset", offset_width)
        << "|" << center("user_id", user_id_width)
        << "|" << center("client_version", client_version_width)
        << "|" << center("op_code", op_code_width)
        << "|" << center("name_len", name_len_width)
        << "|" << center("filename", filename_width) << "|\n"
        << "   Header   |-------------------------------------------------------------------------------\n"
        << "            |" << center(to_string(offset), offset_width)
        << "|" << center(to_string(user_id), user_id_width)
        << "|" << center(to_string(static_cast<int>(client_version)), client_version_width)
        << "|" << center(to_string(static_cast<int>(op_code)), op_code_width)
        << "|" << center(to_string(name_len), name_len_width)
        << "|" << center(f_name, filename_width) << "|\n"
        << "--------------------------------------------------------------------------------------------\n"
        << "            |" << center("size", size_width)
        << "|" << center("content", content_width) << "|\n"
        << "   payload  |-------------------------------------------------------------------------------\n"
        << "            |" << center(f_size, size_width)
        << "|" << center(f_content, content_width) << "|\n"
        << "--------------------------------------------------------------------------------------------\n\n";
}

/* void print_request_table(
    uint32_t user_id, 
    uint8_t client_version, 
    uint8_t op_code, 
    uint16_t name_len, 
    const string& filename, 
    uint32_t file_size, 
    const unsigned char* file_content) 
{
    string operation = "";
	bool has_payload = false;
	size_t offset = sizeof(user_id) + sizeof(client_version) + sizeof(op_code) + sizeof(name_len) + filename.size() + sizeof(file_size) + sizeof(file_content);

    switch (op_code)
    {

        case static_cast<uint8_t>(Command::SAVE_FILE):
            operation = "Request to backup the file:";
            has_payload = true;
			break;

        case static_cast<uint8_t>(Command::RESTORE_FILES):
			operation = "Request to restore the file:";
            has_payload = false;
            break;

        case static_cast<uint8_t>(Command::DELETE_FILE):
		    operation = "Request to delete the file:";
            has_payload = false;
			break;

		case static_cast<uint8_t>(Command::LIST_FILES):
			operation = "Request to list all client files:";
            has_payload = true;
            break;

        default:
			operation = "Unknown operation request:";
			break;
    }

    cout << "\n" << operation << "\n";
    string f_name = (filename.empty() ? "None" : filename);
    string f_size = has_payload ? to_string(file_size) : "None";
	string hex_f_preview = hex_dump_limited(file_content, file_size, 8);
    string f_content = has_payload ? hex_f_preview : "None";

    // Print the header portion
    cout
        << "----------------------------------------------------------------------------------------------------------------------\n"
        << "            |    offset    |    user_id    | client_version |  op_code  | name_len |    filename    |\n"
        << "   Header   |---------------------------------------------------------------------------------------------------------\n"
        << "            |" << setw(12) << offset
        << "|" << setw(12) << user_id
        << "|" << setw(15) << static_cast<int>(client_version)
        << "|" << setw(10) << static_cast<int>(op_code)
        << "|" << setw(9) << name_len
        << "|" << setw(15) << f_name << "|\n"
        << "----------------------------------------------------------------------------------------------------------------------\n"
        << "            |     size     |                                    content                                              |\n"
        << "   payload  |---------------------------------------------------------------------------------------------------------\n"
        << "            |" << setw(12) << f_size
        << "|" << setw(50) << f_content << "|\n"
        << "----------------------------------------------------------------------------------------------------------------------\n";
}*/