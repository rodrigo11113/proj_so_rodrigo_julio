#include "library.h"

char *socket_path = "/tmp/socket";

ssize_t readn(int fd, void *ptr, size_t n){ /* Read "n" bytes from a descriptor  */
	size_t		nleft;
	ssize_t		nread;

	nleft = n;
	while (nleft > 0) {
		if ((nread = read(fd, ptr, nleft)) < 0) {
			if (nleft == n)
				return(-1); /* error, return -1 */
			else
				break;      /* error, return amount read so far */
		} else if (nread == 0) {
			break;          /* EOF */
		}
		nleft -= nread;
		ptr   += nread;
	}
	return(n - nleft);      /* return >= 0 */
}


ssize_t writen(int fd, const void *ptr, size_t n){ /* Write "n" bytes to a descriptor  */
	size_t		nleft;
	ssize_t		nwritten;

	nleft = n;
	while (nleft > 0) {
		if ((nwritten = write(fd, ptr, nleft)) < 0) {
			if (nleft == n)
				return(-1); /* error, return -1 */
			else
				break;      /* error, return amount written so far */
		} else if (nwritten == 0) {
			break;
		}
		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n - nleft);      /* return >= 0 */
}

int main(int argc, char **argv, char **envp){
	int destination, wcfd, fd, connfd, bytes, listenfd;
	int num_pids = atoi(argv[1]);
	int pids[num_pids];
	int N_LINES;
	char  input [100], filename_str[1024];
	char  itoa_pids[5], itoa_pos[5];
	char buf[4096],wc[10];
    struct sockaddr_un channel_srv;

	//-------------------------------READING INPUT-------------------------
	if(argc < 3){ perror("Insufficient arguements.\n"); exit(1);}
	
	strcpy(input,argv[2]);
	destination=open(argv[3], O_CREAT | O_TRUNC | O_WRONLY, 0666);
	sprintf(itoa_pids,"%d", num_pids);

	for(int i=0;i<num_pids;i++){
		if ((pids[i]=fork())==-1){perror("Fork");exit(1);}
		if (pids[i] == 0) {
			
			sprintf(itoa_pos,"%d", i);
			execlp("./client", itoa_pids, itoa_pos, input, NULL);
			perror("Invalid Path");exit(-1);
		}
	}	
	
    if ((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {perror("Error in socket.");exit(-1);}	// Creating the server socket
    unlink(socket_path);
    memset(&channel_srv, 0, sizeof(channel_srv));
    channel_srv.sun_family = AF_UNIX;
    strncpy(channel_srv.sun_path, socket_path, sizeof(channel_srv.sun_path)-1);
    if (bind(listenfd, (struct sockaddr*)&channel_srv, sizeof(channel_srv)) == -1) {perror("Error binding.");exit(-1);} // Binding the server socket to its name
    if (listen(listenfd, 10) == -1) {perror("Error listening.");exit(-1);} // Configuring the listen queue
    // Socket is now set up and bound. Waiting for connections

    for(int j=0;j<num_pids;j++){
        if ((connfd = accept(listenfd, NULL, NULL)) == -1) {perror("Error accepting."); continue;}
        while (1) {
            bytes = readn(connfd, buf, 4096);                           // read from socket
            if (bytes <= 0) break;                                          // check for end of information
            writen(destination, buf, bytes);                                // write bytes to file
        }
        close(connfd);                                                      // close connection
    }
	for(int k = 0; k < num_pids; k++){
		int result;
		waitpid(pids[k], &result, 0);
		if(WIFEXITED(result)){
			printf("Process %d has been terminated.\n", pids[k]);
		}
	}
    close(destination);
    return 0;
}