#pragma once
#include <stddef.h>
#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

// Constantes
#define TAM_MAX_EMPRESA_ACAO 5  // carteira de a��es
#define TAM_MAX_EMPRESAS 30 // tamanho m�ximo de empresas
#define TAM_MAX_USERS 20 // tamanho m�ximo de utilizadores
#define TAM_COMANDO 100 // tamanho m�ximo de um comando
#define TAM_NOME 50 // tamanho m�ximo de um nome
#define TAM_PASSWORD 50 // tamanho m�ximo de uma password
#define TAM_REGISTRY 100 // tamanho m�ximo de uma key do registo
#define NUM_THREADS_SERVER 2
#define MAX_TENTATIVAS_LIGACAO 20 // n�mero m�ximo de tentativas de liga��o ao named pipe
#define COMANDO _T("Comando:  ")
#define WELCOME _T("Bem-vindo ao sistema de compra e venda de a��es\n")
#define COMANDO_CLOSE _T("Escreva 'close' para terminar o programa:\n")

// Nomes
#define NOME_SHARED_MEMORY _T("Dados_Partilhados")
#define NOME_SEMAFORO _T("Semaforo_Bolsa")
#define NOME_MUTEX_BOARD _T("Mutex_Board")
#define NOME_REGISTRY_KEY_NCLIENTES _T("SOFTWARE\\SO2\\NCLIENTES")
#define NOME_NAMED_PIPE _T("\\\\.\\pipe\\Pipe_Servidor_Cliente")

#define NOME_EVENTO_BOARD _T("EventoComunicacaoBolsa")
#define NOME_EVENTO_EXIT _T("EventoSairBolsa")

// Mensagens de erro do sistema
#define ERRO_INVALID_N_ARGS _T("[ERRO] N�mero de argumentos inv�lido\n")
#define ERRO_INVALID_CMD _T("[ERRO] Comando inv�lido\n")
#define ERRO_OPEN_FILE _T("[ERRO] N�o foi poss�vel abrir o ficheiro\n")
#define ERRO_CREATE_KEY_NCLIENTES _T("[ERRO] Erro ao criar a key NCLIENTES\n")
#define ERRO_CREATE_FILE_MAPPING _T("[ERRO] Erro ao criar file mapping\n")
#define ERRO_CREATE_MAP_VIEW _T("[ERRO] Erro ao criar file mapping view\n")
#define ERRO_OPEN_FILE_MAPPING _T("[ERRO] Erro ao abrir file mapping\n")
#define ERRO_CREATE_SEM _T("[ERRO] Erro ao criar sem�foro\n")
#define ERRO_CREATE_MUTEX _T("[ERRO] Erro ao criar mutex\n")
#define ERRO_CREATE_NAMED_PIPE _T("[ERRO] Erro ao criar named pipe\n")
#define ERRO_CREATE_THREAD _T("[ERRO] Erro ao criar thread\n")
#define ERRO_CREATE_EVENT _T("[ERRO] Erro ao criar evento\n")
#define ERRO_RESET_EVENT _T("[ERRO] Erro ao resetar evento\n")
#define ERRO_ESPERAR_THREADS _T("[ERRO] Erro ao esperar que as threads terminem\n")
#define ERRO_MAX_CLIENTES _T("[ERRO] Limite m�ximo de clientes ativos atingido\n")
#define ERRO_INICIALIZAR_DTO _T("[ERRO] Erro a incializar o sistema\n")
#define ERRO_READ_PIPE _T("[ERRO] Erro ao ler do named pipe\n")
#define ERRO_CONNECT_NAMED_PIPE _T("[ERRO] Erro ao conectar ao named pipe\n")
#define ERRO_PIPE_BUSY _T("[ERRO] O named pipe est� ocupado\n")
#define ERRO_BROKEN_PIPE _T("[ERRO] A conex�o foi interrompida\n")
#define ERRO_SET_PIPE_STATE _T("[ERRO] Erro ao definir o estado do named pipe\n")
#define ERRO_LEITURA_MSG _T("[ERRO] Erro ao ler a mensagem\n")
#define ERRO_ESCRITA_MSG _T("[ERRO] Erro ao escrever a mensagem\n")
#define ERRO_INVALID_MSG _T("[ERRO] Mensagem inv�lida\n")
#define ERRO_MEMORIA _T("[ERRO] Erro ao alocar mem�ria\n")
#define ERRO_REGISTRY _T("[ERRO] Erro ao ler o registo\n")
#define ERRO_CREATE_TIMER _T("[ERRO] Erro ao criar o timer\n")
#define ERRO_SET_TIMER _T("[ERRO] Erro ao definir o timer\n")
#define ERRO_MAX_TENTATIVAS _T("[ERRO] N�mero m�ximo de tentativas de liga��es atingido\n")
#define ERRO_LIGAR_BOLSA _T("[ERRO] N�o existe nenhum servidor Bolsa para liga��o\n")
#define ERRO_SEM_JA_INICIADO _T("[ERRO] O sem�foro j� foi iniciado (OUTRO BOLSA LIGADO)\n")
#define ERRO_NUMERO_EMPRESAS _T("[ERRO] N�mero de empresas inv�lido (1 - 10)\n")
#define ERRO_DPARTILHADOS _T("[ERRO] A estrutura DadosPartilhados est� vazia\n")

