#include <global.h>
#include <stdio.h>
#include <leia.h>
#include <veiculo.h>
#include <stdlib.h>
#include <string.h>
#include <funcao-fornecida.h>
#include <convertePrefixo.h>

/*

    CONSTANTES E STRUCTS

*/

#define TAMANHO_DESCREVE_PREFIXO 18
#define TAMANHO_DESCREVE_DATA 35
#define TAMANHO_DESCREVE_LUGARES 42
#define TAMANHO_DESCREVE_LINHA 26
#define TAMANHO_DESCREVE_MODELO 17
#define TAMANHO_DESCREVE_CATEGORIA 20
#define TAMANHO_PREFIXO 5
#define TAMANHO_DATA 10

#define TAMANHO_INICIAL_VEICULO TAMANHO_PREFIXO + TAMANHO_DATA + 4 * sizeof(int)

typedef struct
{
    CabecalhoBase base;
    char descrevePrefixo[TAMANHO_DESCREVE_PREFIXO];
    char descreveData[TAMANHO_DESCREVE_DATA];
    char descreveLugares[TAMANHO_DESCREVE_LUGARES];
    char descreveLinha[TAMANHO_DESCREVE_LINHA];
    char descreveModelo[TAMANHO_DESCREVE_MODELO];
    char descreveCategoria[TAMANHO_DESCREVE_CATEGORIA];
} VeiculoCabecalho;

typedef struct
{
    RegistroBase base;
    char prefixo[TAMANHO_PREFIXO];
    char data[TAMANHO_DATA];
    int quantidadeLugares;
    int codLinha;
    int tamanhoModelo;
    char *modelo;
    int tamanhoCategoria;
    char *categoria;
} Veiculo;

/*

    INTERFACE PRIVADA

*/

/*
    Função que retorna o nome do mês com base em seu número.
*/
static const char *mesString(int mesInt)
{
    static const char *meses[12] = {"janeiro",
                                    "fevereiro",
                                    "março",
                                    "abril",
                                    "maio",
                                    "junho",
                                    "julho",
                                    "agosto",
                                    "setembro",
                                    "outubro",
                                    "novembro",
                                    "dezembro"};
    return meses[mesInt - 1];
}

/*
    Função que imprime no arquivo de saída uma data no formato
        DIA de NOME_DO_MES de ANO
    a partir de uma data no formato
        ANO-MES-DIA
*/
static void printData(FILE *saida, const char *data, char *buffer)
{
    // Verifica se é necessário alocar o buffer
    BOOLEAN alocar = true;
    if (buffer)
        alocar = false;

    int ano, mes, dia;

    // buffer recebe o que está em data
    if (alocar)
        buffer = (char *)malloc(sizeof(char) * 11);
    memcpy(buffer, data, 10);

    buffer[4] = ' ';
    buffer[7] = ' ';
    buffer[10] = '\0';

    // Agora o buffer contém uma string do tipo
    // ANO MES DIA
    sscanf(buffer, "%d %d %d", &ano, &mes, &dia);

    // Impressão
    fprintf(saida, "%02d de %s de %04d", dia, mesString(mes), ano);

    // Desalocar memória caso necessário
    if (alocar)
        free(buffer);
}

/*
    Função necessária para execução de qsort. Compara dois registros veículo e
    retorna
        > 0 para codLinha1 > codLinha2
        = 0 para codLinha1 = codLinha2
        < 0 para codLinha1 < codLinha2
*/
static int comparaCodLinha(const void *veiculoVoid1, const void *veiculoVoid2)
{
    Veiculo *veiculo1 = (Veiculo *)(*((void **)veiculoVoid1));
    Veiculo *veiculo2 = (Veiculo *)(*((void **)veiculoVoid2));
    return veiculo1->codLinha - veiculo2->codLinha;
}

/*

    INTERFACE PÚBLICA

*/

