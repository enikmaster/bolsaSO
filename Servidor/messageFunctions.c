#include "servidor.h"

// fun��es auxilares
BOOL verificaEmpresa(ThreadData* td, Mensagem mensagemRead, DWORD numEmpresas, DWORD* indexEmpresa) {
	for (*indexEmpresa = 0; *indexEmpresa < numEmpresas; ++(*indexEmpresa))
		if (_tcscmp(mensagemRead.empresa, td->dto->dadosP->empresas[*indexEmpresa].nome) == 0)
			return TRUE;
	return FALSE;
}

BOOL verificaUtilizador(ThreadData* td, Mensagem mensagemRead, DWORD numUtilizadores, DWORD* indexUtilizador) {
	for (*indexUtilizador = 0; *indexUtilizador < numUtilizadores; ++(*indexUtilizador)) {
		if (_tcscmp(mensagemRead.nome, td->dto->utilizadores[*indexUtilizador].username) == 0) {
			return TRUE;
		}
	}
	return FALSE;
}

BOOL verificaEmpresaCarteira(ThreadData* td, DWORD indexUtilizador, Mensagem mensagemRead, DWORD numEmpresasAcoes, DWORD* indexEA) {
	for (*indexEA = 0; *indexEA < numEmpresasAcoes; ++(*indexEA)) {
		if (_tcscmp(td->dto->utilizadores[indexUtilizador].carteiraAcoes[*indexEA].nomeEmpresa, mensagemRead.empresa) == 0) {
			return TRUE;
		}
	}
	return FALSE;
}

// fun��es de tratamento de mensagens
void mensagemLogin(ThreadData* td, Mensagem mensagem) {
	pUtilizador uLocais = (pUtilizador)malloc(TAM_MAX_USERS * sizeof(Utilizador));
	if (uLocais == NULL) {
		_tprintf_s(ERRO_MEMORIA);
		return;
	};

	// gerar mensagem de resposta
	Mensagem resposta = { 0 };
	resposta.TipoM = TMensagem_R_LOGIN;
	resposta.sucesso = FALSE;

	DWORD numUtilizadores;
	HANDLE hPipe;
	EnterCriticalSection(&td->dto->pSync->csUtilizadores);
	hPipe = td->hPipeInst;
	numUtilizadores = td->dto->numUtilizadores;
	memcpy(uLocais, td->dto->utilizadores, sizeof(Utilizador) * numUtilizadores);
	LeaveCriticalSection(&td->dto->pSync->csUtilizadores);

	for (DWORD i = 0; i < numUtilizadores; ++i) {
		if (_tcscmp(uLocais[i].username, mensagem.nome) == 0) {
			if (_tcscmp(uLocais[i].password, mensagem.password) == 0) {
				if (!uLocais[i].logado) {
					EnterCriticalSection(&td->dto->pSync->csUtilizadores);
					td->dto->utilizadores[i].logado = TRUE;
					LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
					resposta.sucesso = TRUE;
					break;
				}
			}
		}
	}
	enviarMensagem(hPipe, resposta, td->dto->pSync->csWrite);
	free(uLocais);
}

void mensagemListc(ThreadData* td) {
	Mensagem resposta = { 0 };
	resposta.TipoM = TMensagem_R_LISTC;

	HANDLE hPipe;
	EnterCriticalSection(&td->dto->pSync->csEmpresas);
	hPipe = td->hPipeInst;
	resposta.quantidade = td->dto->dadosP->numEmpresas;
	memcpy(resposta.empresas, td->dto->dadosP->empresas, sizeof(Empresa) * td->dto->dadosP->numEmpresas);
	LeaveCriticalSection(&td->dto->pSync->csEmpresas);

	enviarMensagem(hPipe, resposta, td->dto->pSync->csWrite); //enviarMensagem( para_onde, o_qu�);
}

