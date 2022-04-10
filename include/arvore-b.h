/*
    Esse módulo é um TAD para operar com um arquivo de índice utilizando
        a estrutura Árvore B.
    Mais informações sobre as funções podem ser encontradas no arquivo .c
*/

#ifndef _ARVORE_B_H_
#define _ARVORE_B_H_

#include <stdio.h>
#include <global.h>

typedef struct ArvoreB_t ArvoreB;

void arvoreB_inicializarCabecalho(ArvoreB *arvoreB, char status);
void arvoreB_atualizarCabecalho(ArvoreB *arvoreB);
ArvoreB *arvoreB_criar(FILE *arquivo, int ordem, int tamanhoNo, BOOLEAN sincronizar);
void arvoreB_free(ArvoreB *arvoreB, BOOLEAN freeArquivo);
void arvoreB_setStatus(ArvoreB *arvoreB, char status);
char arvoreB_getStatus(ArvoreB *arvoreB);
BOOLEAN arvoreB_inserir(ArvoreB *arvoreB, int chaveId, long long int chaveByteOffset);
long long int arvoreB_buscar(ArvoreB *arvoreB, int chave);

#endif