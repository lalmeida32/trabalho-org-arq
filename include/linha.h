/*
    Funções para trabalhar com registros LINHA.
    Maiores detalhes sobre cada uma dessas funções no arquivo linha.c
    ou no arquivo operador-generico.h
*/

#ifndef _LINHA_H_
#define _LINHA_H_

#include <global.h>
#include <stdio.h>
#include <operador-generico.h>

void *linha_gerarCabecalho();
void *linha_gerarRegistro();
void linha_freeRegistroInternamente(void *linhaVoid);
void linha_lerCabecalhoCsv(FILE *csv, BOOLEAN *vParada, void *cabecalhoVoid);
void linha_ioCabecalhoBin(FILE *bin, void *cabecalhoVoid, size_t (*f)(void *, size_t, size_t, FILE *));
BOOLEAN linha_lerRegistroCsv(FILE *csv, BOOLEAN *vParada, void *linhaVoid, char *buffer);
void linha_escreverRegistroBin(FILE *bin, void *linhaVoid);
void linha_lerRegistroBin(FILE *bin, void *linhaVoid, BOOLEAN lerRegistroBase);
void linha_printRegistro(FILE *saida, void *linhaVoid, void *cabecalhoVoid);
BOOLEAN linha_checarMatchRegistro(void *linhaVoid, char *nomeDoCampo, char *valorDoCampo);
void linha_lerRegistroEntradaPadrao(void *linhaVoid, char *buffer);
CabecalhoBase *linha_retornarCabecalhoBase(void *cabecalhoVoid);
RegistroBase *linha_retornarRegistroBase(void *linhaVoid);
int linha_retornaIdentificador(void *linhaVoid);
char *linha_retornaNomeIdentificador();
int linha_converteIdentificador(char *valorDoCampo);
BOOLEAN linha_ordena(void **linhasVoid, int numeroDeRegistros, char *nomeDoCampo);
int linha_getCodLinha(void *linhaVoid);
FuncoesGenericas *linha_funcoesGenericas();

#endif