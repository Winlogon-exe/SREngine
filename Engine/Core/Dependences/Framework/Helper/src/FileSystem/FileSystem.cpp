//
// Created by Nikita on 16.11.2020.
//
#define CRT_SECURE_NO_WARNINGS

#include "../../inc/FileSystem/FileSystem.h"

#include <cstdio>
#include <Debug.h>
#include <Utils/StringUtils.h>

#ifdef WIN32
    #include <Windows.h>
#endif

#include <Utils/StringUtils.h>
#include <Debug.h>
#include <direct.h>

using namespace Framework::Helper;

bool Framework::Helper::FileSystem::Delete(const char *file) { return remove(file); }

bool Framework::Helper::FileSystem::FileExists(const std::string& path) {
#ifdef WIN32
    if (FILE* f = fopen(path.c_str(), "r")) {
        fclose(f);
        return true;
    }
    else
        return false;
#else
    Debug::Error("FileSystem::FileExists() : linux not support this function!");
    return false;
#endif
}

std::string Framework::Helper::FileSystem::GetPathToExe() {
#ifdef WIN32
    const std::size_t buf_len = 260;
    auto s = new TCHAR[buf_len];
    auto path_len = GetModuleFileName(GetModuleHandle(nullptr), s, buf_len);
    return StringUtils::GetDirToFileFromFullPath(s);
#else
    Debug::Error("FileSystem::GetPathToExe() : linux not support this function!");
    return "NoDirectory";
#endif
}

std::string Framework::Helper::FileSystem::GetFullPathToExe() {
#ifdef WIN32
    const std::size_t buf_len = 260;
    auto s = new TCHAR[buf_len];
    auto path_len = GetModuleFileName(GetModuleHandle(nullptr), s, buf_len);
    return s;
#else
    Debug::Error("FileSystem::GetPathToExe() : linux not support this function!");
    return "NoDirectory";
#endif
}

void FileSystem::UnmapFile(const char *str) {

}

const char *FileSystem::FileMapView(std::string path) {
    const char *data = nullptr;

#ifdef WIN32

#else

#endif

    return data;
}

char *FileSystem::Load(std::string path) {
    //open file
    std::ifstream infile(path);
    if (!infile.is_open()) {
        Debug::Error("FileSystem::Load() : failed open file!\n\tPath: "+path);
        return nullptr;
    }


    //get length of file
    infile.seekg(0, std::ios::end);
    size_t length = infile.tellg();
    infile.seekg(0, std::ios::beg);

    char* buffer = new char[length];

    //read file
    infile.read(buffer, length);

    return buffer;
}

std::string FileSystem::GetExecutableFileName() {
#ifdef WIN32
    const std::size_t buf_len = 260;
    auto s = new TCHAR[buf_len];
    auto path_len = GetModuleFileName(GetModuleHandle(nullptr), s, buf_len);
    return StringUtils::GetFileNameFromFullPath(s);
#else
    return "Unsupported function";
#endif
}

void FileSystem::Reload() {
    std::string exe = GetFullPathToExe();
    ShellExecute(NULL, "open", exe.c_str(), NULL, NULL, SW_SHOWDEFAULT);
}

std::string FileSystem::ReadAllText(const std::string &path) {
    std::string data = std::string();
    std::ifstream stream(path, std::ios::in);
    if (stream.is_open()) {
        std::string line;
        bool first = false;
        while (getline(stream, line)) {
            if (!first) {
                first = true;
                data += line;
            }
            else
                data += "\n" + line;
        }
        stream.close();
    }
    return data;
}

std::vector<char> FileSystem::ReadBinary(const std::string &path) {
    /*std::ifstream ifd(path,  std::ios::binary |  std::ios::ate);
    int size = ifd.tellg();
    ifd.seekg(0,  std::ios::beg);
    std::vector<char> buffer;
    buffer.resize(size); // << resize not reserve
    ifd.read(buffer.data(), size);*/

    //std::ifstream input(path, std::ios::binary);
    //std::vector<uint32_t> buffer(std::istreambuf_iterator<char>(input), {});

    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        Helper::Debug::Error("FileSystem::ReadBinary() : failed to open \""+path+"\"file!");
        return std::vector<char>();
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

bool FileSystem::CreatePath(const std::string &path) {
#ifdef __MINGW64__
    return mkdir(path.c_str());
#else
    return _mkdir(path.c_str());
#endif
}

std::string FileSystem::SaveFileDialog(const std::string &path, const std::string &filter) {
#ifdef WIN32
    auto newPath = StringUtils::ReplaceAll(path, "/", "\\");

    OPENFILENAME ofn;          // common dialog box structure

    char buf[255] = "\0";
    char cCustomFilter[256] = "\0\0";
    int nFilterIndex = 0;
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = NULL;
    ofn.hInstance = NULL;//GetModuleHandle(NULL);
    ofn.lpstrFilter = filter.c_str();
    ofn.lpstrCustomFilter = cCustomFilter;
    ofn.nMaxCustFilter = 256;
    ofn.nFilterIndex = nFilterIndex;
    ofn.lpstrFile = buf;
    ofn.nMaxFile = 255;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = 0;
    ofn.Flags = OFN_FILEMUSTEXIST;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;
    ofn.lpstrInitialDir = newPath.c_str();

    if (GetSaveFileNameA(&ofn)) {
        return buf;
    }
#else
#endif
    return std::string();
}
