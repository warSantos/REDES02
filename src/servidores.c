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

int criar_socket_escuta (int qtde_con){
    
    int socket_escuta, yes = 1;
    struct sockaddr_in servidor;

    // Abrindo socket para ouvir solicitações de conexão.
    socket_escuta = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_escuta == -1){
        perror("erro");
        exit (1);
    }
    // Verificando se o socket ja esta em uso.
    if (setsockopt (socket_escuta, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
        perror ("setsocket");
        exit (1);
    }
    // Configurando estrutura do servidor.
    servidor.sin_family = AF_INET; // Atribuindo a familia de protocolos para Internet.
    servidor.sin_addr.s_addr = htonl(INADDR_ANY); // Recebendo conexões de qualquer endereço.
    servidor.sin_port = htons(PORT); // Setando e porta em que rodara o processo.
    memset(servidor.sin_zero, 0, sizeof servidor.sin_zero);
    // Criando link entre a estrutura servidor ao ID do socket.
    if(bind(socket_escuta, (struct sockaddr *) &servidor, sizeof(servidor)) < 0){        
        close(socket_escuta);
        perror("bind");
        exit(1);
    }
    // Limitando o número de conexões a uma conexão por vez.
    if (listen(socket_escuta, qtde_con) < 0){
        perror ("listen");
        exit(1);
    }
    return socket_escuta;
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
    struct sockaddr_in cliente;
    socket_escuta = criar_socket_escuta (1);
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

void responde_cliente_paralelo (void *args){
    
    // Aumentando o número de threads.
    pthread_mutex_lock (&threads_rodando_protect);
    threads_rodando++;
    pthread_mutex_unlock (&threads_rodando_protect);
    printf ("Respondendo cliente.\n");
    int socket_cliente = *(int *)args;
    // Recebendo cabeçalho do cliente de requisicao.
    //Requisicao *req = ler_cabecalho (socket_cliente);
    // Declarando estruturas.
    Resposta *res = malloc (sizeof (Resposta));
    res->buffer_resposta = malloc (2048);
    char str[2031];
    FILE *leitor = fopen ("www/index.html", "r");
    if (leitor == NULL){
        perror ("erro");
    }
    //Configurando o protocolo.
    sprintf (res->buffer_resposta, "HTTP/1.0 200 OK\r\n\r\n");

    // Lendo o arquivo solicitado.
    fseek (leitor, 0, SEEK_END);
    u_int32_t tamanho_arquivo = ftell(leitor);
    rewind (leitor);
    fread (str, 1, tamanho_arquivo, leitor);
    sprintf (res->buffer_resposta + 19, "%s", str);
    // Escrevendo resposta para o cliente.
    write (socket_cliente, res->buffer_resposta, strlen(str) + 19);
    
    // Diminuindo o número de threads.
    pthread_mutex_lock (&threads_rodando_protect);
    threads_rodando--;
    pthread_mutex_unlock (&threads_rodando_protect);

    // Liberando estruturas da conexão.
    fclose (leitor);
    free(res->buffer_resposta);
    free(res);
    close (socket_cliente);
    free (args);
}

void servidor_paralelo (){
    
    pthread_mutex_init (&threads_rodando_protect, NULL);

    int socket_escuta, *socket_cliente;
    int sizeSockaddr = sizeof(struct sockaddr_in);
    struct sockaddr_in cliente;
    socket_escuta = criar_socket_escuta (QTDE_CONEXOES);
    // Estruturas de thread.
    threads_rodando = 0;
    // Mantenha o servidor ativo.
    while (1){
        // Iniciando conexão com o cliente.
        socket_cliente = malloc (sizeof(int));
        socket_cliente[0] = accept(socket_escuta, (struct sockaddr *) &cliente, (socklen_t *) &sizeSockaddr);
        if (pthread_create (&threads[threads_rodando],
            NULL, (void *)&responde_cliente_paralelo, (void *)&socket_cliente[0]) < 0){
                perror("create thread error");
            exit(1);
        }
        if (threads_rodando == QTDE_CONEXOES){
            sleep (0.2);
        }
    }
    // Encerrando socket de escuta do servidor.
    close (socket_escuta);
}

void servidor_produtor_consumidor (){

    int socket_escuta, socket_cliente;
    int sizeSockaddr = sizeof(struct sockaddr_in);
    struct sockaddr_in cliente;
    socket_escuta = criar_socket_escuta (QTDE_CONEXOES);
    
    // Iniciando a fila de clientes.
    int i;
    for (i = 0; i < QTDE_CONEXOES; i++) fila_requisicoes[i] = -1;

    // Estruturas de threads.
    pthread_mutex_init (&threads_rodando_protect, NULL);
    threads_rodando = 0;
    // Cria threads (Consumidores).
    for (i = 0; i < QTDE_CONEXOES; i++){
        pthread_mutex_init (&fila_requisicoes_protect[i], NULL);
        if (pthread_create (&threads[threads_rodando],
            NULL, (void *)&consumidor, NULL) < 0){
                perror("create thread error");
            exit(1);
        }        
    }
    // Mantenha o servidor ativo.
    while (1){
        // Iniciando conexão com o cliente.
        socket_cliente = accept(socket_escuta, (struct sockaddr *) &cliente, (socklen_t *) &sizeSockaddr);
        if (threads_rodando == QTDE_CONEXOES){ // Se não houver espaço no buffer.
            sleep (0.1);
        }else{ // Se houver espaço no buffer.
            for (i = 0; i < QTDE_CONEXOES; ++i){
                // Se esta posição no buffer ja estiver sido atendida.
                if (fila_requisicoes[i] == -1){
                    // Se esta posição não estiver fechada.
                    if (!pthread_mutex_trylock(&fila_requisicoes_protect[i])){
                        fila_requisicoes[i] = socket_cliente;
                        pthread_mutex_unlock(&fila_requisicoes_protect[i]);
                        break;
                    }
                }
            }
        }
    }
    // Liberando estruturas.
    pthread_mutex_destroy (&threads_rodando_protect);
    for (i = 0; i < QTDE_CONEXOES; ++i){
        pthread_mutex_destroy (&fila_requisicoes_protect[i]);
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
            if (!pthread_mutex_trylock (&fila_requisicoes_protect[i])){
                if (fila_requisicoes[i] != -1){
                    responde_cliente (fila_requisicoes[i]);
                    // Marque a requisição como atendida.
                    fila_requisicoes[i] = -1;
                }
                pthread_mutex_unlock (&fila_requisicoes_protect[i]);
            }
        }// Se não conseguiu nenhuma requisição para antender durma por 0.2 ms.
        sleep (0.1);
    }
}

