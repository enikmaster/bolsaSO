#include "..\Servidor\constantes.h"
#include "cliente.h"

int _tmain(int argc, TCHAR** argv)
{
	#ifdef UNICODE 
		DWORD x1, x2, x3;
		x1 = _setmode(_fileno(stdin), _O_WTEXT);
		x2 = _setmode(_fileno(stdout), _O_WTEXT);
		x3 = _setmode(_fileno(stderr), _O_WTEXT);
		if (x1 == -1 || x2 == -1 || x3 == -1)
			ExitProcess(-1);
	#endif
	if (argc != 1) {
		_tprintf_s(ERRO_INVALID_N_ARGS);
		ExitProcess(-1);
	}

	// criar named pipe
	HANDLE hPipe = CreateFile(
		NOME_NAMED_PIPE,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0 | FILE_FLAG_OVERLAPPED,
		NULL
	);
	// verificar se a criação do named pipe foi bem sucedida
	if (hPipe == INVALID_HANDLE_VALUE) {
		_tprintf_s(ERRO_CONNECT_NAMED_PIPE);
		ExitProcess(-1);
	}
	// verificar se a conexão foi bem sucedida
	if (GetLastError() == ERROR_PIPE_BUSY) {
		_tprintf_s(ERRO_PIPE_BUSY);
		CloseHandle(hPipe);
		ExitProcess(-1);
	}

	// definir o modo de leitura do named pipe
	DWORD dwMode = PIPE_READMODE_MESSAGE;
	BOOL fSuccess = SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);
	if (!fSuccess) {
		_tprintf_s(ERRO_SET_PIPE_STATE);
		CloseHandle(hPipe);
		ExitProcess(-1);
	}

	// criar uma instância de OVERLAPPED
	OVERLAPPED ov = { 0 };
	// criar um evento para a instância de OVERLAPPED
	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (ov.hEvent == NULL) {
		_tprintf_s(ERRO_CREATE_EVENT);
		CloseHandle(hPipe);
		ExitProcess(-1);
	}

	// thread para lidar com os comandos do cliente
	HANDLE hThread = CreateThread(NULL, 0, threadComandosClienteHandler, &hPipe, 0, NULL);
	if (hThread == NULL) {
		_tprintf_s(ERRO_CREATE_THREAD);
		CloseHandle(ov.hEvent);
		CloseHandle(hPipe);
		ExitProcess(-1);
	}

	DWORD bytesLidos;
	Mensagem mensagemRead = { 0 };
	BOOL continuar = TRUE;

	while (continuar) {
		// limpar a mensagem
		ZeroMemory(&mensagemRead, sizeof(Mensagem));
		// reiniciar o evento
		ov.Offset = 0;
		ov.OffsetHigh = 0;
		ResetEvent(ov.hEvent);
		// ler a mensagem
		fSuccess = ReadFile(hPipe, &mensagemRead, sizeof(Mensagem), &bytesLidos, &ov);
		// esperar que o evento seja sinalizado
		WaitForSingleObject(ov.hEvent, INFINITE);
		// verificar se a leitura foi bem sucedida
		if(fSuccess || bytesLidos != 0) {
			if (!GetOverlappedResult(hPipe, &ov, &bytesLidos, FALSE)) {
				_tprintf_s(ERRO_READ_PIPE);
				break;
			}
		}
		// lidar com a mensagem
		// messageHandlerCliente(mensagemRead);
		switch (mensagemRead.TipoM) {
		case TMensagem_R_LOGIN:
			mensagemRLogin(mensagemRead);
			break;
		case TMensagem_R_LISTC:
			mensagemRListc(mensagemRead);
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
		if(mensagemRead.TipoM == TMensagem_CLOSE || mensagemRead.TipoM == TMensagem_EXIT)
			continuar = mensagemRead.continuar;
	}
	// esperar que a thread termine
	WaitForSingleObject(hThread, INFINITE);
	
	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	
	CloseHandle(hPipe);
	CloseHandle(hThread);

	ExitProcess(0);
}