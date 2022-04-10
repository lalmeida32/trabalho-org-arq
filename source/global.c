/*
    Este arquivo possui funções variadas e úteis para diversos outros
    arquivos deste programa.
*/

#include <global.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <leia.h>

/*

    buffer e vParada

*/

/*
    Essa função gera um buffer e um vetor de parada de uma vez só.
*/
void global_gerarBufferVParada(char **buffer, BOOLEAN **vParada, const char *caracteres)
{
    *buffer = (char *)malloc(TAMANHO_BUFFER * sizeof(char));
    *vParada = vetorDeParada_criar(caracteres);
}

/*
    Essa função libera a memória de um buffer e de um vetor de parada
    de uma vez só.
*/
void global_freeBufferVParada(char *buffer, BOOLEAN *vParada)
{
    free(buffer);
    free(vParada);
}

/*

    CabecalhoBase

*/

/*
    Esta função inicializa um cabeçalho base com valores padrão.
*/
void global_cabecalhoBaseDefault(CabecalhoBase *base)
{
    base->status = '0';
    base->byteProxReg = 0;
    base->nroRegistros = 0;
    base->nroRegRemovidos = 0;
}

/*
    Esta função lê ou escreve um cabeçalho base dentro de um binário.
    O último argumento pode ser &fread ou &fwrite
*/
void global_ioCabecalhoBaseBin(FILE *bin, CabecalhoBase *base, size_t (*f)(void *, size_t, size_t, FILE *))
{
    f(&base->status, sizeof(char), 1, bin);
    f(&base->byteProxReg, sizeof(long long int), 1, bin);
    f(&base->nroRegistros, sizeof(int), 1, bin);
    f(&base->nroRegRemovidos, sizeof(int), 1, bin);
}

/*

    RegistroBase

*/

/*
    Esta função lê ou escreve um registro base dentro de um binário.
    O último argumento pode ser &fread ou &fwrite
*/
void global_ioRegistroBaseBin(FILE *bin, RegistroBase *base, size_t (*f)(void *, size_t, size_t, FILE *))
{
    f(&base->removido, sizeof(char), 1, bin);
    f(&base->tamanhoRegistro, sizeof(int), 1, bin);
}

/*

    Tratamento de valor NULO

*/

/*
    Torna uma string fixa nula (no formato '\0@@@@')
*/
static void stringFixaNula(char *str, int tamanho)
{
    str[0] = '\0';
    for (int i = 1; i < tamanho; i++)
        str[i] = '@';
}

/*
    Compara buffer com a string comparacao.
    Caso comparacao não exista (NULL), define como a string "NULO".
*/
static BOOLEAN stringNulaComparacao(char *buffer, const char *comparacao)
{
    if (!comparacao)
        comparacao = "NULO";
    return !strcmp(buffer, comparacao);
}

/*
    Verifica o valor especial "NULO" para uma string de tamanho fixo.
*/
void global_tratarStringTamanhoFixo(char *buffer, int tamanhoDoCampo, char *string, const char *comparacao)
{
    if (stringNulaComparacao(buffer, comparacao))
        stringFixaNula(string, tamanhoDoCampo);
    else
        memcpy(string, buffer, tamanhoDoCampo);
}

/*
    Verifica o valor especial "NULO" para um valor inteiro.
*/
void global_tratarInteiro(char *buffer, int *pInteiro, const char *comparacao)
{
    if (stringNulaComparacao(buffer, comparacao))
        *pInteiro = -1;
    else
        *pInteiro = atoi(buffer);
}

/*
    Verifica o valor especial "NULO" para uma string de tamanho variável.
    (é necessário desalocar a memória de *pString posteriormente no código)
*/
void global_tratarStringTamanhoVariavel(char *buffer, int *pTamanho, char **pString, const char *comparacao)
{
    if (stringNulaComparacao(buffer, comparacao))
        *pTamanho = 0;
    else
    {
        *pTamanho = strlen(buffer);
        if (*pTamanho > 0)
        {
            *pString = (char *)malloc(sizeof(char) * *pTamanho);
            memcpy(*pString, buffer, *pTamanho);
        }
    }
}

