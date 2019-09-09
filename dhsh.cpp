#include "dhsh.hpp"

using namespace std;

//tab completion

void directory_contents(char *path, vector<char*>& binarys)
{
    struct dirent *de;
    DIR *dir = opendir(path);

    if (!dir)
    {
        printf("COULDNOT OPEN DIR\n" );
        return;
    }

    while ((de = readdir(dir)) != NULL)
    {
        if(de->d_type != DT_DIR)
        {
            char *temp = (char*)calloc(sizeof(char), strlen(de->d_name));
            //printf("%d->Here->%s\n",ccccc++,de->d_name);
            strcpy(temp, de->d_name);
            binarys.push_back(temp);
        }
    }
    closedir(dir);
    return;
}

void construct_trie()
{
    char *all_paths = getenv("PATH");
    char **paths = (char **)calloc(sizeof(char*), MAX_NUM_PATHS);
    char *cur_path = strtok(all_paths, ":");
    int num_of_paths=0;
    while(cur_path)
    {
        paths[num_of_paths] = (char*)calloc(sizeof(char), MAX_PATH_LENGTH);
        strcpy(paths[num_of_paths], cur_path);
        cur_path = strtok(NULL, ":");
        num_of_paths++;
    }

    vector<char*> binaries;

    for(int i = 0; i < 2; i++)
    {
        directory_contents(paths[i], binaries);
    }
    return;
}

char *replaceWord(const char *s, const char *oldW, const char *newW)
{
    char *result;
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);

    for (i = 0; s[i] != '\0'; i++)
    {
        if (strstr(&s[i], oldW) == &s[i])
        {
            cnt++;

            i += oldWlen - 1;
        }
    }

    result = (char *)malloc(i + cnt * (newWlen - oldWlen) + 1);

    i = 0;
    while (*s)
    {
        if (strstr(s, oldW) == s)
        {
            strcpy(&result[i], newW);
            i += newWlen;
            s += oldWlen;
        }
        else
            result[i++] = *s++;
    }

    result[i] = '\0';
    return result;
}

char** env_setup(char *set_line)
{
    char *token = strtok(set_line, "=");
    char **tokens = (char**)calloc(sizeof(char*),2);
    int i = 0;

    while(token)
    {
        tokens[i] = (char*)calloc(sizeof(char), strlen(token));
        strcpy(tokens[i], token);
        token = strtok(NULL, "=");
        i++;
    }
    return tokens;
}

vector<char**> set_up()
{
    vector<char**> env_set;
    int rc_fd = -1;

    int alarm_fd = open("/common/alarm",O_RDONLY);
    if(alarm_fd > 0)
    {
        int read_bytes = -1;
        char *alarm_buff = (char*)calloc(sizeof(char), 256);
        while((read_bytes = read(alarm_fd, alarm_buff, 256))>0)
        {
            write(1, alarm_buff, read_bytes);
        }
        close(alarm_fd);
        remove("/common/alarm");
    }

    rc_fd = open("/common/.dhshrc", O_RDONLY);
    if(rc_fd != -1)
    {
        int read_bytes = 0;
        char *buff = (char*)calloc(sizeof(char),4096);
        if(buff)
        {
            if((read_bytes = read(rc_fd, buff, 4096)))
            {
                vector<char *>  lines_vec;
                char *lines = strtok(buff, "\n");
                while(lines)
                {
                    char *line = (char*)calloc(sizeof(char), strlen(lines));
                    strcpy(line, lines);
                    lines_vec.push_back(line);
                    lines = strtok(NULL, "\n");
                }
                for(int i = 0; i < lines_vec.size(); i++)
                {
                    char** env_ret = env_setup(lines_vec[i]);
                    if(!env_ret)
                    {
                        printf("ERROR SETTING UP ENV VAR\n");
                    }
                    env_set.push_back(env_ret);
                }
            }
            free(buff);
        }
    }
    else
    {
        /* code */
        cout<<".dhshrc FILE NOT FOUND"<<endl;
    }
    return env_set;
}

int exit_session()
{
    return 0;
}

//execute and store command in .dhsh_history