void mensagemBuy(ThreadData* td, Mensagem mensagemRead) {
	DWORD numEmpresas;
	DWORD numUtilizadores;
	Mensagem mensagem = { 0 };
	mensagem.TipoM = TMensagem_R_BUY;
	DWORD indexEmpresa = 0;
	double totalCompra = 0;
	BOOL empresaAcaoAtualizada = FALSE;
	DWORD numEmpresasAcoes = 0;
	DWORD indexEA = 0;
	double taxaVariacao = 1.2;

	// 0. Verificar se está em Pausa
	EnterCriticalSection(&td->dto->pSync->csContinuar);
	if (td->dto->pausado) {
		LeaveCriticalSection(&td->dto->pSync->csContinuar);
		mensagem.sucesso = FALSE;
		enviarMensagem(td->hPipeInst, mensagem, td->dto->pSync->csWrite);
		return;
	}
	LeaveCriticalSection(&td->dto->pSync->csContinuar);

	// 1. Verificar se a empresa existe
	EnterCriticalSection(&td->dto->pSync->csEmpresas);
	numEmpresas = td->dto->dadosP->numEmpresas;
	if (!verificaEmpresa(td, mensagemRead, numEmpresas, &indexEmpresa)) {
		LeaveCriticalSection(&td->dto->pSync->csEmpresas);
		mensagem.sucesso = FALSE;
		enviarMensagem(td->hPipeInst, mensagem, td->dto->pSync->csWrite);
		return;
	}
	// a empresa existe
	// 2. Verificar se o utilizador tem saldo suficiente
	DWORD indexUtilizador = 0;
	EnterCriticalSection(&td->dto->pSync->csUtilizadores);
	numUtilizadores = td->dto->numUtilizadores;
	if (!verificaUtilizador(td, mensagemRead, numUtilizadores, &indexUtilizador)) {
		LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
		LeaveCriticalSection(&td->dto->pSync->csEmpresas);
		mensagem.sucesso = FALSE;
		enviarMensagem(td->hPipeInst, mensagem, td->dto->pSync->csWrite);
		return;
	}
	// o utilizador existe
	// 3. Verificar se a quantidade de ações é válida (oferta inicial)
	if (mensagemRead.quantidade > &td->dto->dadosP->empresas[indexEmpresa].quantidadeAcoes) {
		// quantidade superior ao disponível na empresa
		LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
		LeaveCriticalSection(&td->dto->pSync->csEmpresas);
		mensagem.sucesso = FALSE;
		// Enviar resposta insucesso ao cliente
		enviarMensagem(td->hPipeInst, mensagem, td->dto->pSync->csWrite);
		return;
	}
	// quantidade válida
	totalCompra = mensagemRead.quantidade * td->dto->dadosP->empresas[indexEmpresa].valorAcao;
	if (totalCompra > td->dto->utilizadores[indexUtilizador].saldo) {
		// saldo insuficiente
		LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
		LeaveCriticalSection(&td->dto->pSync->csEmpresas);
		mensagem.sucesso = FALSE;
		// Enviar resposta insucesso ao cliente
		enviarMensagem(td->hPipeInst, mensagem, td->dto->pSync->csWrite);
		return;
	}
	// saldo suficiente
	// 4. Atualizar a carteira de ações do utilizador
	numEmpresasAcoes = td->dto->utilizadores[indexUtilizador].numEmpresasAcoes;
	for (indexEA = 0; indexEA < numEmpresasAcoes; ++indexEA) {
		if (_tcscmp(td->dto->utilizadores[indexUtilizador].carteiraAcoes[indexEA].nomeEmpresa, mensagemRead.empresa) == 0) {
			// empresa já existe na carteira de ações do utilizador
			// aumenta a quantidade de ações
			td->dto->utilizadores[indexUtilizador].carteiraAcoes[indexEA].quantidadeAcoes += mensagemRead.quantidade;
			// alterar o valor das ações
			td->dto->dadosP->empresas[indexEmpresa].valorAcao *= taxaVariacao;
			// diminui a quantidade de ações na empresa
			td->dto->dadosP->empresas[indexEmpresa].quantidadeAcoes -= mensagemRead.quantidade;
			// atualiza o saldo do utilizador
			td->dto->utilizadores[indexUtilizador].saldo -= totalCompra;
			empresaAcaoAtualizada = TRUE;
			break;
		}
	}
	// a empresa não existe na carteira de ações do utilizador
	if (numEmpresasAcoes == TAM_MAX_EMPRESA_ACAO && !empresaAcaoAtualizada) {
		// carteira de ações cheia
		LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
		LeaveCriticalSection(&td->dto->pSync->csEmpresas);
		mensagem.sucesso = FALSE;
		// Enviar resposta insucesso ao cliente
		enviarMensagem(td->hPipeInst, mensagem, td->dto->pSync->csWrite);
		return;
	}
	// 5. acrescentar uma EmpresaAcao na carteira de ações do utilizador
	if (!empresaAcaoAtualizada) {
		// empresa não existe na carteira de ações do utilizador
		// acrescentar uma EmpresaAcao na carteira de ações do utilizador
		td->dto->utilizadores[indexUtilizador].carteiraAcoes[indexEA].quantidadeAcoes = mensagemRead.quantidade;
		memcpy(td->dto->utilizadores[indexUtilizador].carteiraAcoes[indexEA].nomeEmpresa, mensagemRead.empresa, (_tcslen(mensagemRead.empresa) + 1) * sizeof(TCHAR));
	}
	// alterar o valor das ações
	td->dto->dadosP->empresas[indexEmpresa].valorAcao *= taxaVariacao;
	// diminui a quantidade de ações na empresa
	td->dto->dadosP->empresas[indexEmpresa].quantidadeAcoes -= mensagemRead.quantidade;
	// atualiza o numero de ações na carteira de ações do utilizador
	td->dto->utilizadores[indexUtilizador].numEmpresasAcoes++;
	// atualiza o saldo do utilizador
	td->dto->utilizadores[indexUtilizador].saldo -= totalCompra;
	empresaAcaoAtualizada = TRUE;
	// 6. Enviar resposta sucesso ao cliente
	LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
	LeaveCriticalSection(&td->dto->pSync->csEmpresas);
	mensagem.sucesso = TRUE;
	memcpy(mensagem.empresa, mensagemRead.empresa, (_tcslen(mensagemRead.empresa) + 1) * sizeof(TCHAR));

	enviarMensagem(td->hPipeInst, mensagem, td->dto->pSync->csWrite);
}

