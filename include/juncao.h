/*
    Este módulo tem a função de aplicar um algoritmo de junção sobre
    dois registros.

    Campos suportados:
        codLinha (Veiculo + Linha) 
*/

#ifndef _JUNCAO_H_
#define _JUNCAO_H_

#include <global.h>

#define JUNCAO_NUMERO_DE_ALGORITMOS 3

typedef enum
{
    JUNCAO_LOOP_ANINHADO,
    JUNCAO_LOOP_UNICO,
    JUNCAO_ORDENACAO_INTERCALACAO,
} JUNCAO_ALGORITMO;

void juncao_veiculoLinha(BOOLEAN *vParada, char *buffer, JUNCAO_ALGORITMO algoritmo);

#endif