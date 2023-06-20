typedef enum {

   Break,
   Start,
   Info,
   InfoTime,
   InfoCommand,
   InfoUniq,
   Finish


}Type;


typedef struct message {

    pid_t pid;
    double timeStamp;
    char progName[260];
    Type state;
    struct message* next;
    
}Message;

void printMessage(Message msg) {

    char buffer[256];
    int len = snprintf(buffer, sizeof(buffer), "%d %s %.0f ms\n", msg.pid, msg.progName, msg.timeStamp*1000);
    write(1, buffer, len);

}
void printList(Message* head) {
    Message* current = head;
    while (current != NULL) {
        printMessage(*current);
        current = current->next;
    }
}

Message* cloneMessage(Message* original) {
    if (original == NULL) {
        return NULL;
    }

    Message* clone = (Message*)malloc(sizeof(Message));
    if (clone == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    clone->pid = original->pid;
    clone->timeStamp = original->timeStamp;
    strncpy(clone->progName, original->progName, sizeof(clone->progName));
    clone->state = original->state;
    clone->next = NULL;

    return clone;
}

void writeMessageToFile(int fp, Message msg) {
    char endOfLine = '\n';
    char progNameIntro[20] = "Name of the Program:";
    char progPid[4] = "Pid:";
    char progTime[19] = "Execution Time(ms):";
    write(fp,progNameIntro,20);
    write(fp, msg.progName, strlen(msg.progName));
    write(fp, &endOfLine, 1);
    char pidStr[50];
    sprintf(pidStr, "%d", msg.pid);
    write(fp,progPid,4);
    write(fp, pidStr, strlen(pidStr));
    write(fp, &endOfLine, 1);
    char timeStampStr[50];
    sprintf(timeStampStr, "%0.f", msg.timeStamp*1000);
    write(fp,progTime,19);
    write(fp, timeStampStr, strlen(timeStampStr));
    write(fp, &endOfLine, 1);
}

void addToList(Message** head, Message* newElement) {
    if (*head == NULL) {
        *head = newElement;
        newElement->next = NULL;
    } else {
        Message* current = *head;
        int flag = 0;
        
        if (strcmp(current->progName,newElement->progName)==0)
                flag = 1;
                
        while (current->next != NULL && flag == 0) {
            if (strcmp(current->progName,newElement->progName)==0)
                flag = 1;

            current = current->next;
        }
        if (flag == 1){

        }else{
        current->next = newElement;
        newElement->next = NULL;
        }
    }
}

void extract_prog(char input[]) {
    int i = 0;
    int j = 0;
    int remove_args = 0;

    while (input[i] != '\0') {
        if (input[i] == '|') {
            remove_args = 0;
            input[j++] = input[i++]; // Preserve '|'
            input[j++] = ' '; // Add whitespace after '|'
        } else if (input[i]==' ') {
            input[j++] = input[i++];
        } else if (!remove_args) {
            remove_args = 1;
            while (input[i] != '\0' && input[i]!=' ' && input[i] != '|') {
                input[j++] = input[i++]; // Preserve program name
            }
        } else {
            while (input[i] != '\0' && input[i]!=' ' && input[i] != '|') {
                i++;
            }
        }
    }
    input[j] = '\0'; // Null-terminate the modified string
}


Message alterArg(Message msg){

    int nr_delim =0;

    for ( int i = 0;msg.progName[i]!='\0';i++){

        if ( msg.progName[i]=='|')
            nr_delim++;

    }

    if ( nr_delim == 0){
        int i;
        for ( i = 0;msg.progName[i]!=' ';i++){
        }
        msg.progName[i] = '\0';
    }else{
       int av = 0;
       int i = 0;
       while (av < strlen(msg.progName)){
            i = 0;
            for (;msg.progName[av+i]!=' '&& msg.progName[av+i]!='\0';i++){
             }
             
             for (;msg.progName[av+i]!='|'&& msg.progName[av+i]!='\0';i++){

                msg.progName[av+i] = ' ';

             }
             i+=2;
             av += i;
       }
        
    }
return msg;

}