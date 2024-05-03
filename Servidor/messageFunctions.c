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
	resposta.continuar = TRUE;

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

void mensagemListc(DataTransferObject* dto) {
	/*Mensagem resposta = { 0 };
	resposta.TipoM = TMensagem_R_LISTC;
	EnterCriticalSection(&dto->pSync->csEmpresas);
	CopyMemory(resposta.empresas, dto->dadosP->empresas, sizeof(Empresa) * dto->dadosP->numEmpresas);
	LeaveCriticalSection(&dto->pSync->csEmpresas);
	DWORD bytesEscritos;
	BOOL fSuccess = WriteFile(dto->hPipes[pipeIndex], &resposta, sizeof(Mensagem), &bytesEscritos, NULL);
	if (!fSuccess || bytesEscritos == 0)
		_tprintf_s(ERRO_ESCRITA_MSG);*/
}

void mensagemBuy(ThreadData* td) {
	/*DWORD numEmpresas;
	DWORD numUtilizadores;
	Mensagem mensagem = { 0 };
	mensagem.TipoM = TMensagem_R_BUY;
	DWORD bytesEscritos;
	DWORD indexEmpresa = 0;
	// 1. Verificar se a empresa existe
	EnterCriticalSection(&td->dto->pSync->csEmpresas);
		numEmpresas = td->dto->dadosP->numEmpresas;
		for (indexEmpresa; indexEmpresa < numEmpresas; ++indexEmpresa) {
			if (_tcscmp(td->mensagem.nome, td->dto->dadosP->empresas[indexEmpresa].nome) == 0) {
				break;
			}
		}
		// a empresa não existe
		if (indexEmpresa == numEmpresas) {
			LeaveCriticalSection(&td->dto->pSync->csEmpresas);
			mensagem.sucesso = FALSE;
			BOOL fSuccess = WriteFile(td->dto->hPipes[td->pipeIndex], &mensagem, sizeof(Mensagem), &bytesEscritos, NULL);
			if (!fSuccess || !bytesEscritos)
				_tprintf_s(ERRO_ESCRITA_MSG);
			return;
		}
		// a empresa existe
		// 2. Verificar se o utilizador tem saldo suficiente
		DWORD indexUtilizador = 0;
		EnterCriticalSection(&td->dto->pSync->csUtilizadores);
			numUtilizadores = td->dto->numUtilizadores;
			for (indexUtilizador; indexUtilizador < numUtilizadores; ++indexUtilizador) {
				if (_tcscmp(td->mensagem.nome, td->dto->utilizadores[indexUtilizador].username) == 0) {
					break;
				}
			}
			// o utilizador não existe
			if(indexUtilizador == numUtilizadores) {
				LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
				LeaveCriticalSection(&td->dto->pSync->csEmpresas);
				mensagem.sucesso = FALSE;
				BOOL fSucess = WriteFile(td->dto->hPipes[td->pipeIndex], &mensagem, sizeof(Mensagem), &bytesEscritos, NULL);
				if (!fSucess || !bytesEscritos)
					_tprintf_s(ERRO_ESCRITA_MSG);
				return;
			}
			// o utilizador existe
			// 3. Verificar se a quantidade de ações é válida (oferta inicial)
			if (td->mensagem.quantidade > &td->dto->dadosP->empresas[indexEmpresa].quantidadeAcoes) {
				// quantidade superior ao disponível na empresa
				LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
				LeaveCriticalSection(&td->dto->pSync->csEmpresas);
				mensagem.sucesso = FALSE;
				BOOL fSucess = WriteFile(td->dto->hPipes[td->pipeIndex], &mensagem, sizeof(Mensagem), &bytesEscritos, NULL);
				if (!fSucess || !bytesEscritos)
					_tprintf_s(ERRO_ESCRITA_MSG);
				return;
			}
			// TODO: tem de se acrescentar um array de ofertas de vendas para as ações dos utilizadores

			// quantidade válida
			double totalCompra = td->mensagem.quantidade * td->dto->dadosP->empresas[indexEmpresa].valorAcao;
			if (totalCompra > td->dto->utilizadores[indexUtilizador].saldo) {
				// saldo insuficiente
				LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
				LeaveCriticalSection(&td->dto->pSync->csEmpresas);
				mensagem.sucesso = FALSE;
				BOOL fSucess = WriteFile(td->dto->hPipes[td->pipeIndex], &mensagem, sizeof(Mensagem), &bytesEscritos, NULL);
				if (!fSucess || !bytesEscritos)
					_tprintf_s(ERRO_ESCRITA_MSG);
				return;
			}
			// saldo suficiente
			// 4. Atualizar a carteira de ações do utilizador
			BOOL empresaAcaoAtualizada = FALSE;
			DWORD numEmpresasAcoes = td->dto->utilizadores[indexUtilizador].numEmpresasAcoes;
			DWORD indexEA = 0;
			for (; indexEA < numEmpresasAcoes; ++indexEA) {
				if (_tcscmp(td->dto->utilizadores[indexUtilizador].carteiraAcoes[indexEA].nomeEmpresa, td->mensagem.nome) == 0) {
					// empresa já existe na carteira de ações do utilizador
					// aumenta a quantidade de ações
					td->dto->utilizadores[indexUtilizador].carteiraAcoes[indexEA].quantidadeAcoes += td->mensagem.quantidade;
					// diminui a quantidade de ações na empresa
					td->dto->dadosP->empresas[indexEmpresa].quantidadeAcoes -= td->mensagem.quantidade;
					// atualiza o saldo do utilizador
					td->dto->utilizadores[indexUtilizador].saldo -= totalCompra;
					empresaAcaoAtualizada = TRUE;
					break;
				}
			}
			// a empresa não existe na carteira de ações do utilizador
			if(numEmpresasAcoes == TAM_MAX_EMPRESA_ACAO && !empresaAcaoAtualizada) {
				// carteira de ações cheia
				LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
				LeaveCriticalSection(&td->dto->pSync->csEmpresas);
				mensagem.sucesso = FALSE;
				BOOL fSucess = WriteFile(td->dto->hPipes[td->pipeIndex], &mensagem, sizeof(Mensagem), &bytesEscritos, NULL);
				if (!fSucess || !bytesEscritos)
					_tprintf_s(ERRO_ESCRITA_MSG);
				return;
			}

			// acrescentar uma EmpresaAcao à carteira de ações do utilizador
			td->dto->utilizadores[indexUtilizador].carteiraAcoes[indexEA].quantidadeAcoes = td->mensagem.quantidade;
			_tcscpy_s(td->dto->utilizadores[indexUtilizador].carteiraAcoes[indexEA].nomeEmpresa, TAM_NOME, td->mensagem.nome);
			td->dto->dadosP->empresas[indexEmpresa].quantidadeAcoes -= td->mensagem.quantidade;
			td->dto->utilizadores[indexUtilizador].numEmpresasAcoes++;
			td->dto->utilizadores[indexUtilizador].saldo -= totalCompra;
			empresaAcaoAtualizada = TRUE;
			// 6. Enviar resposta ao cliente
		LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
	LeaveCriticalSection(&td->dto->pSync->csEmpresas);
	mensagem.sucesso = TRUE;
	BOOL fSucess = WriteFile(td->dto->hPipes[td->pipeIndex], &mensagem, sizeof(Mensagem), &bytesEscritos, NULL);
	if (!fSucess || !bytesEscritos)
		_tprintf_s(ERRO_ESCRITA_MSG); */
}

