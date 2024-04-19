#include "servidor.h"

// funções das threads
void WINAPI threadConnectionHandler(PVOID p) {
	DataTransferObject* dto = (DataTransferObject *)p;
	DWORD numPipes = 0;
	DWORD limiteClientes = dto->limiteClientes;
	BOOL continuar = TRUE;

	do {
		// criar uma instância do named pipe
		// verificar se o limite de clientes foi atingido
		if (numPipes >= limiteClientes) {
			_tprintf_s(ERRO_MAX_CLIENTES);
			continue;
		}
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
			EnterCriticalSection(&dto->csThreads);
			for(; td->pipeIndex < limiteClientes; td->pipeIndex++) {
				if(dto->hPipes[td->pipeIndex] == INVALID_HANDLE_VALUE || dto->hPipes[td->pipeIndex] == NULL) {
					dto->hPipes[td->pipeIndex] = hPipe;
					numPipes = ++(dto->numPipes);
					break;
				}
			}
			LeaveCriticalSection(&dto->csThreads);
		} else {
			_tprintf_s(ERRO_CONNECT_NAMED_PIPE);
			CloseHandle(hPipe);
		}

		EnterCriticalSection(&dto->csThreads);
		continuar = dto->continuar;
		LeaveCriticalSection(&dto->csThreads);

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
	OVERLAPPED ov = { 0 };
	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (ov.hEvent == NULL) {
		_tprintf_s(ERRO_CREATE_EVENT);
		return;
	}

	// TODO: implementar o ciclo de leitura e escrita


	/*
			ReadFile();
			processar a mensagem ( no ambito do exercicio é apenas imprimir a mensagem)
			contruir a mensagem
			WriteFile();
			----- Fora do ciclo -----
			FlusehFileBuffers();
			DisconnectNamedPipe();
			CloseHandle();

		*/
	ConnectNamedPipe(dto->hPipes[pipeIndex], NULL);
	
	while (continuar) {
		// conectar ao named pipe
		// verificar se a conexão foi bem sucedida
		

		// ler a mensagem do cliente
		
		// processar a mensagem
		// construir a mensagem
		// escrever a mensagem para o cliente
		// fora do ciclo
		// limpar os buffers
		// desconectar o named pipe
		// fechar o handle do named pipe
		EnterCriticalSection(&dto->csThreads);
		continuar = dto->continuar;
		LeaveCriticalSection(&dto->csThreads);
	}

	FlushFileBuffers(dto->hPipes[pipeIndex]);
	DisconnectNamedPipe(dto->hPipes[pipeIndex]);
	CloseHandle(dto->hPipes[pipeIndex]);
	dto->hPipes[pipeIndex] = INVALID_HANDLE_VALUE;
	CloseHandle(ov.hEvent);
}

void WINAPI threadWriteHandler(PVOID p) {
	DataTransferObject* dto = (DataTransferObject*)p;
	// TODO: implementar o ciclo de leitura e escrita
}
