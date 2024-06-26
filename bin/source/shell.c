#include "shell.h"
#include <linux/limits.h>


// vars to load system programs
char output_file_path[PATH_MAX];
char local_path[PATH_MAX] = "/bin/";
char program_name[PATH_MAX];

/*
  Custom color helper function
*/
void red()
{
  printf("\033[1;31m");
}
void yellow()
{ 
  printf("\033[1;33m");
}
void reset()
{
  printf("\033[0m");
}

void setup_program_path(char **args)
{
  char *name = args[0];
  strcpy(program_name, output_file_path);
  strcat(program_name, local_path);
  strcat(program_name, name);
}

void print_error_message(char **args)
{
  char error_message[SHELL_BUFFERSIZE];
  sprintf(error_message, "Command %s not found", args[0]);
  perror(error_message);
}

int exec_sys_prog(char **args)
{
  setup_program_path(args);

  if (execvp(program_name, args) == -1)
  {
    print_error_message(args);
    exit(EXIT_FAILURE);
  }

  // This line will only be reached if execvp fails to create new process image
  return -1;
}

/****************** Internal Shell Functions ************************/
/**
   Change shell current directory, internal shell code
 */
int shell_cd(char **args)
{
  if (args[1] == NULL)
  {
    fprintf(stderr, "CSEShell: expected argument to \"cd\"\n");
  }
  else
  {
    // chdir() changes the current working directory of the calling process
    // to the directory specified in path.
    if (chdir(args[1]) != 0)
    { // use chdir
      perror("CSEShell:");
    }
  }

  return 1;
}

/**
   Prints out the usage and
   list of commands implemented
 */
/*
  Builtin function implementations.
*/
int shell_usage(char **args)
{
  int function_index = -1;

  if (current_number_tokens < 2)
  {
    printf("Command not given. Type usage <command>.\n");
    return 1;
  }
  // Check if the commands exist in the command list
  for (int i = 0; i < num_builtin_functions(); i++)
  {
    if (strcmp(args[1], builtin_commands[i]) == 0)
    {
      // pass it to the functions
      function_index = i;
    }
  }

  switch (function_index)
  {
  case 0:
    printf("Type: cd directory_name to change the current working directory of the shell\n");
    break;
  case 1:
    printf("Type: help for supported commands\n");
    break;
  case 2:
    printf("Type: exit to terminate the shell gracefully\n");
    break;
  case 3:
    printf("Type: usage cd/help/exit\n");
    break;
  }

  return 1;
}

/**
   Prints out the usage and
   list of commands implemented
 */
int shell_help(char **args)
{
  int i;
  printf("CSEShell Interface\n");
  printf("Usage: command arguments\n");
  printf("The following commands are implemented within the shell:\n");

  for (i = 0; i < num_builtin_functions(); i++)
  {
    printf("  %s\n", builtin_commands[i]); // print all the commands that have been implemented in the shell program
  }

  printf("This shell also supports: listdir, listdirall, summond, checkdaemon, find, and countline.\n");

  return 1;
}

/**
  Returns 0, to terminate execution from the main_loop
 */
int shell_exit(char **args)
{
  return 0;
}

/**************************************************************/

/**
   Call shell builtin functions if the command matches builtin_commands
   Otherwise, execute the system program
 */
int process_command(char **args)
{
  int child_exit_status = -1;
  int status;

  // TASK 3 /

  // 1. Check if args[0] is NULL. If it is, an empty command is entered, return 1
  // 2. Otherwise, check if args[0] is in any of our builtin_commands: cd, help, exit, or usage.
  // 3. If conditions in (2) are satisfied, call builtin shell commands, otherwise perform fork() to exec the system program. Check if fork() is successful.
  // 4. For the child process, call exec_sys_prog(args) to execute the matching system program. exec_sys_prog is already implemented for you.
  // 5. For the parent process, wait for the child process to complete and fetch the child's exit status value to child_exit_status
  // DO NOT PRINT ANYTHING TO THE OUTPUT

  /*** BEGIN ANSWER HERE ***/

  //1. Check if args[0] is NULL. If it is, an empty command is entered, return 1
  if (args[0] == NULL)
  {
    return 1;
  }


  //2. Otherwise, check if args[0] is in any of our builtin_commands: cd, help, exit, or usage.
  for(int i = 0; i < num_builtin_functions(); i++){

    if(strcmp(args[0], builtin_commands[i]) == 0){

        //3. If conditions in (2) are satisfied, call builtin shell commands
        status = builtin_command_func[i](args);
        return status;
    }}

  
    // Otherwise perform fork() to exec the system program. Check if fork() is successful.
    pid_t pid = fork();

    if (pid < 0){
      // Fork has failed. Exiting now
      return 1; // exit error
    }
    else if (pid == 0){
      status = exec_sys_prog(args);
      printf("%i",status);
    }

    else
    {
      wait(&status);
      child_exit_status = WEXITSTATUS(status);
    }
  
  /*********************/
  if (child_exit_status != 1)
  {
    printf("Command %s has terminated abruptly.\n", args[0]);
  }

  return status;
}

