#include <global.h>
#include <stdio.h>
#include <leia.h>
#include <linha.h>
#include <stdlib.h>
#include <string.h>
#include <funcao-fornecida.h>

/*

    CONSTANTES E STRUCTS

*/

#define TAMANHO_DESCREVE_CODIGO 15
#define TAMANHO_DESCREVE_CARTAO 13
#define TAMANHO_DESCREVE_NOME 13
#define TAMANHO_DESCREVE_LINHA 24

#define TAMANHO_INICIAL_LINHA 3 * sizeof(int) + sizeof(char)

#define MENSAGEM_CARTAO_S "PAGAMENTO SOMENTE COM CARTAO SEM PRESENCA DE COBRADOR"
#define MENSAGEM_CARTAO_N "PAGAMENTO EM CARTAO E DINHEIRO"
#define MENSAGEM_CARTAO_F "PAGAMENTO EM CARTAO SOMENTE NO FINAL DE SEMANA"

typedef struct
{
    CabecalhoBase base;
    char descreveCodigo[TAMANHO_DESCREVE_CODIGO];
    char descreveCartao[TAMANHO_DESCREVE_CARTAO];
    char descreveNome[TAMANHO_DESCREVE_NOME];
    char descreveLinha[TAMANHO_DESCREVE_LINHA];
} LinhaCabecalho;

typedef struct
{
    RegistroBase base;
    int codLinha;
    char aceitaCartao;
    int tamanhoNome;
    char *nomeLinha;
    int tamanhoCor;
    char *corLinha;
} Linha;

/*

    INTERFACE PRIVADA

*/

/*
    Função que recebe um caractere de "aceitaCartao"
    e imprime a mensagem correspondente.
*/
static void printCartao(FILE *saida, char aceitaCartao)
{
    if (aceitaCartao == 'S')
        fprintf(saida, "%s", MENSAGEM_CARTAO_S);
    else if (aceitaCartao == 'N')
        fprintf(saida, "%s", MENSAGEM_CARTAO_N);
    else if (aceitaCartao == 'F')
        fprintf(saida, "%s", MENSAGEM_CARTAO_F);
}

/*
    Função necessária para execução de qsort. Compara dois registros linha e
    retorna
        > 0 para codLinha1 > codLinha2
        = 0 para codLinha1 = codLinha2
        < 0 para codLinha1 < codLinha2
*/
static int comparaCodLinha(const void *linhaVoid1, const void *linhaVoid2)
{
    Linha *linha1 = (Linha *)(*((void **)linhaVoid1));
    Linha *linha2 = (Linha *)(*((void **)linhaVoid2));
    return linha1->codLinha - linha2->codLinha;
}

/*

    INTERFACE PÚBLICA

*/

/*
    As duas funções a seguir geram uma das duas estruturas
        definidas no arquivo, como ponteiro void (útil ao trabalhar
        com funções genéricas)
*/
void *linha_gerarCabecalho()
{
    return malloc(sizeof(LinhaCabecalho));
}

void *linha_gerarRegistro()
{
    return malloc(sizeof(Linha));
}

/*
    Essa função é responsável por liberar a memória interna do registro,
    mas mantê-lo alocado na memória.
*/
void linha_freeRegistroInternamente(void *linhaVoid)
{
    Linha *linha = (Linha *)linhaVoid;
    if (linha->tamanhoNome > 0)
        free(linha->nomeLinha);
    if (linha->tamanhoCor > 0)
        free(linha->corLinha);
}

