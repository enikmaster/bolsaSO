#pragma once

#ifndef BOLSASO_CLIENTE_H
#define BOLSASO_CLIENTE_H

#include "..\Servidor\constantes.h"

// funções da plataforma
DWORD verificaComando(TCHAR*);

// comandos do cliente
BOOL comandoLogin(HANDLE*, TCHAR*, TCHAR*);

void comandoListc();

void comandoBuy(TCHAR*, DWORD);

void comandoSell(TCHAR*, DWORD);

void comandoBalance();

void comandoWallet();

void comandoExit();

BOOL enviarMensagem(HANDLE*, Mensagem);

// funções de threads
void WINAPI threadComandosClienteHandler(PVOID);

void messageHandlerCliente(Mensagem);

void PrintLastError(TCHAR*, DWORD);

// funções de tratamento de mensagens no cliente
void mensagemRLogin(Mensagem);

void mensagemRListc();

void mensagemRBuy();

void mensagemRSell();

void mensagemRBalance();

void mensagemRWallet();

void mensagemAddc();

void mensagemStock();

void mensagemPause();

void mensagemResume();

void mensagemLoad();

void mensagemCloseC();

void mensagemExit();

#endif