/*
    As duas funções a seguir geram uma das duas estruturas
        definidas no arquivo, como ponteiro void (útil ao trabalhar
        com funções genéricas)
*/
void *veiculo_gerarCabecalho()
{
    return malloc(sizeof(VeiculoCabecalho));
}

void *veiculo_gerarRegistro()
{
    return malloc(sizeof(Veiculo));
}

/*
    Essa função é responsável por liberar a memória interna do registro,
    mas mantê-lo alocado na memória.
*/
void veiculo_freeRegistroInternamente(void *veiculoVoid)
{
    Veiculo *veiculo = (Veiculo *)veiculoVoid;
    if (veiculo->tamanhoCategoria > 0)
        free(veiculo->categoria);
    if (veiculo->tamanhoModelo > 0)
        free(veiculo->modelo);
}

/*
    Essa função lê um arquivo csv e armazena as informações em um
    VeiculoCabecalho
*/
void veiculo_lerCabecalhoCsv(FILE *csv, BOOLEAN *vParada, void *cabecalhoVoid)
{
    VeiculoCabecalho *cabecalho = (VeiculoCabecalho *)cabecalhoVoid;
    global_cabecalhoBaseDefault(&cabecalho->base);
    leia_lerString(csv, vParada, cabecalho->descrevePrefixo, TAMANHO_DESCREVE_PREFIXO, false);
    leia_lerString(csv, vParada, cabecalho->descreveData, TAMANHO_DESCREVE_DATA, false);
    leia_lerString(csv, vParada, cabecalho->descreveLugares, TAMANHO_DESCREVE_LUGARES, false);
    leia_lerString(csv, vParada, cabecalho->descreveLinha, TAMANHO_DESCREVE_LINHA, false);
    leia_lerString(csv, vParada, cabecalho->descreveModelo, TAMANHO_DESCREVE_MODELO, false);
    leia_lerString(csv, vParada, cabecalho->descreveCategoria, TAMANHO_DESCREVE_CATEGORIA, false);
}

/*
    Essa função serve para escrita e leitura de VeiculoCabecalho em arquivos
    binários. Último parâmetro pode ser &fread ou &fwrite.
*/
void veiculo_ioCabecalhoBin(FILE *bin, void *cabecalhoVoid, size_t (*f)(void *, size_t, size_t, FILE *))
{
    VeiculoCabecalho *cabecalho = (VeiculoCabecalho *)cabecalhoVoid;
    global_ioCabecalhoBaseBin(bin, &cabecalho->base, f);
    f(cabecalho->descrevePrefixo, sizeof(char), TAMANHO_DESCREVE_PREFIXO, bin);
    f(cabecalho->descreveData, sizeof(char), TAMANHO_DESCREVE_DATA, bin);
    f(cabecalho->descreveLugares, sizeof(char), TAMANHO_DESCREVE_LUGARES, bin);
    f(cabecalho->descreveLinha, sizeof(char), TAMANHO_DESCREVE_LINHA, bin);
    f(cabecalho->descreveModelo, sizeof(char), TAMANHO_DESCREVE_MODELO, bin);
    f(cabecalho->descreveCategoria, sizeof(char), TAMANHO_DESCREVE_CATEGORIA, bin);
}

