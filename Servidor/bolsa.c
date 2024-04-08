#include "servidor.h"
#include "constantes.h"

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
	if (argc != 2) {
		_tprintf_s(INVALID_N_ARGS);
		return(1);
	}

	Utilizador utilizadores[MAX_USERS];
	DWORD numUtilizadores = lerUtilizadores(&utilizadores, argv[1]);

	Empresa empresas[MAX_EMPRESAS];
	DWORD numEmpresas = 0;

	// TODO: criar threads para as diferente funcionalidades necessárias

	DWORD controlo = 0;
	TCHAR comando[TAM_COMANDO];
	TCHAR comandoTemp[TAM_COMANDO];
	TCHAR argumento1[TAM_COMANDO];
	TCHAR argumento2[TAM_COMANDO];
	TCHAR argumento3[TAM_COMANDO];
	TCHAR failSafe[TAM_COMANDO];
	DWORD repetir = 0;
	int numArgumentos;
	do {
		memset(comandoTemp, 0, sizeof(comando));
		memset(argumento1, 0, sizeof(argumento1));
		memset(argumento2, 0, sizeof(argumento2));
		memset(argumento3, 0, sizeof(argumento3));
		memset(failSafe, 0, sizeof(failSafe));
		_tprintf_s(_T("Comando: "));
		_fgetts(comando, sizeof(comando)/sizeof(comando[0]), stdin);
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
			if(numArgumentos != 4) {
				_tprintf_s(INVALID_N_ARGS);
				break;
			}
			else {
				DWORD numeroAcoes = _tstoi(argumento2);
				double precoAcao = _tstof(argumento3);
				if (comandoAddc(argumento1, numeroAcoes, precoAcao, &empresas, numEmpresas) == -1)
					_tprintf_s(ERRO_ADDC);
				else
				{
					++numEmpresas;
					_tprintf_s(INFO_ADDC);
				}
			}
			break;
		case 2: // comando listc
			comandoListc(numEmpresas, empresas);
			break;
		case 3: // comando stock
			_tprintf_s(_T("[INFO] Comando stock\n")); // para apagar
			numArgumentos = _stscanf_s(
				comando,
				_T("%s %s %s %s"),
				comandoTemp, (unsigned)_countof(comandoTemp),
				argumento1, (unsigned)_countof(argumento1),
				argumento2, (unsigned)_countof(argumento2),
				failSafe, (unsigned)_countof(failSafe));
			if (numArgumentos != 3) {
				_tprintf_s(INVALID_N_ARGS);
			} else {
				double valorAcao = _tstof(argumento2);
				if (comandoStock(argumento1, valorAcao, &empresas, numEmpresas))
					_tprintf_s(_T("[INFO] Valor alterado com sucesso"));
				else
					_tprintf_s(_T("[ERRO] Erro a altera o valor da ação"));
			}
			break;
		case 4: // comando users
			comandoUsers(numUtilizadores, utilizadores);
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
				_tprintf_s(INVALID_N_ARGS);
				break;
			}
			else {
				DWORD numeroSegundos = _tstoi(argumento1);
				comandoPause(numeroSegundos);
				// TODO: falta qq coisa mas não sei o que é para já
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
				_tprintf_s(INVALID_N_ARGS);
				break;
			}
			else {
				if(numEmpresas < MAX_EMPRESAS) {
					numEmpresas = comandoLoad(&empresas, numEmpresas, argumento1);
					_tprintf_s(INFO_LOAD);
				} else
					_tprintf_s(ERRO_LOAD);
			}
			
			break;
		case 7: // comando close
			_tprintf_s(_T("[INFO] Comando close\n")); // para apagar
			comandoClose();
			repetir = 1;
			break;
		case 0:
		default: // casos de erro
			_tprintf_s(INVALID_CMD);
			break;
		}
	} while (repetir == 0);

	return 0;
}