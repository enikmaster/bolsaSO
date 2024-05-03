#include "cliente.h"

void WINAPI threadComandosClienteHandler(PVOID p) {
	HANDLE* hPipe = (HANDLE*)p;
	DWORD controlo = 0;
	TCHAR comando[TAM_COMANDO];
	TCHAR comandoTemp[TAM_COMANDO];
	TCHAR argumento1[TAM_COMANDO];
	TCHAR argumento2[TAM_COMANDO];
	TCHAR failSafe[TAM_COMANDO];
	BOOL repetir = TRUE;
	BOOL logado = FALSE;
	int numArgumentos;
	while (repetir) {
		memset(comandoTemp, 0, sizeof(comandoTemp));
		memset(argumento1, 0, sizeof(argumento1));
		memset(argumento2, 0, sizeof(argumento2));
		memset(failSafe, 0, sizeof(failSafe));
		if(!logado) 
			_tprintf_s(_T("Efetue login primeiro\nComando:  "));
		_fgetts(comando, sizeof(comando) / sizeof(comando[0]), stdin);
		comando[_tcslen(comando) - 1] = _T('\0');
		controlo = verificaComando(comando);
		numArgumentos = 0;
		system("cls");
		switch (controlo) {
		case 1: // comando login
			if (!logado) {
				numArgumentos = _stscanf_s(comando, _T("%s %s %s %s"),
					comandoTemp, (unsigned)_countof(comandoTemp),
					argumento1, (unsigned)_countof(argumento1),
					argumento2, (unsigned)_countof(argumento2),
					failSafe, (unsigned)_countof(failSafe));
				if (numArgumentos != 3)
					_tprintf_s(ERRO_INVALID_N_ARGS);
				else {
					logado = comandoLogin(hPipe, argumento1, argumento2);
				}
			}
			else {
				_tprintf_s(ERRO_ALREADY_LOGIN);
			}

			break;
		case 2: // comando listc
			if (logado)
				comandoListc(hPipe);
			else
				_tprintf_s(ERRO_NO_LOGIN);
			break;
		case 3: // comando buy
			if (logado) {
				numArgumentos = _stscanf_s(comando, _T("%s %s %s %s"),
					comandoTemp, (unsigned)_countof(comandoTemp),
					argumento1, (unsigned)_countof(argumento1),
					argumento2, (unsigned)_countof(argumento2),
					failSafe, (unsigned)_countof(failSafe));
				if (numArgumentos != 3)
					_tprintf_s(ERRO_INVALID_N_ARGS);
				else
					comandoBuy(argumento1, _tstoi(argumento2));
			}
			else {
				_tprintf_s(ERRO_NO_LOGIN);
			}
			break;
		case 4: // comando sell
			if (logado) {
				numArgumentos = _stscanf_s(comando, _T("%s %s %s %s"),
					comandoTemp, (unsigned)_countof(comandoTemp),
					argumento1, (unsigned)_countof(argumento1),
					argumento2, (unsigned)_countof(argumento2),
					failSafe, (unsigned)_countof(failSafe));
				if (numArgumentos != 3)
					_tprintf_s(ERRO_INVALID_N_ARGS);
				else
					comandoSell(argumento1, _tstoi(argumento2));
			}
			else {
				_tprintf_s(ERRO_NO_LOGIN);
			}
			break;
		case 5: // comando balance
			if (logado)
				comandoBalance();
			else
				_tprintf_s(ERRO_NO_LOGIN);
			break;
		case 6: // comando wallet
			if (logado)
				comandoWallet();
			else
				_tprintf_s(ERRO_NO_LOGIN);
			break;
		case 7: // comando exit
			_tprintf_s(_T("[INFO] Comando exit\n"));
			comandoExit();
			repetir = FALSE;
			break;
		case 0: // comando inválido
		default:
			_tprintf_s(ERRO_INVALID_CMD);
			break;
		}
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