/*
    Lê um Veiculo de um arquivo csv.
    Retorna falso se não há mais registros pra ler daquele arquivo.
*/
BOOLEAN veiculo_lerRegistroCsv(FILE *csv, BOOLEAN *vParada, void *veiculoVoid, char *buffer)
{
    Veiculo *veiculo = (Veiculo *)veiculoVoid;

    leia_lerString(csv, vParada, buffer, TAMANHO_BUFFER, true);
    if (feof(csv)) // Checando se chegou no fim do arquivo.
        return false;

    // PREFIXO
    if (buffer[0] == '*') // Tratando para removido
    {
        veiculo->base.removido = '0';
        memcpy(veiculo->prefixo, buffer + 1, TAMANHO_PREFIXO);
    }
    else // Tratando para não removido
    {
        veiculo->base.removido = '1';
        memcpy(veiculo->prefixo, buffer, TAMANHO_PREFIXO);
    }

    // DATA
    leia_lerString(csv, vParada, buffer, TAMANHO_BUFFER, true);
    global_tratarStringTamanhoFixo(buffer, TAMANHO_DATA, veiculo->data, NULL);

    // QUANTIDADE LUGARES
    leia_lerString(csv, vParada, buffer, TAMANHO_BUFFER, true);
    global_tratarInteiro(buffer, &veiculo->quantidadeLugares, NULL);

    // COD LINHA
    leia_lerString(csv, vParada, buffer, TAMANHO_BUFFER, true);
    global_tratarInteiro(buffer, &veiculo->codLinha, NULL);

    // TAMANHO MODELO / MODELO
    leia_lerString(csv, vParada, buffer, TAMANHO_BUFFER, true);
    global_tratarStringTamanhoVariavel(buffer, &veiculo->tamanhoModelo, &veiculo->modelo, NULL);

    // TAMANHO CATEGORIA / CATEGORIA
    leia_lerString(csv, vParada, buffer, TAMANHO_BUFFER, true);
    global_tratarStringTamanhoVariavel(buffer, &veiculo->tamanhoCategoria, &veiculo->categoria, NULL);

    // TAMANHO REGISTRO
    veiculo->base.tamanhoRegistro = TAMANHO_INICIAL_VEICULO + veiculo->tamanhoModelo + veiculo->tamanhoCategoria;

    return true;
}

/*
    Escreve um Veiculo em um arquivo binário.
*/
void veiculo_escreverRegistroBin(FILE *bin, void *veiculoVoid)
{
    Veiculo *veiculo = (Veiculo *)veiculoVoid;
    global_ioRegistroBaseBin(bin, &veiculo->base, (size_t(*)(void *, size_t, size_t, FILE *)) & fwrite);
    fwrite(veiculo->prefixo, sizeof(char), TAMANHO_PREFIXO, bin);
    fwrite(veiculo->data, sizeof(char), TAMANHO_DATA, bin);
    fwrite(&veiculo->quantidadeLugares, sizeof(int), 1, bin);
    fwrite(&veiculo->codLinha, sizeof(int), 1, bin);
    global_escreverStringTamanhoVariavelBin(bin, &veiculo->tamanhoModelo, veiculo->modelo);
    global_escreverStringTamanhoVariavelBin(bin, &veiculo->tamanhoCategoria, veiculo->categoria);
}

/*
    Lê um Veiculo de um arquivo binário. É possível fazer com que
    a base do registro não seja lida.
*/
void veiculo_lerRegistroBin(FILE *bin, void *veiculoVoid, BOOLEAN lerRegistroBase)
{
    Veiculo *veiculo = (Veiculo *)veiculoVoid;
    if (lerRegistroBase)
        global_ioRegistroBaseBin(bin, &veiculo->base, &fread);
    fread(veiculo->prefixo, sizeof(char), TAMANHO_PREFIXO, bin);
    fread(veiculo->data, sizeof(char), TAMANHO_DATA, bin);
    fread(&veiculo->quantidadeLugares, sizeof(int), 1, bin);
    fread(&veiculo->codLinha, sizeof(int), 1, bin);
    global_lerStringTamanhoVariavelBin(bin, &veiculo->tamanhoModelo, &veiculo->modelo);
    global_lerStringTamanhoVariavelBin(bin, &veiculo->tamanhoCategoria, &veiculo->categoria);
}

