/*
    Este arquivo possui funções para facilitar leitura.
*/

#ifndef _LEIA_H_
#define _LEIA_H_

#include <global.h>
#include <stdio.h>

// VETOR DE PARADA
BOOLEAN *vetorDeParada_criar(const char *caracteres);
void vetorDeParada_adicionarCaractere(BOOLEAN *vParada, char c);
void vetorDeParada_removerCaractere(BOOLEAN *vParada, char c);

// LEIA
void leia_limparEntrada(FILE *entrada, BOOLEAN *vParada);
int leia_lerInteiro(FILE *entrada, BOOLEAN *vParada);
size_t leia_lerString(FILE *entrada, BOOLEAN *vParada, char *buffer, size_t tamanhoMaximo, BOOLEAN colocarTerminador);

#endif