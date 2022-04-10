/*
    Este módulo tem a função de aplicar um algoritmo de junção sobre
    dois registros.

    Campos suportados:
        codLinha (Veiculo + Linha) 
*/

#include <juncao.h>
#include <veiculo.h>
#include <linha.h>
#include <stdlib.h>
#include <leia.h>
#include <string.h>
#include <arvore-b.h>

/*

    STRUCTS

*/

/*
    Struct que serve de container para vários parâmetros passados às funções
        de algoritmo.
*/
typedef struct
{
    // Entrada
    BOOLEAN *vParada;
    char *buffer;

    // Veículo
    void *veiculoCabecalho;
    void *veiculoRegistro;
    CabecalhoBase *veiculoCabecalhoBase;
    RegistroBase *veiculoRegistroBase;

    // Linha
    void *linhaCabecalho;
    void *linhaRegistro;
    CabecalhoBase *linhaCabecalhoBase;
    RegistroBase *linhaRegistroBase;
} Params;

/*

    INTERFACE PRIVADA

*/

/*
    Função auxiliar que executa o algoritmo de junção LOOP ANINHADO 
*/
static int veiculoLinhaLoopAninhado(FILE *binVeiculo, FILE *binLinha, Params *p)
{
    // Ler cabeçalho
    veiculo_ioCabecalhoBin(binVeiculo, p->veiculoCabecalho, &fread);
    linha_ioCabecalhoBin(binLinha, p->linhaCabecalho, &fread);

    long long int linhaByteOffsetPrimeiroRegistro = ftell(binLinha);
    int nroRegistrosExibidos = 0;

    /* LOOP EXTERNO */

    // Passa registro por registro dentro do arquivo de VEICULO
    for (int veiculoIndex = 0; veiculoIndex < p->veiculoCabecalhoBase->nroRegistros;)
    {
        global_ioRegistroBaseBin(binVeiculo, p->veiculoRegistroBase, &fread);

        // Pular registros removidos
        if (p->veiculoRegistroBase->removido == '0')
            fseek(binVeiculo, p->veiculoRegistroBase->tamanhoRegistro, SEEK_CUR);
        else
        {

            // Ler restante do registro
            veiculo_lerRegistroBin(binVeiculo, p->veiculoRegistro, false);

            // Voltar ao início do arquivo de linha
            fseek(binLinha, linhaByteOffsetPrimeiroRegistro, SEEK_SET);

            /* LOOP INTERNO */

            // Passa registro por registro dentro do arquivo de LINHA
            for (int linhaIndex = 0; linhaIndex < p->linhaCabecalhoBase->nroRegistros;)
            {
                global_ioRegistroBaseBin(binLinha, p->linhaRegistroBase, &fread);

                // Pular registros removidos
                if (p->linhaRegistroBase->removido == '0')
                    fseek(binLinha, p->linhaRegistroBase->tamanhoRegistro, SEEK_CUR);
                else
                {
                    // Ler restante do registro
                    linha_lerRegistroBin(binLinha, p->linhaRegistro, false);

                    // Checa compatibilidade
                    if (veiculo_getCodLinha(p->veiculoRegistro) == linha_getCodLinha(p->linhaRegistro))
                    {
                        nroRegistrosExibidos++;
                        veiculo_printRegistro(stdout, p->veiculoRegistro, p->veiculoCabecalho);
                        linha_printRegistro(stdout, p->linhaRegistro, p->linhaCabecalho);
                        fputc('\n', stdout);

                        // Não é necessário continuar percorrendo o arquivo de linha,
                        // já que se trata de uma chave primária
                        linha_freeRegistroInternamente(p->linhaRegistro);
                        break;
                    }

                    // Libera espaço armazenado dentro do registro
                    linha_freeRegistroInternamente(p->linhaRegistro);
                    linhaIndex++;
                }
            }

            // Libera espaço armazenado dentro do registro
            veiculo_freeRegistroInternamente(p->veiculoRegistro);
            veiculoIndex++;
        }
    }

    // Ou não há registros no arquivo, ou nenhum registro é compatível
    if (nroRegistrosExibidos == 0)
        return ERRO_REGISTRO;

    return SUCESSO;
}

