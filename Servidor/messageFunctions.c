#include "servidor.h"

// funções de tratamento de mensagens
void mensagemLogin(ThreadData* td) {
	pUtilizador uLocais = (pUtilizador)malloc(TAM_MAX_USERS * sizeof(Utilizador));
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
	free(uLocais);
}

void mensagemListc(DataTransferObject* dto, DWORD pipeIndex) {
	Mensagem resposta = { 0 };
	resposta.TipoM = TMensagem_R_LISTC;
	EnterCriticalSection(&dto->pSync->csEmpresas);
	CopyMemory(resposta.empresas, dto->dadosP->empresas, sizeof(Empresa) * dto->dadosP->numEmpresas);
	LeaveCriticalSection(&dto->pSync->csEmpresas);
	DWORD bytesEscritos;
	BOOL fSuccess = WriteFile(dto->hPipes[pipeIndex], &resposta, sizeof(Mensagem), &bytesEscritos, NULL);
	if (!fSuccess || bytesEscritos == 0)
		_tprintf_s(ERRO_ESCRITA_MSG);
}

void mensagemBuy(ThreadData* td) {
	DWORD numEmpresas, numUtilizadores;
	Mensagem mensagem = { 0 };
	mensagem.TipoM = TMensagem_R_BUY;
	DWORD bytesEscritos;
	DWORD indexEmpresa = 0;
	// 1. Verificar se a empresa existe
	EnterCriticalSection(&td->dto->pSync->csEmpresas);
		for (indexEmpresa; indexEmpresa < td->dto->dadosP->numEmpresas; ++indexEmpresa) {
			if (_tcscmp(td->mensagem.nome, td->dto->dadosP->empresas[indexEmpresa].nome) == 0) {
				break;
			}
		}
		// a empresa não existe
		if (indexEmpresa == td->dto->dadosP->numEmpresas) {
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
			for (indexUtilizador; indexUtilizador < numUtilizadores; ++indexUtilizador) {
				if (_tcscmp(td->mensagem.nome, td->dto->utilizadores[indexUtilizador].username) == 0) {
					break;
				}
			}
			// o utilizador não existe
			if(indexUtilizador == td->dto->numUtilizadores) {
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
			if (td->mensagem.quantidade >= &td->dto->dadosP->empresas[indexEmpresa]) {
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
			if (totalCompra >= td->dto->utilizadores[indexUtilizador].saldo) {
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
			for (DWORD i = 0; i < numEmpresasAcoes; ++i) {
				if (_tcscmp(td->dto->utilizadores[indexUtilizador].carteiraAcoes[i].nomeEmpresa, td->mensagem.nome) == 0) {
					// empresa já existe na carteira de ações do utilizador
					// aumenta a quantidade de ações
					td->dto->utilizadores[indexUtilizador].carteiraAcoes[i].quantidadeAcoes += td->mensagem.quantidade;
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
			td->dto->utilizadores[indexUtilizador].carteiraAcoes[numEmpresasAcoes].quantidadeAcoes = td->mensagem.quantidade;
			_tcscpy_s(td->dto->utilizadores[indexUtilizador].carteiraAcoes[numEmpresasAcoes].nomeEmpresa, TAM_NOME, td->mensagem.nome);
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
		_tprintf_s(ERRO_ESCRITA_MSG);
}

void mensagemSell() {
	// TODO: mensagemSell
}

void mensagemBalance() {
	// TODO: mensagemBalance
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