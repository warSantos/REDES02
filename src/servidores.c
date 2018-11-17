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

void responde_cliente (Requisicao *req, int socket_cliente){


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
        // Recebendo cabeçalho do cliente de requisicao.
        Requisicao *req = ler_cabecalho (socket_cliente);
        // Respondendo cliente.
        responde_cliente (req, socket_cliente);
        // Encerrando conexão.
        close (socket_cliente);
    }
    // Encerrando socket de escuta do cliente.
    close (socket_escuta);

}