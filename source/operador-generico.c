/*
    O operador genérico possui funções para operar sobre arquivos
    de dados. Recebe um conjunto de "FuncoesGenericas" para
    trabalhar com determinado registro de forma correta.

*/

#include <global.h>
#include <operador-generico.h>
#include <leia.h>
#include <stdlib.h>
#include <string.h>
#include <funcao-fornecida.h>
#include <arvore-b.h>

/*
    Essa struct auxiliar vai ser útil para passar strings de match como
    parâmetro de uma função de uma só vez 
*/
typedef struct
{
    char nomeDoCampo[TAMANHO_BUFFER];
    char valorDoCampo[TAMANHO_BUFFER];
} MatchStruct;

/*
 * 
 *
 *      AQUI COMEÇAM AS FUNÇÕES DO TRABALHO PRÁTICO 1
 *
 * 
 */

/*
    Função que efetivamente lê um arquivo csv e armazena os registros em um
    arquivo binário.
*/
static void lerCsvEscreverBinAuxiliar(FILE *csv, FILE *bin, BOOLEAN *vParada, char *buffer, FuncoesGenericas *funcoes)
{
    void *cabecalho = funcoes->gerarCabecalho();
    void *registro = funcoes->gerarRegistro();

    CabecalhoBase *cabecalhoBase = funcoes->retornarCabecalhoBase(cabecalho);
    RegistroBase *registroBase = funcoes->retornarRegistroBase(registro);

    // Lendo o cabeçalho do CSV e escrevendo no BINÁRIO
    funcoes->lerCabecalhoCsv(csv, vParada, cabecalho);
    funcoes->ioCabecalhoBin(bin, cabecalho, (size_t(*)(void *, size_t, size_t, FILE *)) & fwrite);
    fflush(bin); // Forçar escrita

    // Enquanto ainda há registros para ler
    while (funcoes->lerRegistroCsv(csv, vParada, registro, buffer))
    {
        // Escreva o registro no binário
        funcoes->escreverRegistroBin(bin, registro);

        // Atualizar cabeçalho
        if (registroBase->removido == '0')
            cabecalhoBase->nroRegRemovidos++;
        else
            cabecalhoBase->nroRegistros++;

        // Liberando a memória auxiliar alocada dentro do registro
        funcoes->freeRegistroInternamente(registro);
    }

    // Atualizando cabeçalho
    fseek(bin, 0, SEEK_END);
    cabecalhoBase->byteProxReg = ftell(bin);
    cabecalhoBase->status = '1';

    // Sobrescrevendo as informações base do cabeçalho
    fseek(bin, 0, SEEK_SET);
    global_ioCabecalhoBaseBin(bin, cabecalhoBase, (size_t(*)(void *, size_t, size_t, FILE *)) & fwrite);
    fflush(bin); // Forçar escrita

    free(registro);
    free(cabecalho);
}

/*
    Função que lê da entrada padrão os arquivos csv e binário, chama a função
    original e executa a função binarioNaTela.
*/
void opGen_lerCsvEscreverBin(BOOLEAN *vParada, char *buffer, FuncoesGenericas *funcoes)
{
    // Leitura do nome do arquivo CSV
    leia_lerString(stdin, vParada, buffer, TAMANHO_BUFFER, true);
    FILE *csv = fopen(buffer, "r");
    if (!csv)
    {
        printf(ERRO_FALHA_ARQUIVO_MENSAGEM);
        return;
    }

    // Leitura do nome do arquivo binário
    leia_lerString(stdin, vParada, buffer, TAMANHO_BUFFER, true);
    FILE *bin = fopen(buffer, "wb");
    if (!bin)
    {
        printf(ERRO_FALHA_ARQUIVO_MENSAGEM);
        fclose(csv);
        return;
    }

    // Armazenar o nome do arquivo binário para futura chamada de binarioNaTela
    char *nomeBin = (char *)malloc(sizeof(char) * (strlen(buffer) + 1));
    strcpy(nomeBin, buffer);

    // Condições de parada do CSV (em todo caractere ',' ou '\n')
    vetorDeParada_adicionarCaractere(vParada, ',');
    vetorDeParada_removerCaractere(vParada, ' ');

    lerCsvEscreverBinAuxiliar(csv, bin, vParada, buffer, funcoes);

    // Fechando os arquivos e chamando a binarioNaTela
    fclose(csv);
    fclose(bin);
    binarioNaTela(nomeBin);
    free(nomeBin);
}

