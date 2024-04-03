#pragma once
#include <stddef.h>
#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define MAX_EMPRESAS 5 //var ambiente dps
#define TAM_COMANDO 50 //var ambiente dps

// Mensagens de erro
#define INVALID_N_ARGS _T("[ERRO] Número de argumentos inválido\n")
#define INVALID_CMD _T("[ERRO] Comando inválido\n")
#define ERROR_ADDC _T("[ERRO] Erro ao adicionar a empresa\n")

// TODO: alterar os valores hardcoded dos tamanhos dos arrays para outros valores

// Tipos de transação
typedef enum {
    COMPRA,
    VENDA
} TipoTransacao;

// Tipos de mensagem
typedef enum {
    LOGIN,
    LOGOUT,
    OPERACAO_COMPRA,
    OPERACAO_VENDA,
    LISTA_EMPRESAS,
    VERIFICA_SALDO,
    RESPOSTA
} TipoMensagem;

// Estrutura que representa informações gerais de uma empresa
typedef struct Empresa Empresa, * pEmpresa;
struct Empresa {
    char nome[50];
    DWORD quantidadeAcoes;
    double valorAcao;
};

// Estrutura que representa um utilizador do sistema
typedef struct Utilizador Utilizador, * pUtilizador;
struct Utilizador {
    char username[50];
    char password[50];
    double saldo;
};

// Estrutura que representa a relação entre uma empresa e as ações que um utilizador tem dessa empresa
typedef struct EmpresaAcao EmpresaAcao, * pEmpresaAcao;
struct EmpresaAcao {
    char nomeEmpresa[50];
    DWORD quantidadeAcoes;
};

// Estrutura que detalha transação de compra ou venda
typedef struct DetalhesTransacao DetalhesTransacao, * pDetalhesTransacao;
struct DetalhesTransacao {
    TipoTransacao TipoT;
    char nomeEmpresa[50];
    DWORD quantidadeAcoes;
    double precoPorAcao;
};


// Estrutura para comunicação entre cliente e servidor
typedef struct Mensagem Mensagem, * pMensagem;
struct Mensagem {
    TipoMensagem TipoM;
    char data[256]; // TODO: alterar o tamanho do array para deixar de ser harcoded
};