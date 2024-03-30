#pragma once
#ifndef BOLSASO_SERVIDOR_H
#define BOLSASO_SERVIDOR_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>


// funções da plataforma
DWORD verificaComando(TCHAR*);

// comandos do servidor
void addc(TCHAR*, DWORD, double);

void listc();

void stock(TCHAR*, double);

void users();

void pause(DWORD);

void close();

#endif