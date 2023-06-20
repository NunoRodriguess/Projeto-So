void trimString(char *str) {
    int len = strlen(str);
    int start = 0, end = len - 1;
    while (start < len && (str[start] == ' ' || str[start] == '\n' || str[start] == '\0')) {
        start++;
    }
    while (end >= 0 && (str[end] == ' ' || str[end] == '\n' || str[end] == '\0')) {
        end--;
    }
    for (int i = start; i <= end; i++) {
        str[i - start] = str[i];
    }
    str[end - start + 1] = '\0';
}

static int nr_space(char *buf){
    int nr = 0;
    for(int i=0; buf[i]!='\0'; i++){

        if(buf[i]==' ') nr +=1;

    }

    return nr;
}

static int nr_pro(char *buf){
    int nr = 0;
    for(int i=0; buf[i]!='\0'; i++){

        if(buf[i]=='|') nr +=1;

    }

    return nr+1;
}
char* concatPids(int argc, char **argv,int st) {
    size_t totalLength = 0;
    for (int i = st; i < argc; i++) {
        totalLength += strlen(argv[i]) + 1; 
    }
    char *result = malloc(totalLength + 1); 
 
    result[0] = '\0'; 
    for (int i = st+1; i < argc; i++) {
        strcat(result, argv[i]);
        if (i < argc - 1) {
            strcat(result, " ");
        }
    }
    return result;
}

void addTxt(char*buffer){

        int i;
        for (i = 0 ; buffer[i]!='\0';i++){

        }
        buffer[i] = '.';
        buffer[i+1] = 't';
        buffer[i+2] = 'x';
        buffer[i+3] = 't';
        buffer[i+4] = '\0';
    }

char* buildPath(const char* directory,char* filename) {
    const char* separator = "/";
    size_t directoryLength = strlen(directory);
    size_t filenameLength = strlen(filename);
    size_t separatorLength = strlen(separator);

    // Allocate memory for the combined path
    size_t pathLength = directoryLength + separatorLength + filenameLength + 1;
    char* path = (char*)malloc(pathLength);

    if (path == NULL) {
        fprintf(stderr, "Failed to allocate memory for the path.\n");
        return NULL;
    }

    // Copy the directory, separator, and filename to the path
    strcpy(path, directory);
    strcat(path, separator);
    strcat(path, filename);

    return path;
}