/*
    Função que exibe no arquivo saida
    caso matchStruct seja NULL: 
        todos os registros armazenados em um arquivo binário
    caso matchStruct não seja NULL:
        todos os registros que encaixam nas condições de match
*/
static int printRegistrosBin(FILE *saida, FILE *bin, FuncoesGenericas *funcoes, MatchStruct *matchStruct)
{
    void *cabecalho = funcoes->gerarCabecalho();
    void *registro = funcoes->gerarRegistro();

    CabecalhoBase *cabecalhoBase = funcoes->retornarCabecalhoBase(cabecalho);
    RegistroBase *registroBase = funcoes->retornarRegistroBase(registro);

    // Ler cabeçalho
    funcoes->ioCabecalhoBin(bin, cabecalho, &fread);

    // Erro de status inconsistente
    if (cabecalhoBase->status == '0')
    {
        free(cabecalho);
        free(registro);
        return ERRO_FALHA_ARQUIVO;
    }

    int nroRegistrosExibidos = 0;

    // Passa registro por registro dentro do arquivo binário
    for (int regIndex = 0; regIndex < cabecalhoBase->nroRegistros;)
    {
        global_ioRegistroBaseBin(bin, registroBase, &fread);

        // Pular registros removidos
        if (registroBase->removido == '0')
            fseek(bin, registroBase->tamanhoRegistro, SEEK_CUR);
        else
        {
            // Ler restante do registro
            funcoes->lerRegistroBin(bin, registro, false);

            // Checa matchStruct
            if (!matchStruct || funcoes->checarMatchRegistro(registro, matchStruct->nomeDoCampo, matchStruct->valorDoCampo))
            {
                nroRegistrosExibidos++;
                funcoes->printRegistro(saida, registro, cabecalho);
            }

            // Libera espaço armazenado dentro do registro
            funcoes->freeRegistroInternamente(registro);

            regIndex++;
        }
    }

    free(cabecalho);
    free(registro);

    // Ou não há registros no arquivo, ou nenhum registro se encaixa nas condições de match
    if (nroRegistrosExibidos == 0)
        return ERRO_REGISTRO;

    return SUCESSO;
}

/*
    Função que lê o nome de um arquivo binário e lista
        todos os registros não-removidos armazenados.
*/
void opGen_listarRegistrosBin(BOOLEAN *vParada, char *buffer, FuncoesGenericas *funcoes)
{
    // Leitura do nome do arquivo binário
    leia_lerString(stdin, vParada, buffer, TAMANHO_BUFFER, true);
    FILE *bin = fopen(buffer, "rb");
    if (!bin)
    {
        printf(ERRO_FALHA_ARQUIVO_MENSAGEM); // Erro na abertura do arquivo
        return;
    }

    // Exibir todos os registros e, caso necessário, exibir mensagem de erro
    imprimeErro(printRegistrosBin(stdout, bin, funcoes, NULL));

    // Fechando arquivos
    fclose(bin);
}

/*
    Função que lê o nome de um arquivo binário, um campo e um valor de campo
    Imprime todos os registros dentro do arquivo binário que possuem o mesmo
        valor no campo especificado.
*/
void opGen_buscarRegistrosBin(BOOLEAN *vParada, char *buffer, FuncoesGenericas *funcoes)
{
    // Leitura do nome do arquivo binário
    leia_lerString(stdin, vParada, buffer, TAMANHO_BUFFER, true);
    FILE *bin = fopen(buffer, "rb");
    if (!bin)
    {
        printf(ERRO_FALHA_ARQUIVO_MENSAGEM); // Erro na abertura do arquivo
        return;
    }

    // Criando as condições de match para passar como argumento
    MatchStruct *matchStruct = (MatchStruct *)malloc(sizeof(MatchStruct));
    scanf("%s", matchStruct->nomeDoCampo);
    scan_quote_string(matchStruct->valorDoCampo);

    // Imprimindo todos os registros que se encaixam nas condições de matchStruct
    imprimeErro(printRegistrosBin(stdout, bin, funcoes, matchStruct));

    // Liberação de memória
    free(matchStruct);
    fclose(bin);
}