/*
    Imprime no arquivo de saída um registro Veiculo.
*/
void veiculo_printRegistro(FILE *saida, void *veiculoVoid, void *cabecalhoVoid)
{
    Veiculo *veiculo = (Veiculo *)veiculoVoid;
    VeiculoCabecalho *cabecalho = (VeiculoCabecalho *)cabecalhoVoid;

    // PREFIXO
    fprintf(saida, "%.*s: ", TAMANHO_DESCREVE_PREFIXO, cabecalho->descrevePrefixo);
    fprintf(saida, "%.*s\n", TAMANHO_PREFIXO, veiculo->prefixo);

    // MODELO
    fprintf(saida, "%.*s: ", TAMANHO_DESCREVE_MODELO, cabecalho->descreveModelo);
    if (!global_printNuloStringTamanhoVariavel(saida, veiculo->tamanhoModelo))
        fprintf(saida, "%.*s", veiculo->tamanhoModelo, veiculo->modelo);
    fputc('\n', saida);

    // CATEGORIA
    fprintf(saida, "%.*s: ", TAMANHO_DESCREVE_CATEGORIA, cabecalho->descreveCategoria);
    if (!global_printNuloStringTamanhoVariavel(saida, veiculo->tamanhoCategoria))
        fprintf(saida, "%.*s", veiculo->tamanhoCategoria, veiculo->categoria);
    fputc('\n', saida);

    // DATA
    fprintf(saida, "%.*s: ", TAMANHO_DESCREVE_DATA, cabecalho->descreveData);
    if (!global_printNuloStringTamanhoFixo(saida, veiculo->data))
        printData(saida, veiculo->data, NULL);
    fputc('\n', saida);

    // LUGARES
    fprintf(saida, "%.*s: ", TAMANHO_DESCREVE_LUGARES, cabecalho->descreveLugares);
    if (!global_printNuloInteiro(saida, veiculo->quantidadeLugares))
        fprintf(saida, "%d", veiculo->quantidadeLugares);
    fputc('\n', saida);
}

/*
    Retorna verdadeiro se o Veiculo possui o mesmo valor do campo correspondente.
*/
BOOLEAN veiculo_checarMatchRegistro(void *veiculoVoid, char *nomeDoCampo, char *valorDoCampo)
{
    Veiculo *veiculo = (Veiculo *)veiculoVoid;
    if (!strcmp(nomeDoCampo, "prefixo"))
        return global_matchStringTamanhoFixo(veiculo->prefixo, valorDoCampo, TAMANHO_PREFIXO);

    if (!strcmp(nomeDoCampo, "data"))
        return global_matchStringTamanhoFixo(veiculo->data, valorDoCampo, TAMANHO_DATA);

    if (!strcmp(nomeDoCampo, "quantidadeLugares"))
        return global_matchInteiro(veiculo->quantidadeLugares, valorDoCampo);

    if (!strcmp(nomeDoCampo, "modelo"))
        return global_matchStringTamanhoVariavel(veiculo->modelo, valorDoCampo, veiculo->tamanhoModelo);

    if (!strcmp(nomeDoCampo, "categoria"))
        return global_matchStringTamanhoVariavel(veiculo->categoria, valorDoCampo, veiculo->tamanhoCategoria);

    return false;
}

/*
    Lê um Veiculo da entrada padrão.
*/

void veiculo_lerRegistroEntradaPadrao(void *veiculoVoid, char *buffer)
{
    Veiculo *veiculo = (Veiculo *)veiculoVoid;

    veiculo->base.removido = '1';

    // PREFIXO
    scan_quote_string(veiculo->prefixo);

    // DATA
    scan_quote_string(buffer);
    global_tratarStringTamanhoFixo(buffer, TAMANHO_DATA, veiculo->data, "");

    // QUANTIDADE DE LUGARES
    scan_quote_string(buffer);
    global_tratarInteiro(buffer, &veiculo->quantidadeLugares, "");

    // CÓDIGO DA LINHA
    scan_quote_string(buffer);
    global_tratarInteiro(buffer, &veiculo->codLinha, "");

    // MODELO
    scan_quote_string(buffer);
    global_tratarStringTamanhoVariavel(buffer, &veiculo->tamanhoModelo, &veiculo->modelo, "");

    // CATEGORIA
    scan_quote_string(buffer);
    global_tratarStringTamanhoVariavel(buffer, &veiculo->tamanhoCategoria, &veiculo->categoria, "");

    // TAMANHO
    veiculo->base.tamanhoRegistro = TAMANHO_INICIAL_VEICULO + veiculo->tamanhoModelo + veiculo->tamanhoCategoria;
}

