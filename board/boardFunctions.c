#include "../Servidor/constantes.h"
#include "board.h"

int CompareEmpresa(const void* a, const void* b) {
    const Empresa* empresaA = (const Empresa*)a;
    const Empresa* empresaB = (const Empresa*)b;

    double valorMercadoA = empresaA->quantidadeAcoes * empresaA->valorAcao;
    double valorMercadoB = empresaB->quantidadeAcoes * empresaB->valorAcao;

    if (valorMercadoA > valorMercadoB) return -1;
    if (valorMercadoA < valorMercadoB) return 1;
    return 0;
}


void ExibirUltimaTransacao(DadosPartilhados* pDados) {
    _tprintf_s(INFO_ULTIMA_TRANSACAO, pDados->ultimaTransacao.nomeEmpresa,
        pDados->ultimaTransacao.quantidadeAcoes, pDados->ultimaTransacao.precoPorAcao);
}


void OrganizarEExibirEmpresas(DadosPartilhados* pDados, DWORD N) {
    if (pDados == NULL) {
        _tprintf_s(INFO_ERRO_DPARTILHADOS);
        return;
    }
    
    if(pDados->numEmpresas == 0) {
        system("cls");
		_tprintf_s(INFO_EMPRESAS_VAZIA);
		return;
	}
    
    Empresa empresasOrganizadas[TAM_MAX_EMPRESAS];

    //mutex - TODO
    // 
    // Copiar os dados para o array local
    CopyMemory(empresasOrganizadas, pDados->empresas, pDados->numEmpresas * sizeof(Empresa));

    // Ordenar o array local por valor de mercado decrescente
    qsort(empresasOrganizadas, pDados->numEmpresas, sizeof(Empresa), CompareEmpresa);

    // Exibir as N empresas mais valiosas
    system("cls");
    _tprintf_s(INFO_TOP_EMPRESAS, N);
    for (DWORD i = 0; i < N && i < pDados->numEmpresas; i++) {
        _tprintf_s(INFO_EMPRESA_ACOES),
            i + 1, empresasOrganizadas[i].nome, empresasOrganizadas[i].quantidadeAcoes,
            empresasOrganizadas[i].valorAcao, empresasOrganizadas[i].quantidadeAcoes * empresasOrganizadas[i].valorAcao);
    }
    ExibirUltimaTransacao(pDados);
}