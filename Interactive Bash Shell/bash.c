#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

// 2 cases: background or foreground
/*
void NormalForeground(char** normal, char * actual_path1){

  pid_t pid = fork();
    if (pid == -1) {
        perror("fork() failed\n");
        return;
    } else if (pid == 0) {
        execv(actual_path1, normal);
    } else {
        int status;
        // waiting for child to terminate
        pid_t child_pid = waitpid(pid,&status,0);
        return;
    }
}

void NormalBackground(char** normal, char * actual_path1){
  pid_t pid = fork();
    if (pid == -1) {
        printf("\nFailed forking child..");
        return;
    } else if (pid == 0) {
        execv(actual_path1, normal);
    } else {
        // waiting for child to terminate
        waitpid(-1,&status,WNOHANG);
        return;
    }
}

// 2 cases: background or foreground
void PipeForeground (char ** normal, char** pipe; char * actual_path1, char * actual_path2){

}
void PipeBackground (char ** normal, char** pipe, char * actual_path1, char * actual_path2){

}
*/

int count_spaces(char *inny){
    int g = 0;
    int spaces = 0;
  while(inny[g] != '\n'){
    if(inny[g] == ' '){
      spaces++;
    }
    g++;  
  }
  return spaces;
}

int Is_this_foreground(char * str, char ** normal){
  int index = strlen(str);
  if (str[index-1] == '&'){
   int i;
    for (i=0; i<128;i++){
      if (normal[i] == NULL){
        if(strcmp(normal[i-1], "&")==0){
          free(normal[i-1]);
          normal[i-1] = NULL;
        }
        else{
          normal[i-1]=strtok(normal[i-1],"&");
        }
        break;
      }
    }

    return 0;
  }
  return 1;
}

int check_command(char** normal, char** pipe, int piped, char ** paths, char * actual_path1, char * actual_path2, char* mypath, int sp, char * location){
// in case of special case "cd" but what if cd fails?
  if (strcmp(normal[0], "cd")==0){
    errno = 0;
    if(sp < 1){chdir(getenv("HOME"));}
    else if(normal[1][0] == '/'){
      chdir(normal[1]);
    }
    else{
      char *newdir = calloc(1000, sizeof(char));
//      printf("You calloced newdir, size 1000\n");
      strcpy(newdir,location);
      //printf("Old directory is %s\n", location);
      int hold_plz = strlen(newdir);
      newdir[strlen(newdir)] = '/';
      int o = 0;
      //printf("The length of the input is: %d\n",strlen(args[1]));
      while(o < strlen(normal[1])){
        newdir[hold_plz+o+1] = normal[1][o];
        o++;
      }
      newdir[hold_plz+o+1] = '\0';
      //printf("New directory is %s\n", newdir);
      if(chdir(newdir) == -1){errno = 0; fprintf(stderr, "chdir() failed: Not a directory\n");}
      free(newdir);
    }
    return 0;
  }





  int i=0;
  int length=0;
  char * path1 = calloc(100,sizeof(char));
  char * path2 = calloc(100, sizeof(char));
  int size1 =0;
  int size2=0;
  int exist1 = 0;
  int exist2 = 0;
  int rc=0;
  struct stat buffer;
  char * copy = calloc(strlen(mypath)+1,sizeof(char));
  strcpy(copy,mypath);
   char * token = strtok(copy, ":");
    while (token != NULL){
    paths[i] = calloc(strlen(token)+1,sizeof(char));
    strcpy(paths[i],token);
    token = strtok(NULL, ":");
    i++;
    length++;
   }
   free(copy);

  if (piped){

    for (i = 0; i<length; i++){
      size1 = strlen(paths[i])+1+strlen(normal[0]);
      strcpy(path1, paths[i]);
      strcat(path1, "/");
      strcat(path1, normal[0]);
      path1[size1] = '\0';
      size2 = strlen(paths[i])+1+strlen(pipe[0]);
      strcpy(path2, paths[i]);
      strcat(path2, "/");
      strcat(path2, pipe[0]);
      path2[size2] = '\0';
      rc = lstat(path1,&buffer);
      if (rc == 0){
        exist1 = 1;
        strcpy(actual_path1, path1);
      }
      rc = lstat(path2,&buffer);
      if (rc == 0){
        exist2 = 1;
        strcpy(actual_path2, path2);
      }
    }
    free(path1);
    free(path2);

    if (exist1 == 0){
      fprintf(stderr,"ERROR: command \"%s\" not found\n", normal[0]);
    }
    if (exist2 == 0){
      fprintf(stderr, "ERROR: command \"%s\" not found\n", pipe[0]);
    }
    if (exist1 == 0 || exist2 == 0){
      return 0;
    }
    else if (exist1 == 1 && exist2 == 1){
      return 1;
    }
  }
  else{
    for (i = 0; i<length;i++){
      size1 = strlen(paths[i])+1+strlen(normal[0]);
      strcpy(path1, paths[i]);
      strcat(path1, "/");
      strcat(path1, normal[0]);
      path1[size1]= '\0';
       rc = lstat(path1,&buffer);
      if (rc == 0){
        exist1 = 1;
        strcpy(actual_path1, path1);
      }
    }
    free(path1);
    free(path2);
     if (exist1 == 0){
      fprintf(stderr, "ERROR: command \"%s\" not found\n", normal[0]);
      return 0;
    }
    else if (exist1 == 1){
      return 1;
    }
  }

  return 1;
}

