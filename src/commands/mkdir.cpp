#ifndef CMD_MKDIR_H
#define CMD_MKDIR_H

#include <string>
#include <filesystem>

#include "stream_manager.hpp"
#include "variable_manager.h"
#include "commands/cmds.h"

int cmd::mkdir(const std::vector<std::string>& arg, StreamManager& stream_manager, VariableManager& variable_manager) {
    if (arg.size() != 2) {
        stream_manager.err() << "Argument size error.\n";
        return 1;
    }

    auto new_directory = std::filesystem::path(variable_manager.get("PWD")) / arg[1];
    if (!exists(new_directory)) {
        std::filesystem::create_directories(new_directory);
        return 0;
    } else {
        stream_manager.err() << "Directory already exists.\n";
        return 1;
    }
}

#endif