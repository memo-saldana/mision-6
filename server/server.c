/*
- Autores: Roberto Garcia Torres, Ulises Gilberto Lugo Fletes, José Guillermo Saldaña Cárdenas
- Matriculas: A00822089, A00344441, A01039888
- Fecha: 08/06/2020
- Descripcion: Programa que recibe cambios en archivos en un cliente y los actualiza en un directorio del servidor.
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <linux/limits.h>

#define PORT_NUMBER     8080
#define SERVER_ADDRESS  "0.0.0.0"

void add_to_file(char *buffer, int len, FILE *file_name){
	fwrite(buffer, sizeof(char), len, file_name);
	// printf("Receive %d bytes\n", len);
}

void delete_file(char *name){
    remove(name);
	// if (remove(name) == 0) 
	// 	// printf("Deleted successfully %s\n", name); 
	// else
	// 	// printf("Unable to delete the file %s\n", name); 
}


int main(int argc, char **argv) {

    char curr_dir[] = "/home/ulises/Documents/mision-6/server";
    // printf("%s\n", curr_dir);
      
    /* Our process ID and Session ID */
    pid_t pid, sid;
    
    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
            exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then
       we can exit the parent process. */
    if (pid > 0) {
            exit(EXIT_SUCCESS);
    }

    /* Change the file mode mask */
    umask(0);
            
    /* Open any logs here */        
            
     // Create a new SID for the child process 
    sid = setsid();
    if (sid < 0) {
            /* Log the failure */
            exit(EXIT_FAILURE);
    }
    

    
    /* Change the current working directory */
    // if ((chdir(curr_dir)) < 0) {
    //          Log the failure 
    //         exit(EXIT_FAILURE);
    // }
    
    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    /* Daemon-specific initialization goes here */
	int server_socket, client_socket;
	ssize_t len;
	struct sockaddr_in server_address, client_address;
	char buffer[BUFSIZ];
	int file_size;
	FILE *received_file;
	int remain_data = 0;

	memset(&client_address, 0, sizeof(client_address));
	/* Zeroing server_address struct */
	memset(&server_address, 0, sizeof(server_address));

	/* Construct server_address struct */
	server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(PORT_NUMBER);

	/* Create client socket */
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1) {
		fprintf(stderr, "Error creating server socket: %s\n", strerror(errno));

		exit(EXIT_FAILURE);
	}

	/* Connect to the server */
	
	if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
		fprintf(stderr, "Error on connect: %s\n", strerror(errno));

		exit(EXIT_FAILURE);
	}
	listen(server_socket, 10);
    // printf("listen");
    int clilen = sizeof(client_address);
    client_socket = accept(server_socket, (struct sockaddr *) &client_address, &clilen);
	while (1) {
		
		if (client_socket == -1) {
			fprintf(stderr, "Error creating client socket: %d\n", errno);

			exit(EXIT_FAILURE);
		}
        int mask, file_size;
        char file_name[256];
        char complete_name[1256];
		// mask
    	recv(client_socket, buffer, BUFSIZ, 0);
	    // printf("Mask received: %s", buffer);
		mask = atoi(buffer);
        // nombre
		recv(client_socket, file_name, BUFSIZ, 0);
        // printf("mask : %d, in_delete: %d\n", mask, IN_DELETE);
        strcpy(complete_name,curr_dir);
        strcat(complete_name,"/");
        strcat(complete_name,file_name);

        if (mask == IN_DELETE) {
            // printf("borrar");
            delete_file(file_name);
        } else {
            // file size
            recv(client_socket, buffer, BUFSIZ, 0);
            file_size = atoi(buffer);
            // file data
            FILE *received_file = fopen(complete_name, "w");
            remain_data = file_size;

            if (received_file == NULL) {
                fprintf(stderr, "Failed to open file: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            
            // printf("todo bien en casa %d\n", remain_data);
            while ((remain_data > 0) && ((len = recv(client_socket, buffer, BUFSIZ, 0)) > 0)) {
                // printf("remained: %d\n", remain_data);
				add_to_file(buffer, len, received_file);
                remain_data -= len;
            }
            fclose(received_file);
        }
	}

	return 0;
}