char* user_prompt()
{

    char* username = NULL;
    char *PS1_prompt = NULL;
    PS1_prompt = getenv("PS1");
    // cout<<"empty"<<PS1_prompt<<endl;
    if(PS1_prompt)
    {
        username = (char *)calloc(sizeof(char), strlen(PS1_prompt));
        strcpy(username, PS1_prompt);
    }
    else
    {
        pid_t child_pid;

        int  status;

        if ((child_pid = fork()) < 0)
        {
            printf("ERROR FORKING PROCESS\n");
            exit(1);
        }
        else if (child_pid == 0)
        {
            int user_fd = open("/common/.user_temp", O_RDWR|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO);
            dup2(user_fd, 1);
            char *whoami = (char*)calloc(strlen("whoami"), sizeof(char));
            strcpy(whoami, "whoami");
            char *args[] = {whoami, NULL};
            if (execvp(args[0], args) < 0) {
                printf("ERROR FETCHING USERNAME\n");
                exit(1);
            }
            free(whoami);
            close(user_fd);
        }
        else
        {
            while (wait(&status) != child_pid);

            int user_fd = open("/common/.user_temp", O_RDONLY);

            if(user_fd)
            {
                char *uname_buff = (char*)calloc(USERNAME_MAX_SIZE, sizeof(char));

                if(uname_buff)
                {
                    int read_bytes = read(user_fd, uname_buff, USERNAME_MAX_SIZE);
                    if(read_bytes > 0)
                    {
                        username = (char*) calloc(sizeof(char), USERNAME_MAX_SIZE + 1);

                        int i = strlen(uname_buff) - 1;
                        while(uname_buff[i] == '\n' || uname_buff[i] == ' ' || uname_buff[i] == '\t')
                        {
                            i--;
                        }
                        uname_buff[i + 1] = '\0';
                        strcpy(username, uname_buff);
                    }
                    else
                    {
                        /* code */
                        printf("NO CONTENT WRITTEN TO THE USER_TEMP FILE\n");
                    }

                    free(uname_buff);
                }
                else
                {
                    /* code */
                    printf("ERROR CREATING UNAME BUFF\n");
                }
                close(user_fd);
                remove("/common/.user_temp");
            }
            else
            {
                /* code */
                printf("ERROR OPENING THE USER_TEMP FILE\n");
            }

        }
    }
    return username;
}


char * command_fetcher(struct termios old_terminal_settings)
{
    char *command = NULL;
    int command_char = '\0';

    struct termios new_terminal_settings;
	memcpy(&new_terminal_settings, &old_terminal_settings, sizeof(struct termios));

    new_terminal_settings.c_lflag &= ~(ICANON| ECHO);
    new_terminal_settings.c_cc[VTIME] = 0;
    new_terminal_settings.c_cc[VMIN] = 1;
    tcsetattr(fileno(stdin), TCSANOW, &new_terminal_settings);
    tcsetattr(fileno(stdin), TCSANOW, &new_terminal_settings);

    command = (char *)calloc(sizeof(char), CMD_MAX_SIZE);
    size_t top;
    top = 0;

    while (read(fileno(stdin), &command_char, sizeof command_char) == 1)
    {
        int empty_command = -1;
        switch (command_char)
        {
            case BACKSPACE_KEY:

                if (top)
                {
                    --top;
                    if(top <= 0)
                        empty_command = 1;
                    const char delbuf[] = "\b \b";
                    write(fileno(stdout), delbuf, strlen(delbuf));
                }
                break;

            case NEW_LINE:
                {
                    write(fileno(stdout), &command_char, sizeof command_char);
                    // write(fileno(stdout), command, top);
                    char* temp_command = (char*)calloc(sizeof(char), top);
                    strncpy(temp_command, command, top);
                    strcpy(command, temp_command);
                    free(temp_command);
                    top = 0;
                    if(strlen(command))
                        empty_command = 0;
                }
                break;

	        case TAB_KEY:
                {
                    //call trie
                    char *temp = (char*)calloc(sizeof(char), top);
                    strncpy(temp, command, top);
                }
                break;

            default:
                command[top++] = command_char;
                write(fileno(stdout), &command_char, sizeof command_char);
                break;
         }
         if(!top && !empty_command)
            break;
    }

    //reset terminal
    tcsetattr(fileno(stdin), TCSANOW, &old_terminal_settings);

    return command;
}


