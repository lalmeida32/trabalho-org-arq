#include <stdio.h>
#include <global.h>
#include <arvore-b.h>
#include <stdlib.h>
#include <string.h>

/*

    CONSTANTES E STRUCTS

*/

#define INSERIR_HOUVE_PROMOCAO 1
#define INSERIR_SUCESSO 0
#define INSERIR_ERRO_CHAVE_JA_EXISTE -1

// Estrutura para trafegar os dados da promoção da inserção
typedef struct
{
    int chave;
    long long int referencia;
    int ponteiro;
} ArvoreBPromocao;

// Estrutura para guardar um registro de dados
typedef struct
{
    char folha;
    int nroChavesIndexadas;
    int RRNdoNo;

    // IMPORTANTE: Vi o vídeo do aulão novamente e vi a recomendação para
    //      usar P, C e Pr
    // Não conseguirei arrumar a tempo, mas acredito que os nomes são sugestivos
    int *chaves;                // C
    long long int *referencias; // Pr
    int *ponteiros;             // P

} ArvoreBRegistro;

// Estrutura principal que vai guardar informações úteis para diversas operações
//      da interface pública, a fim de diminuir o número de acessos a disco
struct ArvoreB_t
{
    FILE *arquivo;
    int ordem;
    int tamanhoNo;
    int noRaiz;
    int RRNproxNo;
};

/*

    INTERFACE PRIVADA

*/

/*
    Funções para operar com ArvoreBRegistro
*/

//  Cria um registro do arquivo de índice (em RAM)
static ArvoreBRegistro *registro_criar(int RRN, int ordem, char folha)
{
    // Alocação dinâmica
    ArvoreBRegistro *registro = (ArvoreBRegistro *)malloc(sizeof(ArvoreBRegistro));

    // Principais informações
    registro->folha = folha;
    registro->nroChavesIndexadas = 0;
    registro->RRNdoNo = RRN;

    // Criando dinamicamente vetores para as chaves, suas referências e ponteiros
    // Usando a função memset para preencher com o valor -1

    // Chaves -> tamanho do vetor "ordem - 1"
    registro->chaves = (int *)malloc(sizeof(int) * (ordem - 1));
    memset(registro->chaves, -1, sizeof(int) * (ordem - 1));

    // Referências -> tamanho do vetor "ordem - 1"
    registro->referencias = (long long int *)malloc(sizeof(long long int) * (ordem - 1));
    memset(registro->referencias, -1, sizeof(long long int) * (ordem - 1));

    // Ponteiros -> tamanho do vetor "ordem - 1"
    registro->ponteiros = (int *)malloc(sizeof(int) * ordem);
    memset(registro->ponteiros, -1, sizeof(int) * ordem);

    return registro;
}

//  Realiza uma busca binária pela chave
//  IMPORTANTE: Se não encontrar a chave, retorna o índice do primeiro valor
//      maior ou igual a chave. Caso a chave seja maior que todos os valores do vetor,
//      retorna um índice "fora de escopo", isto é, com o valor do tamanho do vetor.
static int registro_buscaPorChave(ArvoreBRegistro *registro, int chave)
{
    int l = 0;
    int r = registro->nroChavesIndexadas - 1;

    //  Algoritmo iterativo da busca binária, em que divide-se
    //      o vetor ordenado ao meio em cada iteração
    while (l <= r)
    {
        int meio = (l + r) / 2;

        if (chave < registro->chaves[meio])
            r = meio - 1;
        else if (chave > registro->chaves[meio])
            l = meio + 1;
        else
            return meio;
    }

    // Não encontrou a chave. Retornar índice do "sucessor"
    return l;
}

