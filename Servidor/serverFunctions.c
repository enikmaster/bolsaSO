#include "constantes.h"
#include "servidor.h"

DWORD verificaComandos(TCHAR* comando) {
	const TCHAR listaComandos[][TAM_COMANDO] = { _T("addc"), _T("listc"), _T("stock"), _T("users"), _T("pause"), _T("close")};

	// comando sem argumentos (listc, users e close)
	if (StrChr(comando, _T(' ')) == NULL) {
		for(DWORD i = 0; i < sizeof(listaComandos) / sizeof(listaComandos[0]); ++i) {
			if (_tcscmp(comando, listaComandos[i]) == 0) {
				return i + 1;
			}
		}
		// comando sem argumentos mas comando nao existe
		return 0;
	}
	
	// comando com 1 ou + argumentos (pause, addc e stock)
	if (StrChr(comando, _T(' ')) != NULL) {
		TCHAR comandoTemp[TAM_COMANDO];
		TCHAR argumentoOne[TAM_COMANDO];
		TCHAR argumentoTwo[TAM_COMANDO];
		TCHAR controlo[TAM_COMANDO];
		memset(comandoTemp, 0, sizeof(comandoTemp));
		memset(argumentoOne, 0, sizeof(argumentoOne));
		memset(argumentoTwo, 0, sizeof(argumentoTwo));
		memset(controlo, 0, sizeof(controlo));
		DWORD numeroArgumentos = _stscanf_s(comando, _T("%s %s %s"), comandoTemp, _countof(comandoTemp), argumentoOne, _countof(argumentoOne), argumentoTwo, _countof(argumentoTwo), controlo, _countof(controlo));

		// comando com 1 argumento (pause)
		if (numeroArgumentos == 2) {
			for(DWORD i = 0; i <sizeof(listaComandos) / sizeof(listaComandos[0]); ++i) {
				if (_tcscmp(comandoTemp, listaComandos[i]) == 0) {
					return ++i;
				}
			}
			// comando com 1 argumento mas comando nao existe
			return 0;
		}

		// comando com 2 argumentos (addc, stock)
		if(numeroArgumentos == 3) {
			for(DWORD i = 0; i < sizeof(listaComandos) / sizeof(listaComandos[0]); ++i) {
				if (_tcscmp(comandoTemp, listaComandos[i]) == 0) {
					return ++i;
				}
			}
			// comando com 2 argumentos mas comando nao existe
			return 0;
		}
		// mais do que 2 argumentos devolve erro
		return 0;
	}
}