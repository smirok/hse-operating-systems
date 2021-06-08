#include <sys/stat.h>
#include <iostream>
#include <ArchiverException.hpp>
#include "FileReader.hpp"

namespace utar {

    FileReader::FileReader(const std::string &fileName) {
        inputFileStream.open(fileName, std::ifstream::in);

        if (inputFileStream.fail()) {
            throw ArchiverException("Cannot open file");
        }

        inputFileStream.seekg(0, std::ios::end);
        fileSize = inputFileStream.tellg();
        inputFileStream.seekg(0, std::ios::beg);
    }

    std::string FileReader::readDirectoryMark() {
        char unusedType;
        inputFileStream.read(reinterpret_cast<char *>(&unusedType), 1);

        std::uint8_t directoryNameSize;
        inputFileStream.read(reinterpret_cast<char *>(&directoryNameSize), 1);

        std::string directoryName;
        directoryName.assign((int) directoryNameSize, 'a');
        inputFileStream.read(directoryName.data(), directoryNameSize);

        return directoryName;
    }

    RecordType FileReader::getRecordType() {
        return filetypes[{static_cast<char>(inputFileStream.peek())}];
    }

    bool FileReader::isAllReaded() {
        return inputFileStream.tellg() >= fileSize;
    }

    FileReader::FileData FileReader::readFile() {
        FileData fileData;

        char type;
        inputFileStream.read(reinterpret_cast<char *>(&type), 1);
        fileData.type = filetypes[{type}];

        std::uint8_t fileNameSize;
        inputFileStream.read(reinterpret_cast<char *>(&fileNameSize), 1);

        fileData.fileName.assign(static_cast<int>(fileNameSize), 'a');
        inputFileStream.read(fileData.fileName.data(), static_cast<int>(fileNameSize));

        if (fileData.type != DIRECTORY) {
            off_t dataSize;

            inputFileStream.read(reinterpret_cast<char *>(&dataSize), sizeof(off_t));

            fileData.data.assign(static_cast<long long>(dataSize), 'a');
            inputFileStream.read(reinterpret_cast<char *> (fileData.data.data()), static_cast<long long>(dataSize));
        }

        inputFileStream.read(reinterpret_cast<char *>(&fileData.fileStat), sizeof(fileData.fileStat));

        return fileData;
    }

    FileReader::FileData FileReader::readFileOnOffset(off_t offset) {
        off_t prevCur = inputFileStream.tellg();

        inputFileStream.seekg(offset, std::ifstream::beg);

        FileData fileData = readFile();

        inputFileStream.seekg(prevCur, std::ifstream::beg);

        return fileData;
    }

    FileReader::FileData FileReader::readHardLink() {
        inputFileStream.get();

        std::uint8_t fileNameSize;
        inputFileStream.read(reinterpret_cast<char *>(&fileNameSize), 1);

        std::string fileName;
        fileName.assign((int) fileNameSize, 'a');
        inputFileStream.read(fileName.data(), (int) fileNameSize);

        off_t offsetOnDataFile;
        inputFileStream.read(reinterpret_cast<char *>(&offsetOnDataFile), sizeof(off_t));

        FileData fileData = readFileOnOffset(offsetOnDataFile);
        fileData.type = HARDLINK;
        fileData.fileName = fileName;

        return fileData;
    }
}