//  Insere uma chave no registro de dados (em RAM), de maneira ordenada
//  Retorna FALSE caso não haja espaço no vetor e TRUE caso contrário
static BOOLEAN registro_inserirChaveOrdenado(ArvoreBRegistro *registro, int ordem, int chave, long long int referencia, int ponteiro)
{
    // Não há espaço no vetor para realizar a inserção
    if (registro->nroChavesIndexadas == ordem - 1)
        return false;

    // Passando pelos registros do final para o começo, realizando um shift right
    //      enquanto os valores forem maiores que a chave desejada.
    //  |----||----||----||----||----|
    //  |  1 ||  4 ||  7 ||  8 ||  8 | chave: 3
    //  |----||----||----||----||----|
    //                       |     ^
    //                       |-----|

    // Após iterações
    //  |----||----||----||----||----|
    //  |  1 ||  4 ||  4 ||  7 ||  8 | chave: 3
    //  |----||----||----||----||----|

    //  |----||----||----||----||----|
    //  |  1 ||  3 ||  4 ||  7 ||  8 | chave: 3
    //  |----||----||----||----||----|     |
    //           ^                         |
    //           |-------------------------|

    int indice;
    for (indice = registro->nroChavesIndexadas; indice > 0 && registro->chaves[indice - 1] > chave; indice--)
    {
        registro->chaves[indice] = registro->chaves[indice - 1];
        registro->referencias[indice] = registro->referencias[indice - 1];
        registro->ponteiros[indice + 1] = registro->ponteiros[indice];
    }
    registro->chaves[indice] = chave;
    registro->referencias[indice] = referencia;
    registro->ponteiros[indice + 1] = ponteiro;

    registro->nroChavesIndexadas++;

    return true;
}

// Preenche os valores de um registro com -1 e zera o número de chaves indexadas
static void registro_limpar(ArvoreBRegistro *registro)
{
    registro->ponteiros[0] = -1;
    for (int i = 0; i < registro->nroChavesIndexadas; i++)
    {
        registro->chaves[i] = -1;
        registro->referencias[i] = -1;
        registro->ponteiros[i + 1] = -1;
    }
    registro->nroChavesIndexadas = 0;
}

// Libera a memória alocada dinâmicamente por um registro
static void registro_free(ArvoreBRegistro *registro)
{
    free(registro->chaves);
    free(registro->referencias);
    free(registro->ponteiros);
    free(registro);
}

// Pode receber como parâmetro a função "fread" ou "fwrite"
//      -> "fread":  Envia dados do registro do DISCO para a RAM, sobrescrevendo os dados anteriores
//      -> "fwrite": Envia dados do registro da RAM para o DISCO, sobrescrevendo os dados anteriores
static void registro_io(ArvoreBRegistro *registro, FILE *arquivo, int ordem, int tamanhoNo, size_t (*f)(void *, size_t, size_t, FILE *))
{
    // Vai para a posição do registro no arquivo, usando o seu RRN
    fseek(arquivo, (registro->RRNdoNo + 1) * tamanhoNo, SEEK_SET);

    // Executa a função "fread" ou "fwrite" para cada um dos campos
    f(&registro->folha, sizeof(char), 1, arquivo);
    f(&registro->nroChavesIndexadas, sizeof(int), 1, arquivo);
    f(&registro->RRNdoNo, sizeof(int), 1, arquivo);

    // Executa o mesmo para os campos de ponteiros, chaves e referências, mas na forma de vetor
    f(&registro->ponteiros[0], sizeof(int), 1, arquivo);
    for (int i = 1; i < ordem; i++)
    {
        f(&registro->chaves[i - 1], sizeof(int), 1, arquivo);
        f(&registro->referencias[i - 1], sizeof(long long int), 1, arquivo);
        f(&registro->ponteiros[i], sizeof(int), 1, arquivo);
    }
}

/*
    Funções para operar com ArvoreB
*/

//  Função auxiliar da inserirRecursivo. É executada quando há espaço no registro para inserir uma nova chave,
//      seja ela a chave dada para a função inserir, ou uma chave de promoção.
//  Retorna INSERIR_SUCESSO pois não há promoção nem erro de "chave já existente"
static int arvoreB_inserirRecursivo_temEspaco(ArvoreBRegistro *registro, ArvoreB *arvoreB)
{
    registro_io(registro, arvoreB->arquivo, arvoreB->ordem, arvoreB->tamanhoNo,
                (size_t(*)(void *, size_t, size_t, FILE *)) & fwrite);
    registro_free(registro);
    return INSERIR_SUCESSO;
}

