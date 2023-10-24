#include "library.h"

//Variaveis globais
pthread_mutex_t mutex_ocupations = PTHREAD_MUTEX_INITIALIZER;
int ocupations = 0;
int N_THREADS;
int N_LINES;
int destination;
LINES* linesX;

/**
 * Funcao de threads que escreve para ficheiro as ocupacoes e usa 1 semaforo quando incrementa as ocupacoes
 * Por ser uma variavel partilhada
 * @param params = Recebe os parametros passados na thread
 * @return 
 */
void* threadsOcupations(void* params){
    LINES* ptr1 = linesX;
    LINES* ptr2 = linesX;
    char buf[100];
    int i = (int)params;
    ptr2 += i;
    printf("\n");
    printf("Thread = %d | Line = %d", i, N_LINES);
    long x_admission = 0, x_trial = 0, x_waiting=0, x_medic=0, timestamp;
    for(int j = i; j<N_LINES; j+= N_THREADS){
        long timestamp = ptr2->admission;
        for(int k=0; k<N_LINES-1; k++)
        {
            if(ptr1->admission 		< timestamp && timestamp <= ptr1->start_trial 	)	x_admission++;
            if(ptr1->start_trial 	< timestamp && timestamp <= ptr1->end_trial 	)	x_trial++;
            if(ptr1->end_trial 		< timestamp && timestamp <= ptr1->start_medic 	)	x_waiting++;
            if(ptr1->start_medic 	< timestamp && timestamp <= ptr1->end_medic 	)	x_medic++;
            ptr1++;
        }
        sprintf(buf,"%d,%ld,Espera_Triagem#%ld\n",timestamp, x_admission);
        write(destination,buf,strlen(buf));
        sprintf(buf,"%d,%ld,Triagem#%ld\n",timestamp, x_trial);
        write(destination,buf,strlen(buf));
        sprintf(buf,"%d,%ld,Sala_Espera#%ld\n",timestamp, x_waiting);
        write(destination,buf,strlen(buf));
        sprintf(buf,"%d,%ld,Consulta#%ld\n",timestamp, x_medic);
        write(destination,buf,strlen(buf));

        pthread_mutex_lock(&mutex_ocupations);
        ocupations++;
        pthread_mutex_unlock(&mutex_ocupations);
        x_admission = x_trial = x_waiting = x_medic = 0;
        ptr1=linesX;
        ptr2 += N_THREADS;
    }
    pthread_exit(0);

}


int main (int argc, char **argv, char **envp){
    int wcf,checkOcupations;
    char temp[1],wc[10];

    //-------------------------------READING INPUT-------------------------
    if(argc < 3){ perror("Insufficient arguements.\n"); exit(1);}
    system("wc -l < all_timestamps.csv > /tmp/wc.txt");
    wcf = open("/tmp/wc.txt", O_RDONLY);
    if(wcf == -1){ perror("Opening WCF...\n"); exit(1); }
    read(wcf, wc, sizeof(wc));
    N_LINES = atoi(wc);
    N_THREADS = atoi(argv[1]);
    destination = open(argv[3], O_CREAT | O_TRUNC | O_WRONLY, 0666);
    if(destination == -1) {perror("Opening Destination file...\n"); exit(1);}

    linesX = (LINES*) calloc(N_LINES, sizeof(LINES));
    FILE * fp = fopen("all_timestamps.csv", "r");
    if(fp == NULL){puts("Error opening file.\n"); exit(-1);}
    //-------------------------------------------------------------------\\

    fscanf(fp, "%*s", temp);    //Ignora a primeira linha do input
    LINES *ptr3 = linesX;       //Apontadores temporarios para o inicio da struct

    for (int i = 0; i < N_LINES; i++){ // N_LINES -1?
        fscanf(fp, "%ld %*[;] %ld %*[;] %ld %*[;] %ld %*[;] %ld %*[\n]", &(ptr3->admission), &(ptr3->start_trial), &(ptr3->end_trial), &(ptr3->start_medic), &(ptr3->end_medic));
        ptr3++;
    }
    fclose(fp);
    pthread_t workerThreads[N_THREADS];
    for(int i = 0; i < N_THREADS; i++){pthread_create(&workerThreads[i], NULL, threadsOcupations, (void*)i);}
    while(checkOcupations<N_LINES){
        sleep(1);
        printf("\n");
        printf("Ocupations calculated = %d\n",ocupations);
        pthread_mutex_lock(&mutex_ocupations);
        checkOcupations = ocupations;
        pthread_mutex_unlock(&mutex_ocupations);
    }
    for(int k = 0; k <N_THREADS; k++){pthread_join(workerThreads[k],NULL);}
    close(destination);
    return 0;
}