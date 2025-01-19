// Utility.cpp
#include "utility.h"

#ifdef _WIN32
#include <direct.h>
#include <cerrno>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>
#endif

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