//  Função auxiliar da inserirRecursivo. É executada quando NÃO há espaço no registro para inserir uma nova chave,
//      seja ela a chave dada para a função inserir, ou uma chave de promoção.
//  Retornar INSERIR_HOUVE_PROMOCAO para informar a chamada recursiva de cima que há informação útil dentro de promocao
//      e que essa informação deve ser tratada
static int arvoreB_inserirRecursivo_naoTemEspaco(ArvoreBRegistro *registro, ArvoreB *arvoreB, int chave,
                                                 long long int referencia, int ponteiro, ArvoreBPromocao *promocao)
{
    // Cria um novo registro e aloca dinamicamente vetores auxiliares (com 1 espaço a mais que o normal, para
    //      que a nova chave caiba)
    ArvoreBRegistro *novoRegistro = registro_criar(arvoreB->RRNproxNo++, arvoreB->ordem, registro->folha);
    int *chaves = (int *)malloc(arvoreB->ordem * sizeof(int));
    long long int *referencias = (long long int *)malloc(arvoreB->ordem * sizeof(long long int));
    int *ponteiros = (int *)malloc((arvoreB->ordem + 1) * sizeof(int));

    // Armazenando dentro dos vetores todas as chaves, referencias e ponteiros que são
    //      MENORES QUE A CHAVE
    int i = 0;
    ponteiros[0] = registro->ponteiros[0];
    for (; i < arvoreB->ordem - 1; i++)
    {
        if (chave < registro->chaves[i])
            break;
        chaves[i] = registro->chaves[i];
        referencias[i] = registro->referencias[i];
        ponteiros[i + 1] = registro->ponteiros[i + 1];
    }

    // Armazenando dentro dos vetores a CHAVE DADA
    chaves[i] = chave;
    referencias[i] = referencia;
    ponteiros[i + 1] = ponteiro;

    // Armazenando dentro dos vetores todas as chaves, referencias e ponteiros que são
    //      MAIORES QUE A CHAVE
    for (; i < arvoreB->ordem - 1; i++)
    {
        chaves[i + 1] = registro->chaves[i];
        referencias[i + 1] = registro->referencias[i];
        ponteiros[i + 2] = registro->ponteiros[i + 1];
    }

    // Nesse ponto, dentro dos vetores auxiliares os valores estão ordenados pela chave.

    // Verificando em qual posição deve ocorrer o SPLIT
    int promocaoIndice = arvoreB->ordem / 2;
    promocao->chave = chaves[promocaoIndice];
    promocao->referencia = referencias[promocaoIndice];
    promocao->ponteiro = novoRegistro->RRNdoNo;

    // Realizando o SPLIT
    // Limpando o registro dado, já que as informações das chaves, referências e ponteiros estão seguras
    //      nos vetores auxiliares, e ordenadas
    registro_limpar(registro);

    // Colocando a parte da ESQUERDA do split no registro original
    registro->ponteiros[0] = ponteiros[0];
    for (int i = 0; i < promocaoIndice; i++)
    {
        registro->chaves[i] = chaves[i];
        registro->referencias[i] = referencias[i];
        registro->ponteiros[i + 1] = ponteiros[i + 1];
        registro->nroChavesIndexadas++;
    }

    // Colocando a parte da DIREITA do split no registro novo
    novoRegistro->ponteiros[0] = ponteiros[promocaoIndice + 1];
    for (int i = promocaoIndice + 1, j = 0; i < arvoreB->ordem; i++, j++)
    {
        novoRegistro->chaves[j] = chaves[i];
        novoRegistro->referencias[j] = referencias[i];
        novoRegistro->ponteiros[j + 1] = ponteiros[i + 1];
        novoRegistro->nroChavesIndexadas++;
    }

    // Escrevendo no disco
    registro_io(registro, arvoreB->arquivo, arvoreB->ordem, arvoreB->tamanhoNo,
                (size_t(*)(void *, size_t, size_t, FILE *)) & fwrite);
    registro_io(novoRegistro, arvoreB->arquivo, arvoreB->ordem, arvoreB->tamanhoNo,
                (size_t(*)(void *, size_t, size_t, FILE *)) & fwrite);

    // Libera memória
    free(chaves);
    free(referencias);
    free(ponteiros);
    registro_free(registro);
    registro_free(novoRegistro);

    return INSERIR_HOUVE_PROMOCAO;
}