/*
    Essa função lê um arquivo csv e armazena as informações em um
    LinhaCabecalho
*/
void linha_lerCabecalhoCsv(FILE *csv, BOOLEAN *vParada, void *cabecalhoVoid)
{
    LinhaCabecalho *cabecalho = (LinhaCabecalho *)cabecalhoVoid;
    global_cabecalhoBaseDefault(&cabecalho->base);
    leia_lerString(csv, vParada, cabecalho->descreveCodigo, TAMANHO_DESCREVE_CODIGO, false);
    leia_lerString(csv, vParada, cabecalho->descreveCartao, TAMANHO_DESCREVE_CARTAO, false);
    leia_lerString(csv, vParada, cabecalho->descreveNome, TAMANHO_DESCREVE_NOME, false);
    leia_lerString(csv, vParada, cabecalho->descreveLinha, TAMANHO_DESCREVE_LINHA, false);
}

/*
    Essa função serve para escrita e leitura de LinhaCabecalho em arquivos
    binários. Último parâmetro pode ser &fread ou &fwrite.
*/
void linha_ioCabecalhoBin(FILE *bin, void *cabecalhoVoid, size_t (*f)(void *, size_t, size_t, FILE *))
{
    LinhaCabecalho *cabecalho = (LinhaCabecalho *)cabecalhoVoid;
    global_ioCabecalhoBaseBin(bin, &cabecalho->base, f);
    f(cabecalho->descreveCodigo, sizeof(char), TAMANHO_DESCREVE_CODIGO, bin);
    f(cabecalho->descreveCartao, sizeof(char), TAMANHO_DESCREVE_CARTAO, bin);
    f(cabecalho->descreveNome, sizeof(char), TAMANHO_DESCREVE_NOME, bin);
    f(cabecalho->descreveLinha, sizeof(char), TAMANHO_DESCREVE_LINHA, bin);
}

/*
    Lê uma Linha de um arquivo csv.
    Retorna falso se não há mais registros pra ler daquele arquivo.
*/
BOOLEAN linha_lerRegistroCsv(FILE *csv, BOOLEAN *vParada, void *linhaVoid, char *buffer)
{
    Linha *linha = (Linha *)linhaVoid;

    leia_lerString(csv, vParada, buffer, TAMANHO_BUFFER, true);
    if (feof(csv)) // Checando se chegou no fim do arquivo.
        return false;

    // COD LINHA
    if (buffer[0] == '*') // Tratando para removido
    {
        linha->base.removido = '0';
        linha->codLinha = atoi(buffer + 1);
    }
    else // Tratando para não removido
    {
        linha->base.removido = '1';
        linha->codLinha = atoi(buffer);
    }

    // ACEITA CARTAO
    leia_lerString(csv, vParada, buffer, TAMANHO_BUFFER, true);
    global_tratarStringTamanhoFixo(buffer, sizeof(char), &linha->aceitaCartao, NULL);

    // TAMANHO NOME / NOME LINHA
    leia_lerString(csv, vParada, buffer, TAMANHO_BUFFER, true);
    global_tratarStringTamanhoVariavel(buffer, &linha->tamanhoNome, &linha->nomeLinha, NULL);

    // TAMANHO COR / COR LINHA
    leia_lerString(csv, vParada, buffer, TAMANHO_BUFFER, true);
    global_tratarStringTamanhoVariavel(buffer, &linha->tamanhoCor, &linha->corLinha, NULL);

    // TAMANHO REGISTRO
    linha->base.tamanhoRegistro = TAMANHO_INICIAL_LINHA + linha->tamanhoNome + linha->tamanhoCor;

    return true;
}

/*
    Escreve uma Linha em um arquivo binário.
*/
void linha_escreverRegistroBin(FILE *bin, void *linhaVoid)
{
    Linha *linha = (Linha *)linhaVoid;
    global_ioRegistroBaseBin(bin, &linha->base, (size_t(*)(void *, size_t, size_t, FILE *)) & fwrite);
    fwrite(&linha->codLinha, sizeof(int), 1, bin);
    fwrite(&linha->aceitaCartao, sizeof(char), 1, bin);
    global_escreverStringTamanhoVariavelBin(bin, &linha->tamanhoNome, linha->nomeLinha);
    global_escreverStringTamanhoVariavelBin(bin, &linha->tamanhoCor, linha->corLinha);
}

