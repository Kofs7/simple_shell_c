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
#define TOK_BUFFSIZE 64

char prompt[] = "> ";
char delimiters[] = " \t\r\n";
extern char **environ;

char *read_line();
char **split_line();
void cmd_exec();


int main() {
  // Stores the string typed into the command line.
  char command_line[MAX_COMMAND_LINE_LEN];
  char cmd_bak[MAX_COMMAND_LINE_LEN];

  // Stores the tokenized command line input.
  char *arguments[MAX_COMMAND_LINE_ARGS];
    
  while (true) {
    
    do{ 
      // Print the shell prompt.
      printf("%s%s", getcwd(command_line, sizeof(command_line)), prompt);
      fflush(stdout);

      // Read input from stdin and store it in command_line. If there's an
      // error, exit immediately. (If you want to learn more about this line,
      // you can Google "man fgets")
  
      // if ((fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) && ferror(stdin)) {
      //   fprintf(stderr, "fgets error");
      //   exit(0);          
      // }

      // Split the input into an array of tokens which are interpretted as command and arguments
      char *line = read_line();
      char **tokens = split_line(line);

      // Execute command if command available
      if (tokens[0] != NULL) {
        cmd_exec(tokens);
      }

      // Free up allocated memory before looping to read next command
      free(tokens);
      free(line);

      // Changing the current working directory


      // Echoing to command line

      
      // Print the current working directory
      // if (strcmp(command_line, "pwd\n") == 0) {
      //   printf("%s\n", getcwd(command_line, sizeof(command_line)));
      // }  

      // // Exiting the shell
      // if (strcmp(command_line, "exit\n") == 0) {
      //   exit(0);
      // }

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
  }
  // This should never be reached.
  return -1;
}

// Built-in commands implementation

// void printWorkingDir(char *dir) {
//   printf("%s", dir);
// }

void changeDir(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "cd: missing argument\n");
  }
  else {
    if (chdir(args[1]) != 0) {
      perror("cd");
    }
  }
}

void exitShell(char **args) {
  exit(0);
}

void setEnv() {

}

struct builtin {
  char *command;
  void (*func)(char **args);
};

struct builtin builtins[] = {
  {"cd", changeDir},
  {"exit", exitShell},
  // {"setenv", setEnv},
};

int num_builtins() {
  return sizeof(builtins) / sizeof(struct builtin);
}

char *read_line() {
  char *line = NULL;
  size_t bufflen = 0;
  errno = 0;

  ssize_t strlen = getline(&line, &bufflen, stdin);
  if (strlen < 0) {
    if (errno) {
      perror("**Error**");
    }
    exit(0);
  }
  return line;
}

char **split_line(char *line) {
  int buffsize = TOK_BUFFSIZE, position = 0;
  char **tokens = malloc(buffsize * sizeof(char *));
  char *token;

  if (!tokens) {
    perror("***Error***");
    exit(0);
  }

  token = strtok(line, delimiters);

  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= buffsize) {
      buffsize += TOK_BUFFSIZE;
      tokens = realloc(tokens, buffsize * sizeof(char *));

      if (!tokens) {
        perror("***Error***");
        exit(0);
      }
    }
    token = strtok(NULL, delimiters);
  }
  tokens[position] = NULL;

  return tokens;
}

void cmd_exec(char **args) {
  int x;
  for(x = 0; x < num_builtins(); x++) {
    if (strcmp(args[0], builtins[x].command) == 0) {
      builtins[x].func(args);
      return;
    }
  }

  pid_t pid = fork();

  if (pid == 0) {
    execvp(args[0], args);
    perror("***Error***");
    exit(0);
  }
  else if (pid > 0) {
    int status;
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  else {
    perror("***Error***");
  }
}

