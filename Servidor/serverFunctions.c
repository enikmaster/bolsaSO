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

DWORD lerUtilizadores(Utilizador* utilizadores, const TCHAR* nomeFicheiro) {
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
			utilizadores[i].username, (unsigned)_countof(utilizadores[i].username),
			utilizadores[i].password, (unsigned)_countof(utilizadores[i].password),
			&(utilizadores[i].saldo));
		utilizadores[i++].logado = FALSE;
	};
	fclose(file);
	return i;
}

// lê o valor de NCLIENTES do registo, se nexistir cria a key
DWORD lerCriarRegistryKey() {
	HKEY hKey;
	TCHAR nomeKey[TAM_REGISTRY];
	DWORD tamanho = 0;
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

	// criar o semáforo da bolsa
	dto->hSemBolsa = CreateSemaphore(
		NULL,
		0,
		1,
		NOME_SEMAFORO);
	if (dto->hSemBolsa == NULL) {
		_tprintf_s(ERRO_CREATE_SEM);
		UnmapViewOfFile(dto->pView);
		CloseHandle(dto->hMap);
		return FALSE;
	}

	// criar o mutex da bolsa
	dto->hMtxBolsa = CreateMutex(
		NULL,
		FALSE,
		NULL);
	if (dto->hMtxBolsa == NULL) {
		_tprintf_s(ERRO_CREATE_MUTEX);
		UnmapViewOfFile(dto->pView);
		CloseHandle(dto->hMap);
		CloseHandle(dto->hSemBolsa);
		return FALSE;
	}

	// criar o critical section dos dados partilhados
	InitializeCriticalSection(&dto->csDados);

	// criar o critical section dos utilizadores
	InitializeCriticalSection(&dto->csUtilizadores);

	// criar o critical section das threads
	InitializeCriticalSection(&dto->csThreads);

	dto->sharedData = (DadosPartilhados*)dto->pView;
	dto->sharedData->numEmpresas = 0;

	return TRUE;
}

void terminarDTO(DataTransferObject* dto) {
	// garantir que saiu das secções críticas
	LeaveCriticalSection(&dto->csDados);
	LeaveCriticalSection(&dto->csUtilizadores);
	LeaveCriticalSection(&dto->csThreads);
	// apagar os CriticalSections
	DeleteCriticalSection(&dto->csDados);
	DeleteCriticalSection(&dto->csUtilizadores);
	DeleteCriticalSection(&dto->csThreads);
	// libertar o semáforo
	ReleaseSemaphore(dto->hSemBolsa, 1, NULL);
	// desmapear a memória partilhada
	UnmapViewOfFile(dto->pView);
	// fechar os handles por ordem inversa da sua criação
	CloseHandle(dto->hMap);
	CloseHandle(dto->hSemBolsa);
	CloseHandle(dto->hMtxBolsa);
	for (DWORD i = dto->numThreads; i > 0; --i)
		CloseHandle(dto->hThreads[i - 1]);
	
}

BOOL instanciarNamedPipe(DataTransferObject* dto) {
	// criar o named pipe
	if(dto->numPipes >= dto->limiteClientes) {
		_tprintf_s(ERRO_MAX_CLIENTES);
		return FALSE;
	}
	dto->hPipes[dto->numPipes] = CreateNamedPipe(
		NOME_NAMED_PIPE,
		PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		TAM_MAX_USERS,
		sizeof(Mensagem) * 2,
		sizeof(Mensagem) * 2,
		0,
		NULL);
	if (dto->hPipes[dto->numPipes] == INVALID_HANDLE_VALUE) {
		_tprintf_s(ERRO_CREATE_NAMED_PIPE);
		return FALSE;
	}
	dto->numPipes++;
	return TRUE;
}

