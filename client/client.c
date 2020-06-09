/*
- Autores: Roberto Garcia Torres, Ulises Gilberto Lugo Fletes, José Guillermo Saldaña Cárdenas
- Matriculas: A00822089, A00344441, A01039888
- Fecha: 08/06/2020
- Descripcion: Programa que manda cambios de un directorio a un servidor que actualiza un directorio espejo en el
*/

#include <sys/inotify.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>

#define PORT_NUMBER     8080
#define SERVER_ADDRESS  "0.0.0.0"
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

int main (int argc, char *argv[]) {
    
    // /* Our process ID and Session ID */
    // pid_t pid, sid;
    
    // /* Fork off the parent process */
    // pid = fork();
    // if (pid < 0) {
    //         exit(EXIT_FAILURE);
    // }
    // /* If we got a good PID, then
    //    we can exit the parent process. */
    // if (pid > 0) {
    //         exit(EXIT_SUCCESS);
    // }

    // /* Change the file mode mask */
    // umask(0);
            
    // /* Open any logs here */        
            
    // /* Create a new SID for the child process */
    // sid = setsid();
    // if (sid < 0) {
    //         /* Log the failure */
    //         exit(EXIT_FAILURE);
    // }
    

    
    // /* Change the current working directory */
    // if ((chdir("/")) < 0) {
    //         /* Log the failure */
    //         exit(EXIT_FAILURE);
    // }
    
    // /* Close out the standard file descriptors */
    // close(STDIN_FILENO);
    // close(STDOUT_FILENO);
    // close(STDERR_FILENO);
    
    // /* Daemon-specific initialization goes here */

	int fd, wd, server_socket, remain_data, sent_bytes=0;
	off_t offset;
	char buff[BUF_LEN];
	char curr_dir[PATH_MAX];
	int len, i = 0;
	struct sockaddr_in remote_addr;
	struct stat file_stat;
	char file_size[256];

	// Set struct to 0s
	memset(&remote_addr, 0, sizeof(remote_addr));
	// Get current working directory
	getcwd(curr_dir, sizeof(curr_dir));

	// Constructs addr struct
	remote_addr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVER_ADDRESS, &(remote_addr.sin_addr));
	remote_addr.sin_port = htons(PORT_NUMBER);

	// Creates client socket
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1) {
		fprintf(stderr, "Error creating socket: %s\n", strerror(errno));
	}

	// Connect to server
	if (connect(server_socket, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1) {
		fprintf(stderr, "Error connecting to server: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	

	fd = inotify_init();
	if (fd < 0) perror("inotify_init");

	wd = inotify_add_watch(fd, 
					curr_dir,
					IN_MODIFY | IN_CREATE | IN_DELETE);

	if (wd < 0) perror("inotify_add_watch");
	while(1) {
		i=0;
		len = read(fd, buff, BUF_LEN);
		while(i < len) {
			struct inotify_event *event;
			event = (struct inotify_event *) &buff[i];
			printf ("wd=%d mask=%u cookie=%u len=%u\n",
	                event->wd, event->mask,
	                event->cookie, event->len);

	        if (event->len) {
				char filename[256];
				strcpy(filename,event->name);
	            printf ("name=%s\n", event->name);
				
				char mask[256];
				sprintf(mask, "%u", event->mask);

				int status = send(server_socket, &mask, sizeof(mask), 0);
				if (status < 0) {
					fprintf(stderr, "Error sending mask: %s\n", strerror(errno));
				}
				printf("SENT MASK: %s\n", mask);
				status = send(server_socket, &filename, sizeof(filename), 0);
				if (status < 0) {
					fprintf(stderr, "Error sending name: %s\n", strerror(errno));
				}
				printf("SENT FILENAME\n");
				if(!(event->mask & IN_DELETE)){
					printf("MASK NOT DELETE\n");
					int file = open(filename, O_RDONLY);
					if (file == -1) {
						fprintf(stderr, "Error opening file: %s", strerror(errno));
						exit(EXIT_FAILURE);
					}
					if (fstat(file, &file_stat) < 0)
					{
							fprintf(stderr, "Error fstat: %s", strerror(errno));

							exit(EXIT_FAILURE);
					}
					printf("File Size: %ld bytes\n", file_stat.st_size);
					sprintf(file_size, "%ld", file_stat.st_size);

					/* Sending file size */
					status = send(server_socket, file_size, sizeof(file_size), 0);
					if (status < 0) {
						fprintf(stderr, "Error sending length: %s\n", strerror(errno));
					}


					offset = 0;
					remain_data = file_stat.st_size;
					// Sending file data
					while (((sent_bytes = sendfile(server_socket, file, &offset, BUFSIZ)) > 0) && (remain_data > 0)) {
						fprintf(stdout, "1. Client sent %d bytes from file's data, offset is now : %ld and remaining data = %d\n", sent_bytes, offset, remain_data);
						remain_data -= sent_bytes;
						printf("sending file data");
						fprintf(stdout, "2. Client sent %d bytes from file's data, offset is now : %ld and remaining data = %d\n", sent_bytes, offset, remain_data);
					}
				}
				


			}


	        i += EVENT_SIZE + event->len;
		}
	}
	return 0;
}
