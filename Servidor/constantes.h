#pragma once
#include <stddef.h>

#define MAX_EMPRESAS 5 //var ambiente dps

// TODO: alterar os valores hardcoded dos tamanhos dos arrays para outros valores

// Estrutura que representa informações gerais de uma empresa
typedef struct Empresa Empresa, * pEmpresa;
struct Empresa {
    char nome[50];
    unsigned long quantidadeAcoes;
    double valorAcao;
};

// Estrutura que representa um utilizador do sistema
typedef struct Utilizador Utilizador, * pUtilizador;
struct Utilizador {
    char username[50];
    char password[50];
    unsigned long saldo;
};

// Estrutura que representa a relação entre uma empresa e as ações que um utilizador tem dessa empresa
typedef struct EmpresaAcao EmpresaAcao, * pEmpresaAcao;
struct EmpresaAcao {
    char nomeEmpresa[50];
    unsigned long quantidadeAcoes;
};

// Estrutura que detalha transação de compra ou venda
typedef struct DetalhesTransacao DetalhesTransacao, * pDetalhesTransacao;
struct DetalhesTransacao {
    TipoTransacao TipoT;
    char nomeEmpresa[50];
    unsigned long quantidadeAcoes;
    double precoPorAcao;
};

typedef enum {
    COMPRA,
    VENDA
} TipoTransacao;

typedef enum {
    LOGIN,
    LOGOUT,
    COMPRA,
    VENDA,
    LISTA_EMPRESAS,
    VERIFICA_SALDO,
    RESPOSTA
} TipoMensagem;

// Estrutura para comunicação entre cliente e servidor
typedef struct Mensagem {
    TipoMensagem TipoM;
    char data[256];
};

/*// Estrutura que representa a carteira de ações de um utilizador
typedef struct CarteiraAcoes {
    EmpresaAcao acoes[MAX_EMPRESAS];
    size_t numAcoes;
} CarteiraAcoes;


typedef struct HistoricoTransacao {
    TipoTransacao tipo;      // COMPRA ou VENDA
    char nomeEmpresa[50];
    DWORD quantidadeAcoes;
    double precoTransacao;
} HistoricoTransacao;
*/