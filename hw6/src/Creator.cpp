#include "Creator.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <iostream>
#include <ArchiverException.hpp>

namespace utar {

    std::string Creator::createRegularFile(const FileReader::FileData &fileData, const std::string &currentDirectory) {
        std::string fileName = currentDirectory + fileData.fileName;
        int fd = open(fileName.data(), O_RDWR | O_CREAT | O_TRUNC,
                      fileData.fileStat.st_mode);

        if (fd == -1) {
            throw ArchiverException("Cannot open file " + fileName);
        }

        ssize_t writeRusult = write(fd, fileData.data.data(), fileData.fileStat.st_size);

        if (writeRusult == -1) {
            throw ArchiverException("Cannot write to file " + fileName);
        }

        ssize_t setIdResult = fchown(fd, fileData.fileStat.st_uid, fileData.fileStat.st_gid);

        if (setIdResult == -1) {
            throw ArchiverException("Cannot set uid or gid " + fileName);
        }

        struct timespec buf[2] = {fileData.fileStat.st_atim, fileData.fileStat.st_mtim};
        ssize_t timeResult = futimens(fd, buf);

        if (timeResult == -1) {
            throw ArchiverException("File times cannot be affected " + fileName);
        }

        ssize_t closeResult = close(fd);

        if (closeResult == -1) {
            throw ArchiverException("Cannot close file " + fileName);
        }

        return fileName;
    }


    void Creator::createDirectory(const FileReader::FileData &fileData, const std::string &currentDirectory) {
        std::string directory = currentDirectory + fileData.fileName + "/";

        ssize_t makeDirResult = mkdir(directory.data(), fileData.fileStat.st_mode);

        if (makeDirResult == -1) {
            throw ArchiverException("Cannot create directory " + directory);
        }

        ssize_t setIdResult = chown(directory.data(), fileData.fileStat.st_uid, fileData.fileStat.st_gid);

        if (setIdResult == -1) {
            throw ArchiverException("Cannot set uid or gid " + directory);
        }

        struct timespec buf[2] = {fileData.fileStat.st_atim, fileData.fileStat.st_mtim};
        ssize_t timeResult = utimensat(AT_FDCWD, directory.data(), buf, 0);

        if (timeResult == -1) {
            throw ArchiverException("File times cannot be affected " + directory);
        }
    }

    void Creator::createSymlink(const FileReader::FileData &fileData, const std::string &currentDirectory) {
        std::string fileName = currentDirectory + fileData.fileName;
        ssize_t createSymlinkResult = symlink(fileData.data.data(), fileName.data());

        if (createSymlinkResult == -1) {
            throw ArchiverException("Cannot create symlink on file " + fileName);
        }

        struct timespec buf[2] = {fileData.fileStat.st_atim, fileData.fileStat.st_mtim};
        ssize_t timeResult = utimensat(AT_FDCWD, fileName.data(), buf, 0);

        if (timeResult == -1) {
            throw ArchiverException("File times cannot be affected " + fileName);
        }
    }

    void Creator::createFIFO(const FileReader::FileData &fileData, const std::string &currentDirectory) {
        std::string fileName = currentDirectory + fileData.fileName;

        ssize_t makeFifoResult = mkfifo(fileName.data(), fileData.fileStat.st_mode);

        if (makeFifoResult == -1) {
            throw ArchiverException("Cannot create fifo channel " + fileName);
        }

        ssize_t setIdResult = chown(fileName.data(), fileData.fileStat.st_uid, fileData.fileStat.st_gid);

        if (setIdResult == -1) {
            throw ArchiverException("Cannot set uid or gid " + fileName);
        }

        struct timespec buf[2] = {fileData.fileStat.st_atim, fileData.fileStat.st_mtim};
        ssize_t timeResult = utimensat(AT_FDCWD, fileName.data(), buf, 0);

        if (timeResult == -1) {
            throw ArchiverException("File times cannot be affected " + fileName);
        }
    }

    void Creator::createHardlink(const FileReader::FileData &fileData,
                                 const std::string &linkedFileName,
                                 const std::string &currentDirectory) {
        std::string fileName = currentDirectory + fileData.fileName;

        ssize_t createLinkResult = link(linkedFileName.data(), fileName.data());

        if (createLinkResult == -1) {
            throw ArchiverException("Cannot create hardlink on file " + linkedFileName);
        }
    }
}