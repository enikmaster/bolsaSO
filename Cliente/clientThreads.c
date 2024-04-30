#include "cliente.h"

void WINAPI threadConnectionHandlerCliente(PVOID p) {
	PipeInstance* pInst = (PipeInstance*)p;

	// TODO: refazer isto tudo





	
	BOOL continuar = TRUE;
	HANDLE hPipeInst;
	// conectar ao named pipe
	hPipeInst = CreateFile(
		NOME_NAMED_PIPE,
		GENERIC_READ | GENERIC_WRITE,
		0, // FILE_SHARE_READ | FILE_SHARE_WRITE -> ver mais informação
		NULL,
		OPEN_EXISTING,
		0| FILE_FLAG_OVERLAPPED,
		NULL);
	// verificar se a conexão foi bem sucedida
	if (hPipeInst == INVALID_HANDLE_VALUE) {
		_tprintf_s(ERRO_CONNECT_NAMED_PIPE);
		return;
	}
	// verificar se o named pipe está ocupado
	if (GetLastError() == ERROR_PIPE_BUSY) {
		_tprintf_s(ERRO_PIPE_BUSY);
		CloseHandle(hPipeInst);
		return;
	}

	DWORD dwMode = PIPE_READMODE_MESSAGE;
	// definir o modo de leitura do named pipe
	BOOL fSuccess = SetNamedPipeHandleState(hPipeInst, &dwMode, NULL, NULL);
	if (!fSuccess) {
		_tprintf_s(ERRO_SET_PIPE_STATE);
		CloseHandle(hPipeInst);
		return;
	}
	// criar um evento
	//ZeroMemory(&pInst->ov, sizeof(OVERLAPPED));
	OVERLAPPED ov = { 0 };
	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if(ov.hEvent == NULL) {
		_tprintf_s(ERRO_CREATE_EVENT);
		CloseHandle(hPipeInst);
		return;
	}
	DWORD bytesLidos;
	Mensagem mensagemRead = { 0 };

	while (continuar) {
		//ZeroMemory(&ov, sizeof(OVERLAPPED));
		ZeroMemory(&mensagemRead, sizeof(Mensagem));
		ResetEvent(ov.hEvent);
		fSuccess = ReadFile(hPipeInst, &mensagemRead, sizeof(Mensagem), &bytesLidos, &ov);
//		if (!fSuccess || &pInst->bytesLidos == 0) {
//			PrintLastError(_T("ReadFile"), GetLastError());
//			if(GetLastError() != ERROR_IO_PENDING) {
//				_tprintf_s(DEBUGGER);
//				_tprintf_s(ERRO_READ_PIPE);
//				//CloseHandle(ov.hEvent);
//				//CloseHandle(hPipe);
//				break;
//			}
			WaitForSingleObject(ov.hEvent, INFINITE);
			if (!GetOverlappedResult(hPipeInst, &ov, &bytesLidos, FALSE)) {
				_tprintf_s(ERRO_READ_PIPE);
				//CloseHandle(ov.hEvent);
				//CloseHandle(hPipe);
				break;
			}
//		}
//		HANDLE hThread = CreateThread(NULL, 0, threadMessageHandlerCliente, &pInst, 0, NULL);
		messageHandlerCliente(mensagemRead);
	}
	// limpar os dados do buffer
	FlushFileBuffers(hPipeInst);
	// desconectar o named pipe
	DisconnectNamedPipe(hPipeInst);
	// fechar o handle do named pipe
	CloseHandle(hPipeInst);
	// fechar o handle do evento
	CloseHandle(ov.hEvent);
}

void messageHandlerCliente(Mensagem mensagem) {
	// Mensagens recebidas pelo cliente vindas do servidor
	// Mensagens com prefixo R_ são respostas do servidor
	switch (mensagem.TipoM) {
	case TMensagem_R_LOGIN:
		mensagemRLogin();
		break;
	case TMensagem_R_LISTC:
		mensagemRListc();
		break;
	case TMensagem_R_BUY:
		mensagemRBuy();
		break;
	case TMensagem_R_SELL:
		mensagemRSell();
		break;
	case TMensagem_R_BALANCE:
		mensagemRBalance();
		break;
	case TMensagem_R_WALLET:
		mensagemRWallet();
		break;
	case TMensagem_ADDC:
		mensagemAddc();
		break;
	case TMensagem_STOCK:
		mensagemStock();
		break;
	case TMensagem_PAUSE:
		mensagemPause();
		break;
	case TMensagem_RESUME:
		mensagemResume();
		break;
	case TMensagem_LOAD:
		mensagemLoad();
		break;
	case TMensagem_CLOSE:
		mensagemCloseC();
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