//  Função que busca recursivamente, registro por registro, uma chave no arquivo de índice
//      que implementa a estrutura Árvore B.
//  Caso haja sucesso, retorna o byte offset daquela chave, caso contrário, retorna -1
static long long int arvoreB_buscarRecursivo(ArvoreB *arvoreB, int RRN, int chave)
{
    // Ler registro atual do DISCO, e armazenando em RAM
    ArvoreBRegistro *registro = registro_criar(RRN, arvoreB->ordem, '1');
    registro_io(registro, arvoreB->arquivo, arvoreB->ordem, arvoreB->tamanhoNo, &fread);

    // Procura dentro do registro a chave em questão
    int indiceDaChave = registro_buscaPorChave(registro, chave);
    if (indiceDaChave < arvoreB->ordem - 1 && registro->chaves[indiceDaChave] == chave)
    {
        // Encontrou! Retorna sua referência (byte offset) e libera e memória alocada
        long long int referencia = registro->referencias[indiceDaChave];
        registro_free(registro);
        return referencia;
    }

    if (registro->folha == '1')
    {
        // Chegou em um nó folha e não encontrou a chave
        // Libera a memória e retorna -1, indicando "não encontrado"
        registro_free(registro);
        return -1;
    }

    // "Desce" um nó da árvore, realizando uma busca recursiva no próximo registro
    // Abusa do fato de que a função "registro_buscaPorChave" retorna o índice do "sucessor" da chave caso
    //      não a encontre. Por isso, registro->ponteiros[indiceDaChave] é o registro correto.
    long long int retorno = arvoreB_buscarRecursivo(arvoreB, registro->ponteiros[indiceDaChave], chave);
    registro_free(registro);
    return retorno;
}

//  Tenta inserir uma chave em um nó folha da Árvore B, e realiza, de forma
//      recursiva, todos os tratamentos necessários.
static int arvoreB_inserirRecursivo(ArvoreB *arvoreB, int RRN, int chave, long long int referencia, ArvoreBPromocao *promocao)
{
    // Ler registro atual, do DISCO para a RAM, baseando-se no RRN.
    ArvoreBRegistro *registro = registro_criar(RRN, arvoreB->ordem, '1');
    registro_io(registro, arvoreB->arquivo, arvoreB->ordem, arvoreB->tamanhoNo, &fread);

    // Procura por uma chave no registro atual, lembrando que "indiceDaChave" deve indicar o "sucessor"
    //      da chave caso ela não tenha sido encontrada, funcionando bem para a chamada recursiva.
    int indiceDaChave = registro_buscaPorChave(registro, chave);
    if (indiceDaChave < arvoreB->ordem - 1 && registro->chaves[indiceDaChave] == chave)
    {
        // De fato encontrou a chave dentro do registro. Ímpossível inseri-la
        registro_free(registro);
        return INSERIR_ERRO_CHAVE_JA_EXISTE;
    }

    // Chegou em nó folha
    if (registro->folha == '1')
        // Verificando se há espaço para inserir a nova chave
        return registro_inserirChaveOrdenado(registro, arvoreB->ordem, chave, referencia, -1)
                   ? arvoreB_inserirRecursivo_temEspaco(registro, arvoreB)                                      // Se sim, insere normalmente
                   : arvoreB_inserirRecursivo_naoTemEspaco(registro, arvoreB, chave, referencia, -1, promocao); // Se não, realiza split e
                                                                                                                //      tratamento de promoção

    // Tenta inserir no próximo registro, cujo RRN está presente em registro->ponteiros[indiceDaChave]
    // Recebe como retorno INSERIR_SUCESSO, INSERIR_HOUVE_PROMOCAO ou INSERIR_ERRO_CHAVE_JA_EXISTE
    int retorno = arvoreB_inserirRecursivo(arvoreB, registro->ponteiros[indiceDaChave], chave, referencia, promocao);

    // É necessário realizar o tratamento da promoção no registro atual
    if (retorno == INSERIR_HOUVE_PROMOCAO)
    {
        int chavePromocao = promocao->chave;
        long long int referenciaPromocao = promocao->referencia;
        int ponteiroPromocao = promocao->ponteiro;

        // Tenta inserir a chave dada na promoção
        // Caso haja espaço, insere normalmente e retorna para as chamadas acima o valor INSERIR_SUCESSO
        // Caso contrário, realiza SPLIT novamente e retorna para as chamadas acima o valor INSERIR_HOUVE_PROMOCAO, bem como
        //      o registro "promocao" atualizado
        return registro_inserirChaveOrdenado(registro, arvoreB->ordem, chavePromocao, referenciaPromocao, ponteiroPromocao)
                   ? arvoreB_inserirRecursivo_temEspaco(registro, arvoreB)
                   : arvoreB_inserirRecursivo_naoTemEspaco(registro, arvoreB, chavePromocao, referenciaPromocao, ponteiroPromocao, promocao);
    }

    // No caso de INSERIR_SUCESSO ou INSERIR_ERRO_CHAVE_JA_EXISTE, não é necessário realizar nenhum tratamento.
    // Propaga o retorno para as chamadas recursivas.
    registro_free(registro);
    return retorno;
}