// comandos do servidor
BOOL comandoAddc(DadosPartilhados* dadosP, const TCHAR* nomeEmpresa, const DWORD numeroAcoes, const double precoAcao, CRITICAL_SECTION csDados) {
	
	EnterCriticalSection(&csDados);
	DWORD numEmpresas = dadosP->numEmpresas;
	if (numEmpresas >= TAM_MAX_EMPRESAS) {
		LeaveCriticalSection(&csDados);
		return FALSE;
	}
	_tcscpy_s(dadosP->empresas[numEmpresas].nome, TAM_NOME, nomeEmpresa);
	dadosP->empresas[numEmpresas].quantidadeAcoes = numeroAcoes;
	dadosP->empresas[numEmpresas++].valorAcao = precoAcao;
	dadosP->numEmpresas = numEmpresas;
	LeaveCriticalSection(&csDados);

	return TRUE;
}

void comandoListc(const DadosPartilhados*dadosP, CRITICAL_SECTION csDados) {
	Empresa eLocal[TAM_MAX_EMPRESAS];
	DWORD numEmpresasLocal = 0;

	EnterCriticalSection(&csDados);
	numEmpresasLocal = dadosP->numEmpresas;
	memcpy(eLocal, dadosP->empresas, numEmpresasLocal * sizeof(Empresa));
	LeaveCriticalSection(&csDados);

	_tprintf_s(_T(" -- Lista de empresas --\n"));
	for (DWORD i = 0; i < numEmpresasLocal; ++i) {
		_tprintf_s(INFO_LISTC, eLocal[i].nome, eLocal[i].quantidadeAcoes, eLocal[i].valorAcao);
	}
}

BOOL comandoStock(DadosPartilhados* dadosP, const TCHAR* nomeEmpresa, const double valorAcao, CRITICAL_SECTION csDados) {
	EnterCriticalSection(&csDados);
	for (DWORD i = 0; i < &(dadosP->numEmpresas); ++i) {
		if (_tcscmp(nomeEmpresa, dadosP->empresas[i].nome) == 0) {
			dadosP->empresas[i].valorAcao = valorAcao;
			LeaveCriticalSection(&csDados);
			return TRUE;
		}
	}
	LeaveCriticalSection(&csDados);
	return FALSE;
}

void comandoUsers(const DWORD numUtilizadores, const Utilizador* utilizadores) {
	_tprintf_s(_T("-- Lista de utilizadores registados --\n"));
	for (DWORD i = 0; i < numUtilizadores; ++i) {
		_tprintf_s(INFO_USERS, utilizadores[i].username, utilizadores[i].saldo, (utilizadores[i].logado ? _T("ligado") : _T("desligado")));
	}
}

void comandoPause(DWORD numeroSegundos) {
	// TODO: parar o servidor por um determinado tempo
}

BOOL comandoLoad(DadosPartilhados* dadosP, TCHAR* nomeFicheiro, CRITICAL_SECTION csDados) {
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

	EnterCriticalSection(&csDados);
	if(TAM_MAX_EMPRESAS - dadosP->numEmpresas > 0) {
		if (numEmpresasLidas <= TAM_MAX_EMPRESAS - dadosP->numEmpresas) {
			memcpy(&dadosP->empresas[dadosP->numEmpresas], eLocal, numEmpresasLidas * sizeof(Empresa));
			dadosP->numEmpresas += numEmpresasLidas;
			LeaveCriticalSection(&csDados);
			return TRUE;
		}
		if (numEmpresasLidas > TAM_MAX_EMPRESAS - dadosP->numEmpresas) {
			memcpy(&dadosP->empresas[dadosP->numEmpresas], eLocal, (TAM_MAX_EMPRESAS - dadosP->numEmpresas) * sizeof(Empresa));
			dadosP->numEmpresas = TAM_MAX_EMPRESAS;
			LeaveCriticalSection(&csDados);
			return TRUE;
		}
	}
	LeaveCriticalSection(&csDados);
	return FALSE;
}

void comandoClose() {
	// TODO: avisar todos os clientes que o servidor vai fechar
	// TODO: mais qq coisa que seja necessária
}