/*
    Função auxiliar que executa o algoritmo de junção LOOP ÚNICO 
*/
static int veiculoLinhaLoopUnico(FILE *binVeiculo, FILE *binLinha, Params *p)
{
    // Leitura do nome do arquivo de índice
    leia_lerString(stdin, p->vParada, p->buffer, TAMANHO_BUFFER, true);
    FILE *indiceLinha = fopen(p->buffer, "rb");
    if (!indiceLinha)
        return ERRO_FALHA_ARQUIVO;

    // Configuração da árvore B
    ArvoreB *arvoreB = arvoreB_criar(indiceLinha, ARVORE_B_ORDEM, ARVORE_B_TAMANHO_NO, true);

    // Erro de status inconsistente
    if (arvoreB_getStatus(arvoreB) == '0')
    {
        arvoreB_free(arvoreB, true);
        return ERRO_FALHA_ARQUIVO;
    }

    // Ler cabeçalho
    veiculo_ioCabecalhoBin(binVeiculo, p->veiculoCabecalho, &fread);
    linha_ioCabecalhoBin(binLinha, p->linhaCabecalho, &fread);

    int nroRegistrosExibidos = 0;

    // Passa registro por registro dentro do arquivo de VEICULO
    for (int veiculoIndex = 0; veiculoIndex < p->veiculoCabecalhoBase->nroRegistros;)
    {
        global_ioRegistroBaseBin(binVeiculo, p->veiculoRegistroBase, &fread);

        // Pular registros removidos
        if (p->veiculoRegistroBase->removido == '0')
            fseek(binVeiculo, p->veiculoRegistroBase->tamanhoRegistro, SEEK_CUR);
        else
        {

            // Ler restante do registro
            veiculo_lerRegistroBin(binVeiculo, p->veiculoRegistro, false);

            // Faz a pesquisa no arquivo de índice usando Árvore B
            long long int byteOffset = arvoreB_buscar(arvoreB, veiculo_getCodLinha(p->veiculoRegistro));

            // Se o registro for encontrado
            if (byteOffset != -1)
            {
                // Procura no arquivo de dados.
                fseek(binLinha, byteOffset, SEEK_SET);
                linha_lerRegistroBin(binLinha, p->linhaRegistro, true);

                // Se o registro não estiver removido
                if (p->linhaRegistroBase->removido == '1')
                {
                    nroRegistrosExibidos++;
                    veiculo_printRegistro(stdout, p->veiculoRegistro, p->veiculoCabecalho);
                    linha_printRegistro(stdout, p->linhaRegistro, p->linhaCabecalho);
                    fputc('\n', stdout);
                }

                linha_freeRegistroInternamente(p->linhaRegistro);
            }

            // Libera espaço armazenado dentro do registro
            veiculo_freeRegistroInternamente(p->veiculoRegistro);
            veiculoIndex++;
        }
    }

    // Liberação da memória alocada pela árvore B
    arvoreB_free(arvoreB, true);

    // Ou não há registros no arquivo, ou nenhum registro é compatível
    if (nroRegistrosExibidos == 0)
        return ERRO_REGISTRO;

    return SUCESSO;
}