/*
    Função que efetivamente lê registro por registro e os adiciona no arquivo binário.
*/
static int adicionarRegistrosBinAuxiliar(FILE *bin, char *buffer, int nRegistros, FuncoesGenericas *funcoes)
{
    void *cabecalho = funcoes->gerarCabecalho();
    void *registro = funcoes->gerarRegistro();

    CabecalhoBase *cabecalhoBase = funcoes->retornarCabecalhoBase(cabecalho);

    // Ler cabeçalho do arquivo binário
    global_ioCabecalhoBaseBin(bin, cabecalhoBase, &fread);

    // Erro de status inconsistente
    if (cabecalhoBase->status == '0')
    {
        free(cabecalho);
        free(registro);
        return ERRO_FALHA_ARQUIVO;
    }

    // Definir status como inconsistente e sobreescrever no arquivo
    cabecalhoBase->status = '0';
    fseek(bin, 0L, SEEK_SET);
    global_ioCabecalhoBaseBin(bin, cabecalhoBase, (size_t(*)(void *, size_t, size_t, FILE *)) & fwrite);
    fflush(bin);

    // Ir para o fim do arquivo
    fseek(bin, cabecalhoBase->byteProxReg, SEEK_SET);

    // Escrever registro por registro no arquivo
    for (int regIndex = 0; regIndex < nRegistros; regIndex++)
    {
        funcoes->lerRegistroEntradaPadrao(registro, buffer);
        funcoes->escreverRegistroBin(bin, registro);
        funcoes->freeRegistroInternamente(registro);
    }

    // Atualizando cabeçalho
    cabecalhoBase->nroRegistros += nRegistros;
    cabecalhoBase->byteProxReg = ftell(bin);
    cabecalhoBase->status = '1';

    // Sobrescrevendo as informações de cabeçalho, mas apenas as que são relevantes nesse contexto
    fseek(bin, 0L, SEEK_SET);
    global_ioCabecalhoBaseBin(bin, cabecalhoBase, (size_t(*)(void *, size_t, size_t, FILE *)) & fwrite);
    fflush(bin);

    free(cabecalho);
    free(registro);

    return SUCESSO;
}

/*
    Função que lê um arquivo e uma quantidade de registros, e adiciona
        cada um dos registros lidos ao fim de um arquivo binário.

*/
void opGen_adicionarRegistrosBin(BOOLEAN *vParada, char *buffer, FuncoesGenericas *funcoes)
{
    // Leitura do nome do arquivo binário
    leia_lerString(stdin, vParada, buffer, TAMANHO_BUFFER, true);
    FILE *bin = fopen(buffer, "rb+");
    if (!bin)
    {
        printf(ERRO_FALHA_ARQUIVO_MENSAGEM); // Erro na abertura do arquivo
        return;
    }

    // Armazenar o nome do arquivo binário para futura chamada de binarioNaTela
    char *nomeBin = (char *)malloc(sizeof(char) * (strlen(buffer) + 1));
    strcpy(nomeBin, buffer);

    int nRegistros = leia_lerInteiro(stdin, vParada);

    // Adicionar registro por registro caso não tenha ocorrido nenhum erro.
    int erro = adicionarRegistrosBinAuxiliar(bin, buffer, nRegistros, funcoes);
    imprimeErro(erro);

    // Fechando arquivo binário e chamando a binarioNaTela
    fclose(bin);
    if (!erro)
        binarioNaTela(nomeBin);
    free(nomeBin);
}

/*
 * 
 *
 *      AQUI COMEÇAM AS FUNÇÕES DO TRABALHO PRÁTICO 2
 *
 * 
 */

