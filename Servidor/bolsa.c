#include "servidor.h"
#include "constantes.h"

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

	// Verificar se o bolsa já foi iniciado
	HANDLE hSemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, NOME_SEMAFORO);
	if (hSemaphore == NULL) {
		hSemaphore = CreateSemaphore(NULL, 1, 1, NOME_SEMAFORO);
		if (hSemaphore == NULL) {
			_tprintf_s(ERRO_CREATE_SEM);
			ExitProcess(-1);
		}
	}
	else {
		_tprintf_s(ERRO_SEM_JA_INICIADO);
		ExitProcess(-1);
	}

	// Inicializar os dados do sistema
	DataTransferObject dto;
	if (!inicializarDTO(&dto)) {
		_tprintf_s(ERRO_INICIALIZAR_DTO);
		ExitProcess(-1);
	}
	dto.limiteClientes = lerCriarRegistryKey();
	if (dto.limiteClientes == 0) {
		_tprintf_s(ERRO_REGISTRY);
		terminarDTO(&dto);
		ExitProcess(-1);
	}
	if (dto.limiteClientes > TAM_MAX_USERS)
		dto.limiteClientes = TAM_MAX_USERS;
	dto.numUtilizadores = lerUtilizadores(&dto, argv[1]);

	// inicializar a lista de estruturas com o limite máximo de utilizadores
	ThreadData listaTD[TAM_MAX_USERS];
	// zerar a lista de estruturas
	memset(listaTD, 0, sizeof(listaTD));

	// criar evento que sinaliza a saída do programa
	HANDLE hExitEvent = CreateEvent(NULL, TRUE, FALSE, NOME_EVENTO_EXIT);
	if (hExitEvent == NULL) {
		_tprintf_s(ERRO_CREATE_EVENT);
		terminarDTO(&dto);
		ExitProcess(-1);
	}
	dto.hExitEvent = hExitEvent;
	// criar evento que sinaliza a atualização do board
	HANDLE hUpdateEvent = CreateEvent(NULL, TRUE, FALSE, NOME_EVENTO_BOARD);
	if (hUpdateEvent == NULL) {
		_tprintf_s(ERRO_CREATE_EVENT);
		terminarDTO(&dto);
		ExitProcess(-1);
	}
	dto.hUpdateEvent = hUpdateEvent;
	// criar evento que sinaliza que já há posições livres na lista de ThreadData
	HANDLE hLimiteClientes = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hLimiteClientes == NULL) {
		_tprintf_s(ERRO_CREATE_EVENT);
		terminarDTO(&dto);
		ExitProcess(-1);
	}
	dto.hLimiteClientes = hLimiteClientes;

	// indica que todas as estruturas estão livres até ao limite de clientes possiveis
	DWORD i;
	for (i = 0; i < TAM_MAX_USERS; i++) {
		listaTD[i].livre = i < dto.limiteClientes;
		listaTD[i].dto = &dto;
	}

	HANDLE hThreads[NUM_THREADS_SERVER];
	// lançar thread para lidar com os comandos de admin
	hThreads[0] = CreateThread(NULL, 0, threadComandosAdminHandler, &listaTD, 0, NULL);
	if (hThreads[0] == NULL) {
		_tprintf_s(ERRO_CREATE_THREAD);
		terminarDTO(&dto);
		ExitProcess(-1);
	}

	//lançar thread para lidar com a variação de preço
	hThreads[1] = CreateThread(NULL, 0, threadVariacaoPrecoHandler, &listaTD, 0, NULL);
	if (hThreads[1] == NULL) {
		_tprintf_s(ERRO_CREATE_THREAD);
		terminarDTO(&dto);
		ExitProcess(-1);
	}

	HANDLE hEvents[2] = { hLimiteClientes, hExitEvent };

	// lançar a thread para lidar com as conexões
	BOOL continuar = TRUE;
	while (continuar) {
		// encontrar a primeira posição livre da lista
		for (i = 0; i < dto.limiteClientes; ++i)
			if (listaTD[i].livre)
				break;
		if (i == dto.limiteClientes) {
			_tprintf_s(ERRO_MAX_CLIENTES);
			ResetEvent(hLimiteClientes);
			// meter aqui um WaitForSingleObject à espera de um evento que diga que já pode continuar
			DWORD dwWaitResulLimiteClientes = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
			if (dwWaitResulLimiteClientes == WAIT_OBJECT_0) {
				// evento que indica que já há posições livres
				continue;
			}
			if (dwWaitResulLimiteClientes == WAIT_OBJECT_0 + 1) {
				// evento para sair do programa
				continuar = FALSE;
				break;
			}
		}
		// criar uma instância do named pipe
		listaTD[i].hPipeInst = CreateNamedPipe(
			NOME_NAMED_PIPE,
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			dto.limiteClientes,
			sizeof(Mensagem) * 2,
			sizeof(Mensagem) * 2,
			0,
			NULL);
		if (listaTD[i].hPipeInst == INVALID_HANDLE_VALUE) {
			_tprintf_s(ERRO_CREATE_NAMED_PIPE);
			continue;
		}
		// criar um evento para a instância do named pipe
		OVERLAPPED ov = { 0 };
		ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (ov.hEvent == NULL) {
			_tprintf_s(ERRO_CREATE_EVENT);
			CloseHandle(listaTD[i].hPipeInst);
			continue;
		}
		ResetEvent(ov.hEvent);

		HANDLE hEvents[2] = { ov.hEvent, dto.hExitEvent };
		DWORD dwWaitResult;

		// conectar o named pipe
		BOOL fConnected = ConnectNamedPipe(listaTD[i].hPipeInst, &ov) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

		dwWaitResult = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
		if (dwWaitResult == WAIT_OBJECT_0 + 1) {
			// evento para sair do programa
			continuar = FALSE;
			break;
		}
		if (dwWaitResult == WAIT_OBJECT_0) {
			// lançar a thread para tratar o cliente
			HANDLE hThread = CreateThread(NULL, 0, threadClientHandler, &listaTD[i], 0, NULL);
			if (hThread == NULL) {
				_tprintf_s(ERRO_CREATE_THREAD);
				CloseHandle(listaTD[i].hPipeInst);
				continue;
			}
			// indicar que a posição da lista de ThreadData está ocupada
			listaTD[i].livre = FALSE;
			_tprintf_s(INFO_CLIENTE_CONECTADO);
		}
		else {
			_tprintf_s(ERRO_CONNECT_NAMED_PIPE);
			CloseHandle(listaTD[i].hPipeInst);
		}

		// caso seja para fechar tudo
		EnterCriticalSection(&dto.pSync->csContinuar);
		continuar = dto.continuar;
		LeaveCriticalSection(&dto.pSync->csContinuar);
	}

	// esperar que as threads terminem
	if (WaitForMultipleObjects(NUM_THREADS_SERVER, hThreads, TRUE, INFINITE) != WAIT_OBJECT_0)
		_tprintf_s(ERRO_ESPERAR_THREADS);
	for (DWORD t = 0; t < NUM_THREADS_SERVER; t++)
		CloseHandle(hThreads[t]);
	CloseHandle(hExitEvent);
	CloseHandle(hUpdateEvent);
	CloseHandle(hLimiteClientes);
	terminarDTO(&dto);
	ReleaseSemaphore(hSemaphore, 1, NULL);
	CloseHandle(hSemaphore);

	ExitProcess(0);
}