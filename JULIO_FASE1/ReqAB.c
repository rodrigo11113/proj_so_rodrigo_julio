#include "library.h"

int main (int argc, char **argv, char **envp){
	int num_pids = atoi(argv[1]);
	int destination, wcf;
	char buf[1024], wc[10],temp[1];
	int fds[2];
	int N_LINES;
	int status = 0;
	pipe(fds);
	pid_t pids[num_pids];

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
	if(fp == NULL){puts("Error opening file.\n"); exit(-1);}
	//-------------------------------------------------------------------\\

	fscanf(fp, "%*s", temp);
	LINES * ptr1 = linesX; //Apontadores temporarios para o inicio da struct
    LINES * ptr2 = linesX; 
	for (int i = 0; i < N_LINES -1; i++){
		fscanf(fp, "%ld %*[;] %ld %*[;] %ld %*[;] %ld %*[;] %ld %*[\n]", &(ptr1->admission), &(ptr1->start_trial), &(ptr1->end_trial), &(ptr1->start_medic), &(ptr1->end_medic));
		ptr1++;
	}
	ptr1 = linesX; //Reset

	//REQUESITO B
	long x_admission = 0, x_trial = 0, x_waiting=0, x_medic=0, timestamp;
	for(int i=0; i<num_pids; i++){ 
		if ((pids[i]=fork())==-1){
			perror("Fork");
			exit(1);
		}
		if (pids[i] == 0) { 
			pid_t my_pid = getpid();
			ptr2 += i;
			for(int j=i;j<N_LINES-1;j+=num_pids){
				timestamp=ptr2->admission;

				for(int k=0;k<N_LINES-1;k++){

						if(ptr1->admission 		< timestamp && timestamp <= ptr1->start_trial	)	x_admission++;
						if(ptr1->start_trial 	< timestamp && timestamp <= ptr1->end_trial		)	x_trial++;
						if(ptr1->end_trial 		< timestamp && timestamp <= ptr1->start_medic	)	x_waiting++;
						if(ptr1->start_medic 	< timestamp && timestamp <= ptr1->end_medic		)	x_medic++;
						ptr1++;
				}
				sprintf(buf,"%d$%d,%ld,Espera Triagem#%ld\n",my_pid,j,timestamp, x_admission);
					write(destination,buf,strlen(buf));
				sprintf(buf,"%d$%d,%ld,Triagem#%ld\n",my_pid,j,timestamp, x_trial);
					write(destination,buf,strlen(buf));
				sprintf(buf,"%d$%d,%ld,Sala de Espera#%ld\n",my_pid,j,timestamp, x_waiting);
					write(destination,buf,strlen(buf));
				sprintf(buf,"%d$%d,%ld,Consulta#%ld\n",my_pid,j,timestamp, x_medic);
					write(destination,buf,strlen(buf));
				
				x_admission = x_trial = x_waiting = x_medic = 0;
				ptr1=linesX;
				ptr2 += num_pids;
				
			}
			exit(0);
		}	
	} 	
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
	fclose(fp);
	return 0;
}