void mensagemSell() {
	// TODO: mensagemSell
}

void mensagemBalance(ThreadData* td) {
	/*
	// TODO: mensagemBalance
	// ao receber um pedido de saldo devo passar por vários passos:
	// 1. Verificar se o utilizador existe
	// 2. Criar mensagem de resposta com o saldo do utilizador
	// 3. Enviar a resposta ao cliente
	// recebo por mensagem o nome do utilizador e o tipo de mensagem
	DWORD numUtilizadores;
	pUtilizador uLocais = (pUtilizador)malloc(TAM_MAX_USERS * sizeof(Utilizador));
	if (uLocais == NULL) {
		_tprintf_s(ERRO_MEMORIA);
		return;
	}

	EnterCriticalSection(&td->dto->pSync->csUtilizadores);
	numUtilizadores = td->dto->numUtilizadores;
	CopyMemory(uLocais, td->dto->utilizadores, sizeof(Utilizador) * numUtilizadores);
	LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
		
	Mensagem mensagem = { 0 };
	mensagem.TipoM = TMensagem_R_BALANCE;
	DWORD bytesEscritos;

	DWORD indexUtilizador = 0;
	for (; indexUtilizador < numUtilizadores; ++indexUtilizador) {
		if (_tcscmp(uLocais[indexUtilizador].username, td->mensagem.nome) == 0) {
			break;
		}
	}
	if(indexUtilizador == numUtilizadores) {
		// utilizador não existe
		mensagem.sucesso = FALSE;
		BOOL fSucess = WriteFile(td->dto->hPipes[td->pipeIndex], &mensagem, sizeof(Mensagem), &bytesEscritos, NULL);
		if (!fSucess || !bytesEscritos)
			_tprintf_s(ERRO_ESCRITA_MSG);
		free(uLocais);
		return;
	}
	// utilizador existe
	mensagem.sucesso = TRUE;
	mensagem.valor = uLocais[indexUtilizador].saldo;
	BOOL fSucess = WriteFile(td->dto->hPipes[td->pipeIndex], &mensagem, sizeof(Mensagem), &bytesEscritos, NULL);
	if (!fSucess || !bytesEscritos)
		_tprintf_s(ERRO_ESCRITA_MSG);
	free(uLocais);*/
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