/*
    Função auxiliar que efetivamente cria o arquivo de índice usando
        a estrutura árvore B.
*/
static int criarIndiceAuxiliar(FILE *arquivoDados, FILE *arquivoIndice, FuncoesGenericas *funcoes)
{
    // Estruturas auxiliares para utilizar nas funções genéricas
    void *cabecalho = funcoes->gerarCabecalho();
    void *registro = funcoes->gerarRegistro();

    CabecalhoBase *cabecalhoBase = funcoes->retornarCabecalhoBase(cabecalho);
    RegistroBase *registroBase = funcoes->retornarRegistroBase(registro);

    // Ler cabeçalho do arquivo de dados
    global_ioCabecalhoBaseBin(arquivoDados, cabecalhoBase, &fread);

    // Erro de status inconsistente
    if (cabecalhoBase->status == '0')
    {
        free(cabecalho);
        free(registro);
        return ERRO_FALHA_ARQUIVO;
    }

    // Criar uma estrutra Árvore B em RAM para manipular arquivo índice
    //      (PS: essa estrutura é necessária apenas para armazenar dados mais frequentes como noRaiz e
    //       RRNproxNo, e para utilizar as funções do TAD Árvore B)
    ArvoreB *arvoreB = arvoreB_criar(arquivoIndice, ARVORE_B_ORDEM, ARVORE_B_TAMANHO_NO, false);
    // Definir status como inconsistente no arquivo de índice
    arvoreB_inicializarCabecalho(arvoreB, '0');

    // Lendo o cabeçalho completamente
    fseek(arquivoDados, 0L, SEEK_SET);
    funcoes->ioCabecalhoBin(arquivoDados, cabecalho, &fread);

    // Para cada registro no arquivo de dados
    for (int regIndex = 0; regIndex < cabecalhoBase->nroRegistros;)
    {
        // Capturar o byteOffset do registro atual
        long long int byteOffset = (long long int)ftell(arquivoDados);

        // Ler a base do registro (campos "removido" e "tamanhoRegistro")
        global_ioRegistroBaseBin(arquivoDados, registroBase, &fread);

        // Pular registro caso ele tenha sido removido
        if (registroBase->removido == '0')
            fseek(arquivoDados, registroBase->tamanhoRegistro, SEEK_CUR);
        else
        {
            // Ler o restante do registro, armazenar sua chave identificadora e inserí-la
            //      no arquivo de índice, utilizando o byteOffset
            funcoes->lerRegistroBin(arquivoDados, registro, false);
            int chaveId = funcoes->retornaIdentificador(registro);
            arvoreB_inserir(arvoreB, chaveId, byteOffset);

            // Atualizar o cabeçalho em DISCO
            arvoreB_atualizarCabecalho(arvoreB);

            // Liberar memória alocada dinamicamente dentro da estrutura de registro
            // Ir para o próximo registro dentro do arquivo de dados
            funcoes->freeRegistroInternamente(registro);
            regIndex++;
        }
    }

    // Atualizar o cabeçalho em DISCO e alterar o status para consistente
    arvoreB_atualizarCabecalho(arvoreB);
    arvoreB_setStatus(arvoreB, '1');

    // Liberação de memória
    free(cabecalho);
    free(registro);
    arvoreB_free(arvoreB, false);

    return SUCESSO;
}

/*
    Função que lê um arquivo de dados binário e cria um arquivo de índice
        utilizando a estrutura árvore B.
*/
void opGen_criarIndice(BOOLEAN *vParada, char *buffer, FuncoesGenericas *funcoes)
{
    // Leitura do nome do arquivo de dados
    leia_lerString(stdin, vParada, buffer, TAMANHO_BUFFER, true);
    FILE *arquivoDados = fopen(buffer, "rb");
    if (!arquivoDados)
    {
        printf(ERRO_FALHA_ARQUIVO_MENSAGEM); // Erro na abertura do arquivo
        return;
    }

    // Leitura do nome do arquivo de índice
    leia_lerString(stdin, vParada, buffer, TAMANHO_BUFFER, true);
    FILE *arquivoIndice = fopen(buffer, "wb+");
    if (!arquivoIndice)
    {
        printf(ERRO_FALHA_ARQUIVO_MENSAGEM); // Erro na abertura do arquivo
        fclose(arquivoDados);
        return;
    }

    // Armazenar o nome do arquivo de dados para futura chamada de binarioNaTela
    char *nomeArquivoDados = (char *)malloc(sizeof(char) * (strlen(buffer) + 1));
    strcpy(nomeArquivoDados, buffer);

    // Tentar gerar arquivo de índice (e exibir um erro caso essa operação falhe)
    int erro = criarIndiceAuxiliar(arquivoDados, arquivoIndice, funcoes);
    imprimeErro(erro);

    // Fechando os arquivos e chamando a binarioNaTela
    fclose(arquivoDados);
    fclose(arquivoIndice);
    if (!erro)
        binarioNaTela(nomeArquivoDados);
    free(nomeArquivoDados);
}

