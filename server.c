/* Copilar com:   gcc server.c -o server $(mysql_config --libs)  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include "sockets.c"

#define true (1==1)
#define false !true
#define MAX 100
#define MYSQL_SERVER_ADDR "127.0.0.1"
#define MYSQL_SERVER_USER "messenger"
#define MYSQL_SERVER_PASSWORD "msg123"
#define MYSQL_SERVER_DB "messenger"

int main(){

	MYSQL *conexao;
	MYSQL_RES *res;
	MYSQL_ROW row;

	socklen_t tamanho;
	struct sockaddr_in local, remoto;
	int socketLocal, socketNovo, porta, ever = 1;
	char *msg, *comando, query[300], 	userLogin[15], passLogin[30], userEncontrado = 0;

	msg = malloc(MAX);
	comando = malloc(MAX);


	conexao = mysql_init(NULL);
	if(mysql_real_connect(conexao, MYSQL_SERVER_ADDR, MYSQL_SERVER_USER, MYSQL_SERVER_PASSWORD,MYSQL_SERVER_DB,0,NULL,0)){
		perror("Conexao MYSQL");
	}else{
		perror("Conexao MYSQL");
	}

	cria_socket(SERVER_ADDR, PORTA, &socketLocal, &local);
	if((bind(socketLocal, (struct sockaddr *)&local, sizeof(struct sockaddr))) == SOCKET_ERROR){
		perror("socket bind");
		exit(0);
	}
	if(listen(socketLocal, 20) == SOCKET_ERROR){
		perror("socket listen");
		exit(0);
	}
	printf("Servidor iniciado!\nAguardando conexão de usuarios...\n");
	
	for(;ever;){
		socketNovo = accept(socketLocal, (struct sockaddr *)&remoto, &tamanho);
		if(socketNovo == SOCKET_ERROR){
			perror("accept");
			exit(0);
		}
		if((recv(socketNovo, msg, MAX, 0)) == SOCKET_ERROR){
			perror("socket recv");
			exit(0);
		}
		sscanf(msg, "%s", comando); /* pega primeira palavra da string msg */
		if(!strcmp(comando, "checaLogin")){
			userEncontrado = false;
			printf("%s tentando login\n", inet_ntoa(remoto.sin_addr));
			sscanf(msg, "%*s %s", userLogin);
			sscanf(msg, "%*s %*s %s", passLogin);
			sprintf(query, "SELECT * FROM cadastro WHERE usuario = '%s'", userLogin);
			if(mysql_query(conexao, query)){
				fprintf(stderr, "%s\n", mysql_error(conexao));
				exit(1);
			}
			res = mysql_use_result(conexao);
			row = mysql_fetch_row(res);
			if (mysql_num_rows(res) > 0){
				userEncontrado = true;
				if (!strcmp(row[2], passLogin)){
					printf("%s conectou-se a partir de: %s\n", row[1], inet_ntoa(remoto.sin_addr));
					if((send(socketNovo, "ok: Senha Correta!", 14, 0)) == SOCKET_ERROR){
						perror("socket send");
						exit(0);
					}
				}else{
					printf("%s falha conexao. \n\tMotivo: senha incorreta\n", inet_ntoa(remoto.sin_addr));
					if((send(socketNovo, "erro: Senha Incorreta!", 22, 0)) == SOCKET_ERROR){
						perror("socket send");
						exit(0);
					}
				}
			}
			if(!userEncontrado){
				printf("Usuario nao cadastrado\n");
				if((send(socketNovo, "erro: Usuario nao cadastrado", 28, 0)) == SOCKET_ERROR){
					perror("socket send");
					exit(0);
				}
			}
			mysql_free_result(res);
		}else 
		if(!strcmp(comando, "cadastrar:")){
			printf("entrou cadastrar\n");
			sscanf(msg, "%*s %s", userLogin);
			printf("ps 1 sscanf\n");
			sscanf(msg, "%*s %*s %s", passLogin);
			printf("ps 2 sscanf\n");
			sscanf(msg, "%*s %*s %*s %d", &porta);
			printf("pre cadastrar\n");
			printf("%s\n", passLogin);
			sprintf(query, "SELECT COUNT(*) FROM cadastro WHERE usuario = '%s'", userLogin);
			if(mysql_query(conexao, query)){
				fprintf(stderr, "%s\n", mysql_error(conexao));
				exit(1);
			}
			res = mysql_use_result(conexao);
			if (mysql_fetch_row(res)){
				userEncontrado = true;
				if((send(socketNovo, "erro: Usuario ja cadastrado", 28, 0)) == SOCKET_ERROR){
					perror("socket send");
					exit(0);
				}
			}
			if(!userEncontrado){
				sprintf(query, "INSERT INTO cadastro (usuario, senha) VALUES ('%s', '%s')", userLogin, passLogin);
				if(mysql_query(conexao, query)){
					fprintf(stderr, "%s\n", mysql_error(conexao));
					exit(1);
				}else{
					perror("Usuario cadastrado");
				}
				if((send(socketNovo, "ok: Usuario cadastrado", 28, 0)) == SOCKET_ERROR){
					perror("socket send");
					exit(0);
				}
			}
		}
	}
	return 0;
}
