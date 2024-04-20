#include "servidor.h"

// funções das threads
void WINAPI threadConnectionHandler(PVOID p) {
	DataTransferObject* dto = (DataTransferObject *)p;
	DWORD numPipes = 0;
	DWORD limiteClientes = dto->limiteClientes; // não é critical section porque depois de definido, não volta a ser alterado
	BOOL continuar = TRUE;

	do {
		// verificar se o limite de clientes foi atingido
		if (numPipes >= limiteClientes) {
			_tprintf_s(ERRO_MAX_CLIENTES);
			continue;
		}
		// criar uma instância do named pipe
		HANDLE hPipe = CreateNamedPipe(
			NOME_NAMED_PIPE, // nome do named pipe
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, // tipo de acesso (leitura e escrita) e modo de sobreposição (overlapped)
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, // tipo de pipe (mensagem), modo de leitura (mensagem) e modo de espera (espera que o cliente esteja pronto)
			limiteClientes, // número máximo de instâncias do named pipe
			sizeof(Mensagem) * 2, // tamanho do buffer de leitura (maior do que o tamanho da mensagem para garantir que a mensagem é lida toda de uma só vez)
			sizeof(Mensagem) * 2, // tamanho do buffer de escrita (maior do que o tamanho da mensagem para garantir que a mensagem é escrita toda de uma só vez)
			0, // tempo de espera para a primeira conexão (0 para esperar indefinidamente)
			NULL); // atributos de segurança (não usados)
		
		// verificar se a instância foi criada com sucesso
		if (hPipe == INVALID_HANDLE_VALUE) {
			_tprintf_s(ERRO_CREATE_NAMED_PIPE);
			continue;
		}

		BOOL fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		if (fConnected) {
			ThreadData* td = (ThreadData*)malloc(sizeof(ThreadData));
			if(td == NULL) {
				_tprintf_s(ERRO_MEM_ALLOC);
				CloseHandle(hPipe);
				continue;
			}
			td->dto = dto;
			td->pipeIndex = 0;

			HANDLE hThread = CreateThread(NULL, 0, threadClientHandler, td, 0, NULL);
			if(hThread == NULL) {
				_tprintf_s(ERRO_CREATE_THREAD);
				CloseHandle(hPipe);
				continue;
			}
			EnterCriticalSection(&dto->pSync->csListaPipes);
			for(; td->pipeIndex < limiteClientes; td->pipeIndex++) {
				if(dto->hPipes[td->pipeIndex] == INVALID_HANDLE_VALUE || dto->hPipes[td->pipeIndex] == NULL) {
					dto->hPipes[td->pipeIndex] = hPipe;
					numPipes = ++(dto->numPipes);
					break;
				}
			}
			LeaveCriticalSection(&dto->pSync->csListaPipes);
		} else {
			_tprintf_s(ERRO_CONNECT_NAMED_PIPE);
			CloseHandle(hPipe);
		}

		EnterCriticalSection(&dto->pSync->csContinuar);
		continuar = dto->continuar;
		LeaveCriticalSection(&dto->pSync->csContinuar);

	} while (continuar);
}

void WINAPI threadClientHandler(PVOID p) {
	ThreadData* td = (ThreadData*)p;

	HANDLE hThreads[2];
	hThreads[0] = CreateThread(NULL, 0, threadReadHandler, td, 0, NULL);
	if(hThreads[0] == NULL) {
		_tprintf_s(ERRO_CREATE_THREAD);
		return;
	}

	hThreads[1] = CreateThread(NULL, 0, threadWriteHandler, td, 0, NULL);
	if(hThreads[1] == NULL) {
		_tprintf_s(ERRO_CREATE_THREAD);
		CloseHandle(hThreads[0]);
		return;
	}

	WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);

	CloseHandle(hThreads[0]);
	CloseHandle(hThreads[1]);
}

void WINAPI threadReadHandler(PVOID p) {
	ThreadData* td = (ThreadData*)p;
	DataTransferObject* dto = td->dto;
	DWORD pipeIndex = td->pipeIndex;
	BOOL continuar = TRUE;
	BOOL res = TRUE;
	DWORD lidos;
	Mensagem mensagemRecebida;
	OVERLAPPED ov;
	ZeroMemory(&ov, sizeof(OVERLAPPED));
	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (ov.hEvent == NULL) {
		_tprintf_s(ERRO_CREATE_EVENT);
		return;
	}
	
	HANDLE hPipe = dto->hPipes[pipeIndex];
	
	while(continuar) {
		ZeroMemory(&ov, sizeof(OVERLAPPED));
		ResetEvent(ov.hEvent);
		if (!ReadFile(
			hPipe,	// handle do named pipe
			&mensagemRecebida,	// buffer de leitura
			sizeof(Mensagem),	// número de bytes a ler
			&lidos,	// número de bytes lidos
			&ov)) {	// estrutura overlapped
			if (GetLastError() != ERROR_IO_PENDING) {
				_tprintf_s(ERRO_READ_PIPE);
				CloseHandle(ov.hEvent);
				CloseHandle(hPipe);
				continue;
			}
			else {
				// leitura pendente
				WaitForSingleObject(ov.hEvent, INFINITE);
				if (!GetOverlappedResult(hPipe, &ov, &lidos, FALSE)) {
					_tprintf_s(ERRO_READ_PIPE);
					CloseHandle(ov.hEvent);
					CloseHandle(hPipe);
					continue;
				}
				trataMensagemRecebida(dto, &mensagemRecebida);
			}
		}
		else {
			trataMensagemRecebida(dto, &mensagemRecebida);
		}
		EnterCriticalSection(&dto->pSync->csContinuar);
		continuar = dto->continuar;
		LeaveCriticalSection(&dto->pSync->csContinuar);
	}
	// limpar os dados do buffer
	FlushFileBuffers(hPipe);
	// desconectar o named pipe
	DisconnectNamedPipe(hPipe);
	// fechar o handle do named pipe na lista do servidor
	CloseHandle(hPipe);
	// decrementar o número de named pipes
	EnterCriticalSection(&dto->pSync->csListaPipes);
	dto->hPipes[pipeIndex] = INVALID_HANDLE_VALUE;
	dto->numPipes--;
	LeaveCriticalSection(&dto->pSync->csListaPipes);
	// fechar o handle do evento
	CloseHandle(ov.hEvent);
}

void WINAPI threadWriteHandler(PVOID p) {
	DataTransferObject* dto = (DataTransferObject*)p;
	// TODO: implementar o ciclo de leitura e escrita
}


// funções de tratamento de mensagens
void trataMensagemRecebida(DataTransferObject* dto, Mensagem* mensagem) {
	// TODO: fazer esta cena
}