void process_echo_string(char** command, int num_of_args)
{
    char *env_variable = NULL;
    char *env_data = (char *)calloc(sizeof(char), MAX_ENV_VAR_SIZE);
    for(int i = 1; i < num_of_args; i++)
    {
        if(command[i])
        {
            if(command[i][0] != '-')
            {
                for(int j = 0; j < strlen(command[i]); j++)
                {
                    if(command[i][j] == '$')
                    {
                        env_variable = (char *)calloc(sizeof(char), 2048);
                        int k = 0;
                        j++;
                        while(isalnum(command[i][j]) || (command[i][j] == '_') || (command[i][j] == '?') || (command[i][j]=='$'))
                        {
                            env_variable[k++] = command[i][j++];
                        }
                        env_variable[k] = '\0';
                        env_data = getenv(env_variable);
                        if(!env_data)
                        {
                            env_data = "";
                        }
                        int m = strlen(env_variable);
                        while(m >= 0)
                        {
                            env_variable[m + 1] = env_variable[m];
                            m--;
                        }
                        env_variable[m+1] = '$';
                        char *result = replaceWord(command[i], env_variable, env_data);
                        strcpy(command[i], result);
                    }
                }
            }
        }
    }
    return;
}

int execute_command(char* command[], int num_of_args, int bg_flag)
{
     if(strcmp(command[0], "exit")==0)
        return 100;

    if(strcmp("echo", command[0]) == 0)
    {
        process_echo_string(command, num_of_args);
    }

    for(int j = 1; j < num_of_args; j++)
        {
            if(command[j])
            {
                if(strcmp(">", command[j])==0)
                {
                    close(1);
                    int _fd = open(command[j+1], O_CREAT|O_TRUNC|O_WRONLY, 0644);
                    if(_fd > 0)
                    command[j] = command[j+1] = NULL;
                    break;
                }
                else if(strcmp(">>", command[j])==0)
                {
                    close(1);
                    int _fd = open(command[j+1], O_CREAT|O_WRONLY|O_APPEND, 0644);
                    if(_fd > 0)
                    command[j] = command[j+1] = NULL;
                    break;
                }
                else if(strcmp("<", command[j])==0)
                {
                    close(0);
                    int _fd = open(command[j+1], O_RDONLY);
                    if(_fd >= 0)
                    command[j] = command[j+1] = NULL;
                    break;
                }
            }
        }

    if(bg_flag==1)
    {   
        cout<<"BG "<<getpid()<<endl;
        pid_t cur_grp = getpgrp();
        command[num_of_args - 2] = NULL;
        // setpgid(0,0);
        // tcsetpgrp(STDIN_FILENO ,cur_grp);
        // tcsetpgrp(STDOUT_FILENO ,cur_grp);
        // tcsetpgrp(STDERR_FILENO ,cur_grp);
        // cout<<"paretngrp "<<cur_grp<<"present grp "<<getpgrp()<<"presnt pid "<<getpid()<<"parentpid "<<getppid()<<endl;

        char *file_name = (char *)calloc(100,sizeof(char));
        sprintf(file_name, "/common/%d", getpid());
        int dev_null_fd = open(file_name, O_CREAT|O_WRONLY, 0777);
        free(file_name);
        dup2(dev_null_fd, STDOUT_FILENO);
    }


    if(execvp(command[0], command) < 0)
    {
        printf("ERROR EXECUTING COMMAND\n");
        exit(1);
    }
    // printf("%s->length%d\n", command[0], num_of_args);
    return 0;
}


void command_trim(char *command)
{
    //end trim
    int i = strlen(command) - 1;
    while((i > 0) && ((command[i] == ' ') || (command[i] == '\t' || command[i] == '\n')))
    {
        i--;
    }
    command[i + 1] = '\0';

    //front trim
    i = 0;
    while((i < strlen(command)) && (command[i] == ' '|| command[i] == '\t'|| command[i] == '\n'))
        i++;


    char *temp_command = (char *)calloc(sizeof(char), strlen(command));
    strcpy(temp_command, command + i);

    strcpy(command, temp_command);

    free(temp_command);

    return;
}


