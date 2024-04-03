#include "servidor.h"
#include "constantes.h"

int _tmain(int argc, TCHAR** argv)
{
#ifdef UNICODE 
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
    if (argc != 1) {
        _tprintf_s(_T("[ERRO] Número de argumentos inválido\n"));
        return(1);
    }
    
    // TODO: ler o ficheiro dos users

    // TODO: criar um array de empresas

    // TODO: criar threads para as diferente funcionalidades necessárias

    DWORD controlo = 0;
    TCHAR comando[TAM_COMANDO];
    TCHAR comandoTemp[TAM_COMANDO];
    TCHAR argumento1[TAM_COMANDO];
    TCHAR argumento2[TAM_COMANDO];
    TCHAR argumento3[TAM_COMANDO];
    TCHAR failSafe[TAM_COMANDO];
    DWORD repetir = 0;
    do {
        memset(comandoTemp, 0, sizeof(comando));
        memset(argumento1, 0, sizeof(argumento1));
        memset(argumento2, 0, sizeof(argumento2));
        memset(argumento3, 0, sizeof(argumento3));
        memset(failSafe, 0, sizeof(failSafe));
        _tprintf_s(_T("Comando: "));
        _fgetts(comando, sizeof(comando)/sizeof(comando[0]), stdin);
        comando[_tcslen(comando) - 1] = _T('\0');
        controlo = verificaComando(comando); // devolve o número do comando

        switch (controlo) {
        case 1: // comando addc
            _tprintf_s(_T("[INFO] Comando addc\n"));
            // TODO: ler os argumentos do comando
            int numArgumentos = _stscanf_s(
				comando,
				_T("%s %s %s %s %s"),
                comandoTemp, TAM_COMANDO,
				argumento1, TAM_COMANDO,
				argumento2, TAM_COMANDO,
				argumento3, TAM_COMANDO,
                failSafe, TAM_COMANDO);
            if(numArgumentos != 4) {
                _tprintf_s(_T("[ERRO] Número de argumentos inválido\n"));
				break;
            }
            else {
                DWORD numeroAcoes = _tstoi(argumento2);
                double precoAcao = _tstof(argumento3);
                comandoAddc(argumento1, numeroAcoes, precoAcao);
                _tprintf_s(_T("Empresa: %s\nN_Ações: %d\nPreço: %f\n"), argumento1, numeroAcoes, precoAcao);
            }
            
			break;
        case 2: // comando listc
			_tprintf_s(_T("[INFO] Comando listc\n"));
			// comandoListc();
			break;
        case 3: // comando stock
            _tprintf_s(_T("[INFO] Comando stock\n"));
            // TODO: ler os argumentos do comando
            // comandoStock(nomeEmpresa, precoAcao);
            break;
        case 4: // comando users
			_tprintf_s(_T("[INFO] Comando users\n"));
			// comandoUsers();
			break;
        case 5: // comando pause
            _tprintf_s(_T("[INFO] Comando pause\n"));
            // TODO: ler os argumentos do comando
			// comandoPause(numeroSegundos);
			break;
        case 6: // comando close
            _tprintf_s(_T("[INFO] Comando close\n"));
            // comandoClose();
            repetir = 1;
            break;
        case 0:
        default:
            _tprintf_s(_T("[ERRO] Comando inválido\n"));
            break;
        }
    } while (repetir == 0);

    return 0;
}