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

int comandoLoad(DataTransferObject*, TCHAR*);

BOOL comandoClose(ThreadData*);

// funções de tratamento de mensagens
void mensagemLogin(ThreadData*, Mensagem);

void mensagemListc(ThreadData*);

void mensagemBuy(ThreadData*, Mensagem);

void mensagemSell(ThreadData*, Mensagem);

void mensagemBalance(ThreadData*, Mensagem);

void mensagemWallet(ThreadData*, Mensagem);

void mensagemExit();

void mensagemAddc(ThreadData*, TCHAR*);

void mensagemStock(ThreadData*, TCHAR*, double);

void mensagemPause();

void mensagemResume();

void mensagemLoad(ThreadData*, int);

void mensagemClose(ThreadData*);

BOOL enviarMensagem(HANDLE, Mensagem, CRITICAL_SECTION);

// funções das threads
void WINAPI threadComandosAdminHandler(PVOID);

void WINAPI threadClientHandler(PVOID);

void WINAPI threadBoardHandler(PVOID);

void PrintLastError(TCHAR*, DWORD);

#endif