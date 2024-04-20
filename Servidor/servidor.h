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

// funções das threads
void WINAPI threadConnectionHandler(PVOID);

void WINAPI threadClientHandler(PVOID);

void WINAPI threadReadHandler(PVOID);

void WINAPI threadWriteHandler(PVOID);

void trataMensagemRecebida(DataTransferObject*, pMensagem);

#endif