int execute_pipe_line(char ***command, int* args_count)
{
	int command_pipe[2];
	pid_t child_pid;
	int prev_out_fd = 0;
    int i = 0;
    if(strcmp(command[0][0], "fg")==0)
    {
        // int cur_group = 0;
        pid_t cur_group = getpgrp();
        // tcsetpgrp(STDIN_FILENO ,atoi(command[0][1]));
        // tcsetpgrp(STDOUT_FILENO ,atoi(command[0][1]));
        // tcsetpgrp(STDERR_FILENO ,atoi(command[0][1]));
        char *file_name = (char *)calloc(100,sizeof(char));
        sprintf(file_name, "/common/%s", command[0][1]);
        int out_file = open(file_name, O_RDWR);
        if(out_file != -1)
        {
            int read_bytes = -1;
            char *data_buff = (char*)calloc(sizeof(char), 1024);
            while((read_bytes = read(out_file, data_buff, 1024))>0)
            {
                write(STDOUT_FILENO, data_buff, read_bytes);
            }
            close(out_file);
            remove(file_name);
            free(file_name);
        }
        else
        {
            /* code */
            printf("NOT OPENING\n");
        }
        return cur_group;
    }

    if(strcmp(command[0][0], "exit")==0)
        return 100;
	while (command[i] != NULL)
    {
		pipe(command_pipe);
		if ((child_pid = fork()) == -1)
        {
			exit(1);
		}
		else if (child_pid == 0)
        {
			dup2(prev_out_fd, 0);
			close(command_pipe[0]);
			if (command[i + 1] != NULL)
            {
				dup2(command_pipe[1], 1);
			}
            int bg = 0;
            if(strcmp(command[i][args_count[i]-2], "&") == 0)
                bg=1;
            execute_command(command[i], args_count[i],bg);
		}
		else
        {
			close(command_pipe[1]);
			prev_out_fd = command_pipe[0];
            i++;
		}
	}
    wait(NULL);
	return 0;
}

void display_message(int s) 
{
     int t = open("/common/alarm",O_CREAT|O_APPEND|O_WRONLY, 0777);
	if(t > 0)
	{
		time_t rawtime;
		struct tm * timeinfo;

		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		char *buff = (char*)calloc(256, sizeof(char));
		sprintf(buff, "Wake-Up Samurai! You Have a city to burn, its %s\n", asctime (timeinfo));
		printf(buff);
		int w=	write(t, buff, strlen(buff));
		close(t);
	}
    signal(SIGALRM, display_message);
}

