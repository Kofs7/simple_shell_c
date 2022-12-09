#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#include <errno.h>

#define MAX_COMMAND_LINE_LEN 1024
#define MAX_COMMAND_LINE_ARGS 128

char prompt[] = "> ";
char delimiters[] = " \t\r\n";
extern char **environ;
int cmd_pid = -1;

void changeDir(char *arguments[]);
void getWorkingDir(char *arguments[]);
void getEnv(char *arguments[]);
void echoFunc(char *arguments[]);
void setEnv(char *arguments[]);
void exitShell(char *arguments[]);
void parse(char *token);
void cmd_exec(char *arguments[]);
void signal_handler(int signum);
void timeout_process(int time, int pid);
bool isBuiltin(char *arg);


int main() {
  // Stores the string typed into the command line.
  int position;
  size_t size = 300;
  char cmd_dir[size];
  char *cmd_dir_ptr;
  char command_line[MAX_COMMAND_LINE_LEN];
  char cmd_bak[MAX_COMMAND_LINE_LEN];

  // Stores the tokenized command line input.
  char *arguments[MAX_COMMAND_LINE_ARGS];
  signal(SIGINT, signal_handler);
    
  while (true) {
    
    do{ 
      fflush(stdout);
      // Print the shell prompt.
      cmd_dir_ptr = getcwd(cmd_dir, size);
      printf("%s%s", cmd_dir_ptr, prompt);
      fflush(stdout);

      // Read input from stdin and store it in command_line. If there's an
      // error, exit immediately. (If you want to learn more about this line,
      // you can Google "man fgets")
  
      if ((fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) && ferror(stdin)) {
        fprintf(stderr, "fgets error");
        exit(0);          
      }

      // Free up allocated memory before looping to read next command
      // free(arguments);

    }while(command_line[0] == 0x0A);  // while just ENTER pressed

    
    // If the user input was EOF (ctrl+d), exit the shell.
    if (feof(stdin)) {
        printf("\n");
        fflush(stdout);
        fflush(stderr);
        return 0;
    }

    // TODO:
    // 
    
    // 0. Modify the prompt to print the current working directory
    
  
    // 1. Tokenize the command line input (split it on whitespace)

  
    // 2. Implement Built-In Commands
  

    // 3. Create a child process which will execute the command line input


    // 4. The parent process should wait for the child to complete unless its a background process
  
  
    // Hints (put these into Google):
    // man fork
    // man execvp
    // man wait
    // man strtok
    // man environ
    // man signals
    
    // Extra Credit
    // man dup2
    // man open
    // man pipes
    
    // Split line and tokenize line inputs
    arguments[0] = strtok(command_line, delimiters);
    position = 0;

    while (arguments[position] != NULL) {
      parse(arguments[position]);
      position++;
      arguments[position] = strtok(NULL, delimiters);
    }

    // Adding background process
    // Check if background process is requested and remove '&' from arguments list
    char *last_token = arguments[position-1];
    bool background = false;
    if (strcmp(last_token, "&") == 0) {
      background = true;
      arguments[position-1] = NULL;
    }
    
    // Implement Built-In Commands
    if (strcmp(arguments[0], "cd") == 0) {
      changeDir(arguments);
    }
    else if (strcmp(arguments[0], "pwd") == 0) {
      getWorkingDir(arguments);
    }
    else if (strcmp(arguments[0], "env") == 0) {
      getEnv(arguments);
    }
    else if (strcmp(arguments[0], "echo") == 0) {
      echoFunc(arguments);
    }
    else if (strcmp(arguments[0], "setenv") == 0) {
      setEnv(arguments);
    }
    else if (strcmp(arguments[0], "exit") == 0) {
      exitShell(arguments);
    }
    else {
      // Fork a child process to execute the command.
      int pid;
      if ((pid = fork()) == 0) {
        // Child process.
        signal(SIGINT, SIG_DFL);
        // cmd_exec(arguments);
        if (execvp(arguments[0], arguments) == -1) {
          perror("execvp() failed");
        }
        else {
          cmd_exec(arguments);
        }
        exit(0);
      }
      else {
        // Parent process.
        if (background) {
          cmd_pid = -1;
        }
        else {
          cmd_pid = pid;
          int pid_2;
          if ((pid_2 = fork()) == 0) {
            signal(SIGINT, SIG_DFL);
            timeout_process(10000, pid);
            exit(0);
          }
          else {
            waitpid(pid, NULL, 0);
            kill(pid_2, SIGINT);
            waitpid(pid_2, NULL, 0);
          }
        }
      }
    }
  }
  // This should never be reached.
  return -1;
}

// Built-in commands implementation

void changeDir(char *arguments[]) {
  if (arguments[1] == NULL) {
    fprintf(stderr, "cd: missing argument\n");
  }
  else {
    if (chdir(arguments[1]) != 0) {
      perror("cd");
    }
  }
}

