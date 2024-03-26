#pragma once
typedef struct Empresa Empresa, pEmpresa;
struct Empresa {
	char nome[50];
	unsigned long quantidadeAcoes;
	unsigned long valorAcao;
};

typedef struct Utilizador Utilizador, * pUtilizador;
struct Utilizador {
	char nome[50];
	char password[50];
	unsigned long saldo;
};

typedef struct EmpresaAcao EmpresaAcao, * pEmpresaAcao;
struct EmpresaAcao {
	char nomeEmpresa[50];
	unsigned long quantidadeAcoes;
};