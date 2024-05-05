#include "servidor.h"

// funções da plataforma
DWORD verificaComando(const TCHAR* comando) {
	const TCHAR listaComandos[][TAM_COMANDO] = { _T("addc"), _T("listc"), _T("stock"), _T("users"), _T("pause"), _T("load"), _T("close")};

	// comando sem argumentos
	if (_tcschr(comando, _T(' ')) == NULL) {
		for(DWORD i = 0; i < sizeof(listaComandos) / sizeof(listaComandos[0]); ++i) {
			if (_tcscmp(comando, listaComandos[i]) == 0) {
				return i + 1;
			}
		}
	} else {
		TCHAR comandoTemp[TAM_COMANDO];
		TCHAR argumentos[TAM_COMANDO];
		memset(comandoTemp, 0, sizeof(comandoTemp));
		memset(argumentos, 0, sizeof(argumentos));
		_stscanf_s(comando, _T("%s %s"), comandoTemp, TAM_COMANDO, argumentos, TAM_COMANDO);

		// garantir que a string é terminada com zero
		comandoTemp[TAM_COMANDO - 1] = _T('\0');

		for(DWORD i = 0; i <sizeof(listaComandos) / sizeof(listaComandos[0]); ++i) {
			if (_tcscmp(comandoTemp, listaComandos[i]) == 0) {
				return ++i;
			}
		}
	}
	return 0;
}

DWORD lerUtilizadores(DataTransferObject* dto, const TCHAR* nomeFicheiro) {
	Utilizador uLocal[TAM_MAX_USERS];
	FILE* file;
	errno_t err = _tfopen_s(&file, nomeFicheiro, _T("r"));
	if (err != 0 || file == NULL) {
		_tprintf_s(ERRO_OPEN_FILE);
		if (file != NULL)
			fclose(file);
		ExitProcess(-1);
	}
	TCHAR linha[MAX_PATH];
	DWORD i = 0;
	while (_fgetts(linha, sizeof(linha) / sizeof(linha[0]), file) != NULL) {
		if (i >= TAM_MAX_USERS)
			break;
		_stscanf_s(
			linha,
			_T("%s %s %lf"),
			uLocal[i].username, (unsigned)_countof(uLocal[i].username),
			uLocal[i].password, (unsigned)_countof(uLocal[i].password),
			&(uLocal[i].saldo));
		uLocal[i++].logado = FALSE;
	};
	fclose(file);
	EnterCriticalSection(&dto->pSync->csUtilizadores);
	CopyMemory(dto->utilizadores, uLocal, i * sizeof(Utilizador));
	LeaveCriticalSection(&dto->pSync->csUtilizadores);
	return i;
}

// lê o valor de NCLIENTES do registo, se nexistir cria a key
DWORD lerCriarRegistryKey() {
	HKEY hKey;
	TCHAR nomeKey[TAM_REGISTRY];
	DWORD tamanho = sizeof(DWORD);
	DWORD nClientes = 5;
	DWORD res;
	DWORD limiteClientes = 0;

	_stprintf_s(nomeKey, TAM_REGISTRY, NOME_REGISTRY_KEY_NCLIENTES);

	res = RegOpenKeyEx(HKEY_CURRENT_USER, nomeKey, 0, KEY_READ, &hKey);
	if (res == ERROR_SUCCESS) {
		RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE)&limiteClientes, &tamanho);
		_tprintf_s(_T("[INFO] O valor lido no registry para NCLIENTES foi: %lu\n"), limiteClientes);
	} else {
		res = RegCreateKeyEx(HKEY_CURRENT_USER, nomeKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &res);
		if (res == ERROR_SUCCESS) {
			RegSetValueEx(hKey, NULL, 0, REG_DWORD, (const BYTE*)&nClientes, sizeof(DWORD));
			_tprintf_s(_T("O valor NCLIENTES escrito no registry foi: %lu\n"), nClientes);
			limiteClientes = nClientes;
		} else 
			_tprintf_s(ERRO_CREATE_KEY_NCLIENTES);
	}

	RegCloseKey(hKey);
	return limiteClientes;
}

