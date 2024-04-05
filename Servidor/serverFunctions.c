#include "constantes.h"
#include "servidor.h"

// funções da plataforma
DWORD verificaComando(TCHAR* comando) {
	const TCHAR listaComandos[][TAM_COMANDO] = { _T("addc"), _T("listc"), _T("stock"), _T("users"), _T("pause"), _T("close")};

	// comando sem argumentos (listc, users e close)
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
		_stscanf_s(
			comando,
			_T("%s %s"),
			comandoTemp, TAM_COMANDO,
			argumentos, TAM_COMANDO);

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

void lerUtilizadores(Utilizador utilizadores[], DWORD numUtilizadores, const TCHAR* nomeFicheiro) {
	FILE* file;
	errno_t err = _tfopen_s(&file, nomeFicheiro, _T("r"));
	if (err != 0 || file == NULL) {
		_tprintf_s(ERRO_OPEN_FILE);
		if (file != NULL)
			fclose(file);
		ExitProcess(-1);
	}
	TCHAR linha[MAX_PATH];
	while (_fgetts(linha, sizeof(linha) / sizeof(linha[0]), file) != NULL) {
		if (numUtilizadores >= MAX_USERS)
			break;
		_stscanf_s(
			linha,
			_T("%s %s %lf"),
			utilizadores[numUtilizadores].username, (unsigned)_countof(utilizadores[numUtilizadores].username),
			utilizadores[numUtilizadores].password, (unsigned)_countof(utilizadores[numUtilizadores].password),
			&(utilizadores[numUtilizadores].saldo));
		utilizadores[numUtilizadores].ligado = FALSE;
		numUtilizadores++;
	};
	fclose(file);
}

// comandos do servidor
DWORD comandoAddc(TCHAR* nomeEmpresa, DWORD numeroAcoes, double precoAcao, Empresa* empresas, DWORD numEmpresas) {
	if (numEmpresas >= MAX_EMPRESAS)
		return -1;
	_tcscpy_s(empresas[numEmpresas].nome, TAM_NOME, nomeEmpresa);
	empresas[numEmpresas].quantidadeAcoes = numeroAcoes;
	empresas[numEmpresas].valorAcao = precoAcao;
	return numEmpresas;
}

void comandoListc() {
	// TODO: listar as empresas
}

void comandoStock(TCHAR* nomeEmpresa, double precoAcao) {
	// TODO: atualizar o preço da ação de uma empresa
}

void comandoUsers(DWORD numUtilizadores, Utilizador* utilizadores) {
	_tprintf_s(_T("Lista de utilizadores registados:\n"));
	for (DWORD i = 0; i < numUtilizadores; ++i) {
		_tprintf_s(_T("Username: %s \tSaldo: %lf \tEstado: %s\n"), utilizadores[i].username, utilizadores[i].saldo, (utilizadores[i].ligado == TRUE ? _T("ligado") : _T("desligado")));
	}
}

void comandoPause(DWORD numeroSegundos) {
	// TODO: parar o servidor por um determinado tempo
}

void comandoClose() {
	// TODO: avisar todos os clientes que o servidor vai fechar
	// TODO: mais qq coisa que seja necessária
}