/*
    Função auxiliar que efetivamente vai pesquisar por uma chave no arquivo de índice
        e retornar para o usuário o registro correspondente no arquivo de dados, caso exista.
*/
static int buscarComIndiceAuxiliar(FILE *saida, FILE *arquivoDados, FILE *arquivoIndice, FuncoesGenericas *funcoes, MatchStruct *matchStruct)
{
    // Caso o valor dado não seja o nome do identificador ("prefixo" ou "codLinha"), retorna ERRO_FALHA_ARQUIVO
    if (strcmp(matchStruct->nomeDoCampo, funcoes->retornaNomeIdentificador()))
        return ERRO_FALHA_ARQUIVO;

    // Estruturas auxiliares para lidar com funções genéricas
    void *cabecalho = funcoes->gerarCabecalho();
    void *registro = funcoes->gerarRegistro();

    CabecalhoBase *cabecalhoBase = funcoes->retornarCabecalhoBase(cabecalho);
    RegistroBase *registroBase = funcoes->retornarRegistroBase(registro);

    // Ler cabeçalho
    funcoes->ioCabecalhoBin(arquivoDados, cabecalho, &fread);

    // Erro de status inconsistente
    if (cabecalhoBase->status == '0')
    {
        free(cabecalho);
        free(registro);
        return ERRO_FALHA_ARQUIVO;
    }

    // Erro de status inconsistente
    ArvoreB *arvoreB = arvoreB_criar(arquivoIndice, ARVORE_B_ORDEM, ARVORE_B_TAMANHO_NO, true);
    if (arvoreB_getStatus(arvoreB) == '0')
    {
        free(cabecalho);
        free(registro);
        arvoreB_free(arvoreB, false);
        return ERRO_FALHA_ARQUIVO;
    }

    // Faz a pesquisa no arquivo de índice usando Árvore B
    long long int byteOffset = arvoreB_buscar(arvoreB, funcoes->converteIdentificador(matchStruct->valorDoCampo));
    arvoreB_free(arvoreB, false);

    // Registro não encontrado
    if (byteOffset == -1)
    {
        free(cabecalho);
        free(registro);
        return ERRO_REGISTRO;
    }

    // Registro encontrado. Procura no arquivo de dados.
    fseek(arquivoDados, byteOffset, SEEK_SET);
    funcoes->lerRegistroBin(arquivoDados, registro, true);

    // Registro removido
    if (registroBase->removido == '0')
    {
        free(cabecalho);
        free(registro);
        return ERRO_REGISTRO;
    }

    // Exibe registro na tela
    funcoes->printRegistro(saida, registro, cabecalho);
    funcoes->freeRegistroInternamente(registro);

    // Liberando memória e retornando
    free(cabecalho);
    free(registro);
    return SUCESSO;
}

/*
    Função que busca uma chave em um arquivo de dados utilizando um arquivo de índice
*/
void opGen_buscarComIndice(BOOLEAN *vParada, char *buffer, FuncoesGenericas *funcoes)
{
    // Leitura do nome do arquivo de dados
    leia_lerString(stdin, vParada, buffer, TAMANHO_BUFFER, true);
    FILE *arquivoDados = fopen(buffer, "rb");
    if (!arquivoDados)
    {
        printf(ERRO_FALHA_ARQUIVO_MENSAGEM); // Erro na abertura do arquivo
        return;
    }

    // Leitura do nome do arquivo de índice
    leia_lerString(stdin, vParada, buffer, TAMANHO_BUFFER, true);
    FILE *arquivoIndice = fopen(buffer, "rb");
    if (!arquivoIndice)
    {
        printf(ERRO_FALHA_ARQUIVO_MENSAGEM); // Erro na abertura do arquivo
        fclose(arquivoDados);
        return;
    }

    // Criando as condições de match para passar como argumento
    MatchStruct *matchStruct = (MatchStruct *)malloc(sizeof(MatchStruct));
    scanf("%s", matchStruct->nomeDoCampo);
    scan_quote_string(matchStruct->valorDoCampo);

    // Imprimindo o registro encontrado, ou um erro caso ele não tenha sido / arquivo com status inconsistente
    imprimeErro(buscarComIndiceAuxiliar(stdout, arquivoDados, arquivoIndice, funcoes, matchStruct));

    // Liberação de memória
    free(matchStruct);
    fclose(arquivoIndice);
    fclose(arquivoDados);
}

