/*
    O operador genérico possui funções para operar sobre arquivos
    de dados. Recebe um conjunto de "FuncoesGenericas" para
    trabalhar com determinado registro de forma correta.
*/

#ifndef _OPERADOR_GENERICO_H_
#define _OPERADOR_GENERICO_H_

#include <stdio.h>
#include <global.h>

typedef struct
{
    void *(*gerarCabecalho)();
    void *(*gerarRegistro)();
    void (*freeRegistroInternamente)(void *registro);
    void (*lerCabecalhoCsv)(FILE *csv, BOOLEAN *vParada, void *cabecalho);
    void (*ioCabecalhoBin)(FILE *bin, void *cabecalho, size_t (*f)(void *, size_t, size_t, FILE *));
    BOOLEAN(*lerRegistroCsv)
    (FILE *csv, BOOLEAN *vParada, void *registro, char *buffer);
    void (*escreverRegistroBin)(FILE *bin, void *registro);
    void (*lerRegistroBin)(FILE *bin, void *registro, BOOLEAN lerRegistroBase);
    void (*printRegistro)(FILE *saida, void *registro, void *cabecalho);
    BOOLEAN(*checarMatchRegistro)
    (void *registro, char *nomeDoCampo, char *valorDoCampo);
    void (*lerRegistroEntradaPadrao)(void *registro, char *buffer);
    CabecalhoBase *(*retornarCabecalhoBase)(void *cabecalho);
    RegistroBase *(*retornarRegistroBase)(void *registro);
    int (*retornaIdentificador)(void *registro);
    char *(*retornaNomeIdentificador)();
    int (*converteIdentificador)(char *valorDoCampo);
    BOOLEAN(*ordena)
    (void **registros, int numeroDeRegistros, char *nomeDoCampo);
} FuncoesGenericas;

void opGen_lerCsvEscreverBin(BOOLEAN *vParada, char *buffer, FuncoesGenericas *funcoes);
void opGen_listarRegistrosBin(BOOLEAN *vParada, char *buffer, FuncoesGenericas *funcoes);
void opGen_buscarRegistrosBin(BOOLEAN *vParada, char *buffer, FuncoesGenericas *funcoes);
void opGen_adicionarRegistrosBin(BOOLEAN *vParada, char *buffer, FuncoesGenericas *funcoes);
void opGen_criarIndice(BOOLEAN *vParada, char *buffer, FuncoesGenericas *funcoes);
void opGen_buscarComIndice(BOOLEAN *vParada, char *buffer, FuncoesGenericas *funcoes);
void opGen_adicionarRegistrosComIndice(BOOLEAN *vParada, char *buffer, FuncoesGenericas *funcoes);
int opGen_ordenaArquivoDeDadosAuxiliar(FILE *arquivoInicial, FILE *arquivoFinal, char *nomeDoCampo, FuncoesGenericas *funcoes);
void opGen_ordenaArquivoDeDados(BOOLEAN *vParada, char *buffer, FuncoesGenericas *funcoes);

#endif

/*
    DETALHES DAS FUNÇÕES:

    void *gerarCabecalho()
        Essa função deve retornar uma struct com um cabecalho do seu arquivo de registros
        (como ponteiro para void).


    void *gerarRegistro()
        Essa função deve retornar uma struct com um registro do seu arquivo de registros
        (como ponteiro para void).


    void freeRegistroInternamente(void *registro)
        Essa função deve liberar toda a memória alocada dinamicamente DENTRO do registro.


    void lerCabecalhoCsv(FILE *csv, BOOLEAN *vParada, void *cabecalho)
        Essa função deve ler de um arquivo csv seu cabeçalho, e armazenar em um registro de cabeçalho
        passado como argumento.


    void ioCabecalhoBin(FILE *bin, void *cabecalho, size_t (*f)(void *, size_t, size_t, FILE *))
        Essa função deve ler ou escrever um registro de cabeçalho em um arquivo binário.
        O parâmetro f poderá ser &fread ou &fwrite.


    BOOLEAN lerRegistroCsv (FILE *csv, BOOLEAN *vParada, void *registro, char *buffer)
        Essa função lerá um registro de um arquivo csv.


    void escreverRegistroBin(FILE *bin, void *registro)
        Essa função escreverá um registro em um arquivo binário.


    void lerRegistroBin(FILE *bin, void *registro, BOOLEAN lerRegistroBase)
        Essa função lerá um registro de um arquivo binário.


    void printRegistro(FILE *saida, void *registro, void *cabecalho)
        Essa função imprimirá no arquivo de saída um registro.


    BOOLEAN checarMatchRegistro(void *registro, char *nomeDoCampo, char *valorDoCampo)
        Essa função checará se o registro possui no campo nomeDoCampo, o valor valorDoCampo.


    void lerRegistroEntradaPadrao(void *registro, char *buffer)
        Essa função lerá um registro da entrada padrão (stdin)


    CabecalhoBase *retornarCabecalhoBase(void *cabecalho)
        Essa função retornará o cabeçalho base de uma struct de cabeçalho


    RegistroBase *retornarRegistroBase(void *registro)
        Essa função retornará o registro base de uma struct de registro


    int (*retornaIdentificador)(void *registro)
        Essa função retorna o identificador de um registro na forma de inteiro

    char *(*retornaNomeIdentificador)()
        Essa função retorna o nome do campo identificador, (por exemplo, "prefixo" ou "codLinha")

    int (*converteIdentificador)(char *valorDoCampo)
        Essa função recebe o valor de um campo identificador no formato de string e retorna
            o valor no formato inteiro

    BOOLEAN(*ordena) (void **registros, int numeroDeRegistros, char *nomeDoCampo)
        Esta função ordena um vetor de registros com base em um campo dado, caso ele seja suportado

*/