int is_this_pipe(char* str, char** tmp){

   int i=0;
   char * copy = calloc(strlen(str)+1, sizeof(char));
   strcpy(copy, str);
   char * token = strtok(copy, "|");
   while (token != NULL){
    tmp[i] = calloc(strlen(token)+1,sizeof(char));
    strcpy(tmp[i],token);
    token = strtok(NULL, "|");
    i++;
  }free(copy);

    if (tmp[1] == NULL) {
        return 0; }// returns zero if no pipe is found.
    else {
        return 1;
}
}

void parseSpace(char * str, char ** parsed){
   int i =0;
   char * copy = calloc(strlen(str)+1, sizeof(char));
   strcpy(copy, str);
   char * token = strtok(copy, " ");
    while (token != NULL){

    parsed[i] = calloc(strlen(token)+1,sizeof(char));
    strcpy(parsed[i],token);
    token = strtok(NULL, " ");
    i++;
   }
   free(copy);

}

int processString(char* str, char** normal, char** pipe, char ** paths, char * actual_path1, char * actual_path2, char * mypath, int space, char * cwd)
// here, determines if the command executable exists by check_command; if normal or pipe.
{
 // int k;
 char ** tmp = calloc(2, sizeof(char*));
  int piped = 0;


  piped = is_this_pipe(str, tmp);
  if (piped){
    parseSpace(tmp[0], normal);
    parseSpace(tmp[1], pipe);
  }
  else{
    parseSpace(str, normal);
  }

  int i;
  for (i =0; i<2;i++){
    free(tmp[i]);
  }
  free(tmp);
  // cases for pipe and none? when checking commands?
  if (!(check_command(normal,pipe,piped,paths, actual_path1, actual_path2, mypath, space, cwd)))
    return 0;
  else
    return 1+piped;

  }