/*
    Função que efetivamente lê registro por registro e os adiciona no arquivo binário e de índice.
*/
static int adicionarRegistrosComIndiceAuxiliar(FILE *arquivoDados, FILE *arquivoIndice, char *buffer, int nRegistros, FuncoesGenericas *funcoes)
{
    // Estruturas auxiliares para lidar com funções genéricas
    void *cabecalho = funcoes->gerarCabecalho();
    void *registro = funcoes->gerarRegistro();

    CabecalhoBase *cabecalhoBase = funcoes->retornarCabecalhoBase(cabecalho);

    // Ler cabeçalho do arquivo de dados
    global_ioCabecalhoBaseBin(arquivoDados, cabecalhoBase, &fread);

    // Erro de status inconsistente
    if (cabecalhoBase->status == '0')
    {
        free(cabecalho);
        free(registro);
        return ERRO_FALHA_ARQUIVO;
    }

    // Erro de status inconsistente
    ArvoreB *arvoreB = arvoreB_criar(arquivoIndice, ARVORE_B_ORDEM, ARVORE_B_TAMANHO_NO, true);
    if (arvoreB_getStatus(arvoreB) == '0')
    {
        free(cabecalho);
        free(registro);
        arvoreB_free(arvoreB, false);
        return ERRO_FALHA_ARQUIVO;
    }

    // Definir status como inconsistente dos dois arquivos
    arvoreB_setStatus(arvoreB, '0');

    cabecalhoBase->status = '0';
    fseek(arquivoDados, 0L, SEEK_SET);
    global_ioCabecalhoBaseBin(arquivoDados, cabecalhoBase, (size_t(*)(void *, size_t, size_t, FILE *)) & fwrite);
    fflush(arquivoDados);

    // Ir para o fim do arquivo de dados
    fseek(arquivoDados, cabecalhoBase->byteProxReg, SEEK_SET);

    // Escrever registro por registro no arquivo
    for (int regIndex = 0; regIndex < nRegistros; regIndex++)
    {
        // Armazenando byteOffset
        cabecalhoBase->byteProxReg = ftell(arquivoDados);

        funcoes->lerRegistroEntradaPadrao(registro, buffer);
        funcoes->escreverRegistroBin(arquivoDados, registro);

        // Inserindo no arquivo de índice com a chave e o byteOffset, utilizando a estrutura Árvore B
        arvoreB_inserir(arvoreB, funcoes->retornaIdentificador(registro), cabecalhoBase->byteProxReg);
        // Atualizar o cabeçalho em DISCO
        arvoreB_atualizarCabecalho(arvoreB);

        funcoes->freeRegistroInternamente(registro);
    }

    // Atualizar o cabeçalho em DISCO e alterando o status para consistente
    arvoreB_atualizarCabecalho(arvoreB);
    arvoreB_setStatus(arvoreB, '1');
    arvoreB_free(arvoreB, false);

    // Atualizando cabeçalho do arquivo de dados
    cabecalhoBase->nroRegistros += nRegistros;
    cabecalhoBase->byteProxReg = ftell(arquivoDados);
    cabecalhoBase->status = '1';

    // Sobrescrevendo as informações de cabeçalho, mas apenas as que são relevantes nesse contexto
    fseek(arquivoDados, 0L, SEEK_SET);
    global_ioCabecalhoBaseBin(arquivoDados, cabecalhoBase, (size_t(*)(void *, size_t, size_t, FILE *)) & fwrite);
    fflush(arquivoDados);

    free(cabecalho);
    free(registro);

    return SUCESSO;
}

