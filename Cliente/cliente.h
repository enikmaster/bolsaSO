#pragma once

#ifndef BOLSASO_CLIENTE_H
#define BOLSASO_CLIENTE_H

#include "..\Servidor\constantes.h"

// funções da plataforma
DWORD verificaComando(TCHAR*);

// comandos do cliente
BOOL comandoLogin(TCHAR*, TCHAR*);

void comandoListc();

void comandoBuy(TCHAR*, DWORD);

void comandoSell(TCHAR*, DWORD);

void comandoBalance();

void comandoWallet();

void comandoExit();

// funções de threads
void WINAPI threadConnectionHandlerCliente(LPVOID);

void WINAPI threadMessageHandlerCliente(LPVOID);

// funções de tratamento de mensagens no cliente
void mensagemRLogin();

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

void mensagemClose();

void mensagemExit();

#endif