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
DWORD verificaComando(const TCHAR*);

DWORD lerUtilizadores(Utilizador*, const TCHAR*);

DWORD lerCriarRegistryKey();

BOOL inicializarDTO(DataTransferObject*);

void terminarDTO(DataTransferObject*);

// comandos do servidor
BOOL comandoAddc(DadosPartilhados*, const TCHAR*, const DWORD, const double, CRITICAL_SECTION);

void comandoListc(DadosPartilhados*, CRITICAL_SECTION);

BOOL comandoStock(DadosPartilhados*,const TCHAR*, const double, CRITICAL_SECTION);

void comandoUsers(const DWORD, const Utilizador*);

void comandoPause(DWORD);

BOOL comandoLoad(DadosPartilhados*, TCHAR*, CRITICAL_SECTION);

void comandoClose();

// funções das threads
void WINAPI threadComandos(PVOID);

void WINAPI threadReceberMensagens(PVOID);

void WINAPI threadEnviarMensagens(PVOID);



#endif