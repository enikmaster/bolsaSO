#include "constantes.h"
#include "servidor.h"

// funções das threads
void WINAPI threadComandos(PVOID p) {
	DadosPartilhados* sharedData = (DadosPartilhados *)p;
}

void WINAPI threadReceberMensagens(PVOID p) {
	DadosPartilhados* sharedData = (DadosPartilhados*)p;
}

void WINAPI threadEnviarMensagens(PVOID p) {
	DadosPartilhados* sharedData = (DadosPartilhados*)p;
}