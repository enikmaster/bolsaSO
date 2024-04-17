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

// Nomes
#define SHM_NAME _T("Dados_Partilhados")
#define SEM_NAME _T("Semaforo_Bolsa")
#define MTX_BOARD _T("Mutex_Board")


// Mensagens de erro
#define INVALID_N_ARGS _T("[ERRO] Número de argumentos inválido\n")
#define INVALID_CMD _T("[ERRO] Comando inválido\n")
#define ERRO_OPEN_FILE _T("[ERRO] Não foi possível abrir o ficheiro\n")
#define ERRO_MEM_ALLOC _T("[ERRO] Erro ao alocar memória para o utilizador\n")
#define ERRO_CREATE_FILE_MAPPING _T("[ERRO] Erro ao criar file mapping\n")
#define ERRO_CREATE_MAP_VIEW _T("[ERRO] Erro ao criar file mapping view\n")
#define ERRO_CREATE_SEM _T("[ERRO] Erro ao criar semáforo\n")
#define ERRO_CREATE_MUTEX _T("[ERRO] Erro ao criar mutex\n")

#define ERRO_INICIALIZAR_DTO _T("[ERRO] Erro a incializar o sistema\n")

// Mensages de erro de comandos do administrador
#define ERRO_ADDC _T("[ERRO] Erro ao adicionar a empresa\n")
#define ERRO_LOAD _T("[ERRO] Erro ao carregar as empresas\n")
#define ERRO_STOCK _T("[ERRO] Erro a altera o valor da ação\n")

#define ERRO_LOGIN _T("[ERRO] Erro ao efetuar login\n")
#define ERRO_NO_LOGIN _T("[ERRO] Efetue o login primeiro\n")
#define ERRO_ALREADY_LOGIN _T("[ERRO] Já efetuou login\n")

// Mensagens de informação
#define INFO_ADDC _T("[INFO] Empresa adicionada com sucesso\n")
#define INFO_LOAD _T("[INFO] Empresas carregadas com sucesso\n")
#define INFO_STOCK _T("[INFO] Valor alterado com sucesso\n")

#define INFO_LOGIN _T("[INFO] Login efetuado com sucesso\n")

// TODO: alterar os valores hardcoded dos tamanhos dos arrays para outros valores

// Tipos de transação
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

// Estrutura que representa informações gerais de uma empresa
typedef struct Empresa Empresa, * pEmpresa;
struct Empresa {
    TCHAR nome[TAM_NOME];
    DWORD quantidadeAcoes;
    double valorAcao;
};

// Estrutura que representa a relação entre uma empresa e as ações que um utilizador tem dessa empresa
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
    BOOL logado;
    EmpresaAcao carteiraAcoes[MAX_EMPRESA_ACAO];
};

// Estrutura que detalha transação de compra ou venda
typedef struct DetalhesTransacao DetalhesTransacao, * pDetalhesTransacao;
struct DetalhesTransacao {
    TipoTransacao TipoT;
    TCHAR nomeEmpresa[TAM_NOME];
    DWORD quantidadeAcoes;
    double precoPorAcao;
};


// Estrutura para comunicação entre cliente e servidor
typedef struct Mensagem Mensagem, * pMensagem;
struct Mensagem {
    TipoMensagem TipoM;
    TCHAR data[256]; // TODO: alterar o tamanho do array para deixar de ser harcoded
};

// Estrutura de Memória partilhada
typedef struct DadosPartilhados DadosPartilhados;
struct DadosPartilhados {
    Empresa empresas[MAX_EMPRESAS];
    DWORD numEmpresas;
    DetalhesTransacao ultimaTransacao;
};

// Estrutura de dados partilhados entre threads do servidor
typedef struct DataTransferObject DataTransferObject;
struct DataTransferObject {
    HANDLE hMap;
    PVOID pView;
	HANDLE hSemBolsa;
	HANDLE hMtxBolsa;
    CRITICAL_SECTION cs;
	DadosPartilhados* sharedData;
    DWORD numUtilizadores;
    Utilizador utilizadores[MAX_USERS];
};