void servidor_select (){

    int socket_escuta, socket_cliente;
    int max_socket, i, atividade;
    int size_sockaddr = sizeof (struct sockaddr_in);
    struct sockaddr_in cliente;
    fd_set read_fds, master;
    FD_ZERO (&master);
    FD_ZERO (&read_fds);

    socket_escuta = criar_socket_escuta (QTDE_CONEXOES);
    // Adicionando o socket_escuta ao set master.
    FD_SET(socket_escuta, &master);
    // Atualizando o valor máximo do conjunto de scokets como o socket do accept.
    max_socket = socket_escuta;
    // Enquanto o servidor estiver ativo.
    while (1){
        read_fds = master;
        // Esperando por algum novo cliente.
        atividade = select (max_socket + 1, &read_fds, NULL, NULL, NULL);
        if (atividade < 0){
            perror ("select");
            exit(1);
        }
        for (i = 0; i <= max_socket; ++i){
            // Se algum dado foi escrito no socket i.
            if (FD_ISSET (i, &read_fds)){
                // Se i for o socket de escuta para receber novos clientes.
                if (i == socket_escuta){
                    socket_cliente = accept (socket_escuta, 
                        (struct sockaddr *)&cliente, (socklen_t *)&size_sockaddr);
                    if (socket_cliente == -1){
                        perror ("accept");
                        exit (1);
                    }else{ // Se a conexão for aceita sem erros.
                        FD_SET (socket_cliente, &master);
                        if (socket_cliente > max_socket){
                            max_socket = socket_cliente;
                        }
                    }
                }else { // Se for o cabeçalho do cliente de algum cliente.
                    responde_cliente (i);
                    FD_CLR (i, &master);
                }
            }
        }
    }
}