/*
    Função que lê um arquivo de dados, um arquivo de índice e uma quantidade de registros, e adiciona
        cada um dos registros aos dois arquivos.

*/
void opGen_adicionarRegistrosComIndice(BOOLEAN *vParada, char *buffer, FuncoesGenericas *funcoes)
{
    // Leitura do nome do arquivo binário
    leia_lerString(stdin, vParada, buffer, TAMANHO_BUFFER, true);
    FILE *arquivoDados = fopen(buffer, "rb+");
    if (!arquivoDados)
    {
        printf(ERRO_FALHA_ARQUIVO_MENSAGEM); // Erro na abertura do arquivo
        return;
    }

    // Leitura do nome do arquivo de índice
    leia_lerString(stdin, vParada, buffer, TAMANHO_BUFFER, true);
    FILE *arquivoIndice = fopen(buffer, "rb+");
    if (!arquivoIndice)
    {
        printf(ERRO_FALHA_ARQUIVO_MENSAGEM); // Erro na abertura do arquivo
        fclose(arquivoDados);
        return;
    }

    // Armazenar o nome do arquivo de índice para futura chamada de binarioNaTela
    char *nomeArquivo = (char *)malloc(sizeof(char) * (strlen(buffer) + 1));
    strcpy(nomeArquivo, buffer);

    int nRegistros = leia_lerInteiro(stdin, vParada);

    // Adicionar registro por registro caso não tenha ocorrido nenhum erro.
    int erro = adicionarRegistrosComIndiceAuxiliar(arquivoDados, arquivoIndice, buffer, nRegistros, funcoes);
    imprimeErro(erro);

    // Fechando arquivos e chamando a binarioNaTela
    fclose(arquivoDados);
    fclose(arquivoIndice);
    if (!erro)
        binarioNaTela(nomeArquivo);
    free(nomeArquivo);
}

/*
 * 
 *
 *      AQUI COMEÇAM AS FUNÇÕES DO TRABALHO PRÁTICO 3
 *
 * 
 */