BOOL inicializarDTO(DataTransferObject* dto) {
	// criar o file mapping
	dto->hMap = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		sizeof(DadosPartilhados),
		NOME_SHARED_MEMORY);
	if (dto->hMap == NULL) {
		_tprintf_s(ERRO_CREATE_FILE_MAPPING);
		return FALSE;
	}

	// criar o map view of file
	dto->pView = MapViewOfFile(
		dto->hMap,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		sizeof(DadosPartilhados));
	if (dto->pView == NULL) {
		_tprintf_s(ERRO_CREATE_MAP_VIEW);
		CloseHandle(dto->hMap);
		return FALSE;
	}
	dto->pSync = (pSync)malloc(sizeof(Sync));
	if (dto->pSync == NULL) {
		_tprintf_s(ERRO_MEMORIA);
		UnmapViewOfFile(dto->pView);
		CloseHandle(dto->hMap);
		return FALSE;
	};

	// criar o semáforo da bolsa
	dto->pSync->hSemBolsa = CreateSemaphore(
		NULL,
		0,
		1,
		NOME_SEMAFORO);
	if (dto->pSync->hSemBolsa == NULL) {
		_tprintf_s(ERRO_CREATE_SEM);
		UnmapViewOfFile(dto->pView);
		CloseHandle(dto->hMap);
		return FALSE;
	}

	// criar o mutex da bolsa
	dto->pSync->hMtxBolsa = CreateMutex(
		NULL,
		FALSE,
		NULL);
	if (dto->pSync->hMtxBolsa == NULL) {
		_tprintf_s(ERRO_CREATE_MUTEX);
		UnmapViewOfFile(dto->pView);
		CloseHandle(dto->hMap);
		CloseHandle(dto->pSync->hSemBolsa);
		return FALSE;
	}

	// criar os critical sections
	InitializeCriticalSection(&dto->pSync->csContinuar);
	InitializeCriticalSection(&dto->pSync->csLimClientes);
	InitializeCriticalSection(&dto->pSync->csEmpresas);
	InitializeCriticalSection(&dto->pSync->csUtilizadores);

	dto->dadosP = (DadosPartilhados*)dto->pView;
	dto->dadosP->numEmpresas = 0;
	dto->continuar = TRUE;

	return TRUE;
}

void terminarDTO(DataTransferObject* dto) {
	// garantir que saiu das secções críticas
	LeaveCriticalSection(&dto->pSync->csContinuar);
	LeaveCriticalSection(&dto->pSync->csLimClientes);
	LeaveCriticalSection(&dto->pSync->csEmpresas);
	LeaveCriticalSection(&dto->pSync->csUtilizadores);
	// apagar os CriticalSections
	DeleteCriticalSection(&dto->pSync->csContinuar);
	DeleteCriticalSection(&dto->pSync->csLimClientes);
	DeleteCriticalSection(&dto->pSync->csEmpresas);
	DeleteCriticalSection(&dto->pSync->csUtilizadores);
	// libertar o semáforo
	ReleaseSemaphore(dto->pSync->hSemBolsa, 1, NULL);
	// desmapear a memória partilhada
	UnmapViewOfFile(dto->pView);
	// fechar os handles por ordem inversa da sua criação
	CloseHandle(dto->hMap);
	CloseHandle(dto->pSync->hSemBolsa);
	CloseHandle(dto->pSync->hMtxBolsa);
	free(dto->pSync);
	
}

// comandos do servidor
BOOL comandoAddc(DataTransferObject* dto, const TCHAR* nomeEmpresa, const DWORD numeroAcoes, const double precoAcao) {
	system("cls");
	EnterCriticalSection(&dto->pSync->csEmpresas);
	DWORD numEmpresas = dto->dadosP->numEmpresas;
	if (numEmpresas >= TAM_MAX_EMPRESAS) {
		LeaveCriticalSection(&dto->pSync->csEmpresas);
		return FALSE;
	}
	_tcscpy_s(dto->dadosP->empresas[numEmpresas].nome, TAM_NOME, nomeEmpresa);
	dto->dadosP->empresas[numEmpresas].quantidadeAcoes = numeroAcoes;
	dto->dadosP->empresas[numEmpresas++].valorAcao = precoAcao;
	dto->dadosP->numEmpresas = numEmpresas;
	LeaveCriticalSection(&dto->pSync->csEmpresas);

	return TRUE;
}

void comandoListc(DataTransferObject* dto) {
	system("cls");
	Empresa eLocal[TAM_MAX_EMPRESAS];
	DWORD numEmpresasLocal = 0;

	EnterCriticalSection(&dto->pSync->csEmpresas);
	numEmpresasLocal = dto->dadosP->numEmpresas;
	CopyMemory(eLocal, dto->dadosP->empresas, numEmpresasLocal * sizeof(Empresa));
	LeaveCriticalSection(&dto->pSync->csEmpresas);

	_tprintf_s(_T(" -- Lista de empresas --\n"));
	for (DWORD i = 0; i < numEmpresasLocal; ++i) {
		_tprintf_s(INFO_LISTC, eLocal[i].nome, eLocal[i].quantidadeAcoes, eLocal[i].valorAcao);
	}

	// Verificação de erro ao assinalar o evento
	if (!SetEvent(dto->dadosP->hEvent)) {
		_tprintf_s(_T("Erro ao assinalar o evento: %lu\n"), GetLastError());
	}
}

