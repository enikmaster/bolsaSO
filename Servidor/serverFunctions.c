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

// comandos do servidor
void comandoAddc(TCHAR* nomeEmpresa, DWORD numeroAcoes, double precoAcao) {
	// TODO: adicionar a empresa ao array de empresas
	//  para isso é necessário passar o ponteiro para o array de empresas
	
	// lógica para adicionar a empresa
	// em caso de sucesso
	_tprintf_s(_T("Empresa: %s\nN_Ações: %lu\nPreço: %lf\n"), nomeEmpresa, numeroAcoes, precoAcao);
	// em caso de erro
	_tprintf_s(ERRO_ADDC);
}

void comandoListc() {
	// TODO: listar as empresas
}

void comandoStock(TCHAR* nomeEmpresa, double precoAcao) {
	// TODO: atualizar o preço da ação de uma empresa
}

void comandoUsers() {
	// TODO: listar os utilizadores
}

void comandoPause(DWORD numeroSegundos) {
	// TODO: parar o servidor por um determinado tempo
}

void comandoClose() {
	// TODO: avisar todos os clientes que o servidor vai fechar
	// TODO: mais qq coisa que seja necessária
}