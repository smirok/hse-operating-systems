#include <experimental/filesystem>
#include <filesystem>
#include <iostream>

#include <vector>
#include <cstring>

#include <sys/types.h>
#include <unistd.h>

#include <filesystem>

#include <dirent.h>
#include <sys/stat.h>
#include "Archiver.hpp"

enum Mode {
    CREATE_MODE = 'c',
    EXTRACT_MODE = 'x'
};

int main(int argc, char *argv[]) {
    Mode mode;
    std::string relativeDirectoryName;
    std::string fileName;
    utar::Archiver archiver;

    std::vector<std::string> arguments;
    arguments.assign(argv + 1, argv + argc);

    for (auto arg = arguments.begin(); arg != arguments.end(); ++arg) {
        if (*arg == "-c" || *arg == "--create") {
            mode = Mode::CREATE_MODE;
        } else if (*arg == "-x" || *arg == "--extract") {
            mode = Mode::EXTRACT_MODE;
        } else if (*arg == "-d" || *arg == "--directory") {
            relativeDirectoryName = *(++arg);
        } else {
            fileName = *arg;
        }
    }

    std::string currentDirectory(get_current_dir_name());

    currentDirectory += "/" + relativeDirectoryName;

    if (mode == Mode::CREATE_MODE) {
        archiver.create(fileName, currentDirectory);
    } else if (mode == Mode::EXTRACT_MODE) {
        archiver.extract(fileName, currentDirectory);
    } else {
        throw new std::exception();
    }

    return 0;
}
