#include "dhsh.hpp"


using namespace std;

vector<char*> history;
struct node *root=NULL;
map<string, string> alias_map;

int main()
{
    //set-up phase
 
    vector<char**> env_set = set_up();
    for(int i = 0; i < env_set.size(); i++)
        setenv(env_set[i][0], env_set[i][1], 1);
    struct termios old_terminal_settings_in, old_terminal_settings_out, old_terminal_settings_err;
    tcgetattr(STDIN_FILENO, &old_terminal_settings_in);
    tcgetattr(STDOUT_FILENO, &old_terminal_settings_out);
    tcgetattr(STDERR_FILENO, &old_terminal_settings_err);
    old_terminal_settings_in.c_lflag |= TOSTOP;
    old_terminal_settings_out.c_lflag |= TOSTOP;
    old_terminal_settings_err.c_lflag |= TOSTOP;

    // cout<<"cuurent grp "<<tcgetpgrp(STDOUT_FILENO)<<"prestn "<<getpid()<<"parent "<<getppid()<<endl;
    // setpgid(0,0);
    // tcsetpgrp(STDIN_FILENO, getpid());
    // tcsetpgrp(STDOUT_FILENO,getpid());
    // tcsetpgrp(STDERR_FILENO,getpid());

    char* buff = (char*)calloc(10, sizeof(char));
    sprintf(buff, "%d", getpid());
    setenv("$",buff,1);
    free(buff);

    char* command = (char*)calloc(sizeof(char), CMD_MAX_SIZE);
    do{
        fflush(stdout);
        printf("%s\n", user_prompt());

        char *temp_command = command_fetcher(old_terminal_settings_in);
        strcpy(command, temp_command);
        int exit_status = -1;
        exit_status = process_command(command, alias_map);
        if(exit_status==100)
        {
            free(command);
            break;
        }

        buff = (char *)calloc(10,sizeof(char));
        sprintf(buff, "%d", exit_status);
        setenv("?",buff, 1);
        free(buff);

        free(command);
        command = (char*)calloc(sizeof(char), CMD_MAX_SIZE);
    }while(1);

    //exit phase
    exit_session();
    return 0;
}
