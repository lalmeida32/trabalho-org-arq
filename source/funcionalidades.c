/*
    Este arquivo contém funções que chamam uma das operações genéricas,
    passando qual estrutura deve ser operada por aquela operação por meio de uma
    struct "FuncoesGenericas" 
*/

#include <global.h>
#include <operador-generico.h>
#include <veiculo.h>
#include <stdlib.h>
#include <linha.h>
#include <juncao.h>

/*
    Essa função auxiliar recebe uma função do "operador genérico"
    e qual função deve ser utilizada para gerar uma struct FuncoesGenericas
    
    Exemplo de uso:
        chamarFuncaoPassandoGenericas(vParada, buffer, &opGen_lerCsvEscreverBin, &veiculo_funcoesGenericas);
        
        irá executar opGen_lerCsvEscreverBin para VEICULOS
*/

static void chamarFuncaoPassandoGenericas(BOOLEAN *vParada, char *buffer,
                                          void (*funcao)(BOOLEAN *, char *, FuncoesGenericas *),
                                          FuncoesGenericas *(*geradorDeFuncoes)())
{
    FuncoesGenericas *funcoes = geradorDeFuncoes();
    funcao(vParada, buffer, funcoes);
    free(funcoes);
}

/*
    Ler dados do CSV e escrever no BINÁRIO
*/

void funcionalidade1(BOOLEAN *vParada, char *buffer)
{
    // Veículo
    chamarFuncaoPassandoGenericas(vParada, buffer, &opGen_lerCsvEscreverBin, &veiculo_funcoesGenericas);
}

void funcionalidade2(BOOLEAN *vParada, char *buffer)
{
    // Linha
    chamarFuncaoPassandoGenericas(vParada, buffer, &opGen_lerCsvEscreverBin, &linha_funcoesGenericas);
}

/*
    Listar todos os registros dentro de um BINÁRIO
*/

void funcionalidade3(BOOLEAN *vParada, char *buffer)
{
    // Veículo
    chamarFuncaoPassandoGenericas(vParada, buffer, &opGen_listarRegistrosBin, &veiculo_funcoesGenericas);
}

void funcionalidade4(BOOLEAN *vParada, char *buffer)
{
    // Linha
    chamarFuncaoPassandoGenericas(vParada, buffer, &opGen_listarRegistrosBin, &linha_funcoesGenericas);
}

/*
    Buscar registros dentro de um BINÁRIO
*/

void funcionalidade5(BOOLEAN *vParada, char *buffer)
{
    // Veículo
    chamarFuncaoPassandoGenericas(vParada, buffer, &opGen_buscarRegistrosBin, &veiculo_funcoesGenericas);
}

void funcionalidade6(BOOLEAN *vParada, char *buffer)
{
    // Linha
    chamarFuncaoPassandoGenericas(vParada, buffer, &opGen_buscarRegistrosBin, &linha_funcoesGenericas);
}

/*
    Adicionar registros ao fim de um BINÁRIO
*/

void funcionalidade7(BOOLEAN *vParada, char *buffer)
{
    // Veículo
    chamarFuncaoPassandoGenericas(vParada, buffer, &opGen_adicionarRegistrosBin, &veiculo_funcoesGenericas);
}

void funcionalidade8(BOOLEAN *vParada, char *buffer)
{
    // Linha
    chamarFuncaoPassandoGenericas(vParada, buffer, &opGen_adicionarRegistrosBin, &linha_funcoesGenericas);
}

/*
    Criar ÍNDICE para um arquivo de dados existente, usando Árvore B
*/

void funcionalidade9(BOOLEAN *vParada, char *buffer)
{
    // Veiculo
    chamarFuncaoPassandoGenericas(vParada, buffer, &opGen_criarIndice, &veiculo_funcoesGenericas);
}
void funcionalidade10(BOOLEAN *vParada, char *buffer)
{
    // Linha
    chamarFuncaoPassandoGenericas(vParada, buffer, &opGen_criarIndice, &linha_funcoesGenericas);
}

/*
    Buscar um registro no arquivo de dados utilizando um arquivo de ÍNDICE que implementa Árvore B
*/

void funcionalidade11(BOOLEAN *vParada, char *buffer)
{
    chamarFuncaoPassandoGenericas(vParada, buffer, &opGen_buscarComIndice, &veiculo_funcoesGenericas);
}
void funcionalidade12(BOOLEAN *vParada, char *buffer)
{
    chamarFuncaoPassandoGenericas(vParada, buffer, &opGen_buscarComIndice, &linha_funcoesGenericas);
}

/*
    Adicionar registros em um arquivo de dados e de ÍNDICE que utiliza Árvore B.
*/

void funcionalidade13(BOOLEAN *vParada, char *buffer)
{
    chamarFuncaoPassandoGenericas(vParada, buffer, &opGen_adicionarRegistrosComIndice, &veiculo_funcoesGenericas);
}
void funcionalidade14(BOOLEAN *vParada, char *buffer)
{
    chamarFuncaoPassandoGenericas(vParada, buffer, &opGen_adicionarRegistrosComIndice, &linha_funcoesGenericas);
}

/*
    Aplicar algoritmos de junção sobre os registros de veículo e linha.
*/

void funcionalidade15(BOOLEAN *vParada, char *buffer)
{
    juncao_veiculoLinha(vParada, buffer, JUNCAO_LOOP_ANINHADO);
}
void funcionalidade16(BOOLEAN *vParada, char *buffer)
{
    juncao_veiculoLinha(vParada, buffer, JUNCAO_LOOP_UNICO);
}

/* Ordenar registros de um arquivo de dados */

void funcionalidade17(BOOLEAN *vParada, char *buffer)
{
    chamarFuncaoPassandoGenericas(vParada, buffer, &opGen_ordenaArquivoDeDados, &veiculo_funcoesGenericas);
}
void funcionalidade18(BOOLEAN *vParada, char *buffer)
{
    chamarFuncaoPassandoGenericas(vParada, buffer, &opGen_ordenaArquivoDeDados, &linha_funcoesGenericas);
}

/*
    Aplicar o algoritmo de junção "ordenação-intercalação" sobre os registros de veículo e linha.
*/

void funcionalidade19(BOOLEAN *vParada, char *buffer)
{
    juncao_veiculoLinha(vParada, buffer, JUNCAO_ORDENACAO_INTERCALACAO);
}