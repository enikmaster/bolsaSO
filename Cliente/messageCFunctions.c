#include "cliente.h"

// funções de tratamento de mensagens no cliente
void mensagemRLogin(Mensagem mensagem){
	system("cls");
	_tprintf_s(mensagem.sucesso ? INFO_LOGIN : ERRO_LOGIN);
	_tprintf_s(COMANDO);
}

void mensagemRListc(){}

void mensagemRBuy(){}

void mensagemRSell(){}

void mensagemRBalance(){}

void mensagemRWallet(){}

void mensagemAddc(){}

void mensagemStock(){}

void mensagemPause(){}

void mensagemResume(){}

void mensagemLoad(){}

void mensagemCloseC(){}

void mensagemExit(){}