/*
    Função que efetivamente faz a ordenação dos registros de um arquivo de dados, e coloca em um
        novo arquivo passado.
*/
int opGen_ordenaArquivoDeDadosAuxiliar(FILE *arquivoInicial, FILE *arquivoFinal, char *nomeDoCampo, FuncoesGenericas *funcoes)
{
    // Estruturas auxiliares para lidar com funções genéricas
    void *cabecalho = funcoes->gerarCabecalho();
    CabecalhoBase *cabecalhoBase = funcoes->retornarCabecalhoBase(cabecalho);

    // Ler cabeçalho do arquivo inicial
    global_ioCabecalhoBaseBin(arquivoInicial, cabecalhoBase, &fread);

    // Erro de status inconsistente
    if (cabecalhoBase->status == '0')
    {
        free(cabecalho);
        return ERRO_FALHA_ARQUIVO;
    }

    // Lendo o cabeçalho completamente
    fseek(arquivoInicial, 0L, SEEK_SET);
    funcoes->ioCabecalhoBin(arquivoInicial, cabecalho, &fread);

    // Lendo registros do arquivo inicial e guardando-os em RAM
    int numeroDeRegistros = cabecalhoBase->nroRegistros;
    void **registros = (void **)malloc(sizeof(void *) * numeroDeRegistros);

    // Para cada registro no arquivo de dados
    for (int regIndex = 0; regIndex < numeroDeRegistros;)
    {
        // Criar um novo registro em RAM
        void *registro = funcoes->gerarRegistro();
        RegistroBase *registroBase = funcoes->retornarRegistroBase(registro);

        // Ler a base do registro (campos "removido" e "tamanhoRegistro")
        global_ioRegistroBaseBin(arquivoInicial, registroBase, &fread);

        // Pular registro caso ele tenha sido removido
        if (registroBase->removido == '0')
        {
            fseek(arquivoInicial, registroBase->tamanhoRegistro, SEEK_CUR);
            free(registro);
        }
        else
        {
            // Ler o restante do registro e armazená-lo no vetor de registros em RAM
            funcoes->lerRegistroBin(arquivoInicial, registro, false);
            registros[regIndex] = registro;
            regIndex++;
        }
    }

    // Ordenar vetor de registros em RAM
    if (!funcoes->ordena(registros, numeroDeRegistros, nomeDoCampo))
    {
        // O campo não é suportado. Liberar a memória e retornar erro.
        for (int regIndex = 0; regIndex < numeroDeRegistros; regIndex++)
        {
            funcoes->freeRegistroInternamente(registros[regIndex]);
            free(registros[regIndex]);
        }
        free(registros);
        free(cabecalho);
        return ERRO_FALHA_ARQUIVO;
    }

    // Escrever status inconsistente e cabeçalho no arquivo final
    global_atualizarStatusBin(arquivoFinal, '0', cabecalhoBase);
    fseek(arquivoFinal, 0L, SEEK_SET);
    funcoes->ioCabecalhoBin(arquivoFinal, cabecalho, (size_t(*)(void *, size_t, size_t, FILE *)) & fwrite);

    // Escrever registro por registro no arquivo final
    for (int regIndex = 0; regIndex < numeroDeRegistros; regIndex++)
    {
        funcoes->escreverRegistroBin(arquivoFinal, registros[regIndex]);
        // Liberação de memória
        funcoes->freeRegistroInternamente(registros[regIndex]);
        free(registros[regIndex]);
    }

    // Atualizar cabeçalho base
    cabecalhoBase->byteProxReg = ftell(arquivoFinal);
    cabecalhoBase->nroRegistros = numeroDeRegistros;
    cabecalhoBase->nroRegRemovidos = 0;
    fseek(arquivoFinal, 0L, SEEK_SET);
    global_ioCabecalhoBaseBin(arquivoFinal, cabecalhoBase, (size_t(*)(void *, size_t, size_t, FILE *)) & fwrite);

    // Status consistente
    global_atualizarStatusBin(arquivoFinal, '1', cabecalhoBase);

    // Liberação de memória
    free(registros);
    free(cabecalho);

    return SUCESSO;
}

/*
    Função que lê um arquivo de dados e gera um novo arquivo de dados com os registros ordenados.

*/
void opGen_ordenaArquivoDeDados(BOOLEAN *vParada, char *buffer, FuncoesGenericas *funcoes)
{
    // Leitura do nome do arquivo de dados inicial
    leia_lerString(stdin, vParada, buffer, TAMANHO_BUFFER, true);
    FILE *arquivoInicial = fopen(buffer, "rb");
    if (!arquivoInicial)
    {
        printf(ERRO_FALHA_ARQUIVO_MENSAGEM); // Erro na abertura do arquivo
        return;
    }

    // Leitura do nome do arquivo de dados final
    leia_lerString(stdin, vParada, buffer, TAMANHO_BUFFER, true);
    FILE *arquivoFinal = fopen(buffer, "wb");
    if (!arquivoFinal)
    {
        printf(ERRO_FALHA_ARQUIVO_MENSAGEM); // Erro na abertura do arquivo
        fclose(arquivoInicial);
        return;
    }

    // Armazenar o nome do arquivo de dados final para futura chamada de binarioNaTela
    char *nomeArquivo = (char *)malloc(sizeof(char) * (strlen(buffer) + 1));
    strcpy(nomeArquivo, buffer);

    // Armazenar nome do campo de ordenação
    leia_lerString(stdin, vParada, buffer, TAMANHO_BUFFER, true);
    char *nomeDoCampo = (char *)malloc(sizeof(char) * (strlen(buffer) + 1));
    strcpy(nomeDoCampo, buffer);

    // Ordenar os registros do arquivo inicial e colocar no arquivo final
    int erro = opGen_ordenaArquivoDeDadosAuxiliar(arquivoInicial, arquivoFinal, nomeDoCampo, funcoes);
    imprimeErro(erro);

    // Fechando arquivos e chamando a binarioNaTela
    fclose(arquivoInicial);
    fclose(arquivoFinal);
    free(nomeDoCampo);
    if (!erro)
        binarioNaTela(nomeArquivo);
    free(nomeArquivo);
}