#include <string>
#include <map>
#include <vector>

#include "./commands/all_commands.h"
#include "./global_var.h"

using namespace std;

int call_function(vector<string> i){
    if(i.at(0) == "cat"){
        cat(i.at(1));
    }
    else if(i.at(0) == "cd"){
        cd(i.at(1));
    }
    else if(i.at(0) == "clear"){
        clear();
    }
    else if(i.at(0) == "date"){
        date();
    }
    else if(i.at(0) == "echo"){
        echo(i);
    }
    else if(i.at(0) == "exit"){
        exit();
    }
    else if(i.at(0) == "help"){
        help(commands);
    }
    else if(i.at(0) == "la"){
        la();
    }
    else if(i.at(0) == "ls"){
        ls();
    }
    else if(i.at(0) == "time"){
        time();
    }
    else if(i.at(0) == "whoami"){
        whoami(user_name);
    }
    else{
        unknown();
        return 1;
    }
    return 0;
}