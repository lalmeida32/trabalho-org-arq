/*
    Trabalho prático 3 - Organização de Arquivos
    
    Nome: Lucas Ferreira de Almeida
    NUSP: 11262063

    2021

*/

#include <leia.h>
#include <global.h>
#include <stdio.h>
#include <stdlib.h>
#include <funcionalidades.h>

int main(void)
{
    /*
        Buffer servirá para armazenar string temporariamente em todo o código.
        vParada servirá para indicar quando uma string ou inteiro deve parar de ser lido.
            irá armazenar, por exemplo, que os caracteres ' ' e '\n' são condição de parada
            da leitura.
    */
    char *buffer;
    BOOLEAN *vParada;

    global_gerarBufferVParada(&buffer, &vParada, " \n");

    // Vetor de ponteiros para funções de funcionalidades
    void (*funcionalidades[NUMERO_DE_FUNCIONALIDADES + 1])(BOOLEAN *, char *) = {
        NULL,
        funcionalidade1,
        funcionalidade2,
        funcionalidade3,
        funcionalidade4,
        funcionalidade5,
        funcionalidade6,
        funcionalidade7,
        funcionalidade8,
        funcionalidade9,
        funcionalidade10,
        funcionalidade11,
        funcionalidade12,
        funcionalidade13,
        funcionalidade14,
        funcionalidade15,
        funcionalidade16,
        funcionalidade17,
        funcionalidade18,
        funcionalidade19,
    };

    // Ler a funcionalidade escolhida e, caso ela esteja no intervalo correto, chamar a respectiva funcionalidade
    int escolha = leia_lerInteiro(stdin, vParada);
    if (escolha <= NUMERO_DE_FUNCIONALIDADES && escolha > 0)
        funcionalidades[escolha](vParada, buffer);

    global_freeBufferVParada(buffer, vParada);
    return 0;
}