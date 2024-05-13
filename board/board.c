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
    DWORD x1, x2, x3;
    x1 = _setmode(_fileno(stdin), _O_WTEXT);
    x2 = _setmode(_fileno(stdout), _O_WTEXT);
    x3 = _setmode(_fileno(stderr), _O_WTEXT);
    if (x1 == -1 || x2 == -1 || x3 == -1)
        ExitProcess(-1);
#endif

    if (argc != 2) {
        _tprintf_s(ERRO_INVALID_N_ARGS);
        return -2;
    }

    DWORD N = _tstoi(argv[1]);
    if (N <= 0 || N > TAM_MAX_EMPRESAS) {
        _tprintf_s(INFO_NUMERO_EMPRESAS);
        return -3;
    }

    EstadoBoard estado = { .running = TRUE, .hMap = NULL, .N = N, NULL};  // Inicializa a estrutura de estado

    estado.hMap = OpenFileMapping(FILE_MAP_READ, FALSE, NOME_SHARED_MEMORY);
    if (estado.hMap == NULL) {
        _tprintf_s(ERRO_OPEN_FILE_MAPPING, GetLastError());
        return -1;
    }

    estado.pDados = (DadosPartilhados*)MapViewOfFile(
        estado.hMap,
        FILE_MAP_READ,
        0,
        0,
        sizeof(DadosPartilhados));

    if (estado.pDados == NULL) {
        _tprintf_s(ERRO_CREATE_FILE_MAPPING, GetLastError());
        CloseHandle(estado.hMap);
        return -1;
    }

    //open do evento
    estado.eventoEscrita = OpenEventW(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, NOME_EVENTO_BOARD);
    if (estado.eventoEscrita == NULL) {
        _tprintf_s(ERRO_CREATE_EVENT);
        UnmapViewOfFile(estado.pDados);
        CloseHandle(estado.hMap);
        return -1;
    }

    // Reset the event
    if (!ResetEvent(estado.eventoEscrita)) {
        _tprintf_s(ERRO_RESET_EVENT);
        UnmapViewOfFile(estado.pDados);
        CloseHandle(estado.hMap);
        CloseHandle(estado.eventoEscrita);
        return -1;
    }

    HANDLE hThreadCommand = CreateThread(NULL, 0, WaitForCloseCommand, &estado, 0, NULL);
    if(hThreadCommand == NULL) {
        _tprintf_s(ERRO_CREATE_THREAD);
        UnmapViewOfFile(estado.pDados);
        CloseHandle(estado.hMap);
        CloseHandle(estado.eventoEscrita);
        return -1;
    }
    HANDLE hThreadDisplay = CreateThread(NULL, 0, DisplayDados, &estado, 0, NULL);
    if(hThreadDisplay == NULL) {
		_tprintf_s(ERRO_CREATE_THREAD);
		UnmapViewOfFile(estado.pDados);
		CloseHandle(estado.hMap);
        CloseHandle(estado.eventoEscrita);
        CloseHandle(hThreadCommand);    
		return -1;
	}
   
    WaitForSingleObject(hThreadCommand, INFINITE);
    estado.running = FALSE;
   
    WaitForSingleObject(hThreadDisplay, INFINITE);

    // Limpeza e fecho de recursos
    UnmapViewOfFile(estado.pDados);
    CloseHandle(estado.hMap);
    CloseHandle(estado.eventoEscrita);
    CloseHandle(hThreadCommand);
	CloseHandle(hThreadDisplay);

    return 0;
}