void mensagemSell(ThreadData* td, Mensagem mensagemRead) {
	Mensagem resposta = { 0 };
	resposta.TipoM = TMensagem_R_SELL;
	DWORD numEmpresas;
	DWORD numUtilizadores;
	DWORD indexEmpresa = 0;
	DWORD indexUtilizador = 0;
	double totalVenda = 0;
	double taxaVariacao = 0.8;
	BOOL empresaAcaoAtualizada = FALSE;
	DWORD numEmpresasAcoes = 0;
	DWORD indexEA = 0;

	// 0. Verificar se está em Pausa
	EnterCriticalSection(&td->dto->pSync->csContinuar);
	if (td->dto->pausado) {
		LeaveCriticalSection(&td->dto->pSync->csContinuar);
		resposta.sucesso = FALSE;
		enviarMensagem(td->hPipeInst, resposta, td->dto->pSync->csWrite);
		return;
	}
	LeaveCriticalSection(&td->dto->pSync->csContinuar);

	// 1. Verificar se a empresa existe
	EnterCriticalSection(&td->dto->pSync->csEmpresas);
	numEmpresas = td->dto->dadosP->numEmpresas;
	if (!verificaEmpresa(td, mensagemRead, numEmpresas, &indexEmpresa)) {
		LeaveCriticalSection(&td->dto->pSync->csEmpresas);
		resposta.sucesso = FALSE;
		enviarMensagem(td->hPipeInst, resposta, td->dto->pSync->csWrite);
		return;
	}

	// a empresa existe
	// 2. Verificar se o utilizador existe
	EnterCriticalSection(&td->dto->pSync->csUtilizadores);
	numUtilizadores = td->dto->numUtilizadores;
	if (!verificaUtilizador(td, mensagemRead, numUtilizadores, &indexUtilizador)) {
		LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
		LeaveCriticalSection(&td->dto->pSync->csEmpresas);
		resposta.sucesso = FALSE;
		enviarMensagem(td->hPipeInst, resposta, td->dto->pSync->csWrite);
		return;
	}

	// o utilizador existe
	// 3. Verificar se a empresa existe na carteira de ações do utilizador
	numEmpresasAcoes = td->dto->utilizadores[indexUtilizador].numEmpresasAcoes;
	if (!verificaEmpresaCarteira(td, indexUtilizador, mensagemRead, numEmpresasAcoes, &indexEA)) {
		LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
		LeaveCriticalSection(&td->dto->pSync->csEmpresas);
		resposta.sucesso = FALSE;
		enviarMensagem(td->hPipeInst, resposta, td->dto->pSync->csWrite);
		return;
	}
	// a empresa não existe na carteira de ações do utilizador
	if (indexEA == numEmpresasAcoes) {
		LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
		LeaveCriticalSection(&td->dto->pSync->csEmpresas);
		resposta.sucesso = FALSE;
		// Enviar resposta insucesso ao cliente
		enviarMensagem(td->hPipeInst, resposta, td->dto->pSync->csWrite);
		return;
	}
	// a empresa existe na carteira de ações do utilizador
	// 4. Verificar se a quantidade de ações é válida
	if (mensagemRead.quantidade > td->dto->utilizadores[indexUtilizador].carteiraAcoes[indexEA].quantidadeAcoes) {
		// quantidade superior ao disponível na carteira de ações do utilizador
		LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
		LeaveCriticalSection(&td->dto->pSync->csEmpresas);
		resposta.sucesso = FALSE;
		// Enviar resposta insucesso ao cliente
		enviarMensagem(td->hPipeInst, resposta, td->dto->pSync->csWrite);
		return;
	}
	// quantidade válida
	totalVenda = mensagemRead.quantidade * td->dto->dadosP->empresas[indexEmpresa].valorAcao;

	// 5. Atualizar a carteira de ações do utilizador
	td->dto->utilizadores[indexUtilizador].carteiraAcoes[indexEA].quantidadeAcoes -= mensagemRead.quantidade;
	if (td->dto->utilizadores[indexUtilizador].carteiraAcoes[indexEA].quantidadeAcoes == 0) {
		// remover a empresa da carteira de ações do utilizador
		for (DWORD i = indexEA; i < numEmpresasAcoes - 1; ++i) {
			td->dto->utilizadores[indexUtilizador].carteiraAcoes[i] = td->dto->utilizadores[indexUtilizador].carteiraAcoes[i + 1];
		}
		td->dto->utilizadores[indexUtilizador].numEmpresasAcoes--;
	}
	td->dto->utilizadores[indexUtilizador].saldo += totalVenda;
	// 6. Atualizar o valor das ações da empresa
	td->dto->dadosP->empresas[indexEmpresa].valorAcao *= taxaVariacao;
	td->dto->dadosP->empresas[indexEmpresa].quantidadeAcoes += mensagemRead.quantidade;
	// 7. Enviar resposta sucesso ao cliente
	LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
	LeaveCriticalSection(&td->dto->pSync->csEmpresas);
	resposta.sucesso = TRUE;
	memcpy(resposta.empresa, mensagemRead.empresa, (_tcslen(mensagemRead.empresa) + 1) * sizeof(TCHAR));

	enviarMensagem(td->hPipeInst, resposta, td->dto->pSync->csWrite);
}

