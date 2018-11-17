#ifndef servidores_h
#define servidores_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include <netinet/in.h> //inet_addr
#include <pthread.h>
#include <unistd.h>

#define PORT 40000
#define QTDE_CONEXOES 10
#define HEADER_SIZE 2048

// ESTRUTURAS PTHREAD
pthread_t threads[10];
u_int32_t threads_rodando;

typedef struct {

    int bytes_lidos;
    char cabecalho[HEADER_SIZE];
} Requisicao;


typedef struct{
    char *buffer_resposta;
} Resposta;

Requisicao *ler_cabecalho (int socket_cliente);

void servidor_sequencial ();

#endif
