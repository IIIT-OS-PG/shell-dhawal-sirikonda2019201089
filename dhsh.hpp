#ifndef _DHSH_HPP_
#define _DHSH_HPP_

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <iostream>
#include <vector>
#include <map>

#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <dirent.h>

#include "command_parse.hpp"
#include "trie.hpp"


#define CMD_MAX_SIZE 4096
#define USERNAME_MAX_SIZE 256
#define HOSTNAME_MAX_SIZE 256
#define MAX_COMMANDS 10
#define MAX_COMMAND_ARGS 20
#define MAX_ARG_LENGTH 4096
#define MAX_ENV_VAR_SIZE 2048
#define MAX_NUM_PATHS 100
#define MAX_PATH_LENGTH 1024

#define TAB_KEY (9)
#define BACKSPACE_KEY (127)
#define CMD_MAX_SIZE 4096
#define NEW_LINE '\n'

using namespace std;

struct command_list{
    char ***command_args;
    int *arg_count;
};

vector<char**> set_up();
int exit_session();


char* user_prompt();

int process_command(char *,map<string, string>&);


char * command_fetcher(struct termios);

void display_message(int second);

#endif //!_DHSH_HPP
