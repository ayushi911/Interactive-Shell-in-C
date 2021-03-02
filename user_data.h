#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include<sys/wait.h>
#include <ctype.h> 
#include<time.h>

#ifndef USER_DATA_H
#define USER_DATA_H

// A linked list node 
struct Node { 
    char data[1024];
    int pid; 
    struct Node* next; 
};

//-----------------------------------------PARSE----------------------------
void parse(char *line, char **args){
    if (strcmp(line, "exit\n") == 0)
            exit(EXIT_SUCCESS);
        char **next = args;
        char *temp = strtok(line, " \n");
        while (temp != NULL)
        {
            *next++ = temp;
            temp = strtok(NULL, " \n");
        }
        *next = NULL;
        for (next = args; *next != 0; next++)
            puts(*next);
}
//----------------------------------------APPEND-------------------------------
void append(struct Node** head, char *new_data,int child_pid) 
{
	struct Node* new_node = (struct Node*) malloc(sizeof(struct Node)); 
	struct Node *last = *head;
  strcpy(new_node-> data , new_data);  
  new_node->pid=child_pid;
	new_node->next = NULL; 
	if (*head == NULL) 
	{ 
	*head = new_node; 
	return; 
	} 
	while (last->next != NULL) 
		last = last->next; 
	last->next = new_node; 
	return;	 
}
//---------------------------------------checking numbers----------------------------------
int digits_only(char *s)
{
    for (int i = 0; i < strlen(s); i++) {
        if ((s[i]<'0' || s[i]>'9')) return 0;
    }

    return 1;
}

//-----------------------------------------Number of nodes------------------------------------
int getCount(struct Node* head) 
{ 
    // Base case 
    if (head == NULL) 
        return 0; 
  
    // count is 1 + count of remaining list 
    return 1 + getCount(head->next); 
} 

//--------------------------------------PRINT-----------------------------------
void printdata(struct Node** head_ref,char* value) {
  struct Node* ptr = *head_ref;
  ///// pid all
  if (strcmp(value,"FULL")==0){
  printf("List of processes with PIDs spawned from this shell:(If no following output then NO Background Process!)\n");
  while(ptr) {
      printf("command name: %s  process id: %d\n",ptr->data,ptr->pid);
      ptr = ptr->next;
  }
  }
  else if (digits_only(value)==1){
    int num=atoi(value);
    int last=0; 
    int count=1;
    int com=getCount(ptr);
    while(ptr) {
      last=getCount(ptr);
      if(last<=num){
         while(ptr) {
      printf("%d %s\n",count,ptr->data);
      count=count+1;
      ptr = ptr->next;
                  } 
          break;
      }
      ptr = ptr->next;

  }
    if (last!=num){
        printf("Only %d commands were executed previously.\n",com);
    }

    }
}

//-------------------------------------FREELIST------------------------------------------------

void freeList(struct Node** head_ref)
{
   struct Node* current = *head_ref;
   struct Node* next;
 
   while (current != NULL) 
   {
       next = current->next;
       free(current);
       current = next;
   }
   *head_ref = NULL;
}

#endif