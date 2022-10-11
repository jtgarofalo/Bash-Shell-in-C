#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
  //initialize some variables
  size_t bufsize = 1000;             //buffer size
  char *cmd;                         //char array to hold command we want to execute
  char *parseCmd;                    //char array to parse each command, then used to cpy to sepCMD
  char *getCount;                    //char array that we use to find how many tokens are being passed through.
  FILE *file;                        //file name we read from, if needed
  char *path[20];                //current path we are using - can be overwritten
  char *blankPath[20];           //blank array i use to reinitialize the path
  char error_message[30] = "An error has occurred\n"; //error message array
  const char alphabet[] = "abcdefghijklmnopqrstuvwxyz0123456789$%^&*()#@!";        //used to ensure we typed something before hitting enter 
  const char delims[] = " \t\n\v\f";
  
  //string allocations 
  cmd = (char *)malloc(bufsize * sizeof(char));
  parseCmd = (char *)malloc(bufsize * sizeof(char));
  getCount = (char *)malloc(bufsize * sizeof(char));
  
  for(int i = 0; i < 20; i++){
    path[i] = malloc(bufsize * sizeof(char));
    blankPath[i] = malloc(bufsize * sizeof(char));
  }
  strcpy(path[0], "/bin");      //initial directory for path
  //check if we are reading from a file or reading commands from shell
  if(argc == 2){
    file = fopen(argv[1], "r");      //we use argument from command line to find file to read
    if(file == NULL){
      exit(1);
    }                                //if file pointer nonexistent exit program
  }
  else if(argc > 2){
    exit(1);           //we were passed more than one input file 
  }
  
  
  //begin while loop
  while(1){
    int cnt = 0;
    int numTokens = 0;
    int carrotInd = 0;
    char *ret;
    bool redirect = false;         //redirection mode set to false by default
    bool accessFail = false;       //did we fail to access an executable?
    //do not read from shell line, read from file
    if(argc == 2){
      getline(&cmd,&bufsize,file);   //reads from file versus shell line
      if(feof(file)){
	break;
      }
    }
    else{
      printf("wish> ");
      getline(&cmd,&bufsize,stdin);  //uses stdin to read from line
      ret = strpbrk(cmd,alphabet);
      if(ret == NULL){
	//WE GOT A PROBLEM! NO INPUT PASSED TO USER!
	continue;
      }
    }
    
    strcpy(getCount, cmd);                 //puts copy of cmd into getCount
    parseCmd = strtok(getCount, delims);
    while(parseCmd != NULL){
     
      //check if we will be using redirection
      if(strchr(parseCmd, '>') != NULL){
	if(redirect){
	  write(STDERR_FILENO, error_message, strlen(error_message));  //print error if we have more than one carrot in string
	}
	redirect = true;
	//'>' operator is attached to string like so: "  >blahblah"
	if((parseCmd[0] == '>') && (strlen(parseCmd) > 1)){
	  carrotInd = numTokens;
	}
	//'>' operator is floating in whitespace like so: "blah  > blah"
	else if(strlen(parseCmd) == 1){
	  carrotInd = numTokens;
	  numTokens--;
	}
	//below case is for '>' operator placed at end or middle of string: "blah>blah" OR "blah>"
	else{
	  int length = strlen(parseCmd);
	  //checks if it is at end of token
	  if(parseCmd[length-1] == '>'){
	    carrotInd = (numTokens + 1);
	  }
	  //handles case where > is in middle of string
	  else{
	    carrotInd = numTokens + 1;
	    numTokens++;
	  }
	}
      }
      numTokens++;
      parseCmd = strtok(NULL, delims);
    }
    
    //DYNAMICALLY ALLOCATE ARRAYS
    char *sepCmd[numTokens + 1];                      //char array to hold parsed commands
    char *redCmd[carrotInd + 1];                      //char array in case of redirection
	  
    //navigate inside double array and allocate individual cells
    for(int i = 0; i < numTokens + 1; i++){
      sepCmd[i] = malloc(bufsize * sizeof(char)); //not sure if bufsize * char is needed
    }
    sepCmd[numTokens] = NULL;
    if(redirect){
      for(int i = 0; i < carrotInd + 1; i++){
	redCmd[i] = malloc(bufsize * sizeof(char));
      }
      redCmd[carrotInd] = NULL;
    }
    parseCmd = strtok(cmd, delims);
    //begin to parse the cmd, store temporary parsed strings in parsCmd,
    //then they are stored permanently in sepCmd
    while(parseCmd != NULL){
       //redirection command will be implemented
      if(strchr(parseCmd, '>') != NULL){
	//handles if '>' is at beginning of token
	if((parseCmd[0] == '>') && (strlen(parseCmd) > 1)){
	  strsep(&parseCmd, ">");
	  strcpy(sepCmd[cnt],parseCmd);
	  cnt++;
	  parseCmd = strtok(NULL, delims);
	}
	//handles if '>' is a single character token
	else if(strlen(parseCmd) == 1){
	  //skip this case, we have made not of carrotInd earlier
	  parseCmd = strtok(NULL, delims);
	}
	//handles if '>' is in middle of token or at the end
	else{
	  int length = strlen(parseCmd);
	  //checks if it is at end of token
	  if(parseCmd[length-1] == '>'){
	    parseCmd = strsep(&parseCmd, ">");
	    strcpy(sepCmd[cnt],parseCmd);
	    cnt++;
	    parseCmd = strtok(NULL, delims);
	  }
	  //handles case where > is in middle of string
	  else{
	    sepCmd[cnt] = strsep(&parseCmd, ">");
	    cnt++;
	    sepCmd[cnt] = parseCmd;
	    cnt++;
	    parseCmd = strtok(NULL, delims);
	  }
	}
      }
      else{
        strcpy(sepCmd[cnt],parseCmd);
        cnt++;
        parseCmd = strtok(NULL, delims);
      }
    }
	  
    //exit condition from wish
    if(strcmp(sepCmd[0], "exit") == 0){
      if(numTokens == 1){
	exit(0);   //exit was called with no arguments, exit with no error
      }
      else{
	write(STDERR_FILENO, error_message, strlen(error_message));
	continue;
	//then continue without exiting
      }
    }
    //built in command cd
    else if(strcmp(sepCmd[0], "cd") == 0){
      //check if cd was called with more than one arg, or less than one arg
      if(numTokens != 2){
       	write(STDERR_FILENO, error_message, strlen(error_message));       //throw error because 0 or >1 args provided to cd
	continue;
      }
      else{
	if(chdir(sepCmd[1]) == -1){
	  //chdir failed, print error
	  write(STDERR_FILENO, error_message, strlen(error_message));
	}
	//dont need to do anything if chdir succeeded
	continue;
      }
    }
    //built in command path
    else if(strcmp(sepCmd[0], "path") == 0){
      //initialize path to a blank double char array
      for(int k = 0; k < 20; k++){
	memset(path[k], 0, bufsize);
      }
      //was path called with no arguments?
      if(numTokens == 1){
	strcpy(path[0], "\0");
	//if so, I believe we overwrite to a blank path
	continue;
      }
      else{
	for(int i = 0; i < numTokens; i++){
	    strcpy(path[i], sepCmd[i]);
	    }
	strcpy(path[numTokens], "\0");
	continue;
      }
    }
    //WE BEGIN THE FORK JOIN!!!!!
    int retval = fork();
    char *sysCmd;
    sysCmd = (char *)malloc(bufsize * sizeof(char));
    //child process
    if(retval == 0){
      for(int i = 0; i < 20; i++){
	if(strcmp(path[i], "\0") == 0){
	  if(accessFail || i == 0){
	    write(STDERR_FILENO, error_message, strlen(error_message));
	  }
	  //do nothing and leave for loop
	  break;
	}
	else{
	  memset(sysCmd, 0, bufsize);
	  strcpy(sysCmd, path[i]);
	  strcat(sysCmd, "/");
	  strcat(sysCmd, sepCmd[0]);

	  //REDIRECTION
	  if(redirect){
	    if(carrotInd == numTokens){ 
	      write(STDERR_FILENO, error_message, strlen(error_message)); //carrot was used in command with no file given
		      continue;
	    }
	    else if(numTokens > (carrotInd + 1)){
	      write(STDERR_FILENO, error_message, strlen(error_message));
	      continue;
	    }
	    int fd = open(sepCmd[carrotInd], O_CREAT | O_TRUNC | O_WRONLY, 0644);
	    if(fd < 0){                            //ensure open call didn't fail
	      write(STDERR_FILENO, error_message, strlen(error_message));
	    }
	    
	    if(dup2(fd, STDOUT_FILENO) == -1){
	      write(STDERR_FILENO, error_message, strlen(error_message));
	    }
	    if(dup2(fd, STDERR_FILENO) == -1){
	      write(STDERR_FILENO, error_message, strlen(error_message));
	    }
	    for(int k = 0; k < carrotInd; k++){
	      if(k != 0){
		memset(sysCmd, 0, bufsize);
		strcpy(sysCmd, sepCmd[k]);
		strcat(sysCmd, "/");
		strcat(sysCmd, sepCmd[0]);
	      }
	      strcpy(redCmd[k], sepCmd[k]);
	    }
	    
	    if(access(sysCmd, X_OK) == -1){
	      continue;     //skip to next iteration of path
	    }
	    else if(access(sysCmd, X_OK) == 0){
	      if(execv(sysCmd, redCmd) < 0){
		//should never reach here, only reach here if execv fails
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(0);
	      
	      }
	    }
	  }
	  //NO REDIRECTION
	  else{
	    if(access(sysCmd, X_OK) == -1){
	      accessFail = true;
	      continue;
	    }
	    else if(access(sysCmd, X_OK) == 0){
	      if(execv(sysCmd, sepCmd) < 0){
		//should never reach here, only reach here if execv fails
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(0);
	      
	      }
	   }
	 }
	}
      }
      exit(0);  
    }
    //parent process
    else{
      wait(NULL);
     
    }
     
    //END OF WHILE LOOP, we free everything and return it to heap
    for(int i = 0; i < numTokens + 1; i++){
      free(sepCmd[i]);
    }
    //free memory if redirect was used
    if(redirect){
      for(int k = 0; k < carrotInd + 1; k++){
	free(redCmd[k]);
      }
    }
    free(sysCmd);
  }
  exit(0);
}
