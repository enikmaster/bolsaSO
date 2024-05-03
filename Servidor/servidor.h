#pragma once

#ifndef BOLSASO_SERVIDOR_H
#define BOLSASO_SERVIDOR_H

#include "constantes.h"

// funções da plataforma
DWORD lerCriarRegistryKey();

DWORD lerUtilizadores(DataTransferObject*, const TCHAR*);

BOOL inicializarDTO(DataTransferObject*);

void terminarDTO(DataTransferObject*);

DWORD verificaComando(const TCHAR*);

// comandos do servidor
BOOL comandoAddc(DataTransferObject*, const TCHAR*, const DWORD, const double);

void comandoListc(DataTransferObject*);

BOOL comandoStock(DataTransferObject*,const TCHAR*, const double);

void comandoUsers(DataTransferObject*);

void comandoPause(DWORD);

BOOL comandoLoad(DataTransferObject*, TCHAR*);

BOOL comandoClose(DataTransferObject*);

// funções de tratamento de mensagens
void mensagemLogin(ThreadData*, Mensagem);

void mensagemListc(ThreadData*);

void mensagemBuy(ThreadData*);

void mensagemSell();

void mensagemBalance(ThreadData*);

void mensagemWallet();

void mensagemExit();

void mensagemAddc();

void mensagemStock();

void mensagemPause();

void mensagemResume();

void mensagemLoad();

void mensagemClose();

BOOL enviarMensagem(HANDLE, Mensagem);

// funções das threads
void WINAPI threadComandosAdminHandler(PVOID);

void WINAPI threadClientHandler(PVOID);

void WINAPI threadBoardHandler(PVOID);

void messageHandler(PVOID, Mensagem);

BOOL ConnectToNewClient(HANDLE, LPOVERLAPPED);

void PrintLastError(TCHAR*, DWORD);

#endif