BOOL comandoStock(DataTransferObject* dto, const TCHAR* nomeEmpresa, const double valorAcao) {
	system("cls");
	EnterCriticalSection(&dto->pSync->csEmpresas);
	for (DWORD i = 0; i < &dto->dadosP->numEmpresas; ++i) {
		if (_tcscmp(nomeEmpresa, dto->dadosP->empresas[i].nome) == 0) {
			dto->dadosP->empresas[i].valorAcao = valorAcao;
			LeaveCriticalSection(&dto->pSync->csEmpresas);
			return TRUE;
		}
	}
	LeaveCriticalSection(&dto->pSync->csEmpresas);
	return FALSE;
}

void comandoUsers(DataTransferObject* dto) {
	system("cls");
	Utilizador uLocal[TAM_MAX_USERS];
	DWORD numUtilizadoresLocal = 0;

	EnterCriticalSection(&dto->pSync->csUtilizadores);
	numUtilizadoresLocal = dto->numUtilizadores;
	CopyMemory(uLocal, dto->utilizadores, numUtilizadoresLocal * sizeof(Utilizador));
	LeaveCriticalSection(&dto->pSync->csUtilizadores);

	_tprintf_s(_T("-- Lista de utilizadores registados --\n"));
	for (DWORD i = 0; i < numUtilizadoresLocal; ++i) {
		_tprintf_s(INFO_USERS, uLocal[i].username, uLocal[i].saldo, (uLocal[i].logado ? _T("ligado") : _T("desligado")));
	}
}

void comandoPause(DWORD numeroSegundos) {
	system("cls");
	// TODO: parar o servidor por um determinado tempo
}

BOOL comandoLoad(DataTransferObject* dto, TCHAR* nomeFicheiro) {
	system("cls");
	DWORD numEmpresasLidas = 0;
	Empresa eLocal[TAM_MAX_EMPRESAS];
	FILE* file;
	errno_t err = _tfopen_s(&file, nomeFicheiro, _T("r"));
	if (err != 0 || file == NULL) {
		_tprintf_s(ERRO_OPEN_FILE);
		if (file != NULL)
			fclose(file);
		return FALSE;
	}
	TCHAR linha[MAX_PATH];
	while (_fgetts(linha, sizeof(linha) / sizeof(linha[0]), file) != NULL) {
		if (numEmpresasLidas >= TAM_MAX_EMPRESAS) {
			break;
		}
		_stscanf_s(
			linha,
			_T("%s %lu %lf"),
			eLocal[numEmpresasLidas].nome, (unsigned)_countof(eLocal[numEmpresasLidas].nome),
			&(eLocal[numEmpresasLidas].quantidadeAcoes),
			&(eLocal[numEmpresasLidas].valorAcao));
		numEmpresasLidas++;
	};
	fclose(file);

	EnterCriticalSection(&dto->pSync->csEmpresas);
	if(TAM_MAX_EMPRESAS - dto->dadosP->numEmpresas > 0) {
		if (numEmpresasLidas <= TAM_MAX_EMPRESAS - dto->dadosP->numEmpresas) {
			CopyMemory(&dto->dadosP->empresas[dto->dadosP->numEmpresas], eLocal, numEmpresasLidas * sizeof(Empresa));
			dto->dadosP->numEmpresas += numEmpresasLidas;
			LeaveCriticalSection(&dto->pSync->csEmpresas);
			return TRUE;
		}
		if (numEmpresasLidas > TAM_MAX_EMPRESAS - dto->dadosP->numEmpresas) {
			CopyMemory(&dto->dadosP->empresas[dto->dadosP->numEmpresas], eLocal, (TAM_MAX_EMPRESAS - dto->dadosP->numEmpresas) * sizeof(Empresa));
			dto->dadosP->numEmpresas = TAM_MAX_EMPRESAS;
			LeaveCriticalSection(&dto->pSync->csEmpresas);
			return TRUE;
		}
	}
	LeaveCriticalSection(&dto->pSync->csEmpresas);
	return FALSE;
}

BOOL comandoClose(DataTransferObject* dto) {
	system("cls");

	// TODO: avisar todos os clientes que o servidor vai fechar
	// TODO: mais qq coisa que seja necessária

	return FALSE;
}