/*
    Lê uma Linha de um arquivo binário. É possível fazer com que
    a base do registro não seja lida.
*/
void linha_lerRegistroBin(FILE *bin, void *linhaVoid, BOOLEAN lerRegistroBase)
{
    Linha *linha = (Linha *)linhaVoid;
    if (lerRegistroBase)
        global_ioRegistroBaseBin(bin, &linha->base, &fread);
    fread(&linha->codLinha, sizeof(int), 1, bin);
    fread(&linha->aceitaCartao, sizeof(char), 1, bin);
    global_lerStringTamanhoVariavelBin(bin, &linha->tamanhoNome, &linha->nomeLinha);
    global_lerStringTamanhoVariavelBin(bin, &linha->tamanhoCor, &linha->corLinha);
}

/*
    Imprime no arquivo de saída um registro Linha.
*/
void linha_printRegistro(FILE *saida, void *linhaVoid, void *cabecalhoVoid)
{
    Linha *linha = (Linha *)linhaVoid;
    LinhaCabecalho *cabecalho = (LinhaCabecalho *)cabecalhoVoid;

    // CODIGO
    fprintf(saida, "%.*s: ", TAMANHO_DESCREVE_CODIGO, cabecalho->descreveCodigo);
    fprintf(saida, "%d\n", linha->codLinha);

    // NOME
    fprintf(saida, "%.*s: ", TAMANHO_DESCREVE_NOME, cabecalho->descreveNome);
    if (!global_printNuloStringTamanhoVariavel(saida, linha->tamanhoNome))
        fprintf(saida, "%.*s", linha->tamanhoNome, linha->nomeLinha);
    fputc('\n', saida);

    // COR
    fprintf(saida, "%.*s: ", TAMANHO_DESCREVE_LINHA, cabecalho->descreveLinha);
    if (!global_printNuloStringTamanhoVariavel(saida, linha->tamanhoCor))
        fprintf(saida, "%.*s", linha->tamanhoCor, linha->corLinha);
    fputc('\n', saida);

    // CARTAO
    fprintf(saida, "%.*s: ", TAMANHO_DESCREVE_CARTAO, cabecalho->descreveCartao);
    if (!global_printNuloStringTamanhoFixo(saida, &linha->aceitaCartao))
        printCartao(saida, linha->aceitaCartao);
    fputc('\n', saida);
}

/*
    Retorna verdadeiro se a Linha possui o mesmo valor do campo correspondente.
*/
BOOLEAN linha_checarMatchRegistro(void *linhaVoid, char *nomeDoCampo, char *valorDoCampo)
{
    Linha *linha = (Linha *)linhaVoid;
    if (!strcmp(nomeDoCampo, "codLinha"))
        return global_matchInteiro(linha->codLinha, valorDoCampo);

    if (!strcmp(nomeDoCampo, "aceitaCartao"))
        return global_matchStringTamanhoFixo(&linha->aceitaCartao, valorDoCampo, sizeof(char));

    if (!strcmp(nomeDoCampo, "nomeLinha"))
        return global_matchStringTamanhoVariavel(linha->nomeLinha, valorDoCampo, linha->tamanhoNome);

    if (!strcmp(nomeDoCampo, "corLinha"))
        return global_matchStringTamanhoVariavel(linha->corLinha, valorDoCampo, linha->tamanhoCor);

    return false;
}

/*
    Lê uma Linha da entrada padrão.
*/

