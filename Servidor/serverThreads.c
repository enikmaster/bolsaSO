#include "constantes.h"
#include "servidor.h"

// fun��es das threads
void WINAPI threadComandos(PVOID p) {
	DadosPartilhados* sharedData = (DadosPartilhados *)p;
}

void WINAPI threadReceberMensagens(PVOID p) {
	DadosPartilhados* sharedData = (DadosPartilhados*)p;
}

void WINAPI threadEnviarMensagens(PVOID p) {
	DadosPartilhados* sharedData = (DadosPartilhados*)p;
}