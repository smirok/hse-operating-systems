#include "FileWriter.hpp"
#include <unistd.h>
#include <iostream>
#include <ArchiverException.hpp>


namespace utar {

    FileWriter::FileWriter(const std::string &fileName) {
        outputFileStream.open(fileName, std::ofstream::out);

        if (outputFileStream.fail()) {
            throw ArchiverException("Cannot open file");
        }
    }

    void FileWriter::writeDirectoryMark(const std::string &directoryName) {
        outputFileStream.write(filetypes[RecordType::CLOSE].c_str(), 1);

        std::uint8_t directoryNameSize = directoryName.size();
        outputFileStream.write(reinterpret_cast<const char *>(&directoryNameSize), 1);

        outputFileStream.write(directoryName.c_str(), directoryNameSize);
    }

    off_t FileWriter::writeFile(const std::string &fileName, const std::string &path, const struct stat &fileStat) {
        off_t result = outputFileStream.tellp();

        if (S_ISLNK(fileStat.st_mode)) {
            outputFileStream.write(filetypes[RecordType::SYMLINK].c_str(), 1);
        } else if (S_ISREG(fileStat.st_mode)) {
            outputFileStream.write(filetypes[RecordType::REGULAR_FILE].c_str(), 1);
        } else if (S_ISDIR(fileStat.st_mode)) {
            outputFileStream.write(filetypes[RecordType::DIRECTORY].c_str(), 1);
        } else {
            outputFileStream.write(filetypes[RecordType::FIFO_CHANNEL].c_str(), 1);
        }

        std::uint8_t fileNameSize = fileName.size();
        outputFileStream.write(reinterpret_cast<const char *>(&fileNameSize), 1);

        outputFileStream.write(fileName.c_str(), fileNameSize);

        if (!S_ISDIR(fileStat.st_mode)) {

            outputFileStream.write(reinterpret_cast<const char *>(&fileStat.st_size), sizeof(off_t));

            if (S_ISLNK(fileStat.st_mode)) {
                writeSymlink(path);
            } else if (S_ISREG(fileStat.st_mode)) {
                writeFileData(path);
            }

        }

        writeFileStat(fileStat);

        return result;
    }

    void FileWriter::writeFileData(const std::string &path) {
        std::ifstream f(path);

        if (f.fail()) {
            throw new ArchiverException("Cannot open file");
        }

        f.seekg(0, std::ios::end);
        size_t size = f.tellg();
        char *buffer = new char[size];
        f.seekg(0, std::ios::beg);
        f.read(buffer, size);

        outputFileStream.write(buffer, size);
        delete[] buffer;
    }

    void FileWriter::writeSymlink(const std::string &path) {
        char *buffer = new char[10000];

        int sz = readlink(path.data(), buffer, 10000);
        outputFileStream.write(buffer, sz);

        delete[] buffer;
    }

    void FileWriter::writeHardLink(const std::string &fileName, off_t offsetOnHardLink) {
        outputFileStream.write(filetypes[RecordType::HARDLINK].c_str(), 1);

        std::uint8_t fileNameSize = fileName.size();
        outputFileStream.write(reinterpret_cast<const char *>(&fileNameSize), 1);

        outputFileStream.write(fileName.c_str(), fileNameSize);

        outputFileStream.write(reinterpret_cast<const char *>(&offsetOnHardLink), sizeof(off_t));
    }

    void FileWriter::writeFileStat(const struct stat &fileStat) {
        outputFileStream.write(reinterpret_cast<const char *>(&fileStat), sizeof(fileStat));
    }

}