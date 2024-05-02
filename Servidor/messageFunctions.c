#include "servidor.h"

// fun��es de tratamento de mensagens
void mensagemLogin(ThreadData* td, Mensagem mensagem) {
	_tprintf_s(DEBUGGER);
	/*pUtilizador uLocais = (pUtilizador)malloc(TAM_MAX_USERS * sizeof(Utilizador));
	if (uLocais == NULL) {
		_tprintf_s(ERRO_MEMORIA);
		return;
	}

	EnterCriticalSection(&td->dto->pSync->csUtilizadores);
	DWORD numUtilizadores = td->dto->numUtilizadores;
	CopyMemory(uLocais, td->dto->utilizadores, sizeof(Utilizador) * numUtilizadores);
	LeaveCriticalSection(&td->dto->pSync->csUtilizadores);

	Mensagem resposta = { 0 };
	resposta.TipoM = TMensagem_R_LOGIN;
	resposta.sucesso = FALSE;
	DWORD bytesEscritos;

	for (DWORD i = 0; i < numUtilizadores; ++i) {
		if (_tcscmp(uLocais[i].username, td->mensagem.nome) == 0) {
			if (_tcscmp(uLocais[i].password, td->mensagem.password) == 0) {
				if (!uLocais[i].logado) {
					resposta.sucesso = TRUE;
					break;
				}
			}
		}
	}

	BOOL fSuccess = WriteFile(td->dto->hPipes[td->pipeIndex], &resposta, sizeof(Mensagem), &bytesEscritos, NULL);
	if (!fSuccess || bytesEscritos == 0)
		_tprintf_s(ERRO_ESCRITA_MSG);
	free(uLocais);*/
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
		// a empresa n�o existe
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
			// o utilizador n�o existe
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
			// 3. Verificar se a quantidade de a��es � v�lida (oferta inicial)
			if (td->mensagem.quantidade > &td->dto->dadosP->empresas[indexEmpresa].quantidadeAcoes) {
				// quantidade superior ao dispon�vel na empresa
				LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
				LeaveCriticalSection(&td->dto->pSync->csEmpresas);
				mensagem.sucesso = FALSE;
				BOOL fSucess = WriteFile(td->dto->hPipes[td->pipeIndex], &mensagem, sizeof(Mensagem), &bytesEscritos, NULL);
				if (!fSucess || !bytesEscritos)
					_tprintf_s(ERRO_ESCRITA_MSG);
				return;
			}
			// TODO: tem de se acrescentar um array de ofertas de vendas para as a��es dos utilizadores

			// quantidade v�lida
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
			// 4. Atualizar a carteira de a��es do utilizador
			BOOL empresaAcaoAtualizada = FALSE;
			DWORD numEmpresasAcoes = td->dto->utilizadores[indexUtilizador].numEmpresasAcoes;
			DWORD indexEA = 0;
			for (; indexEA < numEmpresasAcoes; ++indexEA) {
				if (_tcscmp(td->dto->utilizadores[indexUtilizador].carteiraAcoes[indexEA].nomeEmpresa, td->mensagem.nome) == 0) {
					// empresa j� existe na carteira de a��es do utilizador
					// aumenta a quantidade de a��es
					td->dto->utilizadores[indexUtilizador].carteiraAcoes[indexEA].quantidadeAcoes += td->mensagem.quantidade;
					// diminui a quantidade de a��es na empresa
					td->dto->dadosP->empresas[indexEmpresa].quantidadeAcoes -= td->mensagem.quantidade;
					// atualiza o saldo do utilizador
					td->dto->utilizadores[indexUtilizador].saldo -= totalCompra;
					empresaAcaoAtualizada = TRUE;
					break;
				}
			}
			// a empresa n�o existe na carteira de a��es do utilizador
			if(numEmpresasAcoes == TAM_MAX_EMPRESA_ACAO && !empresaAcaoAtualizada) {
				// carteira de a��es cheia
				LeaveCriticalSection(&td->dto->pSync->csUtilizadores);
				LeaveCriticalSection(&td->dto->pSync->csEmpresas);
				mensagem.sucesso = FALSE;
				BOOL fSucess = WriteFile(td->dto->hPipes[td->pipeIndex], &mensagem, sizeof(Mensagem), &bytesEscritos, NULL);
				if (!fSucess || !bytesEscritos)
					_tprintf_s(ERRO_ESCRITA_MSG);
				return;
			}

			// acrescentar uma EmpresaAcao � carteira de a��es do utilizador
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
	// ao receber um pedido de saldo devo passar por v�rios passos:
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
		// utilizador n�o existe
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