#include "../headers/servidores.h"

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

Requisicao *ler_cabecalho (int socket_cliente){

    Requisicao *req = malloc (sizeof(Requisicao));
    req->bytes_lidos = 0;
    req->bytes_lidos = read (socket_cliente, req->cabecalho, HEADER_SIZE);
    return req;
}

Resposta *carregar_arquivo (char *url){

    // Estrutura para receber o arquivo.
    Resposta *arquivo = malloc (sizeof (Resposta));
    FILE *leitor = fopen (url, "r");
    // Se o arquivo não for encontrado.
    if (leitor == NULL){
        perror ("Not found");
        arquivo->status = -1;
        // Lendo o arquivo 404.
        leitor = fopen ("www/not_found.html", "r");
        fseek (leitor, 0, SEEK_END);
        // Recebendo o tamanho do arquivo 404.
        arquivo->tamanho_mensagem = ftell(leitor);
        arquivo->buffer_resposta = malloc (arquivo->tamanho_mensagem);
        rewind (leitor);
        // Carregando o arquivo 404 para memória.
        fread (arquivo->buffer_resposta, 1, arquivo->tamanho_mensagem, leitor);
        fclose (leitor);
        return arquivo;
    }
    // Lendo o arquivo solicitado.
    fseek (leitor, 0, SEEK_END);
    // Recebendo o tamanho do arquivo.
    arquivo->tamanho_mensagem = ftell(leitor);
    arquivo->buffer_resposta = malloc (arquivo->tamanho_mensagem);
    // Se não existir memória suficiente para o arquivo.
    if (arquivo->buffer_resposta == NULL){
        perror ("Payload");
        arquivo->status = -2;
        // Fechando o file descriptor.
        fclose (leitor);
        // Lendo o arquivo 413.
        leitor = fopen ("www/payload.html", "r");
        fseek (leitor, 0, SEEK_END);
        // Recebendo o tamanho do arquivo 413.
        arquivo->tamanho_mensagem = ftell(leitor);
        arquivo->buffer_resposta = malloc (arquivo->tamanho_mensagem);
        rewind (leitor);
        // Carregando o arquivo 413 para memória.
        fread (arquivo->buffer_resposta, 1, arquivo->tamanho_mensagem, leitor);
        fclose (leitor);
        return arquivo;
    }
    rewind (leitor);
    // Carregando o arquivo.
    fread (arquivo->buffer_resposta, 1, arquivo->tamanho_mensagem, leitor);
    fclose (leitor);
    arquivo->status = 0;
    return arquivo;
}

Resposta *parser_http_header (Requisicao *req){

    // Estrutura para receber o arquivo.
    Resposta *arquivo, *res = malloc (sizeof(Resposta));
    // Definindo estruturas da mensagem.
    char url[URL_SIZE] = {0};
    char header[100] = {0};
    int i, j = 0;
    int header_size;
    // Copiando url do cabeçalho.
    for (i = 0; req->cabecalho[i] != ' '; i++);
    i++;
    for (; req->cabecalho[i] != ' '; i++){
        if (j > 0 || req->cabecalho[i] != '/'){
            url[j] = req->cabecalho[i];
            j++;
        }
    }
    /* Tentando abrir o arquivo. */
    /*struct stat fd;
    stat (url, &fd);
    S_ISDIR(fd.st_mode)
    */
    printf ("URL: %s\n'''''''''''''''''''''\n", url);
    DIR *d = opendir(url);
    // Se o arquivo solicitado for o index ou um diretório/arquivo não regular.
    if ((url[0] == '/' && url[1] == 0) || url[0] == 0 || d != NULL){
        closedir (d);
        printf ("Retorna a barra.\n");
        arquivo = carregar_arquivo ("www/index.html");
    }else{
        printf ("Retorna arquivo.\n");
        // Se a url não tiver o prefixo www/.
        if(strncmp ("www/", url, 4)){
            char temp[URL_SIZE] = {0};
            sprintf (temp, "www/%s", url);
            arquivo = carregar_arquivo (temp);
        }else {
            arquivo = carregar_arquivo (url);
        }
    }
    // Se o arquivo não foi encontrado: 404.
    if (arquivo->status == -1){ 
        sprintf (header, "HTTP/1.0 404 Not Found\nContent-Lenght: %d\r\n\r\n", arquivo->tamanho_mensagem);
        
    }// Se não foi possível alcoar memória para o arquivo: 413.
    else if (arquivo->status == -2){
        sprintf (header, "HTTP/1.0 413 Payload Too Large\nContent-Lenght: %d\r\n\r\n", arquivo->tamanho_mensagem);
    }// Se foi possível carregar o arquivo.
    else {
        sprintf (header, "HTTP/1.0 200 OK\nContent-Lenght: %d\r\n\r\n", arquivo->tamanho_mensagem);
    }
    header_size = strlen (header);
    res->buffer_resposta = malloc (arquivo->tamanho_mensagem + header_size);
    memcpy (res->buffer_resposta, header, header_size);
    memcpy (res->buffer_resposta + header_size, arquivo->buffer_resposta, arquivo->tamanho_mensagem);
    res->tamanho_mensagem = header_size + arquivo->tamanho_mensagem;
    free (arquivo);
    return res;
}

void responde_cliente (int socket_cliente){

    // Recebendo cabeçalho do cliente de requisicao.
    Requisicao *req = ler_cabecalho (socket_cliente);
    // Declarando estruturas.
    Resposta *res = parser_http_header (req);
    write (socket_cliente, res->buffer_resposta, res->tamanho_mensagem);
    // Liberando estruturas da conexão.
    close (socket_cliente);
    free (req);
    free (res->buffer_resposta);
    free (res);
}

void responde_cliente_paralelo (void *args){

    int socket_cliente = *(int *)args;
    responde_cliente (socket_cliente);
    free (args);
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