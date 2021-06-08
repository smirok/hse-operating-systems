#ifndef UTAR_FILEREADER_HPP
#define UTAR_FILEREADER_HPP

#include <string>
#include <fstream>

#include <map>
#include "FileWriter.hpp"

namespace utar {

    class FileReader {
    public:
        struct FileData {
            RecordType type;
            std::string fileName;
            std::string data;
            struct stat fileStat;
        };

        explicit FileReader(const std::string &fileName);

        std::string readDirectoryMark();

        RecordType getRecordType();

        FileData readFile();

        FileData readHardLink();

        FileData readFileOnOffset(off_t offset);

        bool isAllReaded();

    private:
        off_t fileSize;
        std::ifstream inputFileStream;
        std::map<std::string, RecordType> filetypes = {{"r", REGULAR_FILE},
                                                       {"d", DIRECTORY},
                                                       {"f", FIFO_CHANNEL},
                                                       {"s", SYMLINK},
                                                       {"h", HARDLINK},
                                                       {"c", CLOSE}};

    };

}

#endif //UTAR_FILEREADER_HPP
