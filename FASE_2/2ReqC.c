#include "library.h"

//Variaveis globais
int ocupations = 0;
int N_PRODS=0, N_CONS=0, N_LINES, N_THREADS;
int destination, prod_counter = 0, cons_counter = 0;
LINES* linesX;
PRODUCT prateleira[N];
int ptr_prod = 0, ptr_cons = 0;
pthread_mutex_t mutex_prod = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cons = PTHREAD_MUTEX_INITIALIZER;
sem_t run_prod, run_cons;

char *year_timestamp(long x){
    time_t year = x;
    struct tm time;
    time = *localtime(&year);
    char *buf[MAX_BUF];
    strftime(*buf,sizeof(buf),"%Y",&time);
    return *buf;
}


PRODUCT produce(void* params,int val){
    int j = (int) params;
    long x_admission = 0, x_trial = 0, x_waiting=0, x_medic=0;
    LINES* new_ptr = linesX;
    LINES* ptr_incr = linesX; //Incrementa as linhas
    PRODUCT prod;
    new_ptr += j;
    long timestamp = new_ptr->admission;
    //printf("%s",year_timestamp(timestamp));
    switch(val){
        case 0: for (int k = 0; k < N_LINES -1; k++){
                if(ptr_incr->admission < timestamp && timestamp <= ptr_incr->start_trial){x_admission++; ptr_incr++;}
            }
            prod.tmp_stmp = timestamp;
            prod.ocup = x_admission;
            strcpy(prod.sala, "Espera_Triagem");
            break;
        case 1: for (int k = 0; k < N_LINES -1; k++){
                if(ptr_incr->start_trial 	< timestamp && timestamp <= ptr_incr->end_trial ){x_trial++; ptr_incr++;}
            }
            prod.tmp_stmp = timestamp;
            prod.ocup = x_trial;
            strcpy(prod.sala, "Triagem");
            break;
        case 2: for (int k = 0; k < N_LINES -1; k++){
                if(ptr_incr->end_trial 	< timestamp && timestamp <= ptr_incr->start_medic){x_waiting++; ptr_incr++;}
            }
            prod.tmp_stmp = timestamp;
            prod.ocup = x_trial;
            strcpy(prod.sala, "Sala_Espera");
            break;
        case 3: for (int k = 0; k < N_LINES -1; k++){
                if(ptr_incr->start_medic 	< timestamp && timestamp <= ptr_incr->end_medic){x_medic++; ptr_incr++;}
            }
            prod.tmp_stmp = timestamp;
            prod.ocup = x_trial;
            strcpy(prod.sala, "Consulta");
            break;
    }return prod;
}

void* producer(void* params){

    while(prod_counter < N_LINES * COLS ){
        for(int i = 0; i < COLS; i++){

            PRODUCT prod = produce(params, i);
            sem_wait(&run_prod);
            pthread_mutex_lock(&mutex_prod);
            prateleira[ptr_prod] = prod;
            ptr_prod=(ptr_prod+1) % N;
            prod_counter++; //flag
            pthread_mutex_unlock(&mutex_prod);
            sem_post(&run_cons);
        }

    }
}

void* consumer(){

    while(cons_counter < N_LINES*COLS){
        PRODUCT item;
        sem_wait(&run_cons);
        pthread_mutex_lock(&mutex_cons);
        item=prateleira[ptr_cons];
        consume(item);
        ptr_cons=(ptr_cons+1)%N;
        cons_counter++;
        pthread_mutex_unlock(&mutex_cons);
        sem_post(&run_prod);
    }
}

void consume(PRODUCT prod){
    char arr[100];
    sprintf(arr,"%ld,%s#%ld\n",prod.tmp_stmp, prod.sala, prod.ocup);
    write(destination,arr,strlen(arr));
}



int main (int argc, char **argv, char **envp){
    int wcf,checkOcupations;
    char temp[1],wc[10];

    if(argc < 3){ perror("Insufficient arguements.\n"); exit(1);}
    system("wc -l < all_timestamps.csv > /tmp/wc.txt");
    wcf = open("/tmp/wc.txt", O_RDONLY);
    if(wcf == -1){ perror("Opening WCF...\n"); exit(1); }
    read(wcf, wc, sizeof(wc));
    N_LINES = atoi(wc);
    //N_THREADS = atoi(argv[1]);

    N_PRODS = atoi(argv[1]);
    N_CONS =  atoi(argv[2]);

    //destination = open(argv[3], O_CREAT | O_TRUNC | O_WRONLY, 0666);
    //if(destination == -1) {perror("Opening Destination file...\n"); exit(1);}

    linesX = (LINES*) calloc(N_LINES, sizeof(LINES));
    FILE * fp = fopen("all_timestamps.csv", "r");
    if(fp == NULL){puts("Error opening file.\n"); exit(-1);}
    fscanf(fp, "%*s", temp);
    LINES *ptr3 = linesX; //Apontadores temporarios para o inicio da struct
    for (int i = 0; i < N_LINES -1; i++){
        fscanf(fp, "%ld %*[;] %ld %*[;] %ld %*[;] %ld %*[;] %ld %*[\n]", &(ptr3->admission), &(ptr3->start_trial), &(ptr3->end_trial), &(ptr3->start_medic), &(ptr3->end_medic));
        ptr3++;
    }
    fclose(fp);  

    pthread_t consumerThreads[N_CONS];
    pthread_t producerThreads[N_PRODS];

    sem_init(&run_prod, 0, N);
    sem_init(&run_cons, 0, 0);

    for(int i = 0; i < N_PRODS; i++){
        pthread_create(&producerThreads[i], NULL, producer, (void*)i);}
    for(int i = 0; i < N_CONS; i++){
        pthread_create(&consumerThreads[i], NULL, consumer, NULL);}
    for(int k = 0; k <N_PRODS; k++){
        pthread_join(producerThreads[k],NULL);}
    for(int k = 0; k <N_CONS; k++){
        pthread_join(consumerThreads[k],NULL);}

    close(destination);
    return 0;
}