void linha_lerRegistroEntradaPadrao(void *linhaVoid, char *buffer)
{
    Linha *linha = (Linha *)linhaVoid;

    linha->base.removido = '1';

    // CODIGO DA LINHA
    scan_quote_string(buffer);
    linha->codLinha = atoi(buffer);

    // ACEITA CARTAO
    scan_quote_string(buffer);
    global_tratarStringTamanhoFixo(buffer, sizeof(char), &linha->aceitaCartao, "");

    // NOME
    scan_quote_string(buffer);
    global_tratarStringTamanhoVariavel(buffer, &linha->tamanhoNome, &linha->nomeLinha, "");

    // COR
    scan_quote_string(buffer);
    global_tratarStringTamanhoVariavel(buffer, &linha->tamanhoCor, &linha->corLinha, "");

    // TAMANHO
    linha->base.tamanhoRegistro = TAMANHO_INICIAL_LINHA + linha->tamanhoNome + linha->tamanhoCor;
}

/*
    Esta função retorna a base de um LinhaCabecalho
*/
CabecalhoBase *linha_retornarCabecalhoBase(void *cabecalhoVoid)
{
    LinhaCabecalho *cabecalho = (LinhaCabecalho *)cabecalhoVoid;
    return &cabecalho->base;
}

/*
    Esta função retorna a base de uma Linha
*/
RegistroBase *linha_retornarRegistroBase(void *linhaVoid)
{
    Linha *linha = (Linha *)linhaVoid;
    return &linha->base;
}

/*
    Esta função retorna o identificador de um registro de Linha
*/
int linha_retornaIdentificador(void *linhaVoid)
{
    Linha *linha = (Linha *)linhaVoid;
    return linha->codLinha;
}

/*
    Esta função retorna o nome do campo identificador de uma Linha
*/
char *linha_retornaNomeIdentificador()
{
    return "codLinha";
};

/*
    Esta função retorna o valor do campo identificador convertido para inteiro
*/
int linha_converteIdentificador(char *valorDoCampo)
{
    return atoi(valorDoCampo);
}

/*
    Esta função ordena um vetor de linhas com base em um campo dado
    Campos suportados:
        - CodLinha
*/
BOOLEAN linha_ordena(void **linhasVoid, int numeroDeRegistros, char *nomeDoCampo)
{
    if (!strcmp(nomeDoCampo, "codLinha"))
    {
        qsort(linhasVoid, numeroDeRegistros, sizeof(Linha *), &comparaCodLinha);
        return true;
    }

    return false;
}

/*
    Esta função retorna o campo codLinha de um registro Linha
*/

int linha_getCodLinha(void *linhaVoid)
{
    Linha *linha = (Linha *)linhaVoid;
    return linha->codLinha;
}

/*
    Essa função retorna uma instância da struct "FuncoesGenericas"
    contendo todas as funções para operar sobre LINHAS.
*/
FuncoesGenericas *linha_funcoesGenericas()
{
    FuncoesGenericas *funcoes = (FuncoesGenericas *)malloc(sizeof(FuncoesGenericas));
    funcoes->gerarCabecalho = &linha_gerarCabecalho;
    funcoes->gerarRegistro = &linha_gerarRegistro;
    funcoes->freeRegistroInternamente = &linha_freeRegistroInternamente;
    funcoes->lerCabecalhoCsv = &linha_lerCabecalhoCsv;
    funcoes->ioCabecalhoBin = &linha_ioCabecalhoBin;
    funcoes->lerRegistroCsv = &linha_lerRegistroCsv;
    funcoes->escreverRegistroBin = &linha_escreverRegistroBin;
    funcoes->lerRegistroBin = &linha_lerRegistroBin;
    funcoes->printRegistro = &linha_printRegistro;
    funcoes->checarMatchRegistro = &linha_checarMatchRegistro;
    funcoes->lerRegistroEntradaPadrao = &linha_lerRegistroEntradaPadrao;
    funcoes->retornarCabecalhoBase = &linha_retornarCabecalhoBase;
    funcoes->retornarRegistroBase = &linha_retornarRegistroBase;
    funcoes->retornaIdentificador = &linha_retornaIdentificador;
    funcoes->retornaNomeIdentificador = &linha_retornaNomeIdentificador;
    funcoes->converteIdentificador = &linha_converteIdentificador;
    funcoes->ordena = &linha_ordena;
    return funcoes;
}