void mensagemBalance(ThreadData* td, Mensagem mensagem) {
	pUtilizador uLocais = (pUtilizador)malloc(TAM_MAX_USERS * sizeof(Utilizador));
	if (uLocais == NULL) {
		_tprintf_s(ERRO_MEMORIA);
		return;
	}

	// gerar mensagem de resposta
	Mensagem resposta = { 0 };
	resposta.TipoM = TMensagem_R_BALANCE;

	DWORD numUtilizadores;
	HANDLE hPipe;
	EnterCriticalSection(&td->dto->pSync->csUtilizadores);
	hPipe = td->hPipeInst;
	numUtilizadores = td->dto->numUtilizadores;
	memcpy(uLocais, td->dto->utilizadores, sizeof(Utilizador) * numUtilizadores);
	LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
	DWORD i = 0;
	for (; i < numUtilizadores; ++i) {
		if (_tcscmp(uLocais[i].username, mensagem.nome) == 0) {
			break;
		}
	}

	if (i == numUtilizadores) {
		// utilizador não existe
		resposta.sucesso = FALSE;
		enviarMensagem(hPipe, resposta, td->dto->pSync->csWrite);
		free(uLocais);
		return;
	}
	mensagem.sucesso = TRUE;
	EnterCriticalSection(&td->dto->pSync->csUtilizadores);
	resposta.valor = td->dto->utilizadores[i].saldo;
	LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
	enviarMensagem(hPipe, resposta, td->dto->pSync->csWrite);
	free(uLocais);
}

