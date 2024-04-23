#include "cliente.h"

void WINAPI threadConnectionHandlerCliente() {

	BOOL continuar = TRUE;
	
	HANDLE hPipe = CreateFile(
		NOME_NAMED_PIPE,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (hPipe == INVALID_HANDLE_VALUE) {
		_tprintf_s(ERRO_CONNECT_NAMED_PIPE);
		return;
	}
	if (GetLastError() == ERROR_PIPE_BUSY) {
		_tprintf_s(ERRO_PIPE_BUSY);
		CloseHandle(hPipe);
		return;
	}
	DWORD dwMode = PIPE_READMODE_MESSAGE;
	BOOL fSuccess = SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);
	if (!fSuccess) {
		_tprintf_s(ERRO_SET_PIPE_STATE);
		CloseHandle(hPipe);
		return;
	}
	OVERLAPPED ov;
	ZeroMemory(&ov, sizeof(OVERLAPPED));
	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if(ov.hEvent == NULL) {
		_tprintf_s(ERRO_CREATE_EVENT);
		CloseHandle(hPipe);
		return;
	}

	Mensagem mensagem = { 0 };
	DWORD bytesLidos;

	while (continuar) {
		ZeroMemory(&ov, sizeof(OVERLAPPED));
		ResetEvent(ov.hEvent);
		ZeroMemory(&mensagem, sizeof(Mensagem));
		fSuccess = ReadFile(hPipe, &mensagem, sizeof(Mensagem), &bytesLidos, &ov);
		if (!fSuccess || bytesLidos == 0) {
			if(GetLastError() != ERROR_IO_PENDING) {
				_tprintf_s(ERRO_READ_PIPE);
				CloseHandle(ov.hEvent);
				CloseHandle(hPipe);
				continue;
			}
			WaitForSingleObject(ov.hEvent, INFINITE);
			if (!GetOverlappedResult(hPipe, &ov, &bytesLidos, FALSE)) {
				_tprintf_s(ERRO_READ_PIPE);
				CloseHandle(ov.hEvent);
				CloseHandle(hPipe);
				continue;
			}
		}
		HANDLE hThread = CreateThread(NULL, 0, threadMessageHandlerCliente, &mensagem, 0, NULL);
	}
	// limpar os dados do buffer
	FlushFileBuffers(hPipe);
	// desconectar o named pipe
	DisconnectNamedPipe(hPipe);
	// fechar o handle do named pipe
	CloseHandle(hPipe);
	// fechar o handle do evento
	CloseHandle(ov.hEvent);
}

void WINAPI threadMessageHandlerCliente(PVOID p) {
	Mensagem* mensagem = (Mensagem*)p;
	// Mensagens recebidas pelo cliente vindas do servidor
	// Mensagens com prefixo R_ são respostas do servidor
	switch (mensagem->TipoM) {
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
		mensagemClose();
		break;
	case TMensagem_EXIT:
		mensagemExit();
		break;
	default:
		_tprintf_s(ERRO_INVALID_MSG);
		break;
	}
}