#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pwd.h>
#include <termios.h>
#include "user_data.h"

#ifndef UTIL_H
#define UTIL_H

#define TRUE 1
#define FALSE !TRUE

// Shell pid, pgid, terminal modes
static pid_t GBSH_PID;
static pid_t GBSH_PGID;
static int GBSH_IS_INTERACTIVE;
static struct termios GBSH_TMODES;

static char* currentDirectory;
extern char** environ;

struct sigaction act_child;
struct sigaction act_int;

int no_reprint_prmpt;

pid_t pid;

#define LIMIT 256 // max number of tokens for a command
#define MAXLINE 1024 // max number of characters from user input



//Displays the prompt for the shell
void shellPrompt()
{
char host[100];
	strcpy(host,"ayushi@iitjammu");
	char cwd[1024];
	char hostn[1204] = "";
	gethostname(hostn, sizeof(hostn));
	getcwd(cwd, sizeof(cwd));
	if (strncmp(cwd,hostn,strlen(hostn))==0){
		char *b = cwd +strlen(hostn);
		printf("<%s:~%s>",host,b);
	}
	else{
		printf("<%s:%s> ",host,cwd);
	}	
}

//Method to change directory (the shell command chdir or cd)
int changeDirectory(char* args[]){
	// If no path written (only 'cd'), then go to the home directory
	if (args[1] == NULL) {
		chdir(getenv("HOME")); 
		return 1;
	}
	// Else change the directory to the one specified by the argument, if possible
	else{ 
		if (chdir(args[1]) == -1) {
			printf(" %s: no such directory\n", args[1]);
            return -1;
		}
	}
	return 0;
}

//------------------------------------Welcome Screen----------------------------------------
void welcomeScreen(){
        printf("\n\t============================================\n");
        printf("\t               Interactive Shell\n");
        printf("\t============================================\n");
        printf("\n\n");
}

/*
 LAUNCHING THE PROGRAM
*/ 
void launchProg(struct Node** head,struct Node** current,char **args, int background){	 
	 int err = -1;
	 int a=0;
	char line[1024]="";
	
	if(background==1){
	char token[256];
	strncpy(args[0],args[0]+1,strlen(args[0]));
	}
	
	while (args[a]){
		strcat(line,args[a]);
		strcat(line," ");
		a++;
	}
	 if((pid=fork())==-1){
		 printf("Child process could not be created\n");
		 return;
	 }
	if(pid==0){
		// set the child to ignore SIGINT signals
		signal(SIGINT, SIG_IGN);
		
		setenv("parent",getcwd(currentDirectory, 1024),1);	
		
		if (execvp(args[0],args)==err){
			printf("Command not found!!\n");
			//kill(getpid(),SIGTERM);
		}
	 }
	 else{
	 
	 if (background == 0){
		 waitpid(pid,NULL,0);
		 append(head,line,pid);
	 }else{
		 append(current,line,pid);
		 append(head,line,pid);
		 //printf("Process created with PID: %d\n",pid);
	 }}	 
}
 
//Method used to manage pipes.
void pipeHandler(char * args[]){
	// File descriptors
	int filedes[2]; // pos. 0 output, pos. 1 input of the pipe
	int filedes2[2];
	
	int num_cmds = 0;
	
	char *command[256];
	
	pid_t pid;
	
	int err = -1;
	int end = 0;
	
	// Variables used for the different loops
	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;
	
	//calculate the number of commands 
	while (args[l] != NULL){
		if (strcmp(args[l],"|") == 0){
			num_cmds++;
		}
		l++;
	}
	num_cmds++;

	while (args[j] != NULL && end != 1){
		k = 0;

		while (strcmp(args[j],"|") != 0){
			command[k] = args[j];
			j++;	
			if (args[j] == NULL){
				end = 1;
				k++;
				break;
			}
			k++;
		}

		command[k] = NULL;
		j++;		
		if (i % 2 != 0){
			pipe(filedes); // for odd i
		}else{
			pipe(filedes2); // for even i
		}
		
		pid=fork();
		
		if(pid==-1){			
			if (i != num_cmds - 1){
				if (i % 2 != 0){
					close(filedes[1]); // for odd i
				}else{
					close(filedes2[1]); // for even i
				} 
			}			
			printf("Child process could not be created\n");
			return;
		}
		if(pid==0){
			if (i == 0){
				dup2(filedes2[1], STDOUT_FILENO);
			}
			else if (i == num_cmds - 1){
				if (num_cmds % 2 != 0){ // for odd number of commands
					dup2(filedes[0],STDIN_FILENO);
				}else{ // for even number of commands
					dup2(filedes2[0],STDIN_FILENO);
				}
			}else{ // for odd i
				if (i % 2 != 0){
					dup2(filedes2[0],STDIN_FILENO); 
					dup2(filedes[1],STDOUT_FILENO);
				}else{ // for even i
					dup2(filedes[0],STDIN_FILENO); 
					dup2(filedes2[1],STDOUT_FILENO);					
				} 
			}
			
			if (execvp(command[0],command)==err){
				kill(getpid(),SIGTERM);
			}		
		}
				
		// CLOSING DESCRIPTORS ON PARENT
		if (i == 0){
			close(filedes2[1]);
		}
		else if (i == num_cmds - 1){
			if (num_cmds % 2 != 0){					
				close(filedes[0]);
			}else{					
				close(filedes2[0]);
			}
		}else{
			if (i % 2 != 0){					
				close(filedes2[0]);
				close(filedes[1]);
			}else{					
				close(filedes[0]);
				close(filedes2[1]);
			}
		}
				
		waitpid(pid,NULL,0);
				
		i++;	
	}
}
#endif
