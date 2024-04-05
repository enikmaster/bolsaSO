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

void lerUtilizadores(Utilizador[], DWORD, const TCHAR*);

// comandos do servidor
DWORD comandoAddc(TCHAR*, DWORD, double, Empresa*, DWORD);

void comandoListc();

void comandoStock(TCHAR*, double);

void comandoUsers(DWORD, Utilizador* );

void comandoPause(DWORD);

void comandoClose();

#endif