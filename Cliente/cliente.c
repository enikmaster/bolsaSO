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
		_tprintf_s(INVALID_N_ARGS);
		ExitProcess(-1);
	}

	// TODO: criar threads para as diferentes funcionalidades necessárias
	// Thread 1 - ler as mensagens do servidor
	// Thread 2 - ler os comandos do utilizador para enviar para o servidor
	//

	DWORD controlo = 0;
	TCHAR comando[TAM_COMANDO];
	TCHAR comandoTemp[TAM_COMANDO];
	TCHAR argumento1[TAM_COMANDO];
	TCHAR argumento2[TAM_COMANDO];
	TCHAR failSafe[TAM_COMANDO];
	boolean repetir = TRUE;
	boolean logado = FALSE;
	int numArgumentos;
	while(repetir) {
		memset(comandoTemp, 0, sizeof(comandoTemp));
		memset(argumento1, 0, sizeof(argumento1));
		memset(argumento2, 0, sizeof(argumento2));
		memset(failSafe, 0, sizeof(failSafe));
		logado ? _tprintf_s(_T("Comando: ")) : _tprintf_s(_T("Efetue login primeiro\nComando:  "));
		_fgetts(comando, sizeof(comando)/sizeof(comando[0]), stdin);
		comando[_tcslen(comando) - 1] = _T('\0');
		controlo = verificaComando(comando);
		numArgumentos = 0;
		switch (controlo) {
		case 1: // comando login
			if (!logado) {
				numArgumentos = _stscanf_s(comando, _T("%s %s %s %s"),
					comandoTemp, (unsigned)_countof(comandoTemp),
					argumento1, (unsigned)_countof(argumento1),
					argumento2, (unsigned)_countof(argumento2),
					failSafe, (unsigned)_countof(failSafe));
				if (numArgumentos != 3)
					_tprintf_s(INVALID_N_ARGS);
				else {
					logado = comandoLogin(argumento1, argumento2);
					logado ? _tprintf_s(INFO_LOGIN) : _tprintf_s(ERRO_LOGIN);
				}
			} else {
				_tprintf_s(ERRO_ALREADY_LOGIN);
			}
			
			break;
		case 2: // comando listc
			if (logado)
				comandoListc();
			else
				_tprintf_s(ERRO_NO_LOGIN);
			break;
		case 3: // comando buy
			if(logado) {
				numArgumentos = _stscanf_s(comando, _T("%s %s %s %s"),
					comandoTemp, (unsigned)_countof(comandoTemp),
					argumento1, (unsigned)_countof(argumento1),
					argumento2, (unsigned)_countof(argumento2),
					failSafe, (unsigned)_countof(failSafe));
				if (numArgumentos != 3)
					_tprintf_s(INVALID_N_ARGS);
				else
					comandoBuy(argumento1, _tstoi(argumento2));
			} else {
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
					_tprintf_s(INVALID_N_ARGS);
				else
					comandoSell(argumento1, _tstoi(argumento2));
			} else {
				_tprintf_s(ERRO_NO_LOGIN);
			}
			break;
		case 5: // comando balance
			if(logado)
				comandoBalance();
			else
				_tprintf_s(ERRO_NO_LOGIN);
			break;
		case 6: // comando wallet
			if(logado)
				comandoWallet();
			else
				_tprintf_s(ERRO_NO_LOGIN);
			break;
		case 7: // comando exit
			if (logado) {
				_tprintf_s(_T("[INFO] Comando exit\n"));
				comandoExit();
				repetir = FALSE;
			} else {
				_tprintf_s(ERRO_NO_LOGIN);
			}
			break;
		case 0: // comando inválido
		default:
			_tprintf_s(INVALID_CMD);
			break;
		}
	};

	ExitProcess(0);
}