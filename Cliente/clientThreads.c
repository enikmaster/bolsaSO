#include "cliente.h"

void WINAPI threadComandosClienteHandler(PVOID p) {
	ClienteData* cd = (ClienteData*)p;
	HANDLE hPipe = &cd->hPipe;
	DWORD controlo = 0;
	TCHAR comando[TAM_COMANDO];
	TCHAR comandoTemp[TAM_COMANDO];
	TCHAR argumento1[TAM_COMANDO];
	TCHAR argumento2[TAM_COMANDO];
	TCHAR failSafe[TAM_COMANDO];
	BOOL repetir = TRUE;
	TCHAR username[TAM_NOME];
	int numArgumentos;
	HANDLE hStdin = {0};
	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	if (hStdin == INVALID_HANDLE_VALUE) {
		_tprintf_s(ERRO_GET_STDIN);
		return;
	}
	/*OVERLAPPED ol = { 0 };
	ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if(ol.hEvent == NULL) {
		_tprintf_s(ERRO_CREATE_EVENT);
		CloseHandle(hStdin);
		return;
	}
	DWORD bytesLidos;*/
	HANDLE hEvents[2] = { hStdin, cd->hExitEvent };
	DWORD stdinResult = 0;

	DWORD dadosLidos;
	INPUT_RECORD irBuffer[TAM_COMANDO];
	TCHAR chBuffer[TAM_COMANDO] = { 0 };
	int chBufSize = TAM_COMANDO*2;
	int chBufIndex = 0;

	_tprintf_s(WELCOME);
	while(repetir) {
		memset(comandoTemp, 0, sizeof(comandoTemp));
		memset(argumento1, 0, sizeof(argumento1));
		memset(argumento2, 0, sizeof(argumento2));
		memset(failSafe, 0, sizeof(failSafe));
		fflush(stdin);
		if(!cd->logado) 
			_tprintf_s(_T("Efetue login primeiro\nComando:  "));

		// Tentativa #1
		/*hStdin = GetStdHandle(STD_INPUT_HANDLE);
		if (hStdin == INVALID_HANDLE_VALUE) {
			_tprintf_s(ERRO_GET_STDIN);
			return;
		}*/

		// Tentativa que resulta +/-
		//stdinResult = WaitForSingleObject(hStdin, INFINITE);
		/*stdinResult = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
		if (stdinResult == WAIT_OBJECT_0 + 1) {
			repetir = FALSE;
			break;
		}*/
		
		// Tentativa #2
		/*ResetEvent(ol.hEvent);
		BOOL resultResult = ReadFile(hStdin, comando, TAM_COMANDO * sizeof(TCHAR), &bytesLidos, &ol);
		if (!resultResult);
		WaitForSingleObject(ol.hEvent, INFINITE);
		BOOL result = GetOverlappedResult(hStdin, &ol, &bytesLidos, FALSE);
		if (!result || bytesLidos == 0) {
			PrintLastError(_T("GetOverlappedResult"), GetLastError());
			break;
		}*/

		// Tentativa #3
		// espreita o console para ver se há input
		//if (!PeekConsoleInput(hStdin, irBuffer, TAM_COMANDO * 2, &dadosLidos)) {
		//	// TODO: lidar com erro
		//}
		 
		// Tentativa #4
		/*stdinResult = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
		if (stdinResult == WAIT_OBJECT_0 + 1) {
			repetir = FALSE;
			break;
		}*/

		if (!PeekConsoleInput(hStdin, irBuffer, TAM_COMANDO * 2, &dadosLidos)) {
				// TODO: lidar com erro
		}
		
		// se houver input, lê
		//if (stdinResult == WAIT_OBJECT_0) {
		if (dadosLidos > 0) {
			if (!ReadConsoleInput(hStdin, irBuffer, TAM_COMANDO * 2, &dadosLidos)) {
				// TODO: lidar com o erro
			}
			for (int i = 0; i < dadosLidos; i++)
			{
				if (irBuffer[i].EventType == KEY_EVENT && irBuffer[i].Event.KeyEvent.bKeyDown)
				{
					TCHAR ch = irBuffer[i].Event.KeyEvent.uChar.UnicodeChar;
					if (ch != '\n' && chBufIndex < chBufSize - 1)
					{
						chBuffer[chBufIndex++] = ch;
					}
					else if (ch == '\n' && chBufIndex > 0 && chBuffer[chBufIndex - 1] == '\r')
					{
						chBuffer[chBufIndex - 1] = '\0';
						_tcscpy_s(comando, TAM_COMANDO, chBuffer);
						chBufIndex = 0;
					}
				}
			}
			
			//_fgetts(comando, sizeof(comando) / sizeof(comando[0]), stdin);
			//comando[_tcslen(comando) - 1] = _T('\0');
			controlo = verificaComando(comando);
			numArgumentos = 0;
			system("cls");
			switch (controlo) {
			case 1: // comando login
				if (!cd->logado) {
					numArgumentos = _stscanf_s(comando, _T("%s %s %s %s"),
						comandoTemp, (unsigned)_countof(comandoTemp),
						argumento1, (unsigned)_countof(argumento1),
						argumento2, (unsigned)_countof(argumento2),
						failSafe, (unsigned)_countof(failSafe));
					if (numArgumentos != 3)
						_tprintf_s(ERRO_INVALID_N_ARGS);
					else {
						comandoLogin(hPipe, argumento1, argumento2);
						memcpy(username, argumento1, _tcslen(argumento1) * sizeof(TCHAR));
						username[_tcslen(argumento1)] = _T('\0');
					}
				}
				else {
					_tprintf_s(ERRO_ALREADY_LOGIN);
				}

				break;
			case 2: // comando listc
				if (cd->logado)
					comandoListc(hPipe);
				else
					_tprintf_s(ERRO_NO_LOGIN);
				break;
			case 3: // comando buy
				if (cd->logado) {
					numArgumentos = _stscanf_s(comando, _T("%s %s %s %s"),
						comandoTemp, (unsigned)_countof(comandoTemp),
						argumento1, (unsigned)_countof(argumento1),
						argumento2, (unsigned)_countof(argumento2),
						failSafe, (unsigned)_countof(failSafe));
					if (numArgumentos != 3)
						_tprintf_s(ERRO_INVALID_N_ARGS);
					else
						comandoBuy(hPipe, username, argumento1, _tstoi(argumento2));
				}
				else {
					_tprintf_s(ERRO_NO_LOGIN);
				}
				break;
			case 4: // comando sell
				if (cd->logado) {
					numArgumentos = _stscanf_s(comando, _T("%s %s %s %s"),
						comandoTemp, (unsigned)_countof(comandoTemp),
						argumento1, (unsigned)_countof(argumento1),
						argumento2, (unsigned)_countof(argumento2),
						failSafe, (unsigned)_countof(failSafe));
					if (numArgumentos != 3)
						_tprintf_s(ERRO_INVALID_N_ARGS);
					else
						comandoSell(hPipe, username, argumento1, _tstoi(argumento2));
				}
				else {
					_tprintf_s(ERRO_NO_LOGIN);
				}
				break;
			case 5: // comando balance
				if (cd->logado)
					comandoBalance(hPipe, username);
				else
					_tprintf_s(ERRO_NO_LOGIN);
				break;
			case 6: // comando wallet
				if (cd->logado)
					comandoWallet(hPipe, username);
				else
					_tprintf_s(ERRO_NO_LOGIN);
				break;
			case 7: // comando exit
				repetir = comandoExit(hPipe, username);
				if (repetir == FALSE) {
					// usar mutex para garantir que o exit é feito antes de fechar o pipe
					WaitForSingleObject(cd->hMutex, INFINITE);
					SetEvent(cd->hExitEvent);
					ReleaseMutex(cd->hMutex);
				}
				break;
			case 0: // comando inválido
			default:
				_tprintf_s(ERRO_INVALID_CMD);
				break;
			}
		}
		else {
			// se não houver input, verifica se o evento de exit foi sinalizado
			//WaitForSingleObject(cd->hExitEvent, 0);
			if (WaitForSingleObject(cd->hExitEvent, 0) == WAIT_OBJECT_0) {
				repetir = FALSE;
				break;
			}
		}


		// tentativa que funciona +/-
		/*stdinResult = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
		if (stdinResult == WAIT_OBJECT_0 + 1) {
			repetir = FALSE;
			break;
		}*/
	};
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