int main() {
  setvbuf(stdout, NULL, _IONBF, 0);
  int j = 0;
  // the array of strings for regular commands (or for the first process of pipe)
  char **ParsedNormal = calloc(128, sizeof(char*));

  char * mypath = getenv("MYPATH");
  if (mypath == NULL){
    mypath = calloc(7, sizeof(char));
    strcpy(mypath,"/bin:.");
  }

  // the array of strings for the 2nd process of pipe.
  char **ParsedPipe = calloc(128,sizeof(char*));

  // input string by user. No more than 1024 characters.
  char *input = calloc(1024,sizeof(char));

  // string of the current directory. Max 400 as for now.
  char * cwd = calloc(128, sizeof(char));

  // the array of all possible paths by $MYPATH
  char ** paths = calloc(128, sizeof(char*));

  // the path where command resides in.
  char * actual_path1 = calloc(128,sizeof(char));

  // the path where pipe's 2nd command resides in.
  char * actual_path2 = calloc(128,sizeof(char));

  // variable that helps us differentiate b/w regular, pipe, or none command.
  int flag = 0;


    while (1){

      int status;
      int chi;
  
    
      while((chi=waitpid(0,&status,WNOHANG)) > 0){
        if (WIFEXITED(status)){
          status = WEXITSTATUS(status);
        }
         printf("[process %d terminated with exit status %d]\n", chi, status);
      }
    
    

      input = fgets(input, 1024, stdin);

      // prints the current directory

      if (strlen(input) != 1)
          input = strtok(input, "\n");

      int space = count_spaces(input);
      getcwd(cwd, 100);
      printf("%s",cwd);
      printf("$ ");
      // print the input
      if (strcmp(input, "exit") == 0){
        printf("bye\n");
        break;
      }


       flag = processString(input,ParsedNormal,ParsedPipe, paths, actual_path1, actual_path2,  mypath, space, cwd);
       // flag = 0 if no such command found, user error.. etc.



       if (flag ==1){
        if (Is_this_foreground(input,ParsedNormal)){

        pid_t pid = fork();
          if (pid == -1) {
            perror("fork() failed\n");
            return EXIT_FAILURE;
          } else if (pid == 0) {
            execv(actual_path1, ParsedNormal);
             _exit(EXIT_FAILURE);
          } else {
        // waiting for child to terminate
            waitpid(pid,&status,0);
          }
        }



        else {

          printf("[running background process \"%s\"]\n",ParsedNormal[0]);
          pid_t pid = fork();
          
          if (pid == -1) {
            perror("fork() failed");
            return EXIT_FAILURE;
         } else if (pid == 0) {
            execv(actual_path1, ParsedNormal);
            _exit(EXIT_FAILURE);
          } else {
            waitpid(-1,&status,WNOHANG);
          }
        }
      }


// if pipe
       if (flag == 2){
        if((Is_this_foreground(input,ParsedPipe))){
            // 0 is read end, 1 is write end 
        int pipefd[2];  
        pipe(pipefd);
        pid_t p1;

       // first fork
       p1 = fork(); 

    if(p1>0){
      waitpid(p1,&status,0); 
      int p2 = fork();

      if (p2 == 0) { 
        // execute child
        close(pipefd[1]);
        dup2(pipefd[0], 0); 
        execv(actual_path2, ParsedPipe);
        _exit(EXIT_FAILURE);
    }

     else if(p2>0) { 
          close(pipefd[0]);
          close(pipefd[1]);
          waitpid(p2,&status,0);
      }
    }
    else if(p1 == 0){
      close(pipefd[0]);
      dup2(pipefd[1],1);
      execv(actual_path1,ParsedNormal);
      _exit(EXIT_FAILURE);
    }
    }


    else{
        int pipefd[2];  
        pipe(pipefd);
      int pipe_num = fork();
      if (pipe_num >0){
      printf("[running background process \"%s\"]\n", ParsedNormal[0]);
      int next = fork();
      if(next>0){
        printf("[running background process \"%s\"]\n", ParsedPipe[0]);
        int other_next = fork();
        if(other_next==0){
          execv(actual_path2, ParsedPipe);
          _exit(EXIT_FAILURE);
        }
      }
      else if(next==0){
        execv(actual_path1, ParsedNormal);
        _exit(EXIT_FAILURE);
      }
    }
    else if (pipe_num == 0){
        close(pipefd[0]);
        dup2(pipefd[1],1);
        execv(actual_path1,ParsedNormal);
        _exit(EXIT_FAILURE);
    }
    } 
  }

        


       for (j=0; j<128;j++){
         free(ParsedNormal[j]);
       }
       memset(ParsedNormal, '\0', 128);

       for (j=0; j<128;j++){
         free(ParsedPipe[j]);
       }
       memset(ParsedPipe, '\0', 128);

       for (j=0; j<128;j++){
         free(paths[j]);
       }
       memset(paths, '\0', 128);

       memset(cwd,'\0',128);
       memset(input,'\0',1024);
       memset(actual_path1,'\0',128);
       memset(actual_path2,'\0',128);

  }



    for (j=0; j<128;j++){
      free(ParsedNormal[j]);
    }
    free(ParsedNormal);

    for (j=0; j<128;j++){
      free(ParsedPipe[j]);
    }
    free(ParsedPipe);

     for (j=0; j<128;j++){
      free(paths[j]);
    }
    free(paths);

    free(actual_path1);
    free(actual_path2);
    free(cwd);
    free(input);
    if ((strcmp(mypath, "/bin:.")==0)){
    free(mypath);}




    return EXIT_SUCCESS;
  }

