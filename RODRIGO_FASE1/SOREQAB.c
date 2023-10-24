#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include "lib.h"

int main(int argc,const char *argv[]) {
    if (argc != 4)
        exit(0);
    //char filename[]="C:\\Users\\ACER-PC\\Documents\\So_projeto_parte1\\all_timestamps.csv";
    char *filename = argv[2];
    int output = open(argv[3], O_CREAT | O_TRUNC | O_WRONLY, 0666);
    if (output == -1) {
        perror("Opening Destination file...\n");
        exit(1);
    }
    HOSPITAIS hp = {0, NULL, NULL};
    int nfilhos = (int) atoi(argv[1]);
    pid_t child[nfilhos];
    char buf[BUF_SIZE];
    read_lista(&hp, filename);
    HOSPITAL *inicio = hp.phead;
    HOSPITAL *verifica = hp.phead;
    long sala_espera = 0, espera_triagem = 0, triagem = 0, consulta = 0;
    for (int i = 0; i < nfilhos; i++) {
        if ((child[i] = fork()) == -1) {
            perror("Fork Error");
            exit(1);
        }
        if (child[i] == 0) {
            verifica += i;
            for (int j = i; j < hp.n - 1; ++j) {
                for (int k = j - 1; k > 0; k--) {

                    if (verifica->admissao <= inicio->inicio_triagem && verifica->admissao <inicio->admissao)
                        espera_triagem++;
                    if (verifica->inicio_triagem <= inicio->fim_triagem && verifica->inicio_triagem<inicio->inicio_triagem)
                        triagem++;
                    if (verifica->fim_triagem <= inicio->inicio_medico && verifica->fim_triagem<inicio->inicio_medico )
                        sala_espera++;
                    if (verifica->inicio_medico <= inicio->fim_medico && verifica->inicio_medico<inicio->inicio_medico)
                        consulta++;
                        inicio++;
                }
                sprintf(buf, "%d$%d,%ld,espera_triagem#%ld\n", getpid(), j, verifica->admissao, espera_triagem);
                write(output, buf, strlen(buf));
                sprintf(buf, "%d$%d,%ld,sala_triagem#%ld\n", getpid(), j, verifica->inicio_triagem, triagem);
                write(output, buf, strlen(buf));
                sprintf(buf, "%d$%d,%ld,sala_espera#%ld\n", getpid(), j, verifica->fim_triagem, sala_espera);
                write(output, buf, strlen(buf));
                sprintf(buf, "%d$%d,%ld,sala_consulta#%ld\n", getpid(), j, verifica->inicio_medico, consulta);
                write(output, buf, strlen(buf));
                inicio = hp.phead;
                verifica += nfilhos;
                sala_espera = 0; espera_triagem = 0; triagem = 0; consulta = 0;
            }
            exit(0);
        }
    }
    for (int l = 0; l < nfilhos; l++) {
        int result;
        waitpid(child[l], &result, 0);
        printf("Process %d terminatedn\n", child[l]);

    }
    close(output);
    return 0;
}

void read_lista (HOSPITAIS *hp, char * path) {
    long bytes, total=0, size;
long a,b,c,d,e;
    char * cds=NULL;
    char * token;
    char needle[] = "\n";
    int fd = open(path, O_RDONLY);
    if (fd == -1) { perror("File open"); exit(1); }

    size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    cds = (char *) malloc(sizeof(char) * (size+1));
    while ((bytes = read(fd, cds+total, BUF_SIZE)))
        total += bytes;

    close(fd);
    token = strtok(cds, needle);
    while (token != NULL) {
        sscanf(token, "%ld %*[;] %ld %*[;] %ld %*[;] %ld %*[;] %ld %*[\n]",&a,&b,&c,&d,&e);
        insert_lista(hp,a,b,c,d,e);
        token = strtok(NULL, needle);
    }

 free(cds);

    }



void insert_lista(HOSPITAIS *pcs,long  admissao,long inicio_triagem,long fim_triagem,long inicio_medico,long fim_medico)
{
        HOSPITAL *temp = (HOSPITAL *)malloc(sizeof(HOSPITAL));
        temp->pnext=NULL;
        temp->admissao=admissao;
        temp->inicio_triagem=inicio_triagem;
        temp->fim_triagem=fim_triagem;
        temp->inicio_medico=inicio_medico;
        temp->fim_medico=fim_medico;

   if(pcs->phead==NULL){
       temp->pbefore=NULL;
       pcs->phead=temp;
       pcs->ptail=temp;
       pcs->n++;
   }
  else if(pcs->ptail==pcs->phead){
          temp->pbefore=pcs->phead;
         pcs->ptail=temp;
       pcs->phead->pnext=pcs->ptail;
         pcs->n++;
    }
    else{
        temp->pbefore=pcs->ptail;
        pcs->ptail->pnext=temp;
        pcs->ptail=temp;
        pcs->n++;
    }}
void print_clientes(HOSPITAIS cs)
{
    HOSPITAL *pc = cs.phead;
    for (int i = 0; i < cs.n; i++)
    {

            printf("CLIENTE[%ld] |\n", pc->admissao );
            pc = pc->pnext;
        }



}

