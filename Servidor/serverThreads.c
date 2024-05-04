#include "servidor.h"

// funções das threads
void WINAPI threadComandosAdminHandler(PVOID p) {
	ThreadData* td = (ThreadData*)p;

	DWORD controlo = 0;
	TCHAR comando[TAM_COMANDO];
	TCHAR comandoTemp[TAM_COMANDO];
	TCHAR argumento1[TAM_COMANDO];
	TCHAR argumento2[TAM_COMANDO];
	TCHAR argumento3[TAM_COMANDO];
	TCHAR failSafe[TAM_COMANDO];
	BOOL repetir = TRUE;
	int numArgumentos;
	while (repetir) {
		memset(comandoTemp, 0, sizeof(comando));
		memset(argumento1, 0, sizeof(argumento1));
		memset(argumento2, 0, sizeof(argumento2));
		memset(argumento3, 0, sizeof(argumento3));
		memset(failSafe, 0, sizeof(failSafe));
		_tprintf_s(_T("Comando: "));
		_fgetts(comando, sizeof(comando) / sizeof(comando[0]), stdin);
		comando[_tcslen(comando) - 1] = _T('\0');
		controlo = verificaComando(comando);
		numArgumentos = 0;
		switch (controlo) {
		case 1: // comando addc
			numArgumentos = _stscanf_s(
				comando,									  // buffer de onde ler
				_T("%s %s %s %s %s"),                         // formato para "partir" a string
				comandoTemp, (unsigned)_countof(comandoTemp), // variável onde guardar o comando + tamanho do buffer
				argumento1, (unsigned)_countof(argumento1),   // variável onde guardar o 1º argumento + tamanho do buffer
				argumento2, (unsigned)_countof(argumento2),   // variável onde guardar o 2º argumento + tamanho do buffer
				argumento3, (unsigned)_countof(argumento3),   // variável onde guardar o 3º argumento + tamanho do buffer
				failSafe, (unsigned)_countof(failSafe));      // variável de segurança + tamanho do buffer (se preenchida o num de argumentos estará a mais)
			if (numArgumentos != 4) {
				_tprintf_s(ERRO_INVALID_N_ARGS);
			}
			else {
				DWORD numeroAcoes = _tstoi(argumento2);
				double precoAcao = _tstof(argumento3);
				comandoAddc(td->dto, argumento1, numeroAcoes, precoAcao)
					? _tprintf_s(INFO_ADDC)
					: _tprintf_s(ERRO_ADDC);
			}
			break;
		case 2: // comando listc
			comandoListc(td->dto);
			break;
		case 3: // comando stock
			numArgumentos = _stscanf_s(
				comando,
				_T("%s %s %s %s"),
				comandoTemp, (unsigned)_countof(comandoTemp),
				argumento1, (unsigned)_countof(argumento1),
				argumento2, (unsigned)_countof(argumento2),
				failSafe, (unsigned)_countof(failSafe));
			if (numArgumentos != 3) {
				_tprintf_s(ERRO_INVALID_N_ARGS);
			}
			else {
				double valorAcao = _tstof(argumento2);
				comandoStock(td->dto, argumento1, valorAcao)
					? _tprintf_s(INFO_STOCK)
					: _tprintf_s(ERRO_STOCK);
			}
			break;
		case 4: // comando users
			comandoUsers(td->dto);
			break;
		case 5: // comando pause
			_tprintf_s(_T("[INFO] Comando pause\n")); // para apagar
			numArgumentos = _stscanf_s(
				comando,
				_T("%s %s %s"),
				comandoTemp, (unsigned)_countof(comandoTemp),
				argumento1, (unsigned)_countof(argumento1),
				failSafe, (unsigned)_countof(failSafe));
			if (numArgumentos != 2) {
				_tprintf_s(ERRO_INVALID_N_ARGS);
			}
			else {
				DWORD numeroSegundos = _tstoi(argumento1);
				comandoPause(numeroSegundos);
				// TODO: falta qq coisa mas não sei o que é para já
				//	fazer SuspendThread e ResumeThread
			}
			break;
		case 6: // comando load
			numArgumentos = _stscanf_s(
				comando,
				_T("%s %s %s"),
				comandoTemp, (unsigned)_countof(comandoTemp),
				argumento1, (unsigned)_countof(argumento1),
				failSafe, (unsigned)_countof(failSafe));
			if (numArgumentos != 2) {
				_tprintf_s(ERRO_INVALID_N_ARGS);
			}
			else {
				comandoLoad(td->dto, argumento1)
					? _tprintf_s(INFO_LOAD)
					: _tprintf_s(ERRO_LOAD);
			}
			break;
		case 7: // comando close
			_tprintf_s(_T("[INFO] Comando close\n")); // para apagar
			repetir = comandoClose(td->dto);
			break;
		case 0:
		default: // comando inválido
			_tprintf_s(ERRO_INVALID_CMD);
			break;
		}
	};
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
		// zerar a mensagem
		ZeroMemory(&mensagemRead, sizeof(Mensagem));
		mensagemRead.continuar = TRUE;
		// reset do ov
		ov.Offset = 0;
		ov.OffsetHigh = 0;
		ResetEvent(ov.hEvent);
		// ler a mensagem do named pipe
		BOOL fSuccess = ReadFile(
			td->hPipeInst, // handle do named pipe
			&mensagemRead,	// buffer de leitura
			sizeof(Mensagem),	// número de bytes a ler
			&bytesLidos,	// número de bytes lidos
			&ov);	// estrutura overlapped)
		// leitura pendente
		WaitForSingleObject(ov.hEvent, INFINITE);
		// verifica se a leitura foi bem sucedida
		BOOL ovResult = GetOverlappedResult(hPipe, &ov, &bytesLidos, FALSE);
		if (!ovResult || bytesLidos == 0) {
			_tprintf_s(ERRO_READ_PIPE);
			break;
		}
		// leitura imediata
		switch (mensagemRead.TipoM) {
		case TMensagem_LOGIN:
			mensagemLogin(td, mensagemRead);
			break;
		case TMensagem_LISTC:
			mensagemListc(td);
			break;
		case TMensagem_BUY:
			mensagemBuy(td, mensagemRead);
			break;
		case TMensagem_SELL:
			mensagemSell(td, mensagemRead);
			break;
		case TMensagem_BALANCE:
			mensagemBalance(td, mensagemRead);
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

		//EnterCriticalSection(&dto->pSync->csContinuar);
		//continuar = mensagemRead.continuar;
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

	// sair da thread
	ExitThread(0);
}

void WINAPI threadBoardHandler(PVOID p) {
	DataTransferObject* dto = (DataTransferObject*)p;
	// TODO: Marques, faz isto!
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