int process_command(char* command, map<string, string> & alias_map)
{
    int exit_status = -1;
    if(command)
    {
        int len_of_command = strlen(command);
        char **commands_list = (char **)calloc(MAX_COMMANDS, sizeof(char *));
        int num_of_commands = 0;
        if(len_of_command > 0)
        {
            char *command_tokens = strtok(command, "|");
            while(command_tokens)
            {
                commands_list[num_of_commands] = (char *) calloc(sizeof(char), strlen(command_tokens));
                strcpy(commands_list[num_of_commands], command_tokens);

                command_trim(commands_list[num_of_commands]);

                command_tokens = strtok(NULL, "|");
                num_of_commands++;
            }
        }

        char ***command_args = (char***)calloc(sizeof(char **), num_of_commands + 1);
        int *args_count = (int*)calloc(sizeof(int), num_of_commands + 1);
        for(int i = 0; i < num_of_commands; i++)
            command_args[i] = (char **)calloc(MAX_COMMAND_ARGS, sizeof(char *));

        for(int i = 0; i < num_of_commands; i++)
        {
            int num_of_args = 0;

            char *arg_tokens = strtok(commands_list[i], " ");

            while(arg_tokens)
            {
                command_args[i][num_of_args] = (char *)calloc(sizeof(char), MAX_ARG_LENGTH);
                strcpy(command_args[i][num_of_args], arg_tokens);

                command_trim(command_args[i][num_of_args]);

                arg_tokens = strtok(NULL, " ");
                num_of_args++;
            }
            string temp_command(command_args[i][0]);
            if(alias_map.find(temp_command)!=alias_map.end())
            {
                //cout<<alias_map[temp_command]<<endl;
                char **alias_args = (char **)calloc(sizeof(char*), MAX_COMMAND_ARGS);
                char *val_command = (char *)calloc(sizeof(char), alias_map[temp_command].length());
                strcpy(val_command, alias_map[temp_command].c_str());

                char *args_of_alias_command = strtok(val_command, " ");
                int args_count = 0;
                while(args_of_alias_command)
                {
                    alias_args[args_count] = (char *)calloc(sizeof(char), MAX_ARG_LENGTH);
                    strcpy(alias_args[args_count], args_of_alias_command);
                    command_trim(alias_args[args_count]);
                    args_of_alias_command = strtok(NULL, " ");
                    args_count++;
                }

                char **args_final = (char**)calloc(sizeof(char*), MAX_COMMAND_ARGS);
                int k = 0;
                for(int j = 0; j < args_count; j++)
                {
                    args_final[k] = (char*)calloc(sizeof(char), MAX_ARG_LENGTH);
                    strcpy(args_final[k], alias_args[j]);
                    k++;
                }
                for(int j = 1; j < num_of_args; j++)
                {
                    args_final[k] = (char*)calloc(sizeof(char), MAX_ARG_LENGTH);
                    strcpy(args_final[k], command_args[i][j]);
                    k++;
                }
                command_args[i] = args_final;
                num_of_args = num_of_args + args_count;
            }
            command_args[i][num_of_args++] = NULL;
            args_count[i] = num_of_args;
        }
        int i = 0;
        if(strcmp("cd", command_args[i][0]) == 0)
        {
            char *dest_dir = (char*)calloc(sizeof(char), 2048);
            if(command_args[i][1][0] == '~')
            {
                char *home_dir = getenv("HOME");
                int k = 0;
                if(!home_dir)
                {
                    dest_dir[0] = '.';
                    k = 1;
                }
                else
                {
                    strcpy(dest_dir, home_dir);
                    k = strlen(home_dir);
                }
                    strcpy(dest_dir + k, command_args[i][1] + 1);
            }
            else
                strcpy(dest_dir, command_args[i][1]);
            if(chdir(dest_dir)<0)
            {
                printf("NO SUCH DIR EXISTS\n");
            }
            return 0;
        }
        else if (strcmp("alias", command_args[i][0]) == 0)
        {
            string key(command_args[i][1]);
            alias_map[key] = "";
            for(int j = 2; j < args_count[0]; j++)
            {
                if(command_args[i][j] && strcmp(command_args[i][j],"=")!=0)
                {
                    string data(command_args[i][j]);
                    alias_map[key]+=data+ " ";
                }
            }
            //cout<<alias_map[key]<<endl;
            return 0;
        }
        else if(strcmp("open", command_args[i][0]) == 0)
        {
            char *token = strtok(command_args[i][1], ".");
            char *temp = (char*)calloc(sizeof(char), 10);
            while(token)
            {
                strcpy(temp, token);
                token=strtok(NULL, ".");
            }
            char * _env = getenv(temp);
            char **new_args = (char**)calloc(sizeof(char*), MAX_COMMAND_ARGS);
            if(_env)
            {
                int k = 0;
                new_args[k++]=_env;
                new_args[k++]=command_args[i][1];
            }
            command_args[i] = new_args;
        }
        else if (strcmp("export", command_args[i][0]) == 0)
        {
            if(args_count[0]==4)
            {
                int ret_val = setenv(command_args[i][1],command_args[i][2],1);
                exit_status = ret_val;
                if(ret_val<0)
                    printf("SET FAILED\n");
                return exit_status;
            }
            return exit_status;
        }
        else if(strcmp("alarm", command_args[i][0])==0)
        {
            if(args_count[0]==3)
            {
                int time_in_sec = atoi(command_args[i][1]);
                signal(SIGALRM, display_message);
                alarm(time_in_sec);
            }
            return 0;
        }
        exit_status = execute_pipe_line(command_args,args_count);
        // cout<<exit_status<<endl;
        if(exit_status == getpgrp())
        {
            tcsetpgrp(STDIN_FILENO ,exit_status);
            tcsetpgrp(STDOUT_FILENO ,exit_status);
            tcsetpgrp(STDERR_FILENO ,exit_status);
        }
        // pid_t child_fd;
        // if((child_fd = fork()) < 0)
        // {
        //     printf("ERROR CALLING COMMAND\n");
        // }
        // else if(!child_fd)
        // {
        //     pid_t cur_pid = getpid();
        //     setpgid(cur_pid, cur_pid);
        //     exit(execute_pipe_line(command_args,args_count));
        // }
        // else
        // {
        //     int *status = (int *)calloc(1, sizeof(int));

        //     wait(status);
        //     if(WIFEXITED(*status))
        //         exit_status = WEXITSTATUS(*status);
        // }
    }
    return exit_status;
}
