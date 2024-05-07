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
    _tprintf(_T("Ultima Transação: %s - %lu e %.2f\n"), pDados->ultimaTransacao.nomeEmpresa,
        pDados->ultimaTransacao.quantidadeAcoes, pDados->ultimaTransacao.precoPorAcao);
}


void OrganizarEExibirEmpresas(DadosPartilhados* pDados, DWORD N) {
    if (pDados == NULL) {
        _tprintf(_T("Erro: A estrutura DadosPartilhados está vazia\n"));
        return;
    }

    
    Empresa empresasOrganizadas[TAM_MAX_EMPRESAS];

    // Copiar os dados para o array local
    CopyMemory(empresasOrganizadas, pDados->empresas, pDados->numEmpresas * sizeof(Empresa));

    // Ordenar o array local por valor de mercado decrescente
    qsort(empresasOrganizadas, pDados->numEmpresas, sizeof(Empresa), CompareEmpresa);

    // Exibir as N empresas mais valiosas
    system("cls");
    _tprintf(_T("Top %d Empresas Mais Valiosas:\n"), N);
    for (DWORD i = 0; i < N && i < pDados->numEmpresas; i++) {
        _tprintf(_T("%d. %s - Ações: %lu, Valor por Ação: $%.2f, Valor de Mercado: $%.2f\n"),
            i + 1, empresasOrganizadas[i].nome, empresasOrganizadas[i].quantidadeAcoes,
            empresasOrganizadas[i].valorAcao, empresasOrganizadas[i].quantidadeAcoes * empresasOrganizadas[i].valorAcao);
    }
    ExibirUltimaTransacao(pDados);
}