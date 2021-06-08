#ifndef UTAR_CREATOR_HPP
#define UTAR_CREATOR_HPP

#include "FileReader.hpp"

namespace utar {

    class Creator {
    public:
        static std::string createRegularFile(const FileReader::FileData &fileData, const std::string &currentDirectory);

        static void createHardlink(const FileReader::FileData &fileData,
                                   const std::string &linkedFileName,
                                   const std::string &currentDirectory);

        static void createSymlink(const FileReader::FileData &fileData, const std::string &currentDirectory);

        static void createDirectory(const FileReader::FileData &fileData, const std::string &currentDirectory);

        static void createFIFO(const FileReader::FileData &fileData, const std::string &currentDirectory);
    };
}

#endif //UTAR_CREATOR_HPP