// Mensages de erro do administrador
#define ERRO_ADDC _T("[ERRO] Erro ao adicionar a empresa\n")
#define ERRO_LOAD _T("[ERRO] Erro ao carregar as empresas\n")
#define ERRO_STOCK _T("[ERRO] Erro a altera o valor da a��o\n")

// Mensagens de erro do utilizador
#define ERRO_LOGIN _T("[ERRO] Erro ao efetuar login\n")
#define ERRO_NO_LOGIN _T("[ERRO] Efetue o login primeiro\n")
#define ERRO_ALREADY_LOGIN _T("[ERRO] J� efetuou login\n")
#define ERRO_EMPRESA_NAO_EXISTE _T("[ERRO] A empresa n�o existe\n")
#define ERRO_COMPRA _T("[ERRO] N�o foi poss�vel efetuar a compra\n")
#define ERRO_VENDA _T("[ERRO] N�o foi poss�vel efetuar a venda\n")

// Mensagens de informa��o
#define INFO_ADDC _T("[INFO] A empresa %s foi adicionada � bolsa\n")
#define INFO_LOAD _T("[INFO] Foram adicionadas %lu empresas com sucesso\n")
#define INFO_STOCK _T("[INFO] Valor da empresa %s alterado com sucesso para %.2lf\n")
#define INFO_LOGIN _T("[INFO] Login efetuado com sucesso\n")
#define INFO_LISTC _T("Nome: %s \tA��es dispon�veis: %lu \tPre�o atual por a��o: %.2lf\n")
#define INFO_LISTC_VAZIA _T("[INFO] N�o existem empresas dispon�veis\n")
#define INFO_USERS _T("Username: %s \tSaldo: %lf \tEstado: %s\n")
#define INFO_CLIENTE_CONECTADO _T("[INFO] Cliente conectado, thread criada e lan�ada\n")
#define INFO_CLIENTE_DESCONECTADO _T("[INFO] Cliente %s saiu\n")
#define INFO_SALDO _T("[INFO] Saldo atual: %.2lf\n")
#define INFO_COMPRA _T("[INFO] Compra de a��es da empresa %s efetuada com sucesso\n")
#define INFO_VENDA _T("[INFO] Venda de a��es da empresa %s efetuada com sucesso\n")
#define INFO_WALLET_VAZIA _T("[INFO] Carteira de a��es vazia\n")
#define INFO_WALLET _T("Empresa: %s \tQuantidade: %lu\n")
#define INFO_TRANSACAO_COMPRA _T("Ultima Transa��o: Foram compradas %lu a��es da empresa %s por %.2f (%.2f por a��o)\n")
#define INFO_TRANSACAO_VENDA _T("Ultima Transa��o: Foram vendidas %lu a��es da empresa %s por %.2f (%.2f por a��o)\n")
#define INFO_TRANSACAO_UPDATE _T("Atualiza��o de valores das empresas\n")
#define INFO_CLOSEC _T("[INFO] O programa bolsa est� a terminar e este tamb�m\n")
#define INFO_CLOSEB _T("[INFO] O programa board est� a terminar\n")
#define INFO_PAUSE _T("[INFO] As transa��es est�o suspensas por %lu segundos\n")
#define INFO_JA_PAUSADO _T("[INFO] As transa��es j� est�o suspensas\n")
#define INFO_PAUSE_END _T("[INFO] A interrup��o foi suspensa porque o programa bolsa vair terminar\n")
#define INFO_RESUME _T("[INFO] As transa��es foram retomadas\n")


