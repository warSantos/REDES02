#ifndef servidores_h
#define servidores_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <netinet/in.h> //inet_addr
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#define PORT 40000
#define QTDE_CONEXOES 32
#define BUFFER_SIZE 128
#define HEADER_SIZE 2048
#define URL_SIZE 256

// ESTRUTURAS PTHREAD
pthread_t threads[QTDE_CONEXOES];
u_int32_t qtde_requisicoes;
pthread_mutex_t qtde_requisicoes_protect;
pthread_mutex_t fila_requisicoes_protect[BUFFER_SIZE];

// Modelo cliente servidor.
u_int32_t fila_requisicoes[BUFFER_SIZE];

typedef struct {
    int bytes_lidos;
    char cabecalho[HEADER_SIZE];
} Requisicao;


typedef struct{
    u_int32_t tamanho_mensagem;
    u_int8_t status;
    char *buffer_resposta;
} Resposta;

int criar_socket_escuta (int qtde_con);

Requisicao *ler_cabecalho (int socket_cliente);

Resposta *carregar_arquivo (char *url);

Resposta *parser_http_header (Requisicao *req);

void servidor_sequencial (void);

void responde_cliente (int socket_cliente);

void responde_cliente_thread (void *args);

void servidor_paralelo (void);

void servidor_produtor_consumidor (void);

void consumidor (void);

void servidor_select (void);

#endif
