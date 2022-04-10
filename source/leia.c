/*
    Este arquivo possui funções para facilitar leitura.
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <global.h>

/*

    VETOR DE PARADA

*/

/* 
    Essa função gera um vetor de BOOLEAN "vParada", alocado dinamicamente.
    Nesse vetor, estão os caracteres de parada da leitura.
    Esse vetor permite a execução de funções da biblioteca leia de forma otimizada.
*/
BOOLEAN *vetorDeParada_criar(const char *caracteres)
{
    BOOLEAN *vParada = (BOOLEAN *)calloc((UCHAR_MAX + 1), sizeof(BOOLEAN));
    for (int i = 0; caracteres[i]; i++)
        vParada[(unsigned char)caracteres[i]] = true;
    return vParada;
}

/*
    As funções a seguir alteram um dos caracteres definidos em "vParada".
*/
void vetorDeParada_modificarCaractere(BOOLEAN *vParada, char c, BOOLEAN modificacao)
{
    if (!vParada)
        return;
    vParada[(unsigned char)c] = modificacao;
}

void vetorDeParada_adicionarCaractere(BOOLEAN *vParada, char c)
{
    vetorDeParada_modificarCaractere(vParada, c, true);
}

void vetorDeParada_removerCaractere(BOOLEAN *vParada, char c)
{
    vetorDeParada_modificarCaractere(vParada, c, false);
}

/*

    LEIA

*/

/*
    Essa função limpa a entrada até um dos caracteres em "vParada" ou fim de arquivo.
*/
void leia_limparEntrada(FILE *entrada, BOOLEAN *vParada)
{
    int c;
    do
    {
        c = fgetc(entrada);
    } while (c != EOF && !vParada[c]);
}

/*
    Essa função lê um inteiro e limpa a entrada até um dos caracteres em "vParada" ou fim de arquivo.
*/
int leia_lerInteiro(FILE *entrada, BOOLEAN *vParada)
{
    int inteiro;
    fscanf(entrada, "%d", &inteiro);
    leia_limparEntrada(entrada, vParada);
    return inteiro;
}

/*
    Essa função lê uma string de um arquivo de entrada e armazena em buffer. A leitura é interrompida quando:
        a. um dos caracteres de "vParada" é atingido
        b. o tamanho máximo possível é atingido
        c. se chega no fim do arquivo
    No caso (b), a entrada é limpa até a condição (a) ou (c) ser alcançada
    Além disso, pelo último parâmetro é possível decidir se o caractere '\0' será colocado ao fim ou não
*/
size_t leia_lerString(FILE *entrada, BOOLEAN *vParada, char *buffer, size_t tamanhoMaximo, BOOLEAN colocarTerminador)
{

    // Ler byte a byte e colocar na string
    size_t tamanhoAtual = 0;
    int c;

    do
    {
        c = fgetc(entrada);
        buffer[tamanhoAtual++] = (char)c;

        // Verificando as três condições
    } while (c != EOF && tamanhoAtual < tamanhoMaximo && !vParada[c]);

    // Reajuste
    if (c != EOF && !vParada[c])
        leia_limparEntrada(entrada, vParada);

    if (colocarTerminador)
        buffer[tamanhoAtual - 1] = '\0';

    return tamanhoAtual;
}