void mensagemWallet(ThreadData* td, Mensagem mensagem) {
	Mensagem resposta = { 0 };
	resposta.TipoM = TMensagem_R_WALLET;
	DWORD numUtilizadores;
	DWORD numEmpresaAcao;
	DWORD indexUtilizador = 0;
	DWORD indexEA = 0;
	EnterCriticalSection(&td->dto->pSync->csUtilizadores);
	numUtilizadores = td->dto->numUtilizadores;
	for (indexUtilizador; indexUtilizador < numUtilizadores; ++indexUtilizador) {
		if (_tcscmp(mensagem.nome, td->dto->utilizadores[indexUtilizador].username) == 0) {
			break;
		}
	}
	// o utilizador não existe
	if (indexUtilizador == numUtilizadores) {
		LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
		resposta.sucesso = FALSE;
		// Enviar resposta insucesso ao cliente
		enviarMensagem(td->hPipeInst, resposta, td->dto->pSync->csWrite);
		return;
	}
	// o utilizador existe
	numEmpresaAcao = td->dto->utilizadores[indexUtilizador].numEmpresasAcoes;
	for (indexEA; indexEA < numEmpresaAcao; ++indexEA) {
		memcpy(resposta.carteiraAcoes[indexEA].nomeEmpresa, td->dto->utilizadores[indexUtilizador].carteiraAcoes[indexEA].nomeEmpresa, (_tcslen(td->dto->utilizadores[indexUtilizador].carteiraAcoes[indexEA].nomeEmpresa) + 1) * sizeof(TCHAR));
		resposta.carteiraAcoes[indexEA].quantidadeAcoes = td->dto->utilizadores[indexUtilizador].carteiraAcoes[indexEA].quantidadeAcoes;
	}
	LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
	resposta.quantidade = numEmpresaAcao;
	resposta.sucesso = TRUE;
	enviarMensagem(td->hPipeInst, resposta, td->dto->pSync->csWrite);
}

void mensagemExit(ThreadData* td, Mensagem mensagem) {
	// identificar o cliente
	TCHAR* username = mensagem.nome;
	int j = 0;

	// procurar o cliente no array de utilizadores e retirar
	EnterCriticalSection(&td->dto->pSync->csUtilizadores);
	for (int i = 0; i < td->dto->numUtilizadores; i++) {
		if (_tcscmp(td->dto->utilizadores[i].username, username) == 0) {
			// definir como não logado
			td->dto->utilizadores[i].logado = FALSE;
			_tprintf_s(INFO_CLIENTE_DESCONECTADO, username);
			break;
		}
	}

	for (j; j < td->dto->limiteClientes; ++j) {
		if (!td[j].livre) {
			break;
		}
	}
	if (j == td->dto->limiteClientes) {
		// todos os clientes ocupados mas saiu um agora
		SetEvent(td->dto->hLimiteClientes);
	}
	LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
	// fechar o pipe
	CloseHandle(td->hPipeInst);
	// libertar a posição do array de ThreadData
	td->livre = TRUE;
}

void mensagemAddc(ThreadData* td, TCHAR* empresa) {
	// Criar a mensagem
	Mensagem mensagem = { 0 };
	// Definir os dados da mensagem
	mensagem.TipoM = TMensagem_ADDC;
	memcpy(mensagem.empresa, empresa, (_tcslen(empresa) + 1) * sizeof(TCHAR));

	DWORD limiteClientes = 0;
	EnterCriticalSection(&td->dto->pSync->csLimClientes);
	limiteClientes = td->dto->limiteClientes;
	LeaveCriticalSection(&td->dto->pSync->csLimClientes);

	// Enviar a mensagem para todos os clientes
	for (DWORD i = 0; i < limiteClientes; ++i) {
		if (!td[i].livre) {
			enviarMensagem(td[i].hPipeInst, mensagem, td->dto->pSync->csWrite);
		}
	}
}

void mensagemStock(ThreadData* td, TCHAR* empresa, double valorAcao) {
	// Criar a mensagem
	Mensagem mensagem = { 0 };
	// Definir os dados da mensagem
	mensagem.TipoM = TMensagem_STOCK;
	memcpy(mensagem.empresa, empresa, (_tcslen(empresa) + 1) * sizeof(TCHAR));
	mensagem.valor = valorAcao;

	DWORD limiteClientes = 0;
	EnterCriticalSection(&td->dto->pSync->csLimClientes);
	limiteClientes = td->dto->limiteClientes;
	LeaveCriticalSection(&td->dto->pSync->csLimClientes);

	// Enviar a mensagem a todos os clientes
	for (DWORD i = 0; i < limiteClientes; ++i) {
		if (!td[i].livre) {
			enviarMensagem(td[i].hPipeInst, mensagem, td->dto->pSync->csWrite);
		}
	}
}

