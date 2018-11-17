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
        //responde_cliente (socket_cliente);
        responde_cliente_paralelo ((void *)&socket_cliente);
    }
    // Encerrando socket de escuta do cliente.
    close (socket_escuta);
}

void responde_cliente_paralelo (void *args){
    
    // Aumentando o número de threads.
    pthread_mutex_lock (&threads_rodando_protect);
    threads_rodando++;
    pthread_mutex_unlock (&threads_rodando_protect);

    int socket_cliente = *(int *)args;
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
    
    // Diminuindo o número de threads.
    pthread_mutex_lock (&threads_rodando_protect);
    threads_rodando--;
    pthread_mutex_unlock (&threads_rodando_protect);

    // Liberando estruturas da conexão.
    close (socket_cliente);
    fclose (leitor);
    free(res->buffer_resposta);
    free(res);
}

void servidor_paralelo (){
    
    pthread_mutex_init (&threads_rodando_protect, NULL);

    int socket_escuta, socket_cliente, sizeSockaddr = sizeof(struct sockaddr_in);
    struct sockaddr_in servidor, cliente;

    socket_escuta = socket(AF_INET, SOCK_STREAM, 0);
    // Abrindo socket para ouvir solicitações de conexão.

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

    // Limitando o número de conexões.
    listen(socket_escuta, QTDE_CONEXOES);
    
    // Estruturas de thread.
    threads_rodando = 0;
    // Mantenha o servidor ativo.
    while (1){
        // Iniciando conexão com o cliente.
        socket_cliente = accept(socket_escuta, (struct sockaddr *) &cliente, (socklen_t *) &sizeSockaddr);
        if (pthread_create (&threads[threads_rodando],
            NULL, (void *)&responde_cliente_paralelo, (void *)&socket_cliente) < 0){
                perror("Create Thread Error");
            exit(1);
        }
        if (threads_rodando == QTDE_CONEXOES){
            sleep (0.5);
        }
    }
    // Encerrando socket de escuta do cliente.
    close (socket_escuta);
}

void servidor_produtor_consumidor (){

    int socket_escuta, socket_cliente, sizeSockaddr = sizeof(struct sockaddr_in);
    struct sockaddr_in servidor, cliente;

    socket_escuta = socket(AF_INET, SOCK_STREAM, 0);
    // Abrindo socket para ouvir solicitações de conexão.

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

    // Limitando o número de conexões.
    listen(socket_escuta, QTDE_CONEXOES);
    
    // Iniciando a fila de clientes.
    int i;
    //fila_requisicoes = malloc (sizeof(u_int32_t) * QTDE_CONEXOES);
    for (i = 0; i < QTDE_CONEXOES; i++) fila_requisicoes[i] = -1;

    // Estruturas de threads.
    pthread_mutex_init (&threads_rodando_protect, NULL);
    threads_rodando = 0;
    // Cria threads (Consumidores).
    for (i = 0; i < QTDE_CONEXOES; i++){
        if (pthread_create (&threads[threads_rodando],
            NULL, (void *)&consumidor, (void *)&socket_cliente) < 0){
                perror("Create Thread Error");
            exit(1);
        }
        pthread_mutex_init (&fila_requisicoes_protect[i], NULL);
    }
    // Mantenha o servidor ativo.
    while (1){
        // Iniciando conexão com o cliente.
        socket_cliente = accept(socket_escuta, (struct sockaddr *) &cliente, (socklen_t *) &sizeSockaddr);
        if (threads_rodando == QTDE_CONEXOES){ // Se não houver espaço no buffer.
            sleep (0.2);
        }else{ // Se houver espaço no buffer.
            for (i = 0; i < QTDE_CONEXOES; ++i){
                if (fila_requisicoes[i] == -1){
                    fila_requisicoes[i] = socket_cliente;
                }
            }
        }
    }
    // Encerrando socket de escuta do cliente.
    close (socket_escuta);
}

void consumidor (){
    int i;
    // Enquanto o servidor estiver ativo.
    while (1){
        for (i = 0; i < QTDE_CONEXOES; ++i){
            // Se estiver alguma requisição na fila para ser atendida.
            if (fila_requisicoes[i] != -1){
                if (!pthread_mutex_trylock (&fila_requisicoes_protect[i])){
                    responde_cliente_paralelo ((void *)&fila_requisicoes[i]);
                    // Marque a requisição como atendida.
                    fila_requisicoes[i] = -1;
                    pthread_mutex_unlock (&fila_requisicoes_protect[i]);
                }
            }
        }// Se não conseguiu nenhuma requisição para antender durma por 0.2 ms.
        sleep (0.2);
    }
}
