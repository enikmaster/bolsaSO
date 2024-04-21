#pragma once

#ifndef BOLSASO_SERVIDOR_H
#define BOLSASO_SERVIDOR_H

#include "constantes.h"
#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>


// funções da plataforma
DWORD lerCriarRegistryKey();

DWORD lerUtilizadores(DataTransferObject*, const TCHAR*);

BOOL inicializarDTO(DataTransferObject*);

void terminarDTO(DataTransferObject*);

DWORD verificaComando(const TCHAR*);

BOOL instanciarNamedPipe(DataTransferObject*);

// comandos do servidor
BOOL comandoAddc(DataTransferObject*, const TCHAR*, const DWORD, const double);

void comandoListc(DataTransferObject*);

BOOL comandoStock(DataTransferObject*,const TCHAR*, const double);

void comandoUsers(DataTransferObject*);

void comandoPause(DWORD);

BOOL comandoLoad(DataTransferObject*, TCHAR*);

void comandoClose();

// funções de tratamento de mensagens
void mensagemLogin(ThreadData*);

void mensagemListc(DataTransferObject*, DWORD);

void mensagemBuy();

void mensagemSell();

void mensagemBalance();

void mensagemWallet();

void mensagemExit();

void mensagemAddc();

void mensagemStock();

void mensagemPause();

void mensagemResume();

void mensagemLoad();

void mensagemClose();

// funções das threads
void WINAPI threadConnectionHandler(PVOID);

void WINAPI threadClientHandler(PVOID);

void WINAPI threadReadHandler(PVOID);

//void WINAPI threadWriteHandler(PVOID);

void WINAPI threadMessageHandler(PVOID);

#endif