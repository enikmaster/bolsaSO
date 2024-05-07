#pragma once

void OrganizarEmpresas(DadosPartilhados* pDados, Empresa empresasOrganizadas[], DWORD maxEmpresas);
int CompareEmpresa(const void* a, const void* b);


//atualizar
void OrganizarEExibirEmpresas(DadosPartilhados* pDados, DWORD N);
void ExibirUltimaTransacao(DadosPartilhados* pDados);