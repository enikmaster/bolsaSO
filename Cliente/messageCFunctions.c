#include "cliente.h"

// funções de tratamento de mensagens no cliente
void mensagemRLogin(Mensagem mensagem){
	_tprintf_s(mensagem.sucesso ? INFO_LOGIN : ERRO_LOGIN);
	_tprintf_s(COMANDO);
}

void mensagemRListc(Mensagem mensagem){
	if(mensagem.quantidade != 0) {
		_tprintf_s(_T("Empresas disponíveis:\n"));
		for (DWORD i = 0; i < mensagem.quantidade; ++i) {
			_tprintf_s(INFO_LISTC, mensagem.empresas[i].nome, mensagem.empresas[i].quantidadeAcoes, mensagem.empresas[i].valorAcao);
		}
	} else {
		_tprintf_s(INFO_LISTC_VAZIA);
	}
	_tprintf_s(COMANDO);
}

void mensagemRBuy(Mensagem mensagem){
	if (mensagem.sucesso)
		_tprintf_s(INFO_COMPRA, mensagem.empresa);
	else
		_tprintf_s(ERRO_COMPRA);
	_tprintf_s(COMANDO);
}

void mensagemRSell(){}

void mensagemRBalance(Mensagem mensagem){
	_tprintf_s(INFO_SALDO, mensagem.valor);
	_tprintf_s(COMANDO);
}

void mensagemRWallet(){}

void mensagemAddc(){}

void mensagemStock(){}

void mensagemPause(){}

void mensagemResume(){}

void mensagemLoad(){}

BOOL mensagemCloseC() {
	return FALSE;
}

BOOL mensagemExit() {
	return FALSE;
}