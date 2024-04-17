#include "constantes.h"
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
		if (i >= MAX_USERS)
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
	TCHAR nomeKey[TAM];
	DWORD tamanho;
	DWORD nClientes = 5;
	DWORD res;
	DWORD limiteClientes;

	_stprintf_s(nomeKey, TAM, REGISTRY_KEY_NCLIENTES);

	res = RegOpenKeyEx(HKEY_CURRENT_USER, nomeKey, 0, KEY_READ, &hKey);
	if (res == ERROR_SUCCESS) {
		RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE)&limiteClientes, &tamanho);
		_tprintf_s(_T("O valor lido para NCLIENTES foi: %d\n"), limiteClientes);
	}
	else {
		res = RegCreateKeyEx(HKEY_CURRENT_USER, nomeKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &res);
		if (res == ERROR_SUCCESS) {
			RegSetValueEx(hKey, NULL, 0, REG_DWORD, (const BYTE*)&nClientes, sizeof(DWORD));
			_tprintf_s(_T("O valor escrito em NCLIENTES foi: %d\n"), nClientes);
			limiteClientes = nClientes;
		}
		else {
			_tprintf_s(ERRO_CREATE_KEY_NCLIENTES);
		}
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
		SHM_NAME);
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
		SEM_NAME);
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

	// criar o critical section do bolsa
	InitializeCriticalSection(&dto->cs);

	dto->sharedData = (DadosPartilhados*)dto->pView;
	dto->sharedData->numEmpresas = 0;

	return TRUE;
}

void terminarDTO(DataTransferObject* dto) {
	// garantir que saiu da secção crítica
	LeaveCriticalSection(&dto->cs);
	// apagar o critical section
	DeleteCriticalSection(&dto->cs);
	// libertar o semáforo
	ReleaseSemaphore(dto->hSemBolsa, 1, NULL);
	// desmapear a memória partilhada
	UnmapViewOfFile(dto->pView);
	// fechar os handles por ordem inversa da sua criação
	CloseHandle(dto->hMap);
	CloseHandle(dto->hSemBolsa);
	CloseHandle(dto->hMtxBolsa);
}

// comandos do servidor
BOOL comandoAddc(DadosPartilhados* dadosP, const TCHAR* nomeEmpresa, const DWORD numeroAcoes, const double precoAcao, CRITICAL_SECTION cs) {
	
	EnterCriticalSection(&cs);
	DWORD numEmpresas = &(dadosP->numEmpresas);
	if (numEmpresas >= MAX_EMPRESAS) {
		LeaveCriticalSection(&cs);
		return FALSE;
	}
	_tcscpy_s(dadosP->empresas[numEmpresas].nome, TAM_NOME, nomeEmpresa);
	dadosP->empresas[numEmpresas].quantidadeAcoes = numeroAcoes;
	dadosP->empresas[numEmpresas++].valorAcao = precoAcao;
	dadosP->numEmpresas = numEmpresas;
	LeaveCriticalSection(&cs);

	return TRUE;
}

void comandoListc(DadosPartilhados*dadosP, CRITICAL_SECTION cs) {
	Empresa eLocal[MAX_EMPRESAS];
	DWORD numEmpresasLocal = 0;

	EnterCriticalSection(&cs);
	numEmpresasLocal = dadosP->numEmpresas;
	memcpy(eLocal, dadosP->empresas, numEmpresasLocal * sizeof(Empresa));
	LeaveCriticalSection(&cs);

	_tprintf_s(_T("Lista de empresas:\n"));
	for (DWORD i = 0; i < numEmpresasLocal; ++i) {
		_tprintf_s(_T("Nome: %s \tAções disponíveis: %lu \tPreço atual por ação: %lf\n"), eLocal[i].nome, eLocal[i].quantidadeAcoes, eLocal[i].valorAcao);
	}
}

BOOL comandoStock(DadosPartilhados* dadosP, const TCHAR* nomeEmpresa, const double valorAcao, CRITICAL_SECTION cs) {
	EnterCriticalSection(&cs);
	for (DWORD i = 0; i < &(dadosP->numEmpresas); ++i) {
		if (_tcscmp(nomeEmpresa, dadosP->empresas[i].nome) == 0) {
			dadosP->empresas[i].valorAcao = valorAcao;
			LeaveCriticalSection(&cs);
			return TRUE;
		}
	}
	LeaveCriticalSection(&cs);
	return FALSE;
}

void comandoUsers(const DWORD numUtilizadores, const Utilizador* utilizadores) {
	_tprintf_s(_T("Lista de utilizadores registados:\n"));
	for (DWORD i = 0; i < numUtilizadores; ++i) {
		_tprintf_s(_T("Username: %s \tSaldo: %lf \tEstado: %s\n"), utilizadores[i].username, utilizadores[i].saldo, (utilizadores[i].logado ? _T("ligado") : _T("desligado")));
	}
}

void comandoPause(DWORD numeroSegundos) {
	// TODO: parar o servidor por um determinado tempo
}

BOOL comandoLoad(DadosPartilhados* dadosP, TCHAR* nomeFicheiro, CRITICAL_SECTION cs) {
	DWORD numEmpresasLidas = 0;
	Empresa eLocal[MAX_EMPRESAS];
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
		if (numEmpresasLidas >= MAX_EMPRESAS) {
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

	EnterCriticalSection(&cs);
	if(MAX_EMPRESAS - dadosP->numEmpresas > 0) {
		if (numEmpresasLidas <= MAX_EMPRESAS - dadosP->numEmpresas) {
			memcpy(&dadosP->empresas[dadosP->numEmpresas], eLocal, numEmpresasLidas * sizeof(Empresa));
			dadosP->numEmpresas += numEmpresasLidas;
			LeaveCriticalSection(&cs);
			return TRUE;
		}
		if (numEmpresasLidas > MAX_EMPRESAS - dadosP->numEmpresas) {
			memcpy(&dadosP->empresas[dadosP->numEmpresas], eLocal, (MAX_EMPRESAS - dadosP->numEmpresas) * sizeof(Empresa));
			dadosP->numEmpresas = MAX_EMPRESAS;
			LeaveCriticalSection(&cs);
			return TRUE;
		}
	}
	LeaveCriticalSection(&cs);
	return FALSE;
}

void comandoClose() {
	// TODO: avisar todos os clientes que o servidor vai fechar
	// TODO: mais qq coisa que seja necessária
}

