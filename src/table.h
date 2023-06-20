#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> /* O_RDONLY, O_WRONLY, O_CREAT, O_* */


#include "message.h"
#include "util.h"

#define MAX_TABLE 256


typedef struct hash_table {
    Message *table[MAX_TABLE];
} HashTable;

int hash_function(pid_t key) {
    return key % MAX_TABLE;
}

void insert_message(HashTable* ht, Message msg) {
    int index = hash_function(msg.pid);
    Message* newMessage = cloneMessage(&msg);
    if (newMessage == NULL) {
        // Handle memory allocation failure
        return;
    }

    Message* temp = ht->table[index];
    if (temp == NULL) {
        ht->table[index] = newMessage;
    } else {
        // Collision occurred, add new message to the front of the linked list
        newMessage->next = temp;
        ht->table[index] = newMessage;
    }
}

Message get_message(HashTable* ht, pid_t key) {
    int index = hash_function(key);
    Message* temp = ht->table[index];
    
    while (temp != NULL) {
        if (temp->pid == key) {
            // Found the matching message
            return *temp;
        }
        temp = temp->next;
    }
    Message m;
    m.pid =0;
    return m;
}
void end_message(HashTable* ht, Message m) {
    pid_t key = m.pid;
    int index = hash_function(key);
    
    Message* temp = ht->table[index];
    while (temp != NULL) {
        if (temp->pid == key) {
            temp->state = m.state;
            return;
        }
        temp = temp->next;
    }
}


void check_and_reset_table(HashTable* ht) {
    for (int i = 0; i < MAX_TABLE; i++) {
        int found_start = 0;
        Message* current = ht->table[i];

        while (current != NULL) {
            if (current->state == Start) {
                found_start = 1;
                break;
            }
            current = current->next;
        }

        if (found_start) {
            return;
        }
    }

    // No "Start" state found, reset the hash table
    for (int i = 0; i < MAX_TABLE; i++) {
        Message* current = ht->table[i];
        while (current != NULL) {
            Message* temp = current;
            current = current->next;
            free(temp);
        }
        ht->table[i] = NULL;
    }
}


void print_table(HashTable* ht, int fd) {
    for (int i = 0; i < MAX_TABLE; i++) {
        Message* current = ht->table[i];
        while (current != NULL) {
            if (current->state == Start) {
                write(fd, current, sizeof(Message));
            }
            current = current->next;
        }
    }
}



void time_table_list (HashTable *ht,char* list,int fd) {

    int malloc_size = nr_space(list)+1;
    char *arguments;
    double total = 0.0;
    for (int i = 0; i < malloc_size; i++){

        arguments = strdup(strsep(&list," "));
        pid_t pidKey = atoi(arguments);
        Message temp = get_message(ht,pidKey);
        if (temp.pid ==0){
            continue;
        }
        total += temp.timeStamp;

    }
    write(fd,&total,sizeof(double));   
}
int isSubstring(char* a,char* b) {
    int len_a = strlen(a);
    int len_b = strlen(b);
   
    if (strcmp(a,b) == 0){
        return 1;
    }
      

    if (len_a > len_b)
        return 0;

    for (int i = 0; i <= len_b - len_a; i++) {
        int j;
        for (j = 0; j < len_a; j++) {
            if (b[i + j] != a[j])
                break;
        }
        if (j == len_a)
            return 1;
    }

    return 0;
}
int showManyTimes(const char* substring, const char* string) {
    int count = 0;
    int substring_len = strlen(substring);
    int string_len = strlen(string);

    for (int i = 0; i <= string_len - substring_len; i++) {
        int j;
        for (j = 0; j < substring_len; j++) {
            if (string[i + j] != substring[j])
                break;
        }
        if (j == substring_len)
            count++;
    }

    return count;
}
void command_table_list (HashTable *ht,char* list,int fd) {

    char *arguments;
    char *progName = strdup(strsep(&list,","));
    int malloc_size = nr_space(list)+1;
    trimString(progName);
    int total = 0;
    for (int i = 0; i < malloc_size; i++){
        
        arguments = strdup(strsep(&list," "));
        pid_t pidKey = atoi(arguments);
        Message temp = get_message(ht,pidKey);
        if (temp.pid ==0){
            continue;
        }
        

        if (isSubstring(progName,temp.progName)==1){
            
            total +=showManyTimes(progName,temp.progName);
            
        }
           

    }
    write(fd,&total,sizeof(int));    
}



int checkIfMember(Message*list,Message*check){

    Message *current = NULL;
    current = list;
    while (current){
        
        if (isSubstring(check->progName,current->progName) == 1)
            return 1;

        current = current->next;
    }

return 0;
}

void uniq_table_list (HashTable *ht,char* list,int fd) {
    char *arguments;
    int size= nr_space(list)+1;
    Message *writeList = NULL;

    for (int i=0; i<size;i++){

        arguments = strdup(strsep(&list," "));
        pid_t pidKey = atoi(arguments);
        Message temp = get_message(ht,pidKey);
        if (temp.pid ==0){
            continue;
        }
        // ver a quantidade de clones e comparar todos.
        int n_clones = nr_pro(temp.progName);
        Message *Programlist = NULL;
        int av = 0;
        for ( int i = 0 ; i < n_clones;i++){
        
        Message *clone = cloneMessage(&temp);
        char *temp_string= strtok(clone->progName+av,"|");
        strcpy(clone->progName,temp_string);
        
        av += 1 + strlen(clone->progName);
        trimString(clone->progName);
        *clone = alterArg(*clone);
        addToList(&Programlist,clone);
       

        }
        Message * current = NULL;
            current = Programlist;
            while (current!=NULL) {
        
                char *temp_progName= strtok(current->progName," ");
                strcpy(current->progName, temp_progName);
            
            current = current->next;
            }
        if ( writeList == NULL){
            writeList = Programlist;
        }else{
            Message * current = NULL;
            current = Programlist;
            while (current!=NULL) {

               if (checkIfMember(writeList,current) == 0){

                Message *clone_in =NULL;
                clone_in = cloneMessage(current);
                clone_in->next = NULL;
                addToList(&writeList,clone_in);

               }

            current = current->next;
            }
        }
        
    }
    while (writeList){
          
            write(fd,writeList,sizeof(struct message));
            writeList = writeList->next;
        }
    

}