/*
    Função auxiliar que executa o algoritmo de junção ORDENAÇÃO-INTERCALAÇÃO 
*/
static int veiculoLinhaOrdenacaoIntercalacao(FILE *binVeiculo, FILE *binLinha, Params *p)
{
    // Criação do arquivo com veículos ordenados
    strcpy(p->buffer, "veiculoOrdenado.bin");
    FILE *binVeiculoOrdenado = fopen(p->buffer, "wb+");
    if (!binVeiculoOrdenado)
        return ERRO_FALHA_ARQUIVO;

    // Criação do arquivo com linhas ordenadas
    strcpy(p->buffer, "linhaOrdenado.bin");
    FILE *binLinhaOrdenado = fopen(p->buffer, "wb+");
    if (!binLinhaOrdenado)
    {
        fclose(binVeiculoOrdenado);
        return ERRO_FALHA_ARQUIVO;
    }

    // Gerando ordenação para VEÍCULO (aproveitando a função auxiliar do operador genérico)
    FuncoesGenericas *funcBuffer = veiculo_funcoesGenericas();
    int erroVeiculo = opGen_ordenaArquivoDeDadosAuxiliar(binVeiculo, binVeiculoOrdenado, "codLinha", funcBuffer);
    free(funcBuffer);

    // Gerando ordenação para LINHA (aproveitando a função auxiliar do operador genérico)
    funcBuffer = linha_funcoesGenericas();
    int erroLinha = opGen_ordenaArquivoDeDadosAuxiliar(binLinha, binLinhaOrdenado, "codLinha", funcBuffer);
    free(funcBuffer);

    // Se ocorreu algum erro durante a ordenação
    if (erroVeiculo || erroLinha)
    {
        fclose(binVeiculoOrdenado);
        fclose(binLinhaOrdenado);
        return ERRO_FALHA_ARQUIVO;
    }

    // Voltando ao início dos dois arquivos ordenados
    int nroRegistrosExibidos = 0;
    fseek(binVeiculoOrdenado, 0L, SEEK_SET);
    fseek(binLinhaOrdenado, 0L, SEEK_SET);

    // Ler cabeçalho
    veiculo_ioCabecalhoBin(binVeiculoOrdenado, p->veiculoCabecalho, &fread);
    linha_ioCabecalhoBin(binLinhaOrdenado, p->linhaCabecalho, &fread);

    // Variáveis auxiliares
    BOOLEAN lerLinha = true;
    BOOLEAN lerVeiculo = true;
    int veiculoCodLinha = -1, linhaCodLinha = -1;

    // Passa registro por registro dentro dos arquivos ordenados, de maneira intercalada
    //      O registro de veículo só será lido no loop quando "lerVeiculo" for true
    //      O registro de linha só será lido no loop quando "lerVeiculo" for true
    //      O loop ocorre até o momento que um dos arquivos chegue ao fim
    for (int veiculoIndex = 0, linhaIndex = 0; veiculoIndex < p->veiculoCabecalhoBase->nroRegistros && linhaIndex < p->linhaCabecalhoBase->nroRegistros;)
    {
        // Ler o próximo registro no arquivo de VEÍCULO
        if (lerVeiculo)
        {
            // Leitura da base (campos "removido" e "tamanhoRegistro")
            global_ioRegistroBaseBin(binVeiculoOrdenado, p->veiculoRegistroBase, &fread);

            // Checando se removido
            if (p->veiculoRegistroBase->removido == '0')
            {
                fseek(binVeiculoOrdenado, p->veiculoRegistroBase->tamanhoRegistro, SEEK_CUR);
                continue; // Se foi removido, ler o próximo registro
            }

            // Não foi removido, logo será usado para verificação posterior
            lerVeiculo = false;

            // Leitura do restante do registro e armazenando o codLinha
            veiculo_lerRegistroBin(binVeiculoOrdenado, p->veiculoRegistro, false);
            veiculoCodLinha = veiculo_getCodLinha(p->veiculoRegistro);
        }

        // Ler o próximo registro no arquivo de LINHA
        if (lerLinha)
        {
            // Leitura da base (campos "removido" e "tamanhoRegistro")
            global_ioRegistroBaseBin(binLinhaOrdenado, p->linhaRegistroBase, &fread);

            // Checando se removido
            if (p->linhaRegistroBase->removido == '0')
            {
                fseek(binLinhaOrdenado, p->linhaRegistroBase->tamanhoRegistro, SEEK_CUR);
                continue; // Se foi removido, ler o próximo registro
            }

            // Não foi removido, logo será usado para verificação posterior
            lerLinha = false;

            // Leitura do restante do registro e armazenando o codLinha
            linha_lerRegistroBin(binLinhaOrdenado, p->linhaRegistro, false);
            linhaCodLinha = linha_getCodLinha(p->linhaRegistro);
        }

        if (veiculoCodLinha > linhaCodLinha)
        {
            // Nesse caso, ler a próxima linha
            linha_freeRegistroInternamente(p->linhaRegistro);
            linhaIndex++;
            lerLinha = true;
        }
        else if (veiculoCodLinha < linhaCodLinha)
        {
            // Nesse caso, ler o próximo veículo
            veiculo_freeRegistroInternamente(p->veiculoRegistro);
            veiculoIndex++;
            lerVeiculo = true;
        }
        else
        {
            // Houve compatibilidade! Imprimir os dois registros
            nroRegistrosExibidos++;
            veiculo_printRegistro(stdout, p->veiculoRegistro, p->veiculoCabecalho);
            linha_printRegistro(stdout, p->linhaRegistro, p->linhaCabecalho);
            fputc('\n', stdout);

            // Nesse caso, ler o próximo veículo (possibilidade em que há
            //  vários veículos com mesma linha)
            veiculo_freeRegistroInternamente(p->veiculoRegistro);
            veiculoIndex++;
            lerVeiculo = true;
        }
    }

    // Arquivo de linha chegou ao fim
    if (lerLinha)
        veiculo_freeRegistroInternamente(p->veiculoRegistro);
    // Arquivo de veículo chegou ao fim
    else if (lerVeiculo)
        linha_freeRegistroInternamente(p->linhaRegistro);

    // Liberação de memória
    fclose(binVeiculoOrdenado);
    fclose(binLinhaOrdenado);

    // Ou não há registros no arquivo, ou nenhum registro é compatível
    if (nroRegistrosExibidos == 0)
        return ERRO_REGISTRO;
    return SUCESSO;
}

