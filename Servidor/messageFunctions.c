#include "servidor.h"

// funções de tratamento de mensagens
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
	enviarMensagem(hPipe, resposta);
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

	enviarMensagem(hPipe, resposta); //enviarMensagem( para_onde, o_quê);
}

void mensagemBuy(ThreadData* td, Mensagem mensagemRead) {
	DWORD numEmpresas;
	DWORD numUtilizadores;
	Mensagem mensagem = { 0 };
	mensagem.TipoM = TMensagem_R_BUY;
	DWORD bytesEscritos;
	DWORD indexEmpresa = 0;
	double totalCompra = 0;
	BOOL empresaAcaoAtualizada = FALSE;
	DWORD numEmpresasAcoes = 0;
	DWORD indexEA = 0;
	double taxaVariacao = 1;
	// 1. Verificar se a empresa existe
	EnterCriticalSection(&td->dto->pSync->csEmpresas);
	numEmpresas = td->dto->dadosP->numEmpresas;
	for (indexEmpresa; indexEmpresa < numEmpresas; ++indexEmpresa)
		if (_tcscmp(mensagemRead.empresa, td->dto->dadosP->empresas[indexEmpresa].nome) == 0)
			break;
	// a empresa não existe
	if (indexEmpresa == numEmpresas) {
		LeaveCriticalSection(&td->dto->pSync->csEmpresas);
		mensagem.sucesso = FALSE;
		// Enviar resposta insucesso ao cliente
		enviarMensagem(td->hPipeInst, mensagem);
		return;
	}
	// a empresa existe
	// 2. Verificar se o utilizador tem saldo suficiente
	DWORD indexUtilizador = 0;
	EnterCriticalSection(&td->dto->pSync->csUtilizadores);
	numUtilizadores = td->dto->numUtilizadores;
	for (indexUtilizador; indexUtilizador < numUtilizadores; ++indexUtilizador) {
		if (_tcscmp(mensagemRead.nome, td->dto->utilizadores[indexUtilizador].username) == 0) {
			break;
		}
	}
	// o utilizador não existe
	if(indexUtilizador == numUtilizadores) {
		LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
		LeaveCriticalSection(&td->dto->pSync->csEmpresas);
		mensagem.sucesso = FALSE;
		// Enviar resposta insucesso ao cliente
		enviarMensagem(td->hPipeInst, mensagem);
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
		enviarMensagem(td->hPipeInst, mensagem);
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
		enviarMensagem(td->hPipeInst, mensagem);
		return;
	}
	// saldo suficiente
	// 4. Atualizar a carteira de ações do utilizador
	for (indexEA = 0; indexEA < numEmpresasAcoes; ++indexEA) {
		if (_tcscmp(td->dto->utilizadores[indexUtilizador].carteiraAcoes[indexEA].nomeEmpresa, mensagemRead.empresa) == 0) {
			// empresa já existe na carteira de ações do utilizador
			// aumenta a quantidade de ações
			td->dto->utilizadores[indexUtilizador].carteiraAcoes[indexEA].quantidadeAcoes += mensagemRead.quantidade;
			// alterar o valor das ações
			taxaVariacao += td->dto->dadosP->empresas[indexEmpresa].quantidadeAcoes / mensagemRead.quantidade;
			td->dto->dadosP->empresas[indexEmpresa].valorAcao *= taxaVariacao;
			// diminui a quantidade de ações na empresa
			td->dto->dadosP->empresas[indexEmpresa].quantidadeAcoes -= mensagemRead.quantidade;
			// atualiza o saldo do utilizador
			td->dto->utilizadores[indexUtilizador].saldo -= totalCompra;
			empresaAcaoAtualizada = TRUE;
			//td->dto->dadosP->empresas[indexEmpresa].valorAcao *= taxaVariacao;
			break;
		}
	}
	// a empresa não existe na carteira de ações do utilizador
	if(numEmpresasAcoes == TAM_MAX_EMPRESA_ACAO && !empresaAcaoAtualizada) {
		// carteira de ações cheia
		LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
		LeaveCriticalSection(&td->dto->pSync->csEmpresas);
		mensagem.sucesso = FALSE;
		// Enviar resposta insucesso ao cliente
		enviarMensagem(td->hPipeInst, mensagem);
		return;
	}
	// 5. acrescentar uma EmpresaAcao à carteira de ações do utilizador
	td->dto->utilizadores[indexUtilizador].carteiraAcoes[indexEA].quantidadeAcoes = mensagemRead.quantidade;
	memcpy(td->dto->utilizadores[indexUtilizador].carteiraAcoes[indexEA].nomeEmpresa, mensagemRead.empresa, _tcslen(mensagemRead.empresa) * sizeof(TCHAR));
	// alterar o valor das ações
	taxaVariacao += (double)mensagemRead.quantidade / (double)td->dto->dadosP->empresas[indexEmpresa].quantidadeAcoes;
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
	memcpy(mensagem.empresa, mensagemRead.empresa, _tcslen(mensagemRead.empresa) * sizeof(TCHAR));
	
	enviarMensagem(td->hPipeInst, mensagem);
}

void mensagemSell() {
	// TODO: mensagemSell
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
	for(; i < numUtilizadores; ++i) {
		if (_tcscmp(uLocais[i].username, mensagem.nome) == 0) {
			break;
		}
	}

	if (i == numUtilizadores) {
		// utilizador não existe
		resposta.sucesso = FALSE;
		enviarMensagem(hPipe, resposta);
		free(uLocais);
		return;
	}
	mensagem.sucesso = TRUE;
	EnterCriticalSection(&td->dto->pSync->csUtilizadores);
	resposta.valor = td->dto->utilizadores[i].saldo;
	LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
	enviarMensagem(hPipe, resposta);
	free(uLocais);
}

void mensagemWallet() {
	// TODO: mensagemWallet
}

void mensagemExit() {
	// TODO: mensagemExit
}

void mensagemAddc() {
	// TODO: mensagemAddc
}

void mensagemStock() {
	// TODO: mensagemStock
}

void mensagemPause() {
	// TODO: mensagemPause
}

void mensagemResume() {
	// TODO: mensagemResume
}

void mensagemLoad() {
	// TODO: mensagemLoad
}

void mensagemClose() {
	// TODO: mensagemClose
}

BOOL enviarMensagem(HANDLE hPipe, Mensagem mensagem) {
	DWORD bytesEscritos;
	OVERLAPPED ov = { 0 };
	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (ov.hEvent == NULL) {
		_tprintf_s(ERRO_CREATE_EVENT);
		return FALSE;
	}
	BOOL fSuccess = WriteFile(hPipe, &mensagem, sizeof(Mensagem), &bytesEscritos, &ov);
	BOOL ovResult = GetOverlappedResult(hPipe, &ov, &bytesEscritos, FALSE);
	if (!ovResult || bytesEscritos == 0) {
		_tprintf_s(ERRO_ESCRITA_MSG);
		return FALSE;
	}
	return TRUE;
}