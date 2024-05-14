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
void comandoLogin(HANDLE* hPipe, TCHAR* username, TCHAR* password) {
	Mensagem mensagem = { 0 };
	mensagem.TipoM = TMensagem_LOGIN;
	memcpy(mensagem.nome, username, (_tcslen(username) + 1) * sizeof(TCHAR));
	memcpy(mensagem.password, password, (_tcslen(password) + 1) * sizeof(TCHAR));
	
	enviarMensagem(hPipe, mensagem);
}

void comandoListc(HANDLE* hPipe) {
	Mensagem mensagem = { 0 };
	mensagem.TipoM = TMensagem_LISTC;
	enviarMensagem(hPipe, mensagem);
}

void comandoBuy(HANDLE* hPipe, TCHAR* nome, TCHAR* empresa, DWORD numAcoes) {
	Mensagem mensagem = { 0 };
	mensagem.TipoM = TMensagem_BUY;
	memcpy(mensagem.nome, nome, (_tcslen(nome) + 1) * sizeof(TCHAR));
	memcpy(mensagem.empresa, empresa, (_tcslen(empresa) + 1) * sizeof(TCHAR));
	mensagem.quantidade = numAcoes;
	enviarMensagem(hPipe, mensagem);
}

void comandoSell(HANDLE* hPipe, TCHAR* nome, TCHAR* empresa, DWORD numAcoes) {
	Mensagem mensagem = { 0 };
	mensagem.TipoM = TMensagem_SELL;
	memcpy(mensagem.nome, nome, (_tcslen(nome) + 1) * sizeof(TCHAR));
	memcpy(mensagem.empresa, empresa, (_tcslen(empresa) + 1) * sizeof(TCHAR));
	mensagem.quantidade = numAcoes;
	enviarMensagem(hPipe, mensagem);
}

void comandoBalance(HANDLE* hPipe, TCHAR* username) {
	Mensagem mensagem = { 0 };
	mensagem.TipoM = TMensagem_BALANCE;
	memcpy(mensagem.nome, username, (_tcslen(username) + 1) * sizeof(TCHAR));
	enviarMensagem(hPipe, mensagem);
}

void comandoWallet(HANDLE* hPipe, TCHAR* username) {
	Mensagem mensagem = { 0 };
	mensagem.TipoM = TMensagem_WALLET;
	memcpy(mensagem.nome, username, (_tcslen(username) + 1)* sizeof(TCHAR));
	enviarMensagem(hPipe, mensagem);
}

BOOL comandoExit(HANDLE* hPipe, TCHAR* username, BOOL logado) {
	Mensagem mensagem = { 0 };
	mensagem.TipoM = TMensagem_EXIT;
	if (logado) 
		memcpy(mensagem.nome, username, (_tcslen(username) + 1) * sizeof(TCHAR)); 
	else 
		mensagem.nome[0] = '\0';
	return !enviarMensagem(hPipe, mensagem);
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
	BOOL ovResult = GetOverlappedResult(*hPipe, &ov, &bytesEscritos, FALSE);
	if (!ovResult || bytesEscritos == 0) {
		_tprintf_s(ERRO_ESCRITA_MSG);
		return FALSE;
	}
	return TRUE;
}