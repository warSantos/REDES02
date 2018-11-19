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
#define QTDE_CONEXOES 5
#define BUFFER_SIZE 256
#define HEADER_SIZE 2048
#define URL_SIZE 256
#define ELDER 404

// ESTRUTURAS PTHREAD
pthread_t threads[QTDE_CONEXOES];
u_int32_t threads_rodando;
pthread_mutex_t threads_rodando_protect;
pthread_mutex_t fila_requisicoes_protect[QTDE_CONEXOES];

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

Requisicao *ler_cabecalho (int socket_cliente);

void servidor_sequencial ();

void responde_cliente_paralelo (void *args);

void servidor_paralelo ();

void servidor_produtor_consumidor ();

void consumidor ();

void servidor_select ();

#endif