/**
   Read line from stdin, return a pointer to the array containing the command string entered by the user
 */
char *read_line_stdin(void)
{
  size_t buf_size = SHELL_BUFFERSIZE;           // size of the buffer
  char *line = malloc(sizeof(char) * buf_size); // allocate memory space for the line*
  
  /** TASK 1 **/
  // read one line from stdin using getline()
  // 1. Check that the char* returned by malloc is not NULL
  // 2. Fetch an entire line from input stream stdin using getline() function. getline() will store user input onto the memory location allocated in (1)
  // 3. Return the char*
  // DO NOT PRINT ANYTHING TO THE OUTPUT

  /***** BEGIN ANSWER HERE *****/
  if (line != NULL){
    //uses ssize_t instead of size_t for error handling - getline() returns -1 if nth was read
    ssize_t read = getline(&line, &buf_size, stdin);
    if ((read == 1 && line[0] == '\n') || read == 0){
      free(line);
      return NULL;
    }
  }
  else{
    // Storage is full!
    return NULL;
  }
  /*********************/
  //getline returns a /n at the end of every line, so remove it
  int len = strlen(line);
  if (line[len - 1] == '\n'){
    line[len-1] = 0;
  }

  return line;
}

/**
 Receives the *line, and return char** that tokenize the line
**/

char **tokenize_line_stdin(char *line)
{

  // create local variables to store the array of pointers to the first char of each word in the line
  int buf_size = SHELL_BUFFERSIZE, position = 0;     // assume there's also BUFFERSIZE amount of token, which is certainly enough because there's only BUFFERSIZE amount of chars
  char **tokens = malloc(buf_size * sizeof(char *)); // an array of pointers to the first char that marks a token in line
  char *token;
  

  /** TASK 2 **/
  // 1. Check that char ** that is returned by malloc is not NULL
  // 2. Tokenize the input *line using strtok() function
  // 3. Store the address to first letter of each word in the command in tokens
  // 4. Add NULL termination in tokens so we know how many "valid" addresses there are in tokens
  /***** BEGIN ANSWER HERE *****/
  if (tokens != NULL){
    token = strtok(line, " ");
    int i = 0;
    current_number_tokens = 0;

    while (token != NULL){
      tokens[i] = token;
      token = strtok(NULL, " ");
      i += 1;
      current_number_tokens += 1;
    }
  }
  else{
    // Error
  }
  /*********************/


  return tokens;
}

/**
  The main loop where one reads line,
  tokenize it, and then executes the command
 */
void main_loop(void)
{
  // instantiate local variables
  char *line;  // to accept the line of string from user
  char **args; // to tokenize them as arguments separated by spaces
  int status;  // if status == 1, prompt new user input. else, terminate the shell program.

  /** TASK 4 **/
  // write a loop where you do the following:
  status = 1;
      
  // DO NOT PRINT ANYTHING TO THE OUTPUT
  do
  {
    // decorate the prompt
    time_t raw_time;
    struct tm *time_info;
    time(&raw_time);
    time_info = localtime(&raw_time);
    printf("🐚");
    yellow();
    char *timeString = asctime(time_info);
    timeString[strlen(timeString) - 1] = '\0';
    printf(" %s", timeString);
    red();
    printf(" CSEShell\n↳ ");
    reset();
    fflush(stdout); // clear the buffer and move the output to the console using fflush
    
    
    // 1. invoke read_line_stdin() and store the output at line
    line = read_line_stdin();

    if (line == NULL){
      continue;
    }

    // 2. invoke tokenize_line_stdin(line) and store the output at args**
    args = tokenize_line_stdin(line);

    // 3. execute the tokens using process_command(args)
    status = process_command(args);     // 6. check if process_command returns 1. If yes, loop back to Step 1 and prompt user with new input. Otherwise, exit the shell.

    // Basic cleanup for the next loop
    // 4. free memory location containing the strings of characters
    free(line);

    // 5. free memory location containing char* to the first letter of each word in the input string
    free(args);    

  } while(status);  
}

int main(int argc, char **argv){

  printf("CSEShell Run successful. Running now: \n");

  // Setup path
  if (getcwd(output_file_path, sizeof(output_file_path)) != NULL)
  {
    printf("Current working dir: %s\n", output_file_path);
  }
  else
  {
    perror("getcwd() error, exiting now.");
    return 1;
  }

  // Run command loop
  main_loop();

  return 0;
}