/*
    Esta função retorna a base de um VeiculoCabecalho
*/
CabecalhoBase *veiculo_retornarCabecalhoBase(void *cabecalhoVoid)
{
    VeiculoCabecalho *cabecalho = (VeiculoCabecalho *)cabecalhoVoid;
    return &cabecalho->base;
}

/*
    Esta função retorna a base de um Veiculo
*/
RegistroBase *veiculo_retornarRegistroBase(void *veiculoVoid)
{
    Veiculo *veiculo = (Veiculo *)veiculoVoid;
    return &veiculo->base;
}

/*
    Esta função retorna o identificador de um registro de Veiculo
*/
int veiculo_retornaIdentificador(void *veiculoVoid)
{
    Veiculo *veiculo = (Veiculo *)veiculoVoid;
    return convertePrefixo(veiculo->prefixo);
}

/*
    Esta função retorna o nome do campo identificador de uma Veiculo
*/
char *veiculo_retornaNomeIdentificador()
{
    return "prefixo";
};

/*
    Esta função retorna o valor do campo identificador convertido para inteiro
*/
int veiculo_converteIdentificador(char *valorDoCampo)
{
    return convertePrefixo(valorDoCampo);
}

/*
    Esta função ordena um vetor de veículos com base em um campo dado
    Campos suportados:
        - CodLinha
*/
BOOLEAN veiculo_ordena(void **veiculosVoid, int numeroDeRegistros, char *nomeDoCampo)
{
    if (!strcmp(nomeDoCampo, "codLinha"))
    {
        qsort(veiculosVoid, numeroDeRegistros, sizeof(Veiculo *), &comparaCodLinha);
        return true;
    }

    return false;
}

/*
    Esta função retorna o campo codLinha de um registro Veiculo
*/

int veiculo_getCodLinha(void *veiculoVoid)
{
    Veiculo *veiculo = (Veiculo *)veiculoVoid;
    return veiculo->codLinha;
}

/*
    Essa função retorna uma instância da struct "FuncoesGenericas"
    contendo todas as funções para operar sobre VEICULOS.
*/

FuncoesGenericas *veiculo_funcoesGenericas()
{
    FuncoesGenericas *funcoes = (FuncoesGenericas *)malloc(sizeof(FuncoesGenericas));
    funcoes->gerarCabecalho = &veiculo_gerarCabecalho;
    funcoes->gerarRegistro = &veiculo_gerarRegistro;
    funcoes->freeRegistroInternamente = &veiculo_freeRegistroInternamente;
    funcoes->lerCabecalhoCsv = &veiculo_lerCabecalhoCsv;
    funcoes->ioCabecalhoBin = &veiculo_ioCabecalhoBin;
    funcoes->lerRegistroCsv = &veiculo_lerRegistroCsv;
    funcoes->escreverRegistroBin = &veiculo_escreverRegistroBin;
    funcoes->lerRegistroBin = &veiculo_lerRegistroBin;
    funcoes->printRegistro = &veiculo_printRegistro;
    funcoes->checarMatchRegistro = &veiculo_checarMatchRegistro;
    funcoes->lerRegistroEntradaPadrao = &veiculo_lerRegistroEntradaPadrao;
    funcoes->retornarCabecalhoBase = &veiculo_retornarCabecalhoBase;
    funcoes->retornarRegistroBase = &veiculo_retornarRegistroBase;
    funcoes->retornaIdentificador = &veiculo_retornaIdentificador;
    funcoes->retornaNomeIdentificador = &veiculo_retornaNomeIdentificador;
    funcoes->converteIdentificador = &veiculo_converteIdentificador;
    funcoes->ordena = &veiculo_ordena;
    return funcoes;
}