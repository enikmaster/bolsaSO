#pragma once

#include "..\Servidor\constantes.h"
#ifndef BOLSASO_CLIENTE_H
#define BOLSASO_CLIENTE_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

// funções da plataforma
DWORD verificaComando(TCHAR*);

// comandos do cliente
boolean comandoLogin(TCHAR*, TCHAR*);

void comandoListc();

void comandoBuy(TCHAR*, DWORD);

void comandoSell(TCHAR*, DWORD);

void comandoBalance();

void comandoWallet();

void comandoExit();
#endif