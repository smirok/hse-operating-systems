#ifndef UTAR_FILEWRITER_HPP
#define UTAR_FILEWRITER_HPP

#include <string>
#include <fstream>
#include <sys/stat.h>
#include <map>

enum RecordType {
    REGULAR_FILE,
    DIRECTORY,
    FIFO_CHANNEL,
    SYMLINK,
    HARDLINK,
    CLOSE
};

namespace utar {

    class FileWriter {
    public:
        FileWriter(const std::string &fileName);

        void writeDirectoryMark(const std::string &directoryName);

        off_t writeFile(const std::string &fileName, const std::string &path, const struct stat &fileStat);

        void writeSymlink(const std::string &fileName);

        void writeFileData(const std::string &fileName);

        void writeHardLink(const std::string &fileName, off_t offsetOnHardLink);

        void writeFileStat(const struct stat &fileStat);

    private:
        std::ofstream outputFileStream;
        std::map<RecordType, std::string> filetypes = {{REGULAR_FILE, "r"},
                                                       {DIRECTORY,    "d"},
                                                       {FIFO_CHANNEL, "f"},
                                                       {SYMLINK,      "s"},
                                                       {HARDLINK,     "h"},
                                                       {CLOSE,        "c"}};
    };

}

#endif //UTAR_FILEWRITER_HPP
