#ifndef UTAR_ARCHIVER_HPP
#define UTAR_ARCHIVER_HPP

#include <string>
#include <unordered_map>
#include "FileWriter.hpp"
#include "FileReader.hpp"

struct hash_pair {
    template<class T1, class T2>
    size_t operator()(const std::pair<T1, T2> &p) const {
        auto hash1 = std::hash<T1>{}(p.first);
        auto hash2 = std::hash<T2>{}(p.second);
        return hash1 ^ hash2;
    }
};

namespace utar {

    class Archiver {
    public:
        void create(const std::string &fileName, const std::string &directory);

        void extract(const std::string &fileName, const std::string &directory);

    private:
        void dfsToWrite(FileWriter &fileWriter, const std::string &directory, const std::string &bannedFileName = ".");

        void readAllRecords(FileReader &fileReader);

        void createAllSymlinks(
                const std::map<std::string, std::vector<std::pair<FileReader::FileData, std::string>>> &symlinkGraph);

        void dfsCreateAllSymlinks(const std::string &root,
                                  const std::map<std::string, std::vector<std::pair<FileReader::FileData, std::string>>> &symlinkGraph);

        std::unordered_map<std::pair<ino_t, dev_t>, off_t, hash_pair> inodes;

        std::unordered_map<std::pair<ino_t, dev_t>, std::string, hash_pair> hardlinks;

        std::string currentDirectory;
    };
}

#endif //UTAR_ARCHIVER_HPP
