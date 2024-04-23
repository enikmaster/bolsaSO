#include "servidor.h"
#include "constantes.h"

int _tmain(int argc, TCHAR** argv) {
	#ifdef UNICODE
	DWORD x1, x2, x3;
		x1 = _setmode(_fileno(stdin), _O_WTEXT);
		x2 = _setmode(_fileno(stdout), _O_WTEXT);
		x3 = _setmode(_fileno(stderr), _O_WTEXT);
		if (x1 == -1 || x2 == -1 || x3 == -1)
			ExitProcess(-1);
	#endif
	if (argc != 2) {
		_tprintf_s(ERRO_INVALID_N_ARGS);
		ExitProcess(-1);
	}
	
	// Inicializar os dados do sistema
	DataTransferObject dto;
	if(!inicializarDTO(&dto)) {
		_tprintf_s(ERRO_INICIALIZAR_DTO);
		ExitProcess(-1);
	}
	dto.limiteClientes = lerCriarRegistryKey();
	dto.numUtilizadores = lerUtilizadores(&dto, argv[1]);
	dto.numPipes = 0;

	HANDLE hThread = CreateThread(NULL, 0, threadConnectionHandler, &dto,	0, NULL);
	if (hThread == NULL) {
		_tprintf_s(ERRO_CREATE_THREAD);
		terminarDTO(&dto);
		ExitProcess(-1);
	}

	DWORD controlo = 0;
	TCHAR comando[TAM_COMANDO];
	TCHAR comandoTemp[TAM_COMANDO];
	TCHAR argumento1[TAM_COMANDO];
	TCHAR argumento2[TAM_COMANDO];
	TCHAR argumento3[TAM_COMANDO];
	TCHAR failSafe[TAM_COMANDO];
	BOOL repetir = TRUE;
	int numArgumentos;
	while(repetir) {
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
				_tprintf_s(ERRO_INVALID_N_ARGS);
			} else {
				DWORD numeroAcoes = _tstoi(argumento2);
				double precoAcao = _tstof(argumento3);
				comandoAddc(&dto, argumento1, numeroAcoes, precoAcao)
				? _tprintf_s(INFO_ADDC)
				: _tprintf_s(ERRO_ADDC);
			}
			break;
		case 2: // comando listc
			comandoListc(&dto);
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
			} else {
				double valorAcao = _tstof(argumento2);
				comandoStock(&dto, argumento1, valorAcao)
				? _tprintf_s(INFO_STOCK)
				: _tprintf_s(ERRO_STOCK);
			}
			break;
		case 4: // comando users
			comandoUsers(&dto);
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
			} else {
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
			} else {
				comandoLoad(&dto, argumento1)
					? _tprintf_s(INFO_LOAD)
					: _tprintf_s(ERRO_LOAD);
			}
			break;
		case 7: // comando close
			_tprintf_s(_T("[INFO] Comando close\n")); // para apagar
			comandoClose(); // TODO: passar o dto por referência
			repetir = FALSE;
			break;
		case 0:
		default: // comando inválido
			_tprintf_s(ERRO_INVALID_CMD);
			break;
		}
	};

	// esperar que as threads terminem
	if (WaitForSingleObject(hThread, INFINITE) != WAIT_OBJECT_0)
		_tprintf_s(ERRO_ESPERAR_THREADS);
	CloseHandle(hThread);
	terminarDTO(&dto);

	ExitProcess(0);
}