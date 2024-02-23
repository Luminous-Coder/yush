#include "shell.h"
#include "command.h"

#include <string>
#include <iostream>
#include <filesystem>
#include <unistd.h>
#include <signal.h>
#include <termios.h>

#include <sys/wait.h>

#include <fmt/format.h>

#include "feature/string_parser.h"
#include "common.hpp"

extern char** environ;

Command::Command() {}

Command::Command(std::string_view cmd) {
    this->command = cmd;
}

int Command::assign(std::string_view cmd) {
    this->command = cmd;
    return 0;
}

int Command::assign(std::vector<std::string>& args) {
    this->args = args;
    return 0;
}

int Command::parse() {
    /*
    if (Shell::alias.exist(this->command)) {
        std::string alias_cmd(Shell::alias.get(this->command));
        return this->assign(alias_cmd);
    }
    */

    std::vector<std::string> result;
    std::size_t begin = std::string::npos;

    for (std::size_t i = 0; i < this->command.size(); i++) {
        if (this->command[i] == ' ') {
            if (begin != std::string::npos) {
                result.push_back(this->command.substr(begin, i - begin));
                begin = std::string::npos;
            }

            continue;
        }

        if (this->command[i] == '$') {
            if (begin != std::string::npos) {
                result.push_back(this->command.substr(begin, i - begin));
                begin = std::string::npos;
            }

            std::size_t end = this->command.find_first_not_of(
                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_",
                i + 1);

            if (end == std::string::npos) {
                end = this->command.size();
            }

            std::string var_name = this->command.substr(i + 1, end - i - 1);
            result.emplace_back(shell.vars.get(var_name));
            i = end - 1;
            continue;
        }

        if (this->command[i] == '"') {
            if (begin == std::string::npos || begin == i - 1) {
                continue;
            }

            if (begin != std::string::npos) {
                result.push_back(this->command.substr(begin, i - begin));
                begin = std::string::npos;
            }

            std::size_t end = this->command.find_first_of('"', i + 1);

            if (end == std::string::npos) {
                end = this->command.size();
            }

            result.emplace_back(this->command.substr(i + 1, end - i - 1));
            i = end;

            continue;
        }

        if (this->command[i] == '\'') {
            if (begin == std::string::npos || begin == i - 1) {
                std::size_t end = this->command.find_first_of('\'', i + 1);

                if (end == std::string::npos) {
                    end = this->command.size();
                }

                result.emplace_back(this->command.substr(i + 1, end - i - 1));
                i = end;

                continue;
            }

            if (begin != std::string::npos) {
                result.push_back(this->command.substr(begin, i - begin));
                begin = std::string::npos;
            }
        }

        if (this->command[i] == '#') {
            break;
        }

        if (begin == std::string::npos) {
            begin = i;
        }
    }

    if (begin != std::string::npos) {
        result.push_back(this->command.substr(begin));
    }

    this->args = result;

    return 0;
}

int Command::exec() {
    if (this->args.empty()) {
        return SUCCESS;
    }

    /*
    if (functions.exist(this->args[0])) {
        for (const auto& cmd : string_parser(functions.get(args[0]), '\n')) {
            Command command(cmd);
            this->runtime_status = command.exec();
        }

        return this->runtime_status;
    }
    */

    int shell_builtin_ans =
        shell.exec_shell_builtin(this->args);

    if (shell_builtin_ans != NOT_FOUND) {
        return shell_builtin_ans;
    }

    std::unique_ptr<char *[]> argv = std::make_unique<char *[]>(args.size() + 1);
    for (size_t i = 0; i < this->args.size(); i++) {
        argv[i] = this->args[i].data();
    }

    std::string cmd_path_str;

    if (std::filesystem::exists(args[0]) && std::filesystem::is_regular_file(args[0])) {
        cmd_path_str = this->args[0];
    } else {
        std::vector<std::string> cmd_paths =
            string_parser(shell.vars.get("PATH"), ':');

        for (const auto& cmd : cmd_paths) {
            std::filesystem::path cmd_path = cmd / std::filesystem::path(args[0]);

            if (std::filesystem::exists(cmd_path) &&
                std::filesystem::is_regular_file(cmd_path)) {
                cmd_path_str = cmd_path.lexically_normal().string();
                break;
            }
        }
    }

    if (cmd_path_str.empty()) {
        fmt::print(stderr, "Error: command `{}` not found.\n", args[0]);
        return NOT_FOUND;
    }

    pid_t pid = fork();

    if (pid == PID_FAILURE) {
        return PID_FAILURE;
    }

    if (pid > 0) {
        int status;
        waitpid(pid, &status, SUCCESS);
        return status;
    }

    signal(SIGINT, SIG_DFL);

    execve(cmd_path_str.c_str(), argv.get(), environ);
    unreachable();
}

bool Command::empty() {
    return this->command.empty();
}

std::vector<std::string> Command::arg() {
    return this->args;
}