/*

    Print se campo é nulo

*/

/*
    Se uma determinada condição for satisfeita, imprime a string em
    CAMPO_NULO no arquivo de saída.
*/
static BOOLEAN printNulo(FILE *saida, BOOLEAN condicao)
{
    if (condicao)
    {
        fprintf(saida, CAMPO_NULO);
        return true;
    }
    return false;
}

/*
    Checa e imprime, caso necessário, a string em CAMPO_NULO caso
        a string de tamanho fixo dada seja nula. 
*/
BOOLEAN global_printNuloStringTamanhoFixo(FILE *saida, char *string)
{
    return printNulo(saida, string[0] == '\0');
}

/*
    Checa e imprime, caso necessário, a string em CAMPO_NULO caso
        o valor inteiro dado seja nulo. 
*/
BOOLEAN global_printNuloInteiro(FILE *saida, int inteiro)
{
    return printNulo(saida, inteiro < 0);
}

/*
    Checa e imprime, caso necessário, a string em CAMPO_NULO caso
        a string de tamanho variável dada seja nula. 
*/
BOOLEAN global_printNuloStringTamanhoVariavel(FILE *saida, int tamanhoString)
{
    return printNulo(saida, tamanhoString == 0);
}

/*

    Match

*/

/*
    Retorna TRUE se a string de tamanho fixo dada bate com o valor do campo.
*/
BOOLEAN global_matchStringTamanhoFixo(const char *campo, const char *valorDoCampoDado, int tamanhoDoCampo)
{
    return !memcmp(campo, valorDoCampoDado, tamanhoDoCampo);
}

/*
    Retorna TRUE se o valor inteiro dado bate com o valor do campo.
*/
BOOLEAN global_matchInteiro(int campo, const char *valorDoCampoDado)
{
    return campo == atoi(valorDoCampoDado);
}

/*
    Retorna TRUE se a string de tamanho variável dada bate com o valor do campo.
*/
BOOLEAN global_matchStringTamanhoVariavel(const char *campo, const char *valorDoCampoDado, int tamanhoDoCampo)
{
    return (tamanhoDoCampo == (int)strlen(valorDoCampoDado) && !memcmp(campo, valorDoCampoDado, tamanhoDoCampo));
}

/*

    Leitura/escrita binário

*/

/*
    Lê uma string de tamanho variável de um binário.
*/
void global_lerStringTamanhoVariavelBin(FILE *bin, int *pTamanho, char **string)
{
    fread(pTamanho, sizeof(int), 1, bin);
    const int tamanho = *pTamanho;
    if (tamanho > 0)
    {
        *string = (char *)malloc(sizeof(char) * tamanho);
        fread(*string, sizeof(char), tamanho, bin);
    }
}

/*
    Escreve uma string de tamanho variável em um binário.
*/
void global_escreverStringTamanhoVariavelBin(FILE *bin, int *pTamanho, char *string)
{
    fwrite(pTamanho, sizeof(int), 1, bin);
    const int tamanho = *pTamanho;
    if (tamanho > 0)
        fwrite(string, sizeof(char), tamanho, bin);
}

/*
    Atualiza o status do cabeçalho de um arquivo binário
*/
void global_atualizarStatusBin(FILE *bin, char status, CabecalhoBase *cabecalhoBase)
{
    cabecalhoBase->status = status;
    fseek(bin, 0L, SEEK_SET);
    fwrite(&status, sizeof(char), 1, bin);
    fflush(bin);
}

/*
    Ler status do cabeçalho de um arquivo binário
*/
char global_lerStatusBin(FILE *bin)
{
    char status;
    fseek(bin, 0L, SEEK_SET);
    fread(&status, sizeof(char), 1, bin);
    return status;
}

/*

    Tratamento de erros

*/

/*
    Verifica um erro. Se houver, imprime a respectiva mensagem.
*/
void imprimeErro(int erro)
{
    if (erro == ERRO_FALHA_ARQUIVO)
        printf(ERRO_FALHA_ARQUIVO_MENSAGEM);
    else if (erro == ERRO_REGISTRO)
        printf(ERRO_REGISTRO_MENSAGEM);
}