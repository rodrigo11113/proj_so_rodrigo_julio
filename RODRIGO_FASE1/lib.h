#ifndef _LIB_H
#define _LIB_H
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
void insert_lista(HOSPITAIS *pcs,long  admissao,long inicio_triagem,long fim_triagem,long inicio_medico,long fim_medico);
void read_lista (HOSPITAIS *hp, char * path);
void print_clientes(HOSPITAIS cs);
#define BUF_SIZE  4096

#endif /* _LIB_H */

