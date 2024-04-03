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

// comandos do servidor
void comandoAddc(TCHAR*, DWORD, double);

void comandoListc();

void comandoStock(TCHAR*, double);

void comandoUsers();

void comandoPause(DWORD);

void comandoClose();

#endif