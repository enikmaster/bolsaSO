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
	
	// Inicializar os dados do sistema
	DataTransferObject dto;
	if(!inicializarDTO(&dto)) {
		_tprintf_s(ERRO_INICIALIZAR_DTO);
		ExitProcess(-1);
	}
	dto.limiteClientes = lerCriarRegistryKey();
	if(dto.limiteClientes == 0) {
		_tprintf_s(ERRO_REGISTRY);
		terminarDTO(&dto);
		ExitProcess(-1);
	}
	if(dto.limiteClientes > TAM_MAX_USERS)
		dto.limiteClientes = TAM_MAX_USERS;
	dto.numUtilizadores = lerUtilizadores(&dto, argv[1]);

	// inicializar a lista de estruturas com o limite máximo de utilizadores
	ThreadData listaTD[TAM_MAX_USERS];
	// zerar a lista de estruturas
	memset(listaTD, 0, sizeof(listaTD));
	
	HANDLE hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hExitEvent == NULL) {
		_tprintf_s(ERRO_CREATE_EVENT);
		terminarDTO(&dto);
		ExitProcess(-1);
	}

	// indica que todas as estruturas estão livres até ao limite de clientes possiveis
	DWORD i;
	for(i = 0; i < TAM_MAX_USERS; i++) {
		listaTD[i].livre = i < dto.limiteClientes;
		listaTD[i].dto = &dto;
		listaTD[i].hExitEvent = hExitEvent;
	}

	

	HANDLE hThreads[3];
	// lançar thread para lidar com os comandos de admin
	hThreads[0] = CreateThread(NULL, 0, threadComandosAdminHandler, &listaTD, 0, NULL);
	if (hThreads[0] == NULL) {
		_tprintf_s(ERRO_CREATE_THREAD);
		terminarDTO(&dto);
		ExitProcess(-1);
	}
	// lançar thread para lidar com o Board
	hThreads[1] = CreateThread(NULL, 0, threadBoardHandler, &listaTD, 0, NULL);
	if (hThreads[1] == NULL) {
		_tprintf_s(ERRO_CREATE_THREAD);
		terminarDTO(&dto);
		ExitProcess(-1);
	}

	//lançar thread para lidar com a variação de preço
	hThreads[2] = CreateThread(NULL, 0, threadVariacaoPrecoHandler, &listaTD, 0, NULL);
	if (hThreads[2] == NULL) {
		_tprintf_s(ERRO_CREATE_THREAD);
		terminarDTO(&dto);
		ExitProcess(-1);
	}


	// criar a thread para lidar com as conexões
	BOOL continuar = TRUE;
	while (continuar) {
		// encontrar a primeira posição livre da lista
		for (i = 0; i < dto.limiteClientes; ++i)
			if (listaTD[i].livre)
				break;
		// TODO: alterar este bloco de código em baixo
		if (i == dto.limiteClientes) {
			_tprintf_s(ERRO_MAX_CLIENTES);
			continue;
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
		if(listaTD[i].hPipeInst == INVALID_HANDLE_VALUE) {
			_tprintf_s(ERRO_CREATE_NAMED_PIPE);
			continue;
		}
		// conectar o named pipe
		BOOL fConnected = ConnectNamedPipe(listaTD[i].hPipeInst, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		if (fConnected != 0) {
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
		} else {
			_tprintf_s(ERRO_CONNECT_NAMED_PIPE);
			CloseHandle(listaTD[i].hPipeInst);
		}

		// caso seja para fechar tudo
		EnterCriticalSection(&dto.pSync->csContinuar);
		continuar = dto.continuar;
		LeaveCriticalSection(&dto.pSync->csContinuar);
	}

	// TODO: como lidar com as threads de cliente?
	//		 WaitForSingleObject para cada cliente? não faz sentido...

	// esperar que as threads terminem
	if (WaitForMultipleObjects(3, hThreads, TRUE, INFINITE) != WAIT_OBJECT_0)
		_tprintf_s(ERRO_ESPERAR_THREADS);
	for(DWORD t = 0; t < 3; t++)
		CloseHandle(hThreads[t]);
	//CloseHandle(hThread);
	terminarDTO(&dto);

	ExitProcess(0);
}