/*
    Este arquivo possui funções variadas e úteis para diversos outros
    arquivos deste programa.
*/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdio.h>

/*
    ENUMS
*/

typedef enum
{
    false,
    true
} BOOLEAN;

/*
    STRUCTS
*/

typedef struct
{
    char status;
    long long int byteProxReg;
    int nroRegistros;
    int nroRegRemovidos;
} CabecalhoBase;

typedef struct
{
    char removido;
    int tamanhoRegistro;
} RegistroBase;

/*
    CONSTANTS
*/

#define TAMANHO_BUFFER 100
#define CAMPO_NULO "campo com valor nulo"

#define ERRO_FALHA_ARQUIVO_MENSAGEM "Falha no processamento do arquivo."
#define ERRO_REGISTRO_MENSAGEM "Registro inexistente."

#define ERRO_FALHA_ARQUIVO -1
#define ERRO_REGISTRO -2
#define SUCESSO 0

#define ARVORE_B_ORDEM 5
#define ARVORE_B_TAMANHO_NO 77

/*
    FUNCOES
*/

/* Detalhes de cada uma delas no arquivo .c */

// buffer e vParada
void global_gerarBufferVParada(char **buffer, BOOLEAN **vParada, const char *caracteres);
void global_freeBufferVParada(char *buffer, BOOLEAN *vParada);

// CabecalhoBase
void global_cabecalhoBaseDefault(CabecalhoBase *base);
void global_ioCabecalhoBaseBin(FILE *bin, CabecalhoBase *base, size_t (*f)(void *, size_t, size_t, FILE *));

// RegistroBase
void global_ioRegistroBaseBin(FILE *bin, RegistroBase *base, size_t (*f)(void *, size_t, size_t, FILE *));

// Tratamento de campos "NULO"
void global_tratarStringTamanhoFixo(char *buffer, int tamanhoDoCampo, char *string, const char *comparacao);
void global_tratarInteiro(char *buffer, int *pInteiro, const char *comparacao);
void global_tratarStringTamanhoVariavel(char *buffer, int *pTamanho, char **pString, const char *comparacao);

// Imprimir se "NULO"
BOOLEAN global_printNuloStringTamanhoFixo(FILE *saida, char *string);
BOOLEAN global_printNuloInteiro(FILE *saida, int inteiro);
BOOLEAN global_printNuloStringTamanhoVariavel(FILE *saida, int tamanhoString);

// Verificar match
BOOLEAN global_matchStringTamanhoFixo(const char *campo, const char *valorDoCampoDado, int tamanhoDoCampo);
BOOLEAN global_matchInteiro(int campo, const char *valorDoCampoDado);
BOOLEAN global_matchStringTamanhoVariavel(const char *campo, const char *valorDoCampoDado, int tamanhoDoCampo);

// Escrever e ler de arquivos binários
void global_lerStringTamanhoVariavelBin(FILE *bin, int *pTamanho, char **string);
void global_escreverStringTamanhoVariavelBin(FILE *bin, int *pTamanho, char *string);
void global_atualizarStatusBin(FILE *bin, char status, CabecalhoBase *cabecalhoBase);
char global_lerStatusBin(FILE *bin);

// Tratamento de erros
void imprimeErro(int erro);

#endif