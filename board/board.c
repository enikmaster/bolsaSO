/*

O Board vai apresentar:
[ ] A  lista das N emrpesas mais valiosas apresentadas por ordem decrescente de valor.
[ ] E apresenta a ultima transação realizada.

[ ] O valor de N é dado através de um argumento na linha de comandos.

[ ] BOARD comunica com a bolsa através de mem partilhada


O BOARD vai ter de receber comando do utiliazdor para terminar a execução
[ ]comando "close"

*/

#include "../Servidor/constantes.h"
#include "board.h"

typedef struct {
    volatile BOOL running;  // Marca como volátil para garantir visibilidade entre threads
    HANDLE hMap;
    DWORD N;
    DadosPartilhados* pDados;
    HANDLE eventoEscrita;
} EstadoBoard;

DWORD WINAPI WaitForCloseCommand(LPVOID param) {
    EstadoBoard* estado = (EstadoBoard*)param;
    TCHAR command[10];

    while (estado->running) {
        if (_tscanf_s(_T("%9s"), command, (unsigned)_countof(command)) > 0) {
            if (_tcsicmp(command, _T("close")) == 0) {
                estado->running = FALSE; //crit section
                SetEvent(estado->eventoEscrita);
            }
        }
    }

    return 0;
}

//arranjar lógica para só mostrar as N empresas e a ultima transação quando receber um evento do servidor
//para mostrar só quando houver alterações
DWORD WINAPI DisplayDados(LPVOID param) {
    EstadoBoard* estado = (EstadoBoard*)param;
    //DadosPartilhados* pDados = (DadosPartilhados*)(estado->hMap);

    while (estado->running) {
        WaitForSingleObject(estado->eventoEscrita, 3000); //espera por um evento
       
        OrganizarEExibirEmpresas(estado->pDados, estado->N);

        ResetEvent(estado->eventoEscrita);
    }
    return 0;
}


int _tmain(int argc, TCHAR** argv) {
#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif

    if (argc != 2) {
        _tprintf_s(ERRO_INVALID_N_ARGS);
        return -2;
    }

    DWORD N = _tstoi(argv[1]);
    if (N <= 0 || N > TAM_MAX_EMPRESAS) {
        _tprintf_s(_T("[ERRO] Número de empresas inválido (1 - 10)\n"));
        return -3;
    }

    EstadoBoard estado = { .running = TRUE, .hMap = NULL, .N = N, NULL};  // Inicializa a estrutura de estado

    estado.hMap = OpenFileMapping(FILE_MAP_READ, FALSE, NOME_SHARED_MEMORY);
    if (estado.hMap == NULL) {
        _tprintf(_T("Erro ao abrir o mapeamento do ficheiro: %d\n"), GetLastError());
        return -1;
    }

    estado.pDados = (DadosPartilhados*)MapViewOfFile(
        estado.hMap,
        FILE_MAP_READ,
        0,
        0,
        sizeof(DadosPartilhados));

    if (estado.pDados == NULL) {
        _tprintf(_T("Erro ao mapear a visualização do ficheiro: %d\n"), GetLastError());
        CloseHandle(estado.hMap);
        return -1;
    }

    //open do evento
    estado.eventoEscrita = OpenEventW(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, NOME_EVENTO_BOARD);
    if (estado.eventoEscrita == NULL) {
        _tprintf(_T("Erro ao abrir o evento: %d\n"), GetLastError());
        UnmapViewOfFile(estado.pDados);
        CloseHandle(estado.hMap);
        return -1;
    }

    // Reset the event
    if (!ResetEvent(estado.eventoEscrita)) {
        _tprintf(_T("Erro ao redefinir o evento: %d\n"), GetLastError());
        UnmapViewOfFile(estado.pDados);
        CloseHandle(estado.hMap);
        CloseHandle(estado.eventoEscrita);
        return -1;
    }

    HANDLE hThreadCommand = CreateThread(NULL, 0, WaitForCloseCommand, &estado, 0, NULL);
    HANDLE hThreadDisplay = CreateThread(NULL, 0, DisplayDados, &estado, 0, NULL);
    if(hThreadCommand == NULL || hThreadDisplay == NULL) {
		_tprintf(_T("Erro ao criar a thread: %d\n"), GetLastError());
		UnmapViewOfFile(estado.pDados);
		CloseHandle(estado.hMap);
        CloseHandle(hThreadCommand);
        CloseHandle(hThreadDisplay);
		return -1;
	}

   
    WaitForSingleObject(hThreadCommand, INFINITE);
    estado.running = FALSE;
   
    WaitForSingleObject(hThreadDisplay, INFINITE);

    // Limpeza e fecho de recursos
    UnmapViewOfFile(estado.pDados);
    CloseHandle(estado.hMap);
    CloseHandle(hThreadCommand);
	CloseHandle(hThreadDisplay);

    return 0;
}