void mensagemPause(ThreadData* td, DWORD tempo) {
	// Criar a mensagem
	Mensagem mensagem = { 0 };
	// Definir os dados da mensagem
	mensagem.TipoM = TMensagem_PAUSE;
	mensagem.quantidade = tempo;

	DWORD limiteClientes = 0;
	EnterCriticalSection(&td->dto->pSync->csLimClientes);
	limiteClientes = td->dto->limiteClientes;
	LeaveCriticalSection(&td->dto->pSync->csLimClientes);

	// Enviar a mensagem a todos os clientes
	for (DWORD i = 0; i < limiteClientes; ++i) {
		if (!td[i].livre) {
			enviarMensagem(td[i].hPipeInst, mensagem, td->dto->pSync->csWrite);
		}
	}
}

void mensagemResume(ThreadData* td) {
	// Criar a mensagem
	Mensagem mensagem = { 0 };
	// Definir os dados da mensagem
	mensagem.TipoM = TMensagem_RESUME;

	DWORD limiteClientes = 0;
	EnterCriticalSection(&td->dto->pSync->csLimClientes);
	limiteClientes = td->dto->limiteClientes;
	LeaveCriticalSection(&td->dto->pSync->csLimClientes);

	// Enviar a mensagem a todos os clientes
	for (DWORD i = 0; i < limiteClientes; ++i) {
		if (!td[i].livre) {
			enviarMensagem(td[i].hPipeInst, mensagem, td->dto->pSync->csWrite);
		}
	}

}

void mensagemLoad(ThreadData* td, int numEmpresas) {
	// Criar a mensagem
	Mensagem mensagem = { 0 };
	// Definir os dados da mensagem
	mensagem.TipoM = TMensagem_LOAD;
	mensagem.quantidade = (DWORD)numEmpresas;

	DWORD limiteClientes = 0;
	EnterCriticalSection(&td->dto->pSync->csLimClientes);
	limiteClientes = td->dto->limiteClientes;
	LeaveCriticalSection(&td->dto->pSync->csLimClientes);

	// Enviar a mensagem a todos os clientes
	for (DWORD i = 0; i < limiteClientes; ++i) {
		if (!td[i].livre) {
			enviarMensagem(td[i].hPipeInst, mensagem, td->dto->pSync->csWrite);
		}
	}
}

void mensagemClose(ThreadData* td) {
	// Criar a mensagem
	Mensagem mensagem = { 0 };
	// Definir os dados da mensagem
	mensagem.TipoM = TMensagem_CLOSE;

	DWORD limiteClientes = 0;
	EnterCriticalSection(&td->dto->pSync->csLimClientes);
	limiteClientes = td->dto->limiteClientes;
	LeaveCriticalSection(&td->dto->pSync->csLimClientes);

	// Enviar a mensagem a todos os clientes
	for (DWORD i = 0; i < limiteClientes; ++i) {
		if (!td[i].livre) {
			enviarMensagem(td[i].hPipeInst, mensagem, td->dto->pSync->csWrite);
		}
	}
}

BOOL enviarMensagem(HANDLE hPipe, Mensagem mensagem, CRITICAL_SECTION csWrite) {
	DWORD bytesEscritos;
	OVERLAPPED ov = { 0 };
	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (ov.hEvent == NULL) {
		_tprintf_s(ERRO_CREATE_EVENT);
		return FALSE;
	}
	BOOL fSuccess;
	EnterCriticalSection(&csWrite);
	fSuccess = WriteFile(hPipe, &mensagem, sizeof(Mensagem), &bytesEscritos, &ov);
	LeaveCriticalSection(&csWrite);
	BOOL ovResult = GetOverlappedResult(hPipe, &ov, &bytesEscritos, FALSE);
	if (!ovResult || bytesEscritos == 0) {
		_tprintf_s(ERRO_ESCRITA_MSG);
		return FALSE;
	}
	return TRUE;
}