/*

    INTERFACE PÚBLICA

*/

/*
    Função que chamará as outras funções, executando o código que é comum a todas as outras.
*/
void juncao_veiculoLinha(BOOLEAN *vParada, char *buffer, JUNCAO_ALGORITMO algoritmo)
{

    // Vetor com ponteiros para os algoritmos
    int (*algoritmos[JUNCAO_NUMERO_DE_ALGORITMOS])(FILE *, FILE *, Params *);
    algoritmos[JUNCAO_LOOP_ANINHADO] = &veiculoLinhaLoopAninhado;
    algoritmos[JUNCAO_LOOP_UNICO] = &veiculoLinhaLoopUnico;
    algoritmos[JUNCAO_ORDENACAO_INTERCALACAO] = &veiculoLinhaOrdenacaoIntercalacao;

    // Leitura do nome do arquivo de veículo
    leia_lerString(stdin, vParada, buffer, TAMANHO_BUFFER, true);
    FILE *binVeiculo = fopen(buffer, "rb");
    if (!binVeiculo)
    {
        printf(ERRO_FALHA_ARQUIVO_MENSAGEM); // Erro na abertura do arquivo
        return;
    }

    // Leitura do nome do arquivo de linha
    leia_lerString(stdin, vParada, buffer, TAMANHO_BUFFER, true);
    FILE *binLinha = fopen(buffer, "rb");
    if (!binLinha)
    {
        printf(ERRO_FALHA_ARQUIVO_MENSAGEM); // Erro na abertura do arquivo
        fclose(binVeiculo);
        return;
    }

    // Checar status
    char statusVeiculo = global_lerStatusBin(binVeiculo);
    char statusLinha = global_lerStatusBin(binLinha);

    if (statusVeiculo == '0' || statusLinha == '0')
    {
        // Status inconsistente
        printf(ERRO_FALHA_ARQUIVO_MENSAGEM);
        fclose(binVeiculo);
        fclose(binLinha);
        return;
    }

    // Voltar ao início dos arquivos
    fseek(binVeiculo, 0L, SEEK_SET);
    fseek(binLinha, 0L, SEEK_SET);

    // Verificar se os campos passados são "codLinha codLinha"
    leia_lerString(stdin, vParada, buffer, TAMANHO_BUFFER, true);
    int condCampoVeiculo = strcmp(buffer, "codLinha");

    leia_lerString(stdin, vParada, buffer, TAMANHO_BUFFER, true);
    int condCampoLinha = strcmp(buffer, "codLinha");

    if (condCampoVeiculo || condCampoLinha)
    {
        // Campo não suportado
        printf(ERRO_REGISTRO_MENSAGEM);
        fclose(binVeiculo);
        fclose(binLinha);
        return;
    }

    // Armazenando os parâmetros comuns a todos os algoritmos

    Params *p = (Params *)malloc(sizeof(Params));

    // Entrada
    p->buffer = buffer;

    p->vParada = vParada;
    // Veiculo
    p->veiculoCabecalho = veiculo_gerarCabecalho();
    p->veiculoRegistro = veiculo_gerarRegistro();
    p->veiculoCabecalhoBase = veiculo_retornarCabecalhoBase(p->veiculoCabecalho);
    p->veiculoRegistroBase = veiculo_retornarRegistroBase(p->veiculoRegistro);

    // Linha
    p->linhaCabecalho = linha_gerarCabecalho();
    p->linhaRegistro = linha_gerarRegistro();
    p->linhaCabecalhoBase = linha_retornarCabecalhoBase(p->linhaCabecalho);
    p->linhaRegistroBase = linha_retornarRegistroBase(p->linhaRegistro);

    // Exibir todos os registros e, caso necessário, exibir mensagem de erro
    imprimeErro(algoritmos[algoritmo](binVeiculo, binLinha, p));

    // Liberação de memória
    free(p->veiculoCabecalho);
    free(p->veiculoRegistro);
    free(p->linhaCabecalho);
    free(p->linhaRegistro);
    free(p);
    fclose(binVeiculo);
    fclose(binLinha);
}