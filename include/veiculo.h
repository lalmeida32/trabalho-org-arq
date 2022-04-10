/*
    Funções para trabalhar com registros VEICULO.
    Maiores detalhes sobre cada uma dessas funções no arquivo veiculo.c
    ou no arquivo operador-generico.h
*/

#ifndef _VEICULO_H_
#define _VEICULO_H_

#include <global.h>
#include <stdio.h>
#include <operador-generico.h>

void *veiculo_gerarCabecalho();
void *veiculo_gerarRegistro();
void veiculo_freeRegistroInternamente(void *veiculoVoid);
void veiculo_lerCabecalhoCsv(FILE *csv, BOOLEAN *vParada, void *cabecalhoVoid);
void veiculo_ioCabecalhoBin(FILE *bin, void *cabecalhoVoid, size_t (*f)(void *, size_t, size_t, FILE *));
BOOLEAN veiculo_lerRegistroCsv(FILE *csv, BOOLEAN *vParada, void *veiculoVoid, char *buffer);
void veiculo_escreverRegistroBin(FILE *bin, void *veiculoVoid);
void veiculo_lerRegistroBin(FILE *bin, void *veiculoVoid, BOOLEAN lerRegistroBase);
void veiculo_printRegistro(FILE *saida, void *veiculoVoid, void *cabecalhoVoid);
BOOLEAN veiculo_checarMatchRegistro(void *veiculoVoid, char *nomeDoCampo, char *valorDoCampo);
void veiculo_lerRegistroEntradaPadrao(void *veiculoVoid, char *buffer);
CabecalhoBase *veiculo_retornarCabecalhoBase(void *cabecalhoVoid);
RegistroBase *veiculo_retornarRegistroBase(void *veiculoVoid);
int veiculo_retornaIdentificador(void *veiculoVoid);
char *veiculo_retornaNomeIdentificador();
int veiculo_converteIdentificador(char *valorDoCampo);
BOOLEAN veiculo_ordena(void **veiculosVoid, int numeroDeRegistros, char *nomeDoCampo);
int veiculo_getCodLinha(void *veiculoVoid);
FuncoesGenericas *veiculo_funcoesGenericas();

#endif