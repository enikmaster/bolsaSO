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

	ClienteData cd = { 0 };

	// criar um timer
	HANDLE hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
	if(hTimer==NULL) {
		_tprintf_s(ERRO_CREATE_TIMER);
		ExitProcess(-1);
	}

	LARGE_INTEGER liDueTime;
	liDueTime.QuadPart = -100000000LL;

	int contadorTentativas = 0;
	BOOL isConnected = FALSE;
	while(contadorTentativas < MAX_TENTATIVAS_LIGACAO && !isConnected){

		if (!SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, FALSE)) {
			_tprintf_s(ERRO_SET_TIMER);
			ExitProcess(-1);
		}

		WaitForSingleObject(hTimer, INFINITE);

	// criar named pipe
	cd.hPipe = CreateFile(
		NOME_NAMED_PIPE,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0 | FILE_FLAG_OVERLAPPED,
		NULL
	);

	if(cd.hPipe != INVALID_HANDLE_VALUE) {
		system("cls");
		isConnected = TRUE;
		break;
	}

	// verificar se a conexão foi bem sucedida
	if (GetLastError() == ERROR_PIPE_BUSY) {
		_tprintf_s(ERRO_PIPE_BUSY);
		contadorTentativas++;
	}
	else {
		system("cls");
		_tprintf_s(ERRO_LIGAR_BOLSA);
		CloseHandle(cd.hPipe);
		contadorTentativas++;
		//ExitProcess(-1);
		}	
	}
	if(contadorTentativas == MAX_TENTATIVAS_LIGACAO) {
		_tprintf_s(ERRO_MAX_TENTATIVAS);
		ExitProcess(-1);
	}

	if (!isConnected) {
		_tprintf_s(ERRO_CONNECT_NAMED_PIPE);
		ExitProcess(-1);
	}



	// definir o modo de leitura do named pipe
	DWORD dwMode = PIPE_READMODE_MESSAGE;
	BOOL fSuccess = SetNamedPipeHandleState(cd.hPipe, &dwMode, NULL, NULL);
	if (!fSuccess) {
		_tprintf_s(ERRO_SET_PIPE_STATE);
		CloseHandle(cd.hPipe);
		ExitProcess(-1);
	}

	// criar uma instância de OVERLAPPED
	OVERLAPPED ov = { 0 };
	// criar um evento para a instância de OVERLAPPED
	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (ov.hEvent == NULL) {
		_tprintf_s(ERRO_CREATE_EVENT);
		CloseHandle(cd.hPipe);
		ExitProcess(-1);
	}

	cd.logado = FALSE;

	cd.hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (cd.hExitEvent == NULL) {
		_tprintf_s(ERRO_CREATE_EVENT);
		CloseHandle(ov.hEvent);
		CloseHandle(cd.hPipe);
		ExitProcess(-1);
	}

	HANDLE hEvents[2] = { ov.hEvent, cd.hExitEvent };

	cd.hMutex = CreateMutex(NULL, FALSE, NULL);
	if (cd.hMutex == NULL) {
		_tprintf_s(ERRO_CREATE_MUTEX);
		CloseHandle(ov.hEvent);
		CloseHandle(cd.hPipe);
		CloseHandle(cd.hExitEvent);
		ExitProcess(-1);
	}

	// thread para lidar com os comandos do cliente
	HANDLE hThread = CreateThread(NULL, 0, threadComandosClienteHandler, &cd, 0, NULL);
	if (hThread == NULL) {
		_tprintf_s(ERRO_CREATE_THREAD);
		CloseHandle(ov.hEvent);
		CloseHandle(cd.hPipe);
		ExitProcess(-1);
	}

	DWORD bytesLidos;
	Mensagem mensagemRead = { 0 };
	BOOL continuar = TRUE;
	DWORD dwWaitResult;

	while (continuar) {
		// limpar a mensagem
		ZeroMemory(&mensagemRead, sizeof(Mensagem));
		// reiniciar o evento
		ov.Offset = 0;
		ov.OffsetHigh = 0;
		ResetEvent(ov.hEvent);
		// ler a mensagem
		fSuccess = ReadFile(cd.hPipe, &mensagemRead, sizeof(Mensagem), &bytesLidos, &ov);
		// esperar que o evento seja sinalizado
		dwWaitResult = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
		if (dwWaitResult == WAIT_OBJECT_0 + 1) {
			continuar = FALSE;
			break;
		}
		// verificar se a leitura foi bem sucedida
		if(fSuccess || bytesLidos != 0) {
			if (!GetOverlappedResult(cd.hPipe, &ov, &bytesLidos, FALSE)) {
				_tprintf_s(ERRO_READ_PIPE);
				break;
			}
		}
		// lidar com a mensagem
		switch (mensagemRead.TipoM) {
		case TMensagem_R_LOGIN:
			cd.logado = mensagemRead.sucesso;
			mensagemRLogin(mensagemRead);
			break;
		case TMensagem_R_LISTC:
			mensagemRListc(mensagemRead);
			break;
		case TMensagem_R_BUY:
			mensagemRBuy(mensagemRead);
			break;
		case TMensagem_R_SELL:
			mensagemRSell(mensagemRead);
			break;
		case TMensagem_R_BALANCE:
			mensagemRBalance(mensagemRead);
			break;
		case TMensagem_R_WALLET:
			mensagemRWallet(mensagemRead);
			break;
		case TMensagem_ADDC:
			mensagemAddc(mensagemRead);
			break;
		case TMensagem_STOCK:
			mensagemStock(mensagemRead);
			break;
		case TMensagem_PAUSE:
			mensagemPause(mensagemRead);
			break;
		case TMensagem_RESUME:
			mensagemResume(mensagemRead);
			break;
		case TMensagem_LOAD:
			mensagemLoad(mensagemRead);
			break;
		case TMensagem_CLOSE:
			_tprintf_s(INFO_CLOSEC);
			continuar = FALSE;
			WaitForSingleObject(cd.hMutex, INFINITE);
			SetEvent(cd.hExitEvent);
			
			ReleaseMutex(cd.hMutex);
			CancelSynchronousIo(hThread);
			break;
		//case TMensagem_EXIT: // não é necessário
		//	continuar = mensagemExit(mensagemRead);
		//	break;
		default:
			_tprintf_s(ERRO_INVALID_MSG);
			break;
		}
	}
	// esperar que a thread termine
	WaitForSingleObject(hThread, INFINITE);
	
	FlushFileBuffers(cd.hPipe);
	DisconnectNamedPipe(cd.hPipe);
	
	CloseHandle(cd.hPipe);
	CloseHandle(hThread);

	ExitProcess(0);
}