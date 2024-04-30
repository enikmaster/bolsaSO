#include "servidor.h"

// fun��es das threads
void WINAPI threadConnectionHandler(PVOID p) {
	DataTransferObject* dto = (DataTransferObject* )p;
	DWORD limiteClientes = 0;
	
	EnterCriticalSection(&dto->pSync->csLimClientes);
	limiteClientes = dto->limiteClientes;
	LeaveCriticalSection(&dto->pSync->csLimClientes);

	DWORD numULigados = 0;
	BOOL continuar = TRUE;

	// inicializar a lista de estruturas
	ThreadData listaTD[TAM_MAX_USERS];
	// zerar a lista de estruturas
	memset(listaTD, 0, sizeof(listaTD));
	// indica que todas as estruturas est�o livres at� ao limite de clientes possiveis
	for(DWORD i = 0; i < TAM_MAX_USERS; i++)
		listaTD[i].livre = i < limiteClientes;
	
	DWORD i;
	while(continuar){
		// encontrar a primeira posi��o livre da lista
		for(i = 0; i<limiteClientes ;++i)
			if(listaTD[i].livre)
				break;

		// TODO: alterar este bloco de c�digo em baixo
		if(i == limiteClientes) {
			_tprintf_s(ERRO_MAX_CLIENTES);
			continue;
		}
		
		// criar uma inst�ncia do named pipe
		listaTD[i].hPipeInst = CreateNamedPipe(
			NOME_NAMED_PIPE,
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			limiteClientes,
			sizeof(Mensagem) * 2,
			sizeof(Mensagem) * 2,
			0,
			NULL);
		if (listaTD[i].hPipeInst == INVALID_HANDLE_VALUE) {
			_tprintf_s(ERRO_CREATE_NAMED_PIPE);
			continue;
		}
		listaTD[i].dto = dto;
		// conectar o named pipe
		// BOOL fConnected = ConnectNamedPipe(listaTD[i].pipeInst.hPipeInst, &listaTD[i].pipeInst.ov) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		BOOL fConnected = ConnectNamedPipe(listaTD[i].hPipeInst, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		// if (fConnected == 0) {
		if (fConnected != 0) {
			// lan�ar a thread para tratar o cliente
			HANDLE hThread = CreateThread(NULL, 0, threadClientHandler, &listaTD[i], 0, NULL);
			if (hThread == NULL) {
				_tprintf_s(ERRO_CREATE_THREAD);
				CloseHandle(listaTD[i].hPipeInst);
				continue;
			}
			// indicar que a estrutura est� ocupada
			// (para o ciclo 'for' do in�cio)
			listaTD[i].livre = FALSE;
			_tprintf_s(_T("thread cliente criada e lan�ada\n"));
		}
		else {
			_tprintf_s(ERRO_CONNECT_NAMED_PIPE);
			CloseHandle(listaTD[i].hPipeInst);
		}

		EnterCriticalSection(&dto->pSync->csContinuar);
		continuar = dto->continuar;
		LeaveCriticalSection(&dto->pSync->csContinuar);
	}
}

void WINAPI threadClientHandler(PVOID p) {
	ThreadData* td = (ThreadData*)p;
	DataTransferObject* dto = td->dto;

	OVERLAPPED ov = { 0 };
	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (ov.hEvent == NULL) {
		_tprintf_s(ERRO_CREATE_EVENT);
		return;
	}

	Mensagem mensagemRead = { 0 };
	DWORD bytesLidos;
	HANDLE hPipe = td->hPipeInst;
	BOOL continuar = TRUE;

	while (continuar) {
		// zerar a estrutura overlapped
		ZeroMemory(&ov, sizeof(OVERLAPPED));
		// zerar a mensagem
		ZeroMemory(&mensagemRead, sizeof(Mensagem));
		// reset do evento
		ResetEvent(ov.hEvent);
		// ler a mensagem do named pipe
		ReadFile(
			td->hPipeInst, // handle do named pipe
			&mensagemRead,	// buffer de leitura
			sizeof(Mensagem),	// n�mero de bytes a ler
			&bytesLidos,	// n�mero de bytes lidos
			&ov);	// estrutura overlapped)
//		if (!fSuccess || bytesLidos == 0) {
//			// verifica se a leitura falhou
//			DWORD erro = GetLastError();
//			PrintLastError(_T("CODE"), GetLastError());
//			if (GetLastError() != ERROR_IO_PENDING) {
//				_tprintf_s(ERRO_READ_PIPE);
//				//CloseHandle(&td->pipeInst.ov.hEvent);
//				//CloseHandle(hPipe);
//				break;
//			}
			// leitura pendente
		WaitForSingleObject(ov.hEvent, INFINITE);
		// verifica se a leitura foi bem sucedida
		BOOL ovResult = GetOverlappedResult(hPipe, &ov, &bytesLidos, FALSE);
		if (!ovResult || bytesLidos == 0) {
			_tprintf_s(ERRO_READ_PIPE);
			//CloseHandle(ov.hEvent);
			//CloseHandle(hPipe);
			break;
		}
//		}
		// leitura imediata
//		HANDLE hThread = CreateThread(NULL, 0, threadMessageHandler, &td, 0, NULL);
		messageHandler(&td, mensagemRead);

		//EnterCriticalSection(&dto->pSync->csContinuar);
		continuar = mensagemRead.continuar;
		//LeaveCriticalSection(&dto->pSync->csContinuar);
	}
	// limpar os dados do buffer
	FlushFileBuffers(hPipe);
	// desconectar o named pipe
	DisconnectNamedPipe(hPipe);
	// fechar o handle do named pipe na lista do servidor
	CloseHandle(hPipe);
	// fechar o handle do evento
	CloseHandle(ov.hEvent);
}

// fun��es de tratamento de mensagens
void messageHandler(PVOID p, Mensagem mensagem) {
	// TODO: tratamento de mensagens
	ThreadData* td = (ThreadData*)p;
	//DWORD pipeIndex = td->pipeIndex;

	switch (mensagem.TipoM) {
	case TMensagem_LOGIN:
		mensagemLogin(td, mensagem);
		break;
	case TMensagem_LISTC:
		mensagemListc(td->dto);
		break;
	case TMensagem_BUY:
		mensagemBuy(td);
		break;
	case TMensagem_SELL:
		mensagemSell();
		break;
	case TMensagem_BALANCE:
		mensagemBalance(td);
		break;
	case TMensagem_WALLET:
		mensagemWallet();
		break;
	case TMensagem_EXIT:
		mensagemExit();
		break;
	default:
		_tprintf_s(ERRO_INVALID_MSG);
		break;
	}
}

void PrintLastError(TCHAR* part, DWORD id) {
	PTSTR buffer;
	if (part == NULL)
		part = _T("*");
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		0,
		(PTSTR)&buffer,
		0,
		NULL);
	_tprintf_s(_T("[%s] %lu: %s\n"), part, id, buffer);
	LocalFree(buffer);
}