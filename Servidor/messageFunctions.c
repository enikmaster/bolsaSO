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

void mensagemBuy() {
	// TODO: mensagemBuy
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