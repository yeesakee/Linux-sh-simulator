/***** LAB3 base code *****/ 
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

char gpath[128];    // hold token strings  
char *name[64];     // token string pointers
int  n;             // number of token strings

char dpath[128];    // hold dir strings in PATH
char *dir[64];      // dir string pointers
int  ndir;          // number of dirs   

char *head, *tail;

int tokenize(char *pathname) // YOU have done this in LAB2
{                            // YOU better know how to apply it from now on
  char *s;
  strcpy(gpath, pathname);   // copy into global gpath[]
  s = strtok(gpath, " ");    
  n = 0;
  while(s){
    name[n++] = s;           // token string pointers   
    s = strtok(0, " ");
  }
  name[n] =0;                // name[n] = NULL pointer
}

void ioRedirection() {
  for(int i = 0; i < n; i++) {
    if (strcmp(name[i], "<") == 0) {
      name[i] = 0;
      int fd = open(name[i+1], O_RDONLY);
      dup2(fd, 0);
      close(fd);
    }
    else if (strcmp(name[i], ">") == 0) {
      name[i] = 0;
      int fd = open(name[i+1], O_WRONLY|O_CREAT, 0644);
      dup2(fd,1);
      close(fd);
    }
    else if (strcmp(name[i], ">>") == 0) {
      name[i] = 0;
      int fd = open(name[i+1], O_WRONLY|O_CREAT|O_APPEND);
      dup2(fd,1);
      close(fd);
    }
  }
}

int scanner(char *cmdLine) {
  if (strchr(cmdLine, '|') == NULL) {
    return 0;
  }
  for (int i = strlen(cmdLine)-1; i >= 0; i--) {
    if (cmdLine[i] == '|') {
      cmdLine[i] = 0;
      head = cmdLine;
      tail = cmdLine + i + 1;
      return 1;
    }
  }
  return 1;
}

void do_command(char* cmd) {
  char line[256];
  char temp[256];
  tokenize(cmd);
  ioRedirection();
  int r;
  char* s;
  strncpy(temp, cmd, sizeof(temp));
  s = strtok(temp, " ");
  for (int i = 0; i < ndir; i++) {
    if (i == 0) {
      getcwd(line, 256);
    }
    else {
      strcpy(line, dir[i]);
    }
    strcat(line, "/");
    strcat(line, s);
    r = execvp(line, name);
  }
}

void do_pipe(char* command, int* pd) {
  int pid;
  int lpd[2];
  int hasPipe = scanner(command);
  printf("\nhead: %s\n", head);
  printf("tail: %s\n", tail);
  printf("command: %s\n", command);

  if (pd) {
    close(pd[0]); dup2(pd[1], 1); close(pd[1]);
  }

  if (hasPipe == 1) {
    pipe(lpd);
    pid = fork();
    if (pid) {
      close(lpd[1]);
      dup2(lpd[0], 0);
      close(lpd[0]);
      do_command(tail);
    }
    else {
      do_pipe(head, lpd);
    }
  }
  else {
    do_command(command);
  }
}

int main(int argc, char *argv[ ], char *env[ ])
{
  int i;
  int pid, status;
  char *cmd;
  char line[128];
  // The base code assume only ONE dir[0] -> "./"
  // YOU do the general case of many dirs from PATH !!!!
  dir[0] = "./";
  ndir   = 1;
  int size = 0;
  char command[256];
  while (env[size]) {
    char* p;
    char temp[99999];
    strcpy(temp, env[size]);
    p = strtok(temp, "=");
    if (p != NULL && strcmp(p, "PATH") == 0) {
      break;
    }
    size++;
  }
  strcpy(dpath, env[size]);
  char temp[128];
  strcpy(temp, dpath);
  char *s;
  s = strtok(temp, "=");
  s = strtok(0, ":");
  while (s) {
    dir[ndir++] = s;
    s = strtok(0, ":");
  }
  dir[ndir] = 0;
  // show dirs
  for(i=0; i<ndir; i++)
    printf("dir[%d] = %s\n", i, dir[i]);
  
  while(1) {
    printf("sh %d running\n", getpid());
    printf("enter a command line : ");
    fgets(line, 128, stdin);
    line[strlen(line) - 1] = 0; 
    if (line[0]==0)
      continue;
    strcpy(command, line);
    tokenize(line);
    for (i=0; i<n; i++){  // show token strings   
        printf("name[%d] = %s\n", i, name[i]);
    }

    //    printf("enter a key to continue : "); getchar();
    
    cmd = name[0];         // line = name0 name1 name2 ....

    if (strcmp(cmd, "cd")==0){
      chdir(name[1]);
      continue;
    }
    if (strcmp(cmd, "exit")==0)
      exit(0); 
    
    pid = fork();
     

    if (pid){
      printf("sh %d forked a child sh %d\n", getpid(), pid);
      printf("sh %d wait for child sh %d to terminate\n", getpid(), pid);
      pid = wait(&status);
      printf("\nZOMBIE child=%d exitStatus=%x\n", pid, status); 
      printf("main sh %d repeat loop\n", getpid());
    }
    else{
      printf("child sh %d running\n", getpid());
      int r;
      // ioRedirection();
      // for (int i = 0; i < ndir; i++) {
      //   strcpy(line, dir[i]); strcat(line, "/");strcat(line, cmd);
      //   printf("%s\n", line);
      //   r = execvp(line, name);
      // }
      //do_command(command);
      do_pipe(command, 0);
       exit(1);
    }
  }
}

/********************* YOU DO ***********************
1. I/O redirections:

Example: line = arg0 arg1 ... > argn-1

  check each arg[i]:
  if arg[i] = ">" {
     arg[i] = 0; // null terminated arg[ ] array 
     // do output redirection to arg[i+1] as in Page 131 of BOOK
  }
  Then execve() to change image


2. Pipes:

Single pipe   : cmd1 | cmd2 :  Chapter 3.10.3, 3.11.2

Multiple pipes: Chapter 3.11.2
****************************************************/