void getWorkingDir(char *arguments[]) {
  char cwd[MAX_COMMAND_LINE_LEN];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("%s\n", cwd);
  }
  else {
    perror("getcwd() error");
  } 
}

void getEnv(char *arguments[]) {
  if (arguments[1] != NULL) {
    printf("%s\n", getenv(arguments[1]));
  }   
  else {
    int i = 0;
    while(environ[i]) {
      printf("%s\n", environ[i++]); // prints in form of "variable=value"
    }
  }
}

void exitShell(char *arguments[]) {
  exit(0);
}

void setEnv(char *arguments[]) {
  if (arguments[1] == NULL) {
    fprintf(stderr, "setenv: missing argument\n");
  }
  else {
    char *list[2];
    int i = 0;

    char *ptr = strtok(arguments[1], "=");

    while(ptr != NULL) {
      list[i] = ptr;
      i++;
      ptr = strtok(NULL, "=");
    } 

    if (setenv(list[0], list[1], 1) != 0) {
      perror("setenv");
    }
  }
}

void echoFunc(char *arguments[]) {
  if (arguments[1] == NULL) {
    fprintf(stderr, "echo: missing argument\n");
  }
  else {    
    int x = 1;

    while (arguments[x] != NULL) {
      if (arguments[x][0] == '$') {
        printf("%s ", getenv(arguments[x]+1));
      }
      else {
        printf("%s ", arguments[x]);
      }
      x++;
    }
    printf("\n");
  }
}

// Parse line inputs
void parse(char* token) {
  char start, end;
  bool quotes = false;

  if (token[0] == '\"' || token[0] == '\'') {
    start = token[0];
    quotes = true;
  }
  if (quotes) {
    int n = 0;
    while (token[n] != '\0') {
      end = token[n];
      n++;
    }
    if (start == end) {
      n = 1;
      while (token[n] != '\0') {
        token[n-1] = token[n];
        n++;
      }
      token[n-1] = '\0';
      token[n-2] = '\0';
    }
  }
}

void timeout_process(int time, int pid) {
  sleep(time);
  printf("Foreground process timed out\n");
  kill(pid, SIGINT);
}

void cmd_exec(char *arguments[]) {
  char *piped_args[MAX_COMMAND_LINE_ARGS][MAX_COMMAND_LINE_ARGS];
  int n = 0;
  int cmd_count = 0;
  int cmd_token_count = 0;

  while (arguments[n] != NULL) {
    if (strcmp(arguments[n], "|") == 0) {
      if (cmd_token_count == 0) {
        fprintf(stderr, "Invalid pipe command\n"); //**
        return;
      }

      piped_args[cmd_count][cmd_token_count] = NULL;
      cmd_count++;
      cmd_token_count = 0;
    }
    else {
      piped_args[cmd_count][cmd_token_count] = arguments[n];
      cmd_token_count++;
    }
    n++;
  }

  int pipe_count = cmd_count;
  cmd_count++;
  if (pipe_count == 0) {
    execvp(piped_args[0][0], piped_args[0]);
  }
  else {
    int pipes[pipe_count][2];
    for (n = 0; n < pipe_count; n++) {
      pipe(pipes[n]);
    }

    int pd[cmd_count];
    for (n = 0; n < cmd_count; n++) {
      if ((pd[n] = fork()) == 0) {
        int x;
        if (n == 0) {
          dup2(pipes[0][1], 1);
          close(pipes[0][0]);

          for (x = 1; x < pipe_count; x++) {
            close(pipes[x][0]);
            close(pipes[x][1]);
          }
        }
        else if (n == cmd_count) {
          dup2(pipes[pipe_count-1][0], 0);
          close(pipes[pipe_count-1][1]);

          for (x = 0; x < pipe_count-1; x++) {
            close(pipes[x][0]);
            close(pipes[x][1]);
          }
        }
        else {
          dup2(pipes[n-1][0], 0);
          dup2(pipes[n][1], 1);

          for (x = 0; x < pipe_count; x++) {
            if (x == n-1) {
              close(pipes[x][1]);
            }
            else if (x == n) {
              close(pipes[x][0]);
            }
            else {
              close(pipes[x][0]);
              close(pipes[x][1]);
            }
          }
        }
        execvp(piped_args[n][0], piped_args[n]);
      }
      else if (pd[n] < 0) {
        perror("fork() failed");
        exit(0);
        // printf("An error occurred\n");
        // return;
      }
    }
    for (n = 0; n < pipe_count; n++) {
      close(pipes[n][0]);
      close(pipes[n][1]);
    }
    for (n = 0; n < cmd_count; n++) {
      wait(NULL);
    }
  }
}

void signal_handler(int signum) {
  if (cmd_pid != -1) {
    kill(cmd_pid, SIGINT);
  }
}

