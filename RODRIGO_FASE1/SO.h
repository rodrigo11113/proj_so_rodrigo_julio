//
// Created by ACER-PC on 28/04/2022.
//

#ifndef SO_PROJETO_PARTE1_SO_H
#define SO_PROJETO_PARTE1_SO_H
typedef struct hospital
{
    long  admissao;
    long inicio_triagem;
    long fim_triagem;
    long inicio_medico;
    long fim_medico;
    struct hospital *pnext,*pbefore;
}HOSPITAL;
typedef struct hospitais
{
    long n;
    HOSPITAL *phead,*ptail;
}HOSPITAIS;
//void insert_lista(HOSPITAIS *pcs,long  admissao,long inicio_triagem,long fim_triagem,long inicio_medico,long fim_medico);
//void read_lista (HOSPITAIS *hp, char * path);
//void print_clientes(HOSPITAIS cs);
ssize_t readn(int fd, void *ptr, size_t n);
ssize_t writen(int fd, const void *ptr, size_t n);
#define BUF_SIZE  4096
#endif //SO_PROJETO_PARTE1_SO_H
