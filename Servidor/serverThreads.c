#include "servidor.h"

// fun��es das threads
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
				comandoTemp, (unsigned)_countof(comandoTemp), // vari�vel onde guardar o comando + tamanho do buffer
				argumento1, (unsigned)_countof(argumento1),   // vari�vel onde guardar o 1� argumento + tamanho do buffer
				argumento2, (unsigned)_countof(argumento2),   // vari�vel onde guardar o 2� argumento + tamanho do buffer
				argumento3, (unsigned)_countof(argumento3),   // vari�vel onde guardar o 3� argumento + tamanho do buffer
				failSafe, (unsigned)_countof(failSafe));      // vari�vel de seguran�a + tamanho do buffer (se preenchida o num de argumentos estar� a mais)
			if (numArgumentos != 4) {
				_tprintf_s(ERRO_INVALID_N_ARGS);
			}
			else {
				DWORD numeroAcoes = _tstoi(argumento2);
				double precoAcao = _tstof(argumento3);
				if (comandoAddc(td->dto, argumento1, numeroAcoes, precoAcao)){
					_tprintf_s(INFO_ADDC, argumento1);
					mensagemAddc(td, argumento1);
				} else
					_tprintf_s(ERRO_ADDC);
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
				if (comandoStock(td->dto, argumento1, valorAcao)) {
					_tprintf_s(INFO_STOCK, argumento1, valorAcao);
					mensagemStock(td, argumento1, valorAcao);
				}
				else 
					_tprintf_s(ERRO_STOCK);
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
				// TODO: falta qq coisa mas n�o sei o que � para j�
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
				int numLoad = comandoLoad(td->dto, argumento1);
				if (numLoad == -1)
					_tprintf_s(ERRO_LOAD);
				else {
					_tprintf_s(INFO_LOAD, numLoad);
					mensagemLoad(td, numLoad);
				}
			}
			break;
		case 7: // comando close
			mensagemClose(td);
			repetir = comandoClose(td);
			break;
		case 0:
		default: // comando inv�lido
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

	HANDLE hEvents[2] = { ov.hEvent, td->dto->dadosP->hExitEvent };
	DWORD dwWaitResult;

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
			sizeof(Mensagem),	// n�mero de bytes a ler
			&bytesLidos,	// n�mero de bytes lidos
			&ov);	// estrutura overlapped)
		// leitura pendente
		dwWaitResult = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
		if (dwWaitResult == WAIT_OBJECT_0 + 1) {
			// evento para sair do programa
			continuar = FALSE;
			break;
		}
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
			mensagemWallet(td, mensagemRead);
			break;
		case TMensagem_EXIT:
			mensagemExit(td, mensagemRead);
			break;
		default:
			_tprintf_s(ERRO_INVALID_MSG);
			break;
		}
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

//void WINAPI threadBoardHandler(PVOID p) {
//	ThreadData* td = (ThreadData*)p;
//	
//
//	//criar o evento de reset manual para o board
//	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NOME_EVENTO_BOARD);
//	if (hEvent == NULL) {
//		_tprintf_s(ERRO_CREATE_EVENT);
//		return;
//	}
//	
//	td->dto->dadosP->hEvent = hEvent; //
//
//	WaitForSingleObject(td->hExitEvent, INFINITE);
//}


//thread para alterar o pre�o das a��es
//vai aceder �s empresas, e guardar num array local
	//passado 10segundos, vai verificar se houve altera��es no array das empresas


	//se a quantidade de a�oes disponiveis for maior que a anterior, quer dizer que clientes venderam � bolsa ( pre�o das a��es desce)
	//se a quantidade de a�oes disponiveis for menor que a anterior, quer dizer que clientes compraram � bolsa ( pre�o das a��es sobe)
void WINAPI threadVariacaoPrecoHandler (PVOID p) {
	ThreadData* td = (ThreadData*)p;
	if(td == NULL) {
		_tprintf_s(_T("DADOS THREAD ERRADOS"));
		return;
	}

	// criar o timer
	HANDLE hTimerV = CreateWaitableTimer(NULL, TRUE, NULL);
	if (hTimerV == NULL) {
		_tprintf_s(ERRO_CREATE_TIMER);
		return;
	}

	LARGE_INTEGER liDueTime;
	liDueTime.QuadPart = -100000000; // 10 segundos

	HANDLE hThreads[2] = { hTimerV, td->dto->dadosP->hExitEvent };

	Empresa listaEmpresas[TAM_MAX_EMPRESAS] = { 0 };
	int numEmpresas = 0;
	double percentagem = 0.02; // 2%
	
	while (1) {
		// definir o tempo de espera
		if(!SetWaitableTimer(hTimerV, &liDueTime, 0, NULL, NULL, FALSE)) {
			_tprintf_s(ERRO_SET_TIMER);
			break;
		}
		DWORD i = 0;
		// esperar por eventos
		WaitForMultipleObjects(2, hThreads, FALSE, INFINITE);
		if(WaitForSingleObject(td->dto->dadosP->hExitEvent, 0) == WAIT_OBJECT_0)
			break;

		//verificar varia��o do pre�o das a��es
		EnterCriticalSection(&td->dto->pSync->csEmpresas);
		numEmpresas = td->dto->dadosP->numEmpresas;
		// verificar se a listaEmpresas est� vazia (primeira vez que � chamada)
		if (listaEmpresas->nome[0] == 0) {
			// copiar as empresas para a listaEmpresas (primeira vez)
			memcpy(listaEmpresas, td->dto->dadosP->empresas, numEmpresas * sizeof(Empresa));
		}
		for (i; i < numEmpresas; ++i) {
			(td->dto->dadosP->empresas[i].quantidadeAcoes >= listaEmpresas[i].quantidadeAcoes) ? 
				// o pre�o desce
				(td->dto->dadosP->empresas[i].valorAcao *= (1 - percentagem)) : 
				// o pre�o sobe
				(td->dto->dadosP->empresas[i].valorAcao *= (1 + percentagem));
		}
		// depois de verificar as varia��es, guardar os novos dados para a pr�xima verifica��o
		memcpy(listaEmpresas, td->dto->dadosP->empresas, numEmpresas * sizeof(Empresa));
		LeaveCriticalSection(&td->dto->pSync->csEmpresas);
	}
	CloseHandle(hTimerV);
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