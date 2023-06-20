#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> /* O_RDONLY, O_WRONLY, O_CREAT, O_* */



#include "table.h"

HashTable table;
HashTable table2;

int count_nl(char*string){

    int c = 0;
    for (int i = 0; string[i]!='\0';i++){

        if (string[i] == '\n')
            c++;
    }
    return c;
}
int countDigits(int num) {
    if (num == 0) {
        return 1; 
    }

    int count = 0;
    if (num < 0) {
        count++; 
        num = abs(num); 
    }

    while (num != 0) {
        count++;
        num /= 10;
    }

    return count;
}
void parseFile(int fd) {
    char buffer[260];
    ssize_t bytesRead;
    Message *msg;
    msg = malloc (sizeof(struct message));
    msg->next=NULL;




    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        // Process the data read from the file descriptor
            int av = 20; 
            strcpy(msg->progName,strtok(buffer+av,"\n"));
            av = 20+strlen(msg->progName)+5;
            msg->pid = atoi(strtok(buffer+av,"\n"));
            av = 20+strlen(msg->progName)+5+countDigits(msg->pid)+20;
            msg->state = Finish;
            msg->timeStamp = atof(strtok(buffer+av,"\n"))/1000;
         }

    insert_message(&table2,*msg);
    close(fd);
}

int bootServer (char *directoryOr) {

    int check = 0;
    char *directory = strdup(directoryOr);
    for(;directory[check]!='\0';check++){
    }
    if (directory[check-1]!='/'){
        directory[check]='/';
        directory[check+1]='\0';
    }

    int pipefd[2];
    pipe(pipefd);
    pid_t pid;

    if ((pid = fork()) == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], 1);

        execlp("ls", "ls", directory, NULL);

        printf("Failed to execute ls command.\n");
        exit(1);
    } else {
        close(pipefd[1]);

        int fd = pipefd[0];

        char filepath[260];
        ssize_t bytesRead;

        while ((bytesRead = read(fd, filepath, sizeof(filepath))) > 0) {
            filepath[bytesRead - 1] = '\0';

            int malloc_size = count_nl(filepath)+1;
            char **filesarray = malloc(sizeof(char*)*(malloc_size+1));
            char *temp = strdup(filepath);
            for (int i = 0;i<malloc_size;i++){

                filesarray[i] = strdup(strsep(&temp,"\n"));
                char *path = strdup(directory);
                strcat(path,filesarray[i]);
                int filefd = open(path, O_RDONLY);
            parseFile(filefd);
            }
        
            
            
        }

        wait(NULL);

        close(fd);

        return 0;
    }
}


int main(int argc, char *argv[]) {
    bootServer(argv[1]);
    if ( mkfifo("fifo",0600) == -1){

        perror("mkfifo");
    } // Start fifo

    int fd = open("fifo", O_RDONLY, 0600); // open fifo
    int fd2 = open("fifo", O_WRONLY, 0600); // open fifo
   

    while (1) {  // Keep reading from the FIFO until the program is terminated

        
        Message *buffer = malloc(sizeof(struct message)); // Reading buffer
        ssize_t num_read;
        while((num_read = read(fd, buffer, sizeof(struct message))) > 0) { // Detected message
            if (buffer->state == Start){
        
                insert_message(&table,*buffer);
                     
            }
            if (buffer->state == Finish){
                
                end_message (&table,*buffer);
                insert_message(&table2,*buffer);
                pid_t pid_waiter;
                Message msg = *buffer;
                if((pid_waiter = fork()) == 0){
                    char *path = strdup(argv[1]);
                    char pid_str[12]; // allocate enough space to hold the string representation of the pi
                    sprintf(pid_str, "%d",msg.pid);  
                    addTxt(pid_str);
                    char *fpath = buildPath(path,pid_str);
                    int fp = open(fpath,O_CREAT|O_WRONLY,0600);
                    writeMessageToFile(fp,msg);
                    close(fp);

                    _exit(0);
                }
               
                
              }
              if (buffer->state == Info){
                        Message msg = *buffer;
                        pid_t pid_waiter2;
                        if((pid_waiter2 = fork()) == 0){
                        
                         char pid_str[16]; // allocate enough space to hold the string representation of the pid
                         sprintf(pid_str, "%d",msg.pid);
                         
                        int fd2;
                        fd2 = open(pid_str, O_WRONLY, 0600);// open fifo
                            
                            print_table(&table,fd2);
                        
                                close(fd2);
                                _exit(0);

                        }
                    }

                if (buffer->state == InfoTime){
                        Message msg = *buffer;
                        pid_t pid_waiter2;
                        if((pid_waiter2 = fork()) == 0){
                            
                         char pid_str[16]; // allocate enough space to hold the string representation of the pid
                         sprintf(pid_str, "%d",msg.pid);
                         
                        int fd2;
                        fd2 = open(pid_str, O_WRONLY, 0600);// open fifo 
                        
                        time_table_list(&table2,msg.progName,fd2);
                            
                                close(fd2);
                                _exit(0);

                        }
                    }
                    if (buffer->state == InfoCommand){
                        Message msg = *buffer;
                        pid_t pid_waiter2;
                        if((pid_waiter2 = fork()) == 0){
                            
                         char pid_str[16]; // allocate enough space to hold the string representation of the pid
                         sprintf(pid_str, "%d",msg.pid);
                         
                        int fd2;
                        fd2 = open(pid_str, O_WRONLY, 0600);// open fifo 
                               
                          command_table_list(&table2,msg.progName,fd2);
                                
                                close(fd2);
                                _exit(0);

                        }
                    }
                    if (buffer->state == InfoUniq){
                        Message msg = *buffer;
                        pid_t pid_waiter2;
                        if((pid_waiter2 = fork()) == 0){
                            
                         char pid_str[16]; // allocate enough space to hold the string representation of the pid
                         sprintf(pid_str, "%d",msg.pid);
                         
                        int fd2;
                        fd2 = open(pid_str, O_WRONLY, 0600);// open fifo 
                               
                                uniq_table_list(&table2,msg.progName,fd2);
                                close(fd2);


                                _exit(0);

                        }
                    }
                    check_and_reset_table(&table);
            }

            free(buffer);
            
        }
        
        close(fd);
        close(fd2);
        
    
    unlink("fifo");
    
    return 0;
}