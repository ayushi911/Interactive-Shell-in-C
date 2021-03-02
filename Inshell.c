#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>
#include "util.h"
#include "user_data.h"

struct Node* head = NULL; 
struct Node* current = NULL; 		
/*
 STANDARD INPUT HANDLER
*/ 
int commandHandler(char * args[]){
	int i = 0,j = 0;
	int fileDescriptor,standardOut, aux;
	int background = 0;
	char line[1024]="";
	int a=0;
	int child_pid;
	while (args[a]){
		strcat(line,args[a]);
		strcat(line," ");
		a++;
	}
	char *args_aux[256];
	
	while ( args[j] != NULL){
		if ( (strcmp(args[j],">") == 0) || (strcmp(args[j],"<") == 0) || (strcmp(args[j],"&") == 0)){
			break;
		}
		args_aux[j] = args[j];
		j++;
	}
	// 'exit' command quits the shell
	if(strcmp(args[0],"stop") == 0) {
		//append(&head, "exit");
		freeList(&head);
		printf("Freed memory successfully!!\n");
		printf("Exiting Successfully!!\n");
		exit(0);
		}
	else if (strcmp("pid",args[0])==0 && args[1]){
		if (strcmp("all",args[1])==0){
		printdata(&head,"FULL");
        }
		else if(strcmp("current",args[1])==0){
		printdata(&current,"FULL");
        }
		append(&head,line,getpid()); 
	  }
	else if (strcmp("pid",args[0])==0 ){
		int child_pid;
		printf("command name: ./086_lab5  process id: %d\n",getpid());
        child_pid=getpid();
		append(&head,line,getpid());
        return child_pid;   
	  }
	// 'pwd' command prints the current directory
 	else if (strcmp(args[0],"pwd") == 0){
		 append(&head, "pwd",getpid());
		if (args[j] != NULL){
			if ( (strcmp(args[j],">") == 0) && (args[j+1] != NULL) ){
				fileDescriptor = open(args[j+1], O_CREAT | O_TRUNC | O_WRONLY, 0600); 
				standardOut = dup(STDOUT_FILENO); 	
				dup2(fileDescriptor, STDOUT_FILENO); 
				close(fileDescriptor);
				printf("%s\n", getcwd(currentDirectory, 1024));
				dup2(standardOut, STDOUT_FILENO);
			}
		}else{
			printf("%s\n", getcwd(currentDirectory, 1024));
		}
	} 
	//-------------------------!HISTN----------------------------------
	else if (strncmp(args[0],"!HIST",5)==0){
		strncpy(args[0],args[0]+5,strlen(args[0]));
		int num=atoi(args[0]);
		struct Node *temp=head;
		int count=1;
		while(temp){
			if (count==num){
			char new_args[MAXLINE]; // buffer for the user input
			char * tokens[LIMIT];
			memset ( new_args, '\0', MAXLINE );
			strcpy(new_args,temp->data);
			if((tokens[0] = strtok(new_args," \n\t")) == NULL) continue;
			int numTokens = 1;
			while((tokens[numTokens] = strtok(NULL, " \n\t")) != NULL) numTokens++;
				commandHandler(tokens);
				break;
			}
			count++;
		temp=temp->next;

		}

	}
	    //----------------------------------HISTN------------------------------------------
    else if (strncmp("HIST",args[0],4)==0){
          char num_check[1024];
          strncpy(num_check,args[0]+4,strlen(args[0]));
          if (digits_only(num_check)==1 && strcmp(num_check,"")!=0){
              printdata(&head,num_check);   
          }
          else{
              printf("Did you mean HISTN?\nHIST is to be used with number n, where the n commands exected previously will be shown\n");
          }
          child_pid=getpid();
            append(&head,line,child_pid);
            return child_pid;

      }
 	// 'clear' command clears the screen
	else if (strcmp(args[0],"clear") == 0) {
		//append(&head, "clear");
		system("clear");
	}
	// 'cd' command to change directory
	else if (strcmp(args[0],"cd") == 0) {
	   append(&head, line ,getpid());
	changeDirectory(args);
	}
	
	else{
		while (args[i] != NULL && background == 0){
			if (strncmp(args[i],"&",1) == 0){
				background = 1;
			}
			else if (strcmp(args[i],"|") == 0){
				pipeHandler(args);
				return 1;
			}
			i++;
		}
		args_aux[i] = NULL;
		launchProg(&head,&current,args_aux,background);
	}
return 1;
}
//delete node
void deleteNode(struct Node** head_ref, int key)
{
    // Store head node
    struct Node *temp = *head_ref, *prev;
 
    // If head node itself holds the key to be deleted
    if (temp != NULL && temp->pid == key) {
        *head_ref = temp->next; // Changed head
        free(temp); // free old head
        return;
    }
 
    // Search for the key to be deleted, keep track of the
    // previous node as we need to change 'prev->next'
    while (temp != NULL && temp->pid != key) {
        prev = temp;
        temp = temp->next;
    }
 
    // If key was not present in linked list
    if (temp == NULL)
        return;
 
    // Unlink the node from linked list
    prev->next = temp->next;
 
    free(temp); // Free memory
}
 