/*

    INTERFACE PÚBLICA

*/

//  Escreve o cabeçalho completo dentro do arquivo de índice.
//  Deve ser chamada apenas uma vez, depois do cabeçalho inicializado
//      deve-se utilizar apenas "atualizarCabecalho" e "setStatus"
void arvoreB_inicializarCabecalho(ArvoreB *arvoreB, char status)
{
    arvoreB_setStatus(arvoreB, status);
    arvoreB_atualizarCabecalho(arvoreB);
    for (int i = 0; i < arvoreB->tamanhoNo - 9; i++)
        fwrite("@", sizeof(char), 1, arvoreB->arquivo);
}

// Sobrescreve em DISCO apenas as informações contidas em RAM do cabeçalho, ou seja,
//      o noRaiz e RRNproxNo
void arvoreB_atualizarCabecalho(ArvoreB *arvoreB)
{
    fseek(arvoreB->arquivo, 1L, SEEK_SET);
    fwrite(&arvoreB->noRaiz, sizeof(int), 1, arvoreB->arquivo);
    fwrite(&arvoreB->RRNproxNo, sizeof(int), 1, arvoreB->arquivo);
}

// Cria em RAM uma estrutura para lidar com a árvore B.
// Recebe como parâmetro um valor booleano "sincronizar", que deve ser
//      TRUE apenas quando o arquivo de índice já existe e é necessário fazer um carregamento
//      das informações do cabeçalho do DISCO para a RAM
ArvoreB *arvoreB_criar(FILE *arquivo, int ordem, int tamanhoNo, BOOLEAN sincronizar)
{
    ArvoreB *arvoreB = (ArvoreB *)malloc(sizeof(ArvoreB));

    arvoreB->arquivo = arquivo;
    arvoreB->ordem = ordem;
    arvoreB->tamanhoNo = tamanhoNo;
    arvoreB->noRaiz = -1;
    arvoreB->RRNproxNo = 0;

    if (sincronizar)
    {
        // Informações de cabeçalho DISCO -> RAM
        fseek(arvoreB->arquivo, 1L, SEEK_SET);
        fread(&arvoreB->noRaiz, sizeof(int), 1, arvoreB->arquivo);
        fread(&arvoreB->RRNproxNo, sizeof(int), 1, arvoreB->arquivo);
    }

    return arvoreB;
}

// Libera a memória armazenada dinamicamente em RAM pela estrutra ArvoreB.
void arvoreB_free(ArvoreB *arvoreB, BOOLEAN freeArquivo)
{
    if (freeArquivo)
        fclose(arvoreB->arquivo);
    free(arvoreB);
}

