#pragma once

#ifndef BOLSASO_BOARD_H
#define BOLSASO_BOARD_H

#include "..\Servidor\constantes.h"

int CompareEmpresa(const void*, const void* );

//atualizar
void OrganizarEExibirEmpresas(DadosPartilhados*, DWORD);
void ExibirUltimaTransacao(DadosPartilhados*);

#endif