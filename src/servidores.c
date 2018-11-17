#include "../headers/servidores.h"

Requisicao *ler_cabecalho (int socket_cliente){

    Requisicao *req = malloc (sizeof(Requisicao));
    req->bytes_lidos = 0;
    req->bytes_lidos = read (socket_cliente, req->cabecalho, HEADER_SIZE);
    printf ("%s.\n", req->cabecalho);    
    return req;
}

void parser_http_header (Requisicao *req){


}

void responde_cliente (int socket_cliente){

    // Recebendo cabeçalho do cliente de requisicao.
    //Requisicao *req = ler_cabecalho (socket_cliente);
    // Declarando estruturas.
    Resposta *res = malloc (sizeof (Resposta));
    res->buffer_resposta = malloc (2048);
    char str[2031];
    FILE *leitor = fopen ("www/index.html", "r");
    if (leitor == NULL){
        perror ("Erro");
    }
    //Configurando o protocolo.
    sprintf (res->buffer_resposta, "HTTP/1.0 200 OK\n\n");

    // Lendo o arquivo solicitado.
    fseek (leitor, 0, SEEK_END);
    u_int32_t tamanho_arquivo = ftell(leitor);
    rewind (leitor);
    fread (str, 1, tamanho_arquivo, leitor);
    sprintf (res->buffer_resposta+17, "%s", str);
    // Escrevendo resposta para o cliente.
    write (socket_cliente, res->buffer_resposta, strlen(str) + 17);
    // Liberando estruturas da conexão.
    close (socket_cliente);
    fclose (leitor);
    free(res->buffer_resposta);
    free(res);
}

void servidor_sequencial (){

    int socket_escuta, socket_cliente, sizeSockaddr = sizeof(struct sockaddr_in);
    struct sockaddr_in servidor, cliente;

    // Abrindo socket para ouvir solicitações de conexão.
    socket_escuta = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_escuta == -1){
        perror("Erro");
        exit (1);
    }

    servidor.sin_family = AF_INET; // Atribuindo a familia de protocolos para Internet.
    servidor.sin_addr.s_addr = htonl(INADDR_ANY); // Recebendo conexões de qualquer endereço.
    servidor.sin_port = htons(PORT); // Setando e porta em que rodara o processo.

    memset(servidor.sin_zero, 0, sizeof servidor.sin_zero);

    // Criando link entre a estrutura servidor ao ID do socket.
    if(bind(socket_escuta, (struct sockaddr *) &servidor, sizeof(servidor)) < 0){        
        close(socket_escuta);
        perror("Erro");
        exit(1);
    }

    // Limitando o número de conexões a uma conexão por vez.
    listen(socket_escuta, 1);
    
    // Mantenha o servidor ativo.
    while (1){
        // Iniciando conexão com o cliente.
        socket_cliente = accept(socket_escuta, (struct sockaddr *) &cliente, (socklen_t *) &sizeSockaddr);
        // Respondendo cliente.
        responde_cliente (socket_cliente);
    }
    // Encerrando socket de escuta do cliente.
    close (socket_escuta);
}

void servidor_paralelo (){
    int socket_escuta, socket_cliente, sizeSockaddr = sizeof(struct sockaddr_in);
    struct sockaddr_in servidor, cliente;

    // Abrindo socket para ouvir solicitações de conexão.
    socket_escuta = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_escuta == -1){
        perror("Erro");
        exit (1);
    }

    servidor.sin_family = AF_INET; // Atribuindo a familia de protocolos para Internet.
    servidor.sin_addr.s_addr = htonl(INADDR_ANY); // Recebendo conexões de qualquer endereço.
    servidor.sin_port = htons(PORT); // Setando e porta em que rodara o processo.

    memset(servidor.sin_zero, 0, sizeof servidor.sin_zero);

    // Criando link entre a estrutura servidor ao ID do socket.
    if(bind(socket_escuta, (struct sockaddr *) &servidor, sizeof(servidor)) < 0){        
        close(socket_escuta);
        perror("Erro");
        exit(1);
    }

    // Limitando o número de 10 conexões por vez.
    listen(socket_escuta, 20);
    
    // Estruturas de thread.
    threads_rodando = 0;
    u_int32_t limite_threads = 20;
    // Mantenha o servidor ativo.
    while (1){
        // Iniciando conexão com o cliente.
        socket_cliente = accept(socket_escuta, (struct sockaddr *) &cliente, (socklen_t *) &sizeSockaddr);
        if (pthread_create (&threads[threads_rodando], 
            NULL, &responde_cliente, (void *)&socket_cliente) < 0){
                perror("Create Thread Error");
            exit(1);
        }
    }
    // Encerrando socket de escuta do cliente.
    close (socket_escuta);
}