// Altera o status do arquivo de índice, diretamente no arquivo.
void arvoreB_setStatus(ArvoreB *arvoreB, char status)
{
    fseek(arvoreB->arquivo, 0L, SEEK_SET);
    fwrite(&status, sizeof(char), 1, arvoreB->arquivo);
    fflush(arvoreB->arquivo);
}

// Recebe o status do arquivo de índice, diretamente do arquivo.
char arvoreB_getStatus(ArvoreB *arvoreB)
{
    char status;
    fseek(arvoreB->arquivo, 0L, SEEK_SET);
    fread(&status, sizeof(char), 1, arvoreB->arquivo);
    return status;
}

// Procura por uma chave dentro do arquivo de índice, utilizando o algoritmo de busca recursiva
//      na Árvore B.
// Retorna -1 caso a chave não seja encontrada.
long long int arvoreB_buscar(ArvoreB *arvoreB, int chave)
{
    if (arvoreB->noRaiz == -1)
        return -1; // Árvore não existe
    return arvoreB_buscarRecursivo(arvoreB, arvoreB->noRaiz, chave);
}

//  Tenta inserir uma chave em um nó folha da Árvore B, chamando como subrotina a função
//      recursiva
//  Retorna FALSE quando a chave já existe e por isso não foi possível realizar a inserção
//      e TRUE quando tudo opera bem
BOOLEAN arvoreB_inserir(ArvoreB *arvoreB, int chave, long long int referencia)
{

    // Árvore ainda não existe, é necessário inicializá-la
    if (arvoreB->noRaiz == -1)
    {
        // Atualizando as informações do nó raiz e escrevendo no arquivo de índice
        arvoreB->noRaiz = arvoreB->RRNproxNo;
        ArvoreBRegistro *raiz = registro_criar(arvoreB->RRNproxNo++, arvoreB->ordem, '1');
        registro_inserirChaveOrdenado(raiz, arvoreB->ordem, chave, referencia, -1);
        registro_io(raiz, arvoreB->arquivo, arvoreB->ordem, arvoreB->tamanhoNo,
                    (size_t(*)(void *, size_t, size_t, FILE *)) & fwrite);
        registro_free(raiz);
        return true;
    }

    // Armazena dinamicamente uma estrutura para lidar com a promoção de chaves
    ArvoreBPromocao *promocao = (ArvoreBPromocao *)malloc(sizeof(ArvoreBPromocao));
    // Chama a subrotina para tentar inserir recursivamente
    // Recebe como retorno INSERIR_SUCESSO, INSERIR_HOUVE_PROMOCAO ou INSERIR_ERRO_CHAVE_JA_EXISTE
    int retorno = arvoreB_inserirRecursivo(arvoreB, arvoreB->noRaiz, chave, referencia, promocao);

    // Caso em que há promoção no nó raiz. É criado um novo nó raiz e a chave é inserida.
    if (retorno == INSERIR_HOUVE_PROMOCAO)
    {
        // Criando novo nó raiz
        int noRaizAnterior = arvoreB->noRaiz;
        arvoreB->noRaiz = arvoreB->RRNproxNo;
        ArvoreBRegistro *raiz = registro_criar(arvoreB->RRNproxNo++, arvoreB->ordem, '0');

        // Inserindo a chave promovida e os ponteiros relacionados a ela
        raiz->ponteiros[0] = noRaizAnterior;
        registro_inserirChaveOrdenado(raiz, arvoreB->ordem, promocao->chave, promocao->referencia, promocao->ponteiro);

        // Escrevendo em disco e liberando a estrutura auxiliar
        registro_io(raiz, arvoreB->arquivo, arvoreB->ordem, arvoreB->tamanhoNo,
                    (size_t(*)(void *, size_t, size_t, FILE *)) & fwrite);
        registro_free(raiz);
    }

    // Libera a estrutura auxiliar e retorna FALSE caso o retorno da função
    // recursiva seja INSERIR_ERRO_CHAVE_JA_EXISTE e TRUE caso contrário
    free(promocao);
    return retorno != INSERIR_ERRO_CHAVE_JA_EXISTE;
}