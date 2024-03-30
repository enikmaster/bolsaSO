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

    do {
        _tprintf_s(_T("Comando: "));
        _fgetts(comando, sizeof(comando), stdin);
        controlo = verificaComando(comando); // devolve o número do comando
        switch (controlo) {
        case 1: // comando addc
            _tprintf_s(_T("[INFO] Comando addc\n"));
            // addc(nomeEmpresa, numeroAcoes, precoAcao);
			break;
        case 2: // comando listc
			_tprintf_s(_T("[INFO] Comando listc\n"));
			// listc();
			break;
        case 3: // comando stock
            _tprintf_s(_T("[INFO] Comando stock\n"));
            // stock(nomeEmpresa, precoAcao);
            break;
        case 4: // comando users
			_tprintf_s(_T("[INFO] Comando users\n"));
			// users();
			break;
        case 5: // comando pause
            _tprintf_s(_T("[INFO] Comando pause\n"));
			// pause(numeroSegundos);
			break;
        case 6: // comando close
            _tprintf_s(_T("[INFO] Comando close\n"));
            // close();
            break;
        case 0:
        default:
            _tprintf_s(_T("[ERRO] Comando inválido\n"));
            break;
        }
    } while (true);

    return 0;
}