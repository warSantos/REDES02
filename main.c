#include "headers/servidores.h"

void ajuda (){

    printf ("Ajuda -h.\n-s: opção de servidor.\n");
    printf ("-s 1: servidor sequencial.\n");
    printf ("-s 2: servidor paralelo.\n");
    printf ("-s 3: produtor consumidor.\n");
    printf ("-s 4: servidor com select.\n");
}

int main (int argc, char **argv){

    if (argc < 2){
        printf ("Por favor. informe os parâmetros.\n");
        return 1;
    }
    // Opção passada pelo usuário ao programa.
    int opcao;
    char *servidor;
	// Desativa as mensagens de erro da função getopt
	opterr = 0;
	while ((opcao = getopt (argc, argv, "hs:")) != -1){
		switch (opcao) {
			// Usuário passou a opção -y, habilita:
			case 's':
				servidor = optarg;
				break;
			// Usuário escolheu a opção -z, pegar o argumento
			case 'h':
                ajuda ();
				exit(0);
            default:
                ajuda ();
				break;
		}
    }
    if (servidor[0] < 49 || servidor[0] >   52){
        printf ("Valor %c está fora do espaço de opções.\nDigite novamente outro valor.\n",servidor[0]);
        return 1;
    }
    if (servidor[0] == '1'){
        servidor_sequencial ();
    }else if (servidor[0] == '2'){
        servidor_paralelo (); 
    }else if (servidor[0] == '3'){
        servidor_produtor_consumidor();
    }else {
        servidor_select ();
    }
    return 0;
}