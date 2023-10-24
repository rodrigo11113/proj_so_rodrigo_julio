#include "library.h"



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

int main(int argc, char **argv, char **envp){		// Command Line Arguments

	char temp[1],buf[100];
	char msg[100],wc[10];
	int destination,wcf,pipe_communication, N_LINES;
	int num_pids = atoi(argv[1]);
	int pids[num_pids];
	int fds[2];
	pipe(fds);
	
	//-------------------------------READING INPUT-------------------------
	if(argc < 3){ perror("Insufficient arguements.\n"); exit(1);}
	system("wc -l < all_timestamps.csv > /tmp/wc.txt");
	wcf = open("/tmp/wc.txt", O_RDONLY);
	if(wcf == -1){ perror("Opening WCF...\n"); exit(1); }
	read(wcf, wc, sizeof(wc));
	N_LINES = atoi(wc);
	close(wcf);

	destination = open(argv[3], O_CREAT | O_TRUNC | O_WRONLY, 0666);
	if(destination == -1) {perror("Opening Destination file...\n"); exit(1);}

	LINES * linesX = (LINES*) calloc(N_LINES, sizeof(LINES));
	FILE * fp = fopen("all_timestamps.csv", "r");
	if(fp == NULL){puts("Error opening file.\n"); exit(-1);exit(-1);}
	//-------------------------------------------------------------------\\

	fscanf(fp,"%*s",temp);	//Ignora a primeira linha do input
	LINES * ptr1=linesX;	//Apontadores temporarios para o inicio da struct
	LINES * ptr2=linesX;	
	for (int i = 0; i < N_LINES -1; i++){
		fscanf(fp, "%ld %*[;] %ld %*[;] %ld %*[;] %ld %*[;] %ld %*[\n]", &(ptr1->admission), &(ptr1->start_trial), &(ptr1->end_trial), &(ptr1->start_medic), &(ptr1->end_medic));
		ptr1++;
	}
	ptr1 = linesX; //Reset
	long x_admission = 0,x_trial = 0,x_waiting = 0,x_medic = 0, timestamp;
	for(int i=0;i<num_pids;i++){ 
		if ((pids[i]=fork())==-1){
			perror("Fork");
			exit(-1);
		}
		if (pids[i] == 0) { 
			close(fds[0]);
			pid_t my_pid = getpid();
			ptr2 += i;
			for(int j=i;j<N_LINES;j+=num_pids){
				timestamp=ptr2->admission;				
				for(int k=0;k<N_LINES;k++){

						if(ptr1->admission 		< timestamp && timestamp <= ptr1->start_trial 	)	x_admission++;
						if(ptr1->start_trial 	< timestamp && timestamp <= ptr1->end_trial 	)	x_trial++;
						if(ptr1->end_trial 		< timestamp && timestamp <= ptr1->start_medic 	)	x_waiting++;
						if(ptr1->start_medic 	< timestamp && timestamp <= ptr1->end_medic 	)	x_medic++;
						ptr1++;
				}
				sprintf(buf,"%d$%d,%ld,Espera_Triagem#%ld\n",my_pid,j,timestamp, x_admission);
					write(destination,buf,strlen(buf));
				sprintf(buf,"%d$%d,%ld,Triagem#%ld\n",my_pid,j,timestamp, x_trial);
					write(destination,buf,strlen(buf));
				sprintf(buf,"%d$%d,%ld,Sala_Espera#%ld\n",my_pid,j,timestamp, x_waiting);
					write(destination,buf,strlen(buf));
				sprintf(buf,"%d$%d,%ld,Consulta#%ld\n",my_pid,j,timestamp, x_medic);
					write(destination,buf,strlen(buf));

				x_admission=x_trial=x_waiting=x_medic=0;
				ptr1=linesX;
				ptr2+=num_pids;
			}
			close(fds[1]);
			exit(0);
		}	
	}
	close(fds[1]);
	while(pipe_communication=readn(fds[0],msg,strlen(msg))>0){
		writen(destination,msg,strlen(msg));
	}
	close(fds[0]);
	for(int k = 0; k < num_pids; k++){
		int result;
		waitpid(pids[k],&result,0);
		if(WIFEXITED(result)){
			printf("Process %d terminated.\n", pids[k]);
		}
	}
	close(destination);
	free(ptr1);
	free(ptr2);
	return 0;

}