#define INFO_TOP_EMPRESAS _T("Top %d Empresas Mais Valiosas:\n")
#define INFO_EMPRESA_ACOES _T("%d. %s - A��es: %lu, Valor por A��o: %.2f, Valor de Mercado: %.2f\n")
#define INFO_EMPRESAS_VAZIA _T("[INFO] Ainda n�o existem empresas cotadas em bolsa\n")

// Mensagens de debug
#define DEBUGGER _T("\n[DEBUG] Estou aqui\n")

// Tipos de trnsa��o
typedef enum {
    TTransacao_COMPRA,
    TTransacao_VENDA,
    TTransacao_UPDATE
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

// Estruturas
typedef struct Empresa Empresa, * pEmpresa;
typedef struct EmpresaAcao EmpresaAcao, * pEmpresaAcao;
typedef struct Utilizador Utilizador, * pUtilizador;
typedef struct DetalhesTransacao DetalhesTransacao, * pDetalhesTransacao;
typedef struct Mensagem Mensagem, * pMensagem;
typedef struct DadosPartilhados DadosPartilhados;
typedef struct Sync Sync, * pSync;
typedef struct DataTransferObject DataTransferObject;
typedef struct ThreadData ThreadData;
typedef struct ClienteData ClienteData;
typedef struct EstadoBoard EstadoBoard;

// Estrutura que representa informa��es gerais de uma empresa
struct Empresa {
    TCHAR nome[TAM_NOME];
    DWORD quantidadeAcoes;
    double valorAcao;
};

// Estrutura que representa a rela��o entre uma empresa e as a��es que um utilizador tem dessa empresa
struct EmpresaAcao {
    TCHAR nomeEmpresa[50];
    DWORD quantidadeAcoes;
};

// Estrutura que representa um utilizador do sistema
struct Utilizador {
    TCHAR username[TAM_NOME];
    TCHAR password[TAM_PASSWORD];
    double saldo;
    BOOL logado;
    EmpresaAcao carteiraAcoes[TAM_MAX_EMPRESA_ACAO];
    DWORD numEmpresasAcoes;
};

// Estrutura que detalha transa��es de compra ou venda
struct DetalhesTransacao {
    TipoTransacao TipoT;
    TCHAR nomeEmpresa[TAM_NOME];
    DWORD quantidadeAcoes;
    double precoPorAcao;
};

// Estrutura para comunica��o entre cliente e servidor
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

// Estrutura de mem�ria partilhada
struct DadosPartilhados {
    Empresa empresas[TAM_MAX_EMPRESAS];
    DWORD numEmpresas;
    DetalhesTransacao ultimaTransacao;
};

// Estrutura de mecanismos de sincroniza��o
struct Sync {
    HANDLE hMtxBolsa;
    CRITICAL_SECTION csContinuar;
    CRITICAL_SECTION csLimClientes;
    CRITICAL_SECTION csEmpresas;
    CRITICAL_SECTION csUtilizadores;
    CRITICAL_SECTION csWrite;
};

// Estrutura de dados partilhados entre threads do servidor
struct DataTransferObject {
    HANDLE hMap;
    PVOID pView;
    pSync pSync;
    DadosPartilhados* dadosP;
    DWORD numUtilizadores;
    Utilizador utilizadores[TAM_MAX_USERS];
    DWORD limiteClientes;
    BOOL continuar;
    BOOL pausado;
    DWORD numSegundos;
    HANDLE hLimiteClientes;
    HANDLE hExitEvent; // handle para evento de sa�da
    HANDLE hUpdateEvent; // handle p evento de altera��o da board
};

// Estrutura para lidar com threads no servidor
struct ThreadData {
    DataTransferObject* dto;
    HANDLE hPipeInst;
    BOOL livre; // indica se a thread est� livre para ser usada
};

// Estrutura para lidar com threads no cliente
struct ClienteData {
    BOOL logado;
    HANDLE hPipe;
    HANDLE hExitEvent;
    HANDLE hMutex;
};

// Estrutura para lidar com threads no board
struct EstadoBoard {
    HANDLE hMap;
    DWORD N;
    DadosPartilhados* pDados;
    HANDLE eventoEscrita;
    HANDLE eventoExit;
    HANDLE eventoLocalExit;
};