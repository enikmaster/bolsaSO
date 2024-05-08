#pragma once
#include <stddef.h>
#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

// Constantes
#define TAM_MAX_EMPRESA_ACAO 5  // carteira de ações
#define TAM_MAX_EMPRESAS 30 // tamanho máximo de empresas
#define TAM_MAX_USERS 20 // tamanho máximo de utilizadores
#define TAM_COMANDO 100 // tamanho máximo de um comando
#define TAM_NOME 50 // tamanho máximo de um nome
#define TAM_PASSWORD 50 // tamanho máximo de uma password
#define TAM_REGISTRY 100 // tamanho máximo de uma key do registo
#define COMANDO _T("Comando:  ")

// Nomes
#define NOME_SHARED_MEMORY _T("Dados_Partilhados")
#define NOME_SEMAFORO _T("Semaforo_Bolsa")
#define NOME_MUTEX_BOARD _T("Mutex_Board")
#define NOME_REGISTRY_KEY_NCLIENTES _T("SOFTWARE\\SO2\\NCLIENTES")
#define NOME_NAMED_PIPE _T("\\\\.\\pipe\\Pipe_Servidor_Cliente")

#define NOME_EVENTO_BOARD _T("EventoComunicacaoBolsa")

// Mensagens de erro do sistema
#define ERRO_INVALID_N_ARGS _T("[ERRO] Número de argumentos inválido\n")
#define ERRO_INVALID_CMD _T("[ERRO] Comando inválido\n")
#define ERRO_OPEN_FILE _T("[ERRO] Não foi possível abrir o ficheiro\n")
#define ERRO_MEM_ALLOC _T("[ERRO] Erro ao alocar memória para o utilizador\n")
#define ERRO_CREATE_KEY_NCLIENTES _T("[ERRO] Erro ao criar a key NCLIENTES\n")
#define ERRO_CREATE_FILE_MAPPING _T("[ERRO] Erro ao criar file mapping\n")
#define ERRO_CREATE_MAP_VIEW _T("[ERRO] Erro ao criar file mapping view\n")
#define ERRO_OPEN_FILE_MAPPING _T("[ERRO] Erro ao abrir file mapping\n")
#define ERRO_CREATE_SEM _T("[ERRO] Erro ao criar semáforo\n")
#define ERRO_CREATE_MUTEX _T("[ERRO] Erro ao criar mutex\n")
#define ERRO_CREATE_NAMED_PIPE _T("[ERRO] Erro ao criar named pipe\n")
#define ERRO_CREATE_THREAD _T("[ERRO] Erro ao criar thread\n")
#define ERRO_CREATE_EVENT _T("[ERRO] Erro ao criar evento\n")
#define ERRO_RESET_EVENT _T("[ERRO] Erro ao resetar evento\n")
#define ERRO_ESPERAR_THREADS _T("[ERRO] Erro ao esperar que as threads terminem\n")
#define ERRO_MAX_CLIENTES _T("[ERRO] Limite máximo de clientes ativos atingido\n")
#define ERRO_INICIALIZAR_DTO _T("[ERRO] Erro a incializar o sistema\n")
#define ERRO_READ_PIPE _T("[ERRO] Erro ao ler do named pipe\n")
#define ERRO_CONNECT_NAMED_PIPE _T("[ERRO] Erro ao conectar ao named pipe\n")
#define ERRO_PIPE_BUSY _T("[ERRO] O named pipe está ocupado\n")
#define ERRO_BROKEN_PIPE _T("[ERRO] A conexão foi interrompida\n")
#define ERRO_SET_PIPE_STATE _T("[ERRO] Erro ao definir o estado do named pipe\n")
#define ERRO_LEITURA_MSG _T("[ERRO] Erro ao ler a mensagem\n")
#define ERRO_ESCRITA_MSG _T("[ERRO] Erro ao escrever a mensagem\n")
#define ERRO_INVALID_MSG _T("[ERRO] Mensagem inválida\n")
#define ERRO_MEMORIA _T("[ERRO] Erro ao alocar memória\n")
#define ERRO_REGISTRY _T("[ERRO] Erro ao ler o registo\n")
#define ERRO_SEM_JA_INICIADO _T("[ERRO] O semáforo já foi iniciado (OUTRO BOLSA LIGADO)\n")

// Mensages de erro do administrador
#define ERRO_ADDC _T("[ERRO] Erro ao adicionar a empresa\n")
#define ERRO_LOAD _T("[ERRO] Erro ao carregar as empresas\n")
#define ERRO_STOCK _T("[ERRO] Erro a altera o valor da ação\n")

// Mensagens de erro do utilizador
#define ERRO_LOGIN _T("[ERRO] Erro ao efetuar login\n")
#define ERRO_NO_LOGIN _T("[ERRO] Efetue o login primeiro\n")
#define ERRO_ALREADY_LOGIN _T("[ERRO] Já efetuou login\n")
#define ERRO_EMPRESA_NAO_EXISTE _T("[ERRO] A empresa não existe\n")
#define ERRO_COMPRA _T("[ERRO] Não foi possível efetuar a compra\n")
#define ERRO_VENDA _T("[ERRO] Não foi possível efetuar a venda\n")

