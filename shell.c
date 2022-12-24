/* 
Basic Shell coded is C language and has Built-in commands pwd,clear,pwd,exit,history(with up,down arrow key function)
It handles input output redirection and pipes.
It runs multiple commands on single line separated by semi colons.
It maintain command history and accessing them using !n command.
It also creates new environment variables.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include<readline/readline.h>
#include<readline/history.h>

#define MAXLEN 512
#define MAXARGS 10
#define ARGLEN 30

int execute(char* arglist[],char* cmdline, int background);
char** tokenize(char* cmdline);
char* readcmd(char*, FILE*);
void redirection(char** temp1,int size, int r);
void parsing(char** arglist,char *line, const char* sym, int* size);
void removeSpace(char* cmdline);
void handlePipes(char** temp, int size);
char* storeInarray();
void writeHelp();

int main(){
   char *cmdline;
   char s[100], cwd[100];
   int size=0;
   int i=0,background=0;
   char** arglist;
   char** temp;
   char* prompt = strcat(getcwd(s,100),":-"); 
   char hostname[200];
   hostname[199] = '\0';  //0-->199
   gethostname(hostname, 200);
   strcat(hostname,":");
   strcat(hostname,prompt);
   read_history("history.txt"); //add the contents of FILENAME to the history list
   for(;;){
      cmdline = readline(hostname);
      add_history(cmdline); // place string at the end of the history list
      i++;
      if(strrchr(cmdline,'&'))  background=1; //check last occurence
      if(strchr(cmdline,'=')){
         putenv(cmdline);
      }
      if(strchr(cmdline,'>')){
         parsing(arglist,cmdline,">",&size);
         if(size == 2) // to check correct number of cmd arguments
            redirection(arglist,size,1);
         else{
            printf("Incorrect\n");
            exit(1);
         }
      }
      else if(strchr(cmdline,'<')){
         parsing(arglist,cmdline,"<",&size);
         if(size == 2)
            redirection(arglist,size,0);
         else{
            printf("Incorrect\n");
            exit(1);
         }
      }
      else if(strstr(cmdline,">>")){
         parsing(arglist,cmdline,">>",&size);
         if(size == 2) // to check correct number of cmd arguments
            redirection(arglist,size,2);
         else{
            printf("Incorrect\n");
            exit(1);
         }
      }
      else if(strchr(cmdline,'|')){
         parsing(arglist,cmdline,"|",&size);
         handlePipes(arglist,size);
      }
      else if((arglist = tokenize(cmdline)) != NULL){
         if (strcmp (cmdline, "history") == 0){
            // A structure used to pass around the current state of the history.

         HISTORY_STATE *state = history_get_history_state(); //initializing history and state management.
         HIST_ENTRY **list; // array of history entries (list,timestamp, data)

         register int i;  //store in cpu registers instead of memory (faster accessability)
    
         list = history_list (); // info about istory list
         if (list){
            for (i = 0; list[i]; i++)
               fprintf (stderr, "%d %s\r\n", i+1, list[i]->line);
           }
        }
         if(!strcmp(arglist[0],"cd")){ 
            chdir(arglist[1]);
            char host[200];
            host[199] = '\0';
            gethostname(host, 200);
            prompt = strcat(getcwd(s,100),":-");
            strcat(host,prompt);
            strcpy(hostname,host); // copy prompt in hostname
         }
         else if(!strcmp(arglist[0],"exit")){
            write_history("history.txt");
            exit(0);
         }
         else if(!strcmp(arglist[0],"pwd")){
            printf("%s\n", getcwd(cwd, 100));
         }
         else if(!strcmp(arglist[0],"clear")){
            system("clear");
         }
         else if(!strcmp(arglist[0],"help")){
            writeHelp();
         }
         else if(!strcmp(arglist[0],"echo")){
            parsing(arglist,cmdline,"$",&size);
            const char* var = getenv(arglist[1]);
            printf("%s\n",var);
         }
         else{
            execute(arglist,cmdline,background);
      for(int j=0; j < MAXARGS+1; j++){
            free(arglist[j]);
         }
         free(arglist);
         free(cmdline);
      }
  }
}
   printf("\n");
   return 0;
}
int execute(char* arglist[],char* cmdline, int background){
    if(strchr(cmdline,';')){
      int i=0;
      char s[100];
      int size=0;
      char **temp;
      parsing(arglist,cmdline,";",&size);
      while(i< size){
         system(arglist[i]);
         i++;
      }
   }
   if(strchr(arglist[0],'!')){
      char *c;
      int i=1,j=1,size=0;
      char** temp, **copytemp;
      c = strtok(arglist[0],"!");
      char *t = storeInarray(c);
      temp = tokenize(t);
      parsing(temp,t," ",&size);
      system(temp[0]);
      for(int j=0; j < MAXARGS+1; j++){
            free(temp[j]);
         }
         free(temp);
   }
   else{
      int status;
      int cpid = fork();
         if(cpid == -1){
            perror("fork failed");
            exit(1);
         }
         else if(cpid == 0){
            execvp(arglist[0], arglist);
            perror("Command not found:");
            exit(1);
         }
         if(background == 0){
            waitpid(cpid, &status, 0);
         }
         else if(background == 1){
            printf("[1] %d\n",cpid);
         }
   }
}
// This function tokenize user input and remove whitespaces and tabs.
char** tokenize(char* cmdline){
   char** arglist = (char**)malloc(sizeof(char*)* (MAXARGS+1));
   for(int j=0; j < MAXARGS+1; j++){
      arglist[j] = (char*)malloc(sizeof(char)* ARGLEN);
      bzero(arglist[j],ARGLEN);
    }
   if(cmdline[0] == '\0')
      return NULL;
   int argnum = 0; 
   char*cp = cmdline; 
   char*start;
   int len;
   while(*cp != '\0'){
      while(*cp == ' ' || *cp == '\t')  // check white space and tab
          cp++;
      start = cp; 
      len = 1;
      while(*++cp != '\0' && !(*cp ==' ' || *cp == '\t'))
         len++;
      strncpy(arglist[argnum], start, len);
      arglist[argnum][len] = '\0';
      argnum++;
   }
   arglist[argnum] = NULL;
   return arglist;
}      
// It reads command and add it in the char array
char* readcmd(char* prompt, FILE* fp){
   printf("%s", prompt);
  int c;
   int pos = 0;
   char* cmdline = (char*) malloc(sizeof(char)*MAXLEN);
   while((c = getc(fp)) != EOF){
       if(c == '\n')
     break;
       cmdline[pos++] = c;
   }
   if(c == EOF && pos == 0) 
      return NULL;
   cmdline[pos] = '\0';
   return cmdline;
}
// It pasre the input
void parsing(char** arglist,char *line, const char* sym, int* size){
   char* t;
   int i=0;
   t = strtok(line, sym);
   int argnum= -1;
   while(t){
      arglist[++argnum] = malloc(sizeof(t)+1);
      strcpy(arglist[argnum],t);
      removeSpace(arglist[argnum]);
      t = strtok(NULL, sym);
   }
   arglist[++argnum] = NULL;
   *size = argnum;
}
// To handel redirection of user input
void redirection(char** temp,int size, int r){
   int argnum, ptr;
   char *arglist[100];
   removeSpace(temp[1]);
   parsing(arglist,temp[0]," ",&argnum);
   int status;
   int cpid = fork();
   switch(cpid){
      case -1:
         perror("fork failed");
         exit(1);
      case 0:{
         switch(r){
            case 0:
               ptr = open(temp[1], O_RDONLY, 0600);
               break;
            case 1:
               ptr = open(temp[1], O_CREAT|O_TRUNC|O_WRONLY, 0600);
               break;
           case 2:
               ptr = open(temp[1], O_WRONLY|O_APPEND, 0600);
               break;
            default:
               return;
         }
         if(ptr < 0){
            perror("Can't open file\n");
            return;
         }
          switch(r){
            case 0:
               dup2(ptr, 0);
               break;
            case 1:
               dup2(ptr, 1);
               break;
            case 2:
               dup2(ptr, 1);
               break;
            default:
               return;
      }
      execvp(arglist[0], arglist); // to run the command 
      perror("Invalid Input");
      exit(1);
   }
      default:
         waitpid(cpid, &status, 0);
         // printf("child exited with status %d \n", status >> 8);
         return;
   } 
}
void removeSpace(char* cmdline){
   if(cmdline[0] == '\0')
      return;
   char*cp = cmdline; 
   int len = strlen(cmdline) -1;
   if(cp[len] == ' ' || cp[len] == '\n')
      cp[len] = '\0';
   if(cp[0] == ' ' || cp[0] == '\n')
      strncpy(cp, cp+1, len+1);
}
void handlePipes(char** temp, int size){
   if(size>5) return;
   
   int fd[5][2],argnum,n;
   char *arglist[100];

   for(argnum=0;argnum<size;argnum++){
      parsing(arglist,temp[argnum]," ",&n);

      if(argnum!=size-1){ 
         if(pipe(fd[argnum])<0){
            perror("Error creating pipe\n");
            return;
         }
      }
      if(fork()==0){
         if(argnum!=size-1){
            dup2(fd[argnum][1],1); //duplicationg o/p fd of 1st pipe
            close(fd[argnum][0]); //close reading end of first pipe
            close(fd[argnum][1]);
         }

         if(argnum!=0){
            dup2(fd[argnum-1][0],0); 
            close(fd[argnum-1][1]); //close writing end of second pipe
            close(fd[argnum-1][0]);
         }
         execvp(arglist[0],arglist);
         perror("Command not found");
         exit(1);
      }
      //parent
      if(argnum!=0){//second process
         close(fd[argnum-1][0]);
         close(fd[argnum-1][1]);
      }
      wait(NULL);
   }
}
// store the history in an array 
char* storeInarray(char* n){
int MAX_LINES = 100;
int MAX_LEN = 1000;
  char data[MAX_LINES][MAX_LEN];
  FILE *file;
  file = fopen("history.txt", "r");
  if (file == NULL)
  {
    printf("Error opening file.\n");
    exit(1);
  }
  int line = 0;
  while (!feof(file) && !ferror(file))
    if (fgets(data[line], MAX_LEN, file) != NULL)
      line++;
  fclose(file);
  int num = atoi(n);
  char *t = data[num-1];
  return t;
}
void writeHelp(){
   puts("\n***********Welcome to My Shell**************"
      "\n********Copyright (C) Khadija Rauf*********"
      "\n\tBuilt-in Commands supported by my Shell are :"
         "\n\tcd"
         "\n\thelp"
         "\n\tpwd"
         "\n\texit"
         "\n\techo"
         "\n\tclear"
         "\n\thistory"
         "\n\tTab completion"
         "\n\t Up and down arrow  function"
         "\n\tSome basic commands"
         "\n\tRedirection"
         "\n\tPipe handling"
         "\n\tUse the man command for information on other programs\n");
}