#include "../Servidor/constantes.h"
#include "board.h"


void WINAPI WaitForCloseCommand(PVOID param) {
    EstadoBoard* estado = (EstadoBoard*)param;
    TCHAR comando[TAM_COMANDO];

    while (1) {
        fflush(stdin);
        if (_fgetts(comando, sizeof(comando) / sizeof(comando[0]), stdin) == NULL)
            break;
        comando[TAM_COMANDO - 1] = _T('\0');
        if (_tcsicmp(comando, _T("close")) == 0) {
            SetEvent(estado->eventoLocalExit);
            break;
        }
    }
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
        ExitProcess(-1);
    }

    DWORD N = _tstoi(argv[1]);
    if (N <= 0 || N > TAM_MAX_EMPRESAS) {
        _tprintf_s(INFO_NUMERO_EMPRESAS);
        ExitProcess(-1);
    }

    EstadoBoard estado = { .hMap = NULL, .N = N, NULL};  // Inicializa a estrutura de estado

    estado.hMap = OpenFileMapping(FILE_MAP_READ, FALSE, NOME_SHARED_MEMORY);
    if (estado.hMap == NULL) {
        _tprintf_s(ERRO_OPEN_FILE_MAPPING);
        ExitProcess(-1);
    }

    estado.pDados = (DadosPartilhados*)MapViewOfFile(
        estado.hMap,
        FILE_MAP_READ,
        0,
        0,
        sizeof(DadosPartilhados));

    if (estado.pDados == NULL) {
        _tprintf_s(ERRO_CREATE_FILE_MAPPING);
        CloseHandle(estado.hMap);
        ExitProcess(-1);
    }

    //open do evento de atualização dos dados
    estado.eventoEscrita = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, NOME_EVENTO_BOARD);
    if (estado.eventoEscrita == NULL) {
        _tprintf_s(ERRO_CREATE_EVENT);
        UnmapViewOfFile(estado.pDados);
        CloseHandle(estado.hMap);
        ExitProcess(-1);
    }

    // Reset the event
    if (!ResetEvent(estado.eventoEscrita)) {
        _tprintf_s(ERRO_RESET_EVENT);
        UnmapViewOfFile(estado.pDados);
        CloseHandle(estado.hMap);
        CloseHandle(estado.eventoEscrita);
        ExitProcess(-1);
    }

    // open do evento exit do servidor
    estado.eventoExit = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, NOME_EVENTO_EXIT);
    if (estado.eventoExit == NULL) {
		_tprintf_s(ERRO_CREATE_EVENT);
		UnmapViewOfFile(estado.pDados);
		CloseHandle(estado.hMap);
		CloseHandle(estado.eventoEscrita);
        ExitProcess(-1);
	}

    estado.eventoLocalExit = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (estado.eventoLocalExit == NULL) {
		_tprintf_s(ERRO_CREATE_EVENT);
		UnmapViewOfFile(estado.pDados);
		CloseHandle(estado.hMap);
		CloseHandle(estado.eventoEscrita);
		CloseHandle(estado.eventoExit);
        ExitProcess(-1);
	}

    HANDLE hEvents[3] = { estado.eventoEscrita, estado.eventoExit, estado.eventoLocalExit };

    HANDLE hThreadCommand = CreateThread(NULL, 0, WaitForCloseCommand, &estado, 0, NULL);
    if(hThreadCommand == NULL) {
        _tprintf_s(ERRO_CREATE_THREAD);
        UnmapViewOfFile(estado.pDados);
        CloseHandle(estado.hMap);
        CloseHandle(estado.eventoEscrita);
        CloseHandle(estado.eventoExit);
        CloseHandle(estado.eventoLocalExit);
        ExitProcess(-1);
    }
  
    DWORD dwWaitResult = 0;
    _tprintf_s(_T("Escreva 'close' para terminar o programa:\n"));
    while (1) {
        
        dwWaitResult = WaitForMultipleObjects(3, hEvents, FALSE, 10000); //espera por um evento
        if(dwWaitResult == WAIT_OBJECT_0 + 1 || dwWaitResult == WAIT_OBJECT_0 + 2) {
            _tprintf_s(INFO_CLOSEC);
			break;
		}

        OrganizarEExibirEmpresas(estado.pDados, estado.N);
        _tprintf_s(_T("Escreva 'close' para terminar o programa:\n"));
        ResetEvent(estado.eventoEscrita);
    }
    // Cancelamento da thread de comando
    CancelSynchronousIo(hThreadCommand);
    
    WaitForSingleObject(hThreadCommand, INFINITE);

    CloseHandle(hThreadCommand);

    // Limpeza e fecho de recursos
    UnmapViewOfFile(estado.pDados);
    CloseHandle(estado.hMap);
    CloseHandle(estado.eventoEscrita);
    CloseHandle(estado.eventoExit);
    CloseHandle(estado.eventoLocalExit);
    //CloseHandle(hThreadCommand);

    ExitProcess(0);
}