// Mensagens de informação
#define INFO_ADDC _T("[INFO] A empresa %s foi adicionada à bolsa\n")
#define INFO_LOAD _T("[INFO] Foram adicionadas %lu empresas com sucesso\n")
#define INFO_STOCK _T("[INFO] Valor da empresa %s alterado com sucesso para %.2lf\n")
#define INFO_LOGIN _T("[INFO] Login efetuado com sucesso\n")
#define INFO_LISTC _T("Nome: %s \tAções disponíveis: %lu \tPreço atual por ação: %.2lf\n")
#define INFO_LISTC_VAZIA _T("[INFO] Não existem empresas disponíveis\n")
#define INFO_USERS _T("Username: %s \tSaldo: %lf \tEstado: %s\n")
#define INFO_CLIENTE_CONECTADO _T("[INFO] Cliente conectado, thread criada e lançada\n")
#define INFO_SALDO _T("[INFO] Saldo atual: %.2lf\n")
#define INFO_COMPRA _T("[INFO] Compra de acões da empresa %s efetuada com sucesso\n")
#define INFO_VENDA _T("[INFO] Venda de acões da empresa %s efetuada com sucesso\n")
#define INFO_WALLET_VAZIA _T("[INFO] Carteira de ações vazia\n")
#define INFO_WALLET _T("Empresa: %s \tQuantidade: %lu\n")
#define INFO_ULTIMA_TRANSACAO _T("Ultima Transação: %s - %lu e %.2f\n")


#define INFO_TOP_EMPRESAS _T("Top %d Empresas Mais Valiosas:\n")
#define INFO_EMPRESA_ACOES _T("%d. %s - Ações: %lu, Valor por Ação: $%.2f, Valor de Mercado: $%.2f\n"
#define INFO_EMPRESAS_VAZIA _T("[ERRO] Não existem empresas cotadas em bolsa\n")
#define INFO_NUMERO_EMPRESAS _T("[ERRO] Número de empresas inválido (1 - 10)\n")
#define INFO_ERRO_DPARTILHADOS _T("[ERRO]_ A estrutura DadosPartilhados está vazia\n")

// Mensagens de debug
#define DEBUGGER _T("\n[DEBUG] Estou aqui\n")

// Tipos de transação
typedef enum {
    TTransacao_COMPRA,
    TTransacao_VENDA
} TipoTransacao;

// Tipos de mensagem
typedef enum {
    // Cliente -> Servidor
    TMensagem_LOGIN,
    TMensagem_R_LOGIN, // resposta ao login
    TMensagem_LISTC,
    TMensagem_R_LISTC, // resposta ao listc
    TMensagem_BUY,
    TMensagem_R_BUY, // resposta ao buy
    TMensagem_SELL,
    TMensagem_R_SELL, // resposta ao sell
    TMensagem_BALANCE,
    TMensagem_R_BALANCE, // resposta ao balance
    TMensagem_WALLET,
    TMensagem_R_WALLET, // resposta ao wallet
    TMensagem_EXIT,
    // Servidor -> Cliente todas em broadcast
    TMensagem_ADDC,
    TMensagem_STOCK,
    TMensagem_PAUSE,
    TMensagem_RESUME,
    TMensagem_LOAD,
    TMensagem_CLOSE
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
    EmpresaAcao carteiraAcoes[TAM_MAX_EMPRESA_ACAO];
    DWORD numEmpresasAcoes;
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
    TCHAR nome[TAM_NOME];
    TCHAR empresa[TAM_NOME];
    TCHAR password[TAM_PASSWORD];
    BOOL sucesso;
    DWORD quantidade;
    Empresa empresas[TAM_MAX_EMPRESAS];
    double valor;
    EmpresaAcao carteiraAcoes[TAM_MAX_EMPRESA_ACAO];
    BOOL continuar;
};

// Estrutura de Memória partilhada
typedef struct DadosPartilhados DadosPartilhados;
struct DadosPartilhados {
    Empresa empresas[TAM_MAX_EMPRESAS];
    DWORD numEmpresas;
    DetalhesTransacao ultimaTransacao;
    HANDLE hEvent; // handle p evento de alteração da board
};

// Estrutura de mecanismos de sincorização
typedef struct Sync Sync, * pSync;
struct Sync {
    HANDLE hSemBolsa;
    HANDLE hMtxBolsa;
    CRITICAL_SECTION csContinuar;
    CRITICAL_SECTION csLimClientes;
    CRITICAL_SECTION csEmpresas;
    CRITICAL_SECTION csUtilizadores;
    CRITICAL_SECTION csWrite;
};

// Estrutura de dados partilhados entre threads do servidor
typedef struct DataTransferObject DataTransferObject;
struct DataTransferObject {
    HANDLE hMap;
    PVOID pView;
    pSync pSync;
	DadosPartilhados* dadosP;
    DWORD numUtilizadores;
    Utilizador utilizadores[TAM_MAX_USERS];
    DWORD limiteClientes;
    BOOL continuar;
};

// Estrutura para lidar com threads no servidor
typedef struct ThreadData ThreadData;
struct ThreadData {
    DataTransferObject* dto;
    HANDLE hPipeInst;
    BOOL livre; // indica se a thread está livre para ser usada
};

// Estrutura para lidar com threads no cliente
typedef struct ClienteData ClienteData;
struct ClienteData {
	HANDLE hPipe;
    BOOL logado;
    HANDLE hExitEvent;
};