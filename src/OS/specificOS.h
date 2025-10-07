#ifndef TOYSQL_SPECIFICOS_H
#define TOYSQL_SPECIFICOS_H

#include <fstream>
#include <vector>
#include <string>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #include <io.h>
#else
    #include <sys/stat.h>
    #include <unistd.h>
    #include <dirent.h>
#endif

namespace os {

    inline bool createDir(const std::string& path) {
#ifdef _WIN32
        return _mkdir(path.c_str()) == 0 || errno == EEXIST;
#else
        return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
    }

    inline bool removeDir(const std::string& path) {
#ifdef _WIN32
        return _rmdir(path.c_str()) == 0;
#else
        return rmdir(path.c_str()) == 0;
#endif
    }

    inline bool removeFile(const std::string& path) {
#ifdef _WIN32
        return DeleteFileA(path.c_str());
#else
        return unlink(path.c_str()) == 0;
#endif
    }

    inline bool fileExists(const std::string& path) {
#ifdef _WIN32
        return _access(path.c_str(), 0) == 0;
#else
        return access(path.c_str(), F_OK) == 0;
#endif
    }
    
    inline std::vector<std::string> listFiles(const std::string& path) {
        std::vector<std::string> files;

#ifdef _WIN32
        WIN32_FIND_DATAA findFileData;
        HANDLE hFind = FindFirstFileA((path + "\\*").c_str(), &findFileData);

        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                const char* name = findFileData.cFileName;
                if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0) {
                    files.push_back(name);
                }
            } while (FindNextFileA(hFind, &findFileData));
            FindClose(hFind);
        }
#else
        DIR* dir = opendir(path.c_str());
        if (dir != nullptr) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                std::string name(entry->d_name);
                if (name != "." && name != "..") {
                    files.push_back(name);
                }
            }
            closedir(dir);
        }
#endif

        return files;
    }

    inline bool createFile(const std::string& path) {
        if (fileExists(path)) return true;
        const std::ofstream ofs(path, std::ios::binary);
        return ofs.good();
    }
}

#ifdef WIN32
#define OS_SEP "\\"
#define ROOT "C:\\"
#else
#define OS_SEP "/"
#define ROOT "~/"
#endif

#endif //TOYSQL_SPECIFICOS_H