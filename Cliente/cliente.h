#pragma once

#ifndef BOLSASO_CLIENTE_H
#define BOLSASO_CLIENTE_H

#include "..\Servidor\constantes.h"

// funções da plataforma
DWORD verificaComando(TCHAR*);

// comandos do cliente
BOOL comandoLogin(HANDLE*, TCHAR*, TCHAR*);

void comandoListc(HANDLE*);

void comandoBuy(HANDLE*, TCHAR*, TCHAR*, DWORD);

void comandoSell(TCHAR*, DWORD);

void comandoBalance(HANDLE*, TCHAR*);

void comandoWallet();

void comandoExit();

BOOL enviarMensagem(HANDLE*, Mensagem);

// funções de threads
void WINAPI threadComandosClienteHandler(PVOID);

void messageHandlerCliente(Mensagem);

void PrintLastError(TCHAR*, DWORD);

// funções de tratamento de mensagens no cliente
void mensagemRLogin(Mensagem);

void mensagemRListc(Mensagem);

void mensagemRBuy(Mensagem);

void mensagemRSell();

void mensagemRBalance(Mensagem);

void mensagemRWallet();

void mensagemAddc();

void mensagemStock();

void mensagemPause();

void mensagemResume();

void mensagemLoad();

BOOL mensagemCloseC();

BOOL mensagemExit();

#endif