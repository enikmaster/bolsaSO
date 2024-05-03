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
			_tprintf_s(_T("%s\n"), mensagem.empresas[i].nome);
		}
	} else {
		_tprintf_s(INFO_LISTC_VAZIA);
	}
	_tprintf_s(COMANDO);
}

void mensagemRBuy(){}

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

void mensagemCloseC(){}

void mensagemExit(){}