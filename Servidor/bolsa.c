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
	
	FILE* file;
	errno_t err = _tfopen_s(&file, argv[1], _T("r"));
	if (err != 0 || file == NULL) {
		_tprintf_s(ERRO_OPEN_FILE);
		if(file != NULL)
			fclose(file);
		ExitProcess(-1);
	}

	TCHAR linha[MAX_PATH];
	Utilizador utilizadores[MAX_USERS] ;
	DWORD numUtilizadores = 0;
	while (_fgetts(linha, sizeof(linha) / sizeof(linha[0]), file) != NULL) {
		if (numUtilizadores >= MAX_USERS)
			break;
		_stscanf_s(
			linha,
			_T("%s %s %lf"),
			utilizadores[numUtilizadores].username, (unsigned)_countof(utilizadores[numUtilizadores].username),
			utilizadores[numUtilizadores].password, (unsigned)_countof(utilizadores[numUtilizadores].password),
			&(utilizadores[numUtilizadores].saldo));
		numUtilizadores++;
	};
	fclose(file);

	// TODO: criar um array de empresas

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
			_tprintf_s(_T("[INFO] Comando addc\n")); // para apagar
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
				comandoAddc(argumento1, numeroAcoes, precoAcao);
				// TODO: falta adicionar a referência ao array de empresas aos argumentos da função
			}
			
			break;
		case 2: // comando listc
			_tprintf_s(_T("[INFO] Comando listc\n"));
			comandoListc();
			// TODO: falta adicionar a referência ao array de empresas aos argumentos da função
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
				break;
			}
			else {
				double precoAcao = _tstof(argumento2);
				comandoStock(argumento1, precoAcao);
				// TODO: falta adicionar a referência ao array de empresas aos argumentos da função
			}
			break;
		case 4: // comando users
			_tprintf_s(_T("[INFO] Comando users\n")); // para apagar
			comandoUsers();
			// TODO: falta adicionar a referência ao array de utilizadores aos argumentos da função
			break;
		case 5: // comando pause
			_tprintf_s(_T("[INFO] Comando pause\n")); // para apagar
			// TODO: ler os argumentos do comando
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
		case 6: // comando close
			_tprintf_s(_T("[INFO] Comando close\n")); // para apagar
			comandoClose();
			repetir = 1;
			break;
		case 0:
		default:
			_tprintf_s(INVALID_CMD);
			break;
		}
	} while (repetir == 0);

	return 0;
}