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


void execute_command(char** args, int read_fd, int write_fd) {
    pid_t pid;
    if ( (pid = fork())==0){
        if (read_fd != -1) { 
            close(0);
            dup2(read_fd, 0); //  input para read_fd
            close(read_fd);
        }

        if (write_fd != -1) { 
            close(1);
            dup2(write_fd, 1); // output para write_fd
            close(write_fd);
        }

        execvp(args[0], args);
        _exit(1);
    }

}

void executeU (char*buffer){

    // inciar o comando
    pid_t pid;

    // tirar o time stamp primerio
    int malloc_size = nr_space(buffer)+2;
    // Criacao de argumentos para enviar ao monitor
    char **arguments = malloc(sizeof(char*) * malloc_size);

    // Povoar o array de argumentos do filho
    char *pr_name = strdup(buffer);
    int i;
    for (i = 0; i < malloc_size-1; i++){

        arguments[i] = strdup(strsep(&buffer," "));

    }

    arguments[i] = NULL;
    int fd;
    while ((fd = open("fifo", O_WRONLY)) == -1) {
    }
    //comunicar com o servidor
    int pd[2];
    pipe(pd);
     if((pid = fork()) == 0){
        
        // child pid
        pid_t child=getpid();
        
    
        // create message to monitor
        Message msg;
        msg.pid = child;
        msg.state = Start;
        strcpy(msg.progName,pr_name);
        struct timeval tm;
        gettimeofday(&tm,NULL);
        double timestamp =((double)tm.tv_sec + (double)tm.tv_usec / 1000000);
        msg.timeStamp = timestamp;
        write(pd[1],&timestamp,sizeof(double));

        //send message to monitor
        write(fd,&msg,sizeof(struct message));
        close(fd);
        //execute prog
        char buffer[256];
        int len = snprintf(buffer, sizeof(buffer), "Running PID %d\n", child);
        write(1, buffer, len);
        execvp(arguments[0],arguments);
        

        printf("Command %s could not initialize properly\n",pr_name);
        _exit(1);

    }
    else {
        // pai
        int status;
        double timeStampst;
        read(pd[0],&timeStampst,sizeof(double));
        waitpid(pid,&status,WUNTRACED);
        // create message to monitor
        char pid_str[16]; // allocate enough space to hold the string representation of the pid
        sprintf(pid_str, "%d",pid);
        
        if ( mkfifo(pid_str,0600) == -1){

            perror("mkfifo");
        } // Start fifo
        Message msg;
        msg.pid = pid;
        msg.state = Finish;
        strcpy(msg.progName,pr_name);
        struct timeval tm;
        gettimeofday(&tm,NULL);
        double timestamp = (double)tm.tv_sec + (double)tm.tv_usec / 1000000;
        double finalTime = timestamp - timeStampst;
        msg.timeStamp =finalTime;
        
        //send message to monitor
        write(fd,&msg,sizeof(struct message));
        close(fd);
        finalTime *=1000;
        char buffer2[256];
        int len2 = snprintf(buffer2, sizeof(buffer2), "Ended in %.0f ms\n", finalTime);
        write(1, buffer2, len2);
        unlink(pid_str);
        
        
    

    }
     

}

