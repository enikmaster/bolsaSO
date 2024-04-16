#pragma once
#include <stddef.h>
#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define MAX_EMPRESA_ACAO 5  //var ambiente dps
#define MAX_EMPRESAS 30     //var ambiente dps
#define MAX_USERS 20        //var ambiente dps
#define TAM_COMANDO 100      //var ambiente dps
#define TAM_NOME 50         //var ambiente dps
#define TAM_PASSWORD 50     //var ambiente dps
#define TAM 100             //define usado no registry

// Mensagens de erro
#define INVALID_N_ARGS _T("[ERRO] N�mero de argumentos inv�lido\n")
#define INVALID_CMD _T("[ERRO] Comando inv�lido\n")
#define ERRO_OPEN_FILE _T("[ERRO] N�o foi poss�vel abrir o ficheiro\n")
#define ERRO_MEM_ALLOC _T("[ERRO] Erro ao alocar mem�ria para o utilizador\n")
#define ERRO_ADDC _T("[ERRO] Erro ao adicionar a empresa\n")
#define ERRO_LOAD _T("[ERRO] Erro ao carregar as empresas\n")
#define ERRO_STOCK _T("[ERRO] Erro a altera o valor da a��o\n")

#define ERRO_LOGIN _T("[ERRO] Erro ao efetuar login\n")
#define ERRO_NO_LOGIN _T("[ERRO] Efetue o login primeiro\n")
#define ERRO_ALREADY_LOGIN _T("[ERRO] J� efetuou login\n")

// Mensagens de informa��o
#define INFO_ADDC _T("[INFO] Empresa adicionada com sucesso\n")
#define INFO_LOAD _T("[INFO] Empresas carregadas com sucesso\n")
#define INFO_STOCK _T("[INFO] Valor alterado com sucesso\n")

#define INFO_LOGIN _T("[INFO] Login efetuado com sucesso\n")

// TODO: alterar os valores hardcoded dos tamanhos dos arrays para outros valores

// Tipos de transa��o
typedef enum {
    TTransacao_COMPRA,
    TTransacao_VENDA
} TipoTransacao;

// Tipos de mensagem
typedef enum {
    TMensagem_LOGIN,
    TMensagem_LOGOUT,
    TMensagem_COMPRA,
    TMensagem_VENDA,
    TMensagem_LISTA_EMPRESAS,
    TMensagem_VERIFICA_SALDO,
    TMensagem_RESPOSTA
} TipoMensagem;

// Estrutura que representa informa��es gerais de uma empresa
typedef struct Empresa Empresa, * pEmpresa;
struct Empresa {
    TCHAR nome[TAM_NOME];
    DWORD quantidadeAcoes;
    double valorAcao;
};

// Estrutura que representa a rela��o entre uma empresa e as a��es que um utilizador tem dessa empresa
typedef struct EmpresaAcao EmpresaAcao, * pEmpresaAcao;
struct EmpresaAcao {
    TCHAR nomeEmpresa[50];
    DWORD quantidadeAcoes;
};

// Estrutura que representa um utilizador do sistema
typedef struct Utilizador Utilizador, * pUtilizador;
struct Utilizador {
    TCHAR username[TAM_NOME];
    TCHAR password[TAM_PASSWORD];
    double saldo;
    boolean ligado;
    EmpresaAcao carteiraAcoes[MAX_EMPRESA_ACAO];
};

// Estrutura que detalha transa��o de compra ou venda
typedef struct DetalhesTransacao DetalhesTransacao, * pDetalhesTransacao;
struct DetalhesTransacao {
    TipoTransacao TipoT;
    TCHAR nomeEmpresa[TAM_NOME];
    DWORD quantidadeAcoes;
    double precoPorAcao;
};


// Estrutura para comunica��o entre cliente e servidor
typedef struct Mensagem Mensagem, * pMensagem;
struct Mensagem {
    TipoMensagem TipoM;
    TCHAR data[256]; // TODO: alterar o tamanho do array para deixar de ser harcoded
};

// Estrutura de Mem�ria partilhada
typedef struct DadosPartilhados DadosPartilhados;
struct DadosPartilhados {
    Empresa empresas[MAX_EMPRESAS];
    DWORD numEmpresas;
    DetalhesTransacao ultimaTransacao;
};