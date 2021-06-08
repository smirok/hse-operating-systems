#include <vector>
#include "Archiver.hpp"
#include "fstream"

#include <sys/types.h>
#include <unistd.h>

#include <dirent.h>
#include <sys/stat.h>
#include <iostream>
#include <FileWriter.hpp>
#include <FileReader.hpp>
#include <Creator.hpp>
#include "ArchiverException.hpp"

namespace utar {

    void Archiver::create(const std::string &fileName, const std::string &directory) {
        FileWriter fileWriter(fileName);

        dfsToWrite(fileWriter, directory, fileName);
    }

    void Archiver::dfsToWrite(FileWriter &fileWriter, const std::string &directory, const std::string &bannedFileName) {
        DIR *dir = opendir(directory.data());

        if (dir == nullptr)
            return;

        struct dirent *node;
        while (node = readdir(dir)) {
            std::string nodeName(node->d_name);
            if (nodeName == "." || nodeName == ".." || nodeName == bannedFileName) continue;

            std::string path = directory + nodeName;

            struct stat st;
            if (lstat(path.c_str(), &st) < 0)
                continue;

            if (inodes.find({st.st_ino, st.st_dev}) == inodes.end()) {
                off_t offsetOnRecord = fileWriter.writeFile(nodeName, path, st);
                inodes[{st.st_ino, st.st_dev}] = offsetOnRecord;
            } else {
                fileWriter.writeHardLink(nodeName, inodes[{st.st_ino, st.st_dev}]);
            }

            dfsToWrite(fileWriter, path);

            if (S_ISDIR(st.st_mode)) {
                fileWriter.writeDirectoryMark(node->d_name);
            }
        }

        if (closedir(dir) == -1) {
            throw ArchiverException("Cannot close directory");
        }
    }

    void Archiver::extract(const std::string &fileName, const std::string &directory) {
        currentDirectory = directory;
        FileReader fileReader(fileName);

        readAllRecords(fileReader);
    }

    void Archiver::readAllRecords(FileReader &fileReader) {
        std::map<std::string, std::vector<std::pair<FileReader::FileData, std::string>>> symlinkGraph; // a -> b <IFF> b is symlink on a
        while (!fileReader.isAllReaded()) {
            RecordType fileType = fileReader.getRecordType();

            if (fileType == RecordType::CLOSE) {
                std::string dirName = fileReader.readDirectoryMark();
                for (int i = 0; i <= dirName.size(); ++i) {
                    currentDirectory.pop_back();
                }
            } else {
                FileReader::FileData fileData;
                if (fileType == RecordType::HARDLINK) {
                    fileData = fileReader.readHardLink();
                } else {
                    fileData = fileReader.readFile();
                }

                if (fileType == REGULAR_FILE) {
                    std::string fileName = Creator::createRegularFile(fileData, currentDirectory);
                    hardlinks[{fileData.fileStat.st_ino, fileData.fileStat.st_dev}] = fileName;
                } else if (fileType == HARDLINK) {
                    Creator::createHardlink(fileData,
                                            hardlinks[{fileData.fileStat.st_ino, fileData.fileStat.st_dev}],
                                            currentDirectory);
                } else if (fileType == DIRECTORY) {
                    Creator::createDirectory(fileData, currentDirectory);
                } else if (fileType == SYMLINK) {
                    symlinkGraph[currentDirectory + fileData.data].emplace_back(fileData, currentDirectory);
                } else if (fileType == FIFO_CHANNEL) {
                    Creator::createFIFO(fileData, currentDirectory);
                }

                if (fileType == DIRECTORY) {
                    currentDirectory += fileData.fileName + '/';
                }
            }
        }

        createAllSymlinks(symlinkGraph);
    }

    void Archiver::createAllSymlinks(
            const std::map<std::string, std::vector<std::pair<FileReader::FileData, std::string>>> &symlinkGraph) {

        std::vector<std::string> connectedComponentRoots;
        for (const auto &node : symlinkGraph) {
            std::ifstream ifs(node.first);

            if (!ifs.fail()) {
                connectedComponentRoots.push_back(node.first);
            }
        }

        for (const std::string &rootName : connectedComponentRoots) {
            dfsCreateAllSymlinks(rootName, symlinkGraph);
        }
    }

    void Archiver::dfsCreateAllSymlinks(const std::string &root,
                                        const std::map<std::string, std::vector<std::pair<FileReader::FileData, std::string>>> &symlinkGraph) {
        if (symlinkGraph.find(root) == symlinkGraph.end()) {
            return;
        }

        for (const auto &pair : symlinkGraph.at(root)) {
            Creator::createSymlink(pair.first, pair.second);
            dfsCreateAllSymlinks(currentDirectory + pair.first.fileName, symlinkGraph);
        }
    }
}