/*SIGNAL HANDLERS
  signal handler for SIGCHLD*/
void signalHandler_child(int p){
	int child;
	int status;
	struct Node *check = current;
	child=waitpid(-1, &status, WNOHANG);
	//printf("%d",child);
	while(check){
		if (check->pid==child){
		printf("The command %s with process id %d is terinated\n",check->data,check->pid);
		deleteNode(&current,child);
		break;	
		}
		check=check->next;
	}	
}

//Signal handler for SIGINT
void signalHandler_int(int p){
	// Sending a SIGTERM signal to the child process
	if (kill(pid,SIGTERM) == 0){
		printf("\nProcess %d received a SIGINT signal\n",pid);
		no_reprint_prmpt = 1;			
	}else{
		printf("\n");
	}
}
void init(){
        GBSH_PID = getpid();  
        GBSH_IS_INTERACTIVE = isatty(STDIN_FILENO);  

		if (GBSH_IS_INTERACTIVE) {
			// Loop until in the foreground
			while (tcgetpgrp(STDIN_FILENO) != (GBSH_PGID = getpgrp()))
					kill(GBSH_PID, SIGTTIN);             
	              
	              
	        // Set the signal handlers for SIGCHILD and SIGINT
			act_child.sa_handler = signalHandler_child;
			act_int.sa_handler = signalHandler_int;			
			sigaction(SIGCHLD, &act_child, 0);
			sigaction(SIGINT, &act_int, 0);
			
			// Put in process group
			setpgid(GBSH_PID, GBSH_PID); 
			GBSH_PGID = getpgrp();
			if (GBSH_PID != GBSH_PGID) {
					printf("Error, the shell is not process group leader");
					exit(EXIT_FAILURE);
			}
			// Grab control of the terminal
			tcsetpgrp(STDIN_FILENO, GBSH_PGID);  
			
			// Save default terminal attributes for shell
			tcgetattr(STDIN_FILENO, &GBSH_TMODES);

			// Get the current directory that will be used in different methods
			currentDirectory = (char*) calloc(1024, sizeof(char));
        } else {
                printf("Could not make the shell interactive.\n");
                exit(EXIT_FAILURE);
        }
}
//--------------------------------------Main method-------------------------------------------- 
int main(int argc, char *argv[], char ** envp) {
	//struct Node* head = NULL; 
	char line[MAXLINE]; // buffer for the user input
	char * tokens[LIMIT]; // array for the different tokens in the command
	int numTokens;
		
	no_reprint_prmpt = 0; 	// to prevent the printing of the shell
							// after certain methods
	pid = -10;
	

	init();
	welcomeScreen();
	environ = envp;

	setenv("shell",getcwd(currentDirectory, 1024),1);

	while(TRUE){

		if (no_reprint_prmpt == 0) shellPrompt();
		no_reprint_prmpt = 0;
		
		memset ( line, '\0', MAXLINE );

		fgets(line, MAXLINE, stdin);
	
		if((tokens[0] = strtok(line," \n\t")) == NULL) continue;
		
		numTokens = 1;
		while((tokens[numTokens] = strtok(NULL, " \n\t")) != NULL) numTokens++;
		
		commandHandler(tokens);
		
	}          

	exit(0);
}