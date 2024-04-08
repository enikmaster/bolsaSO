#include "constantes.h"

#pragma once
#ifndef BOLSASO_SERVIDOR_H
#define BOLSASO_SERVIDOR_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>


// funções da plataforma
DWORD verificaComando(TCHAR*);

DWORD lerUtilizadores(Utilizador*, const TCHAR*);

// comandos do servidor
DWORD comandoAddc(TCHAR*, DWORD, double, Empresa*, DWORD);

void comandoListc(DWORD, Empresa*);

boolean comandoStock(TCHAR*, double, Empresa*, DWORD);

void comandoUsers(DWORD, Utilizador*);

void comandoPause(DWORD);

DWORD comandoLoad(Empresa*, DWORD, TCHAR*);

void comandoClose();

#endif