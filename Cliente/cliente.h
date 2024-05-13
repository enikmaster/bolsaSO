#pragma once

#ifndef BOLSASO_CLIENTE_H
#define BOLSASO_CLIENTE_H

#include "..\Servidor\constantes.h"

// fun��es da plataforma
DWORD verificaComando(TCHAR*);

// comandos do cliente
BOOL comandoLogin(HANDLE*, TCHAR*, TCHAR*);

void comandoListc(HANDLE*);

void comandoBuy(HANDLE*, TCHAR*, TCHAR*, DWORD);

void comandoSell(HANDLE*, TCHAR*, TCHAR*, DWORD);

void comandoBalance(HANDLE*, TCHAR*);

void comandoWallet(HANDLE*, TCHAR*);

BOOL comandoExit(HANDLE*, TCHAR*, BOOL);

BOOL enviarMensagem(HANDLE*, Mensagem);

// fun��es de threads
void WINAPI threadComandosClienteHandler(PVOID);

void PrintLastError(TCHAR*, DWORD);

// fun��es de tratamento de mensagens no cliente
void mensagemRLogin(Mensagem);

void mensagemRListc(Mensagem);

void mensagemRBuy(Mensagem);

void mensagemRSell(Mensagem);

void mensagemRBalance(Mensagem);

void mensagemRWallet(Mensagem);

void mensagemAddc(Mensagem);

void mensagemStock(Mensagem);

void mensagemPause(Mensagem);

void mensagemResume(Mensagem);

void mensagemLoad(Mensagem);

BOOL mensagemCloseC(Mensagem);

BOOL mensagemExit(Mensagem);

#endif