#pragma once
#include "..\Servidor\constantes.h"
#include "cliente.h"

// funções da plataforma
DWORD verificaComando(TCHAR* comando) {
	const TCHAR listaComandos[][TAM_COMANDO] = { _T("login"), _T("listc"), _T("buy"), _T("sell"), _T("balance"), _T("wallet"), _T("exit") };

	// comando sem argumentos
	if (_tcschr(comando, _T(' ')) == NULL) {
		for (DWORD i = 0; i < sizeof(listaComandos) / sizeof(listaComandos[0]); ++i) {
			if (_tcscmp(comando, listaComandos[i]) == 0) {
				return ++i;
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

		for(DWORD i = 0; i < sizeof(listaComandos) / sizeof(listaComandos[0]); ++i) {
			if (_tcscmp(comandoTemp, listaComandos[i]) == 0) {
				return ++i;
			}
		}
	}
	return 0;
}

// comandos do cliente
BOOL comandoLogin(HANDLE* hPipe, TCHAR* username, TCHAR* password) {
	Mensagem mensagem = { 0 };
	mensagem.TipoM = TMensagem_LOGIN;
	memcpy(mensagem.nome, username, _tcslen(username) * sizeof(TCHAR));
	memcpy(mensagem.password, password, _tcslen(password) * sizeof(TCHAR));
	
	return enviarMensagem(hPipe, mensagem);
}

void comandoListc() {
	// TODO: fazer a lógica
	//	envia uma mensagem para o servidor a pedir a lista de empresas
}

void comandoBuy(TCHAR* empresa, DWORD numAcoes) {
	// TODO: fazer a lógica
	//	envia uma mensagem para o servidor a pedir a compra de ações
}

void comandoSell(TCHAR* empresa, DWORD numAcoes) {
	// TODO: fazer a lógica
	//	envia uma mensagem para o servidor a pedir a venda de ações
}

void comandoBalance() {
	// TODO: fazer a lógica
	//	envia uma mensagem para o servidor a pedir o saldo
}

void comandoWallet() {
	// TODO: fazer a lógica
	//	envia uma mensagem para o servidor a pedir a carteira de ações
}

void comandoExit() {
	// TODO: fazer a lógica
	//	envia uma mensagem para o servidor a pedir o logout
}

BOOL enviarMensagem(HANDLE* hPipe, Mensagem mensagem) {
	DWORD bytesEscritos;
	OVERLAPPED ov = { 0 };
	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (ov.hEvent == NULL) {
		_tprintf_s(ERRO_CREATE_EVENT);
		return FALSE;
	}
	BOOL fSuccess = WriteFile(*hPipe, &mensagem, sizeof(Mensagem), &bytesEscritos, &ov);
	//WaitForSingleObject(*hPipe, INFINITE);
	BOOL ovResult = GetOverlappedResult(*hPipe, &ov, &bytesEscritos, FALSE);
	if (!ovResult || bytesEscritos == 0) {
		_tprintf_s(ERRO_ESCRITA_MSG);
		return FALSE;
	}
	return TRUE;
}