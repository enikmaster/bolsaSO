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

void mensagemRSell(Mensagem mensagem){
	if (mensagem.sucesso)
		_tprintf_s(INFO_VENDA, mensagem.empresa);
	else
		_tprintf_s(ERRO_VENDA);
	_tprintf_s(COMANDO);
}

void mensagemRBalance(Mensagem mensagem){
	_tprintf_s(INFO_SALDO, mensagem.valor);
	_tprintf_s(COMANDO);
}

void mensagemRWallet(Mensagem mensagem){
	if(mensagem.quantidade != 0) {
		_tprintf_s(_T("-- Carteira de Ações:\n"));
		for (DWORD i = 0; i < mensagem.quantidade; ++i) {
			_tprintf_s(INFO_WALLET, mensagem.carteiraAcoes[i].nomeEmpresa, mensagem.carteiraAcoes[i].quantidadeAcoes);
		}
	} else {
		_tprintf_s(INFO_WALLET_VAZIA);
	}
	_tprintf_s(COMANDO);
}

void mensagemAddc(Mensagem mensagem){
	_tprintf_s(INFO_ADDC, mensagem.empresa);
	_tprintf_s(COMANDO);
}

void mensagemStock(Mensagem mensagem){}

void mensagemPause(Mensagem mensagem){}

void mensagemResume(Mensagem mensagem){}

void mensagemLoad(Mensagem mensagem){}

BOOL mensagemCloseC(Mensagem mensagem) {
	return FALSE;
}

BOOL mensagemExit(Mensagem mensagem) {
	return FALSE;
}