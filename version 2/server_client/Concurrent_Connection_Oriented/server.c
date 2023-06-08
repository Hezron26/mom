// Concurrent-Connection Oriented server using fork and process
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define PORT 8080

int checkDuplicate(char serialNumber[], char regNumber[]) {
    FILE *file;
    char line[100];
    char tempSerialNumber[100], tempRegNumber[100];
    file = fopen("data.txt", "r"); // open file in read mode
    if (file == NULL) {
        return 0; // return 0 if file cannot be opened
    }
    while (fgets(line, 100, file) != NULL) {
        sscanf(line, "%s %s", tempSerialNumber, tempRegNumber); // read serial number and registration number from file
        if (strcmp(tempSerialNumber, serialNumber) == 0 || strcmp(tempRegNumber, regNumber) == 0) {
            fclose(file);

            return 1; // return 1 if either serial number or registration number is a duplicate
        }
    }
    fclose(file);
    return 0; // return 0 if neither serial number nor registration number is a duplicate
}

char* write_to_text_file(char serial_n[], char reg_n[], char name[]){
     char* status = malloc(sizeof(char) * 110);
     int feed = checkDuplicate(serial_n, reg_n);
     if (feed){
         printf("\nDuplicate data entered: ");
         strcpy(status, "Duplicate Error: either serial number or registration number is a duplicate");
     } else {
       FILE *file;
       file = fopen("data.txt", "a"); // open file in append mode
       fprintf(file, "%s %s %s\n", serial_n, reg_n, name); // write details to file
       fclose(file);
       printf("Details saved to file.\n");
       strcpy(status, "Details saved to file");
     }
     return status;
}

int splitStringByComma(char* str, char** substrings, int maxSubstrings) {
    int numSubstrings = 0;
    char* token;
    token = strtok(str, ",");
    while (token != NULL && numSubstrings < maxSubstrings) {
        substrings[numSubstrings++] = token;
        token = strtok(NULL, ",");
    }
    return numSubstrings;
}

int main() {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char* substrings[3];

    printf("Creating server socket");
    // Create a socket for the server and Bind the socket to a specific port.
    server_fd = socket(AF_INET, SOCK_STREAM, 0) ;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

    // Listen for incoming connections.
    listen(server_fd, 5);


    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);

        int pid = fork(); //	Fork a child process

        if (pid == 0) { // Child process
            close(server_fd);

            read(new_socket, buffer, 1024);
            printf("Received message: %s\n", buffer);

            splitStringByComma(buffer, substrings, 3);

            printf("\n   client data Received  ");
            printf("\n\t1. Serial Number        : %s",substrings[0]);
            printf("\n\t2. Registration Number  : %s",substrings[1]);
            printf("\n\t3. Client Name          : %s",substrings[2]);
            printf("\n\n     ");

            char* status = write_to_text_file(substrings[0], substrings[1], substrings[2] );

            send(new_socket, status, strlen(status), 0);

            memset(buffer, 0, sizeof(buffer));

            close(new_socket);
            exit(EXIT_SUCCESS);
        }

        // Parent process
        close(new_socket);
        wait(NULL);
    }

    return 0;
}