void executeP (char*buffer){

    Message msg;
    pid_t this=getpid();
    msg.pid = this;
    msg.state = Start;
    strcpy(msg.progName,buffer);

   int num_commands = nr_pro(buffer);
   char ** commands = malloc(sizeof(char*)*num_commands);
   int i ;
    for (i = 0; i < num_commands; i++){

        commands[i] = strsep(&buffer,"|");
        trimString(commands[i]);
      
    }

   int pipes[num_commands-1][2];
   for (i = 0; i < num_commands - 1; i++) {
        if (pipe(pipes[i]) == -1) { // Create pipes
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    struct timeval tm;
    gettimeofday(&tm,NULL);
    double timeStamp =((double)tm.tv_sec + (double)tm.tv_usec / 1000000);
    msg.timeStamp = timeStamp;
     int fd;
    while ((fd = open("fifo", O_WRONLY)) == -1) {
}
    char buffer2[256];
    int len2 = snprintf(buffer2, sizeof(buffer2), "Running PID %d\n", this);
    write(1,buffer2, len2);
    write(fd,&msg,sizeof(struct message));

    for (i = 0; i < num_commands; i++) {
        int malloc_size = nr_space(commands[i]) + 2;
        char **arguments = malloc(sizeof(char*) * malloc_size);
        char *pr_name = strdup(commands[i]);
        if (pr_name == NULL) {
             perror("strdup");
            _exit(-1);
        }
        int j;
        for (j = 0; j < malloc_size-1; j++) {
            arguments[j] = strdup(strsep(&pr_name, " "));
            if (arguments[j] == NULL) {
                perror("strdup");
                _exit(-1);
             }
            }

        arguments[j] = NULL;
        if (i == 0) { // primeiro da chain
            execute_command(arguments, -1, pipes[0][1]);
            close(pipes[0][1]);
        } else if (i == num_commands - 1) { // último da chain
            execute_command(arguments, pipes[i - 1][0], -1);
            close(pipes[i - 1][0]);
        } else { // outros cuidado
            execute_command(arguments, pipes[i - 1][0], pipes[i][1]);
            close(pipes[i - 1][0]);
            close(pipes[i][1]);
        }
    }
    int status;
    for (i = 0; i < num_commands; i++) {
        wait(&status); 
    }
    struct timeval tm2;
        gettimeofday(&tm2,NULL);
        double timestamp =((double)tm2.tv_sec + (double)tm2.tv_usec / 1000000);
        msg.timeStamp = timestamp - timeStamp;
        msg.state = Finish;
        double timeStampP = msg.timeStamp*1000;
        char buffer3[256];
        int len = snprintf(buffer3, sizeof(buffer3), "Ended in %.0f ms\n", timeStampP);
        write(1, buffer3, len);
        write(fd,&msg,sizeof(struct message));
        close(fd);
   
}
void executeStatusTime(char* list){   
    
    pid_t this = getpid();
    Message msg;
    char pid_str[16]; // allocate enough space to hold the string representation of the pid
    sprintf(pid_str, "%d", this);
    if( mkfifo(pid_str,0600) == -1){
         perror("mkfifo");
     } // Start fifo
    strcpy(msg.progName,list);             
    msg.pid = this;
    msg.state = InfoTime;
     int fd;
     while ((fd = open("fifo", O_WRONLY)) == -1) {
        }   
    write(fd,&msg,sizeof(struct message));

    int fd2 = open(pid_str, O_RDONLY , 0600); // open fifo
    double sum;
    read(fd2,&sum,sizeof(double));
    sum*= 1000;
    char buffer[256];
    int len = snprintf(buffer, sizeof(buffer), "Total execution time is %.0f ms\n", sum);
    write(1, buffer, len);
    unlink(pid_str);
}

void executeStatusCommand(char* list,char*progName){   
    pid_t this = getpid();
    Message msg;
    char pid_str[16]; // allocate enough space to hold the string representation of the pid
    sprintf(pid_str, "%d", this);
    if( mkfifo(pid_str,0600) == -1){
         perror("mkfifo");
     } // Start fifo
    strcat(progName,",");
    strcat(progName,list);
    strcpy(msg.progName,progName);             
    msg.pid = this;
    msg.state = InfoCommand;
    int fd;
    while ((fd = open("fifo", O_WRONLY)) == -1) {
    }
    write(fd,&msg,sizeof(struct message));

    int fd2 = open(pid_str, O_RDONLY , 0600); // open fifo
    int sum;
    read(fd2,&sum,sizeof(int));
    char* token = strtok(progName, ",");
    char programName[260];
    if (token != NULL) {
    strncpy(programName, token, sizeof(programName));
    programName[sizeof(programName) - 1] = '\0';
    }
    char buffer[256];
    int len = snprintf(buffer, sizeof(buffer), "%s was executed %d times\n", programName, sum);
    write(1, buffer, len);
    unlink(pid_str);
}
void executeStatusUniq(char* list){
      
    pid_t this = getpid();
    Message msg;
    char pid_str[16]; // allocate enough space to hold the string representation of the pid
    sprintf(pid_str, "%d", this);
    if( mkfifo(pid_str,0600) == -1){
         perror("mkfifo");
     } // Start fifo
    strcpy(msg.progName,list);             
    msg.pid = this;
    msg.state = InfoUniq;
    int fd;
    while ((fd = open("fifo", O_WRONLY)) == -1) {
    }
    write(fd,&msg,sizeof(struct message));

    int fd2 = open(pid_str, O_RDONLY , 0600); // open fifo
    size_t num_read;
    Message msg_buffer;
    while ((num_read = read(fd2, &msg_buffer, sizeof(struct message))) > 0){
        
        char buffer[256];
        int len = snprintf(buffer, sizeof(buffer), "%s\n", msg_buffer.progName);
        write(1, buffer, len);
    }

    
    unlink(pid_str);
}

void executeStatus(){   
    
    pid_t this = getpid();
    Message msg;
    char pid_str[16]; // allocate enough space to hold the string representation of the pid
    sprintf(pid_str, "%d", this);
    if( mkfifo(pid_str,0600) == -1){
         perror("mkfifo");
     } // Start fifo
                    
    msg.pid = this;
    msg.state = Info;
     int fd;
    while ((fd = open("fifo", O_WRONLY)) == -1) {
    }
    write(fd,&msg,sizeof(struct message));

    int fd2 = open(pid_str, O_RDONLY , 0600); // open fifo
    Message buffer;
    while((read(fd2,&buffer,sizeof(struct message)))>0){

        struct timeval tm;
        gettimeofday(&tm,NULL);
        double timestamp = (double)tm.tv_sec + (double)tm.tv_usec / 1000000;
        Message temp = buffer;
        temp.timeStamp = timestamp - temp.timeStamp;
        temp = alterArg(temp);
        printMessage(temp);

    }
    unlink(pid_str);
}

int main(int argc, char *argv[]) {

    if(strcmp(argv[1],"execute") == 0){

        if(strcmp(argv[2],"-u") == 0){

            executeU(argv[3]);   

        }
        if(strcmp(argv[2],"-p") == 0){

            executeP(argv[3]);   

        }
    
    }
    if(strcmp(argv[1],"status") == 0){

        executeStatus();
    
    }
    if(strcmp(argv[1],"stats-time") == 0){
                
        executeStatusTime(concatPids(argc,argv,1));
    
    }
    if(strcmp(argv[1],"stats-command") == 0){
                
        executeStatusCommand(concatPids(argc,argv,2),argv[2]);
    
    }
    if(strcmp(argv[1],"stats-uniq") == 0){
                
        executeStatusUniq(concatPids(argc,argv,1));
    
    }
    
    return 0;
}