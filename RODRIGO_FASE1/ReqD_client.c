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
	int N_LINES;
	int N_LINES_STRUCT = 0;
	int num_pids = atoi(argv[1]);
	int pos = atoi(argv[1]);
	int destination, wcf, bytes, c, uds;
	char wc[10], temp[1], buf[4096];
    struct sockaddr_un channel; 


	//-------------------------------READING INPUT-------------------------
	if(argc < 3){ perror("Insufficient arguements.\n"); exit(1);}
	system("wc -l < all_timestamps.csv > /tmp/wc.txt");
	wcf = open("/tmp/wc.txt", O_RDONLY);
	if(wcf == -1){ perror("Opening WCF...\n"); exit(1); }
	read(wcf, wc, sizeof(wc));
	N_LINES = atoi(wc);
	
	if((N_LINES % num_pids)!= 0) 
		N_LINES_STRUCT = 1;
	N_LINES_STRUCT += (N_LINES/num_pids);
	close(wcf);


	LINES * linesX = (LINES*) calloc(N_LINES, sizeof(LINES));
	FILE * fp = fopen("all_timestamps.csv", "r");
	if(fp == NULL){puts("Error opening file.\n"); exit(-1);}
	//-------------------------------------------------------------------\\
	
	fscanf(fp,"%*s",temp);
	for(int i=0;i<pos;i++){	
		fscanf(fp,"%*s",temp); //Avança linhas para comecar a ler na linha correta
	}
	LINES * ptr1=linesX;	//Apontadores temporarios para o inicio da struct
	LINES * ptr2=linesX;	
	for(int i=0;i<N_LINES_STRUCT;i++){
		fscanf(fp,"%ld %*[;] %ld %*[;] %ld %*[;] %ld %*[;] %ld %*[\n]",&(ptr1->admission),&(ptr1->start_trial),&(ptr1->end_trial),&(ptr1->start_medic),&(ptr1->end_medic));
		for(int j=0;j<num_pids-1;j++){
			fscanf(fp,"%*s",temp);
		}
		ptr1++;
	}

	fclose(fp);
	ptr1=linesX;


    if ( (uds = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {perror("Error in socket.");exit(1);}
    memset(&channel, 0, sizeof(channel));
    channel.sun_family= AF_UNIX;
    strncpy(channel.sun_path, socket_path, sizeof(channel.sun_path)-1);
    if (connect(uds, (struct sockaddr*)&channel, sizeof(channel)) == -1) {perror("Error connecting."); exit(1);}
    
    long x_admission=0,x_trial=0,x_waiting=0,x_medic=0,timestamp;
    pid_t my_pid=getpid();
    for(int j=0;j<N_LINES_STRUCT;j++){
    	timestamp=ptr2->admission; 
    		for(int k=0;k<N_LINES_STRUCT;k++){

    			if(ptr1->admission 		< timestamp && timestamp <= ptr1->start_trial 	)	x_admission++;
				if(ptr1->start_trial 	< timestamp && timestamp <= ptr1->end_trial 	)	x_trial++;
				if(ptr1->end_trial 		< timestamp && timestamp <= ptr1->start_medic 	)	x_waiting++;
				if(ptr1->start_medic 	< timestamp && timestamp <= ptr1->end_medic 	)	x_medic++;
				ptr1++;
    		}
				sprintf(buf,"%d$%d,%ld,Espera_Triagem#%ld\n",my_pid,j,timestamp, x_admission);
					writen(uds,buf,strlen(buf));
				sprintf(buf,"%d$%d,%ld,Triagem#%ld\n",my_pid,j,timestamp, x_trial);
					writen(uds,buf,strlen(buf));
				sprintf(buf,"%d$%d,%ld,Sala_Espera#%ld\n",my_pid,j,timestamp, x_waiting);
					writen(uds,buf,strlen(buf));
				sprintf(buf,"%d$%d,%ld,Consulta#%ld\n",my_pid,j,timestamp, x_medic);
					writen(uds,buf,strlen(buf));
        	x_admission = x_trial = x_waiting = x_medic = 0;
        	ptr1=linesX;
    		ptr2++;
    }
    close(uds);
    free(ptr1);
	free(ptr2);
	exit(0);
}