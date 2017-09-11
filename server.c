/* Copilar com:   gcc server.c -o server $(mysql_config --libs) -l json -ansi -Wall  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <json/json.h>
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

	json_object * jobj;
	socklen_t tamanho;
	struct sockaddr_in local, remoto;
	int socketLocal, socketNovo, ip, porta, tamMsg, ever = 1;
	char *msg, *acao, jobj_string[300], query[300], userLogin[15], passLogin[30], userEncontrado = 0;
	long id, id_dest;

	msg = malloc(MAX);
	acao = malloc(MAX);


	conexao = mysql_init(NULL);
	if(mysql_real_connect(conexao, MYSQL_SERVER_ADDR, MYSQL_SERVER_USER, MYSQL_SERVER_PASSWORD,MYSQL_SERVER_DB,0,NULL,0)){
		perror("Conexao MYSQL");
	}else{
		perror("Conexao MYSQL");
		exit(0);
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
		if((recv(socketNovo, jobj_string, 300, 0)) == SOCKET_ERROR){
			perror("socket recv");
			exit(0);
		}
		/*printf("%s\n", jobj_string);*/
		jobj = json_tokener_parse(jobj_string);
		json_object_object_foreach(jobj, key, val){
			/*printf("KEY: %s, VAL: %s\n", key, json_object_get_string(val));*/
			if(!strcmp(key, "acao")) strcpy(acao, json_object_get_string(val));

				if (!strcmp(acao, "login") || !strcmp(acao, "cadastrar")){
					if(!strcmp(key, "usuario")) sprintf(userLogin, "%s", json_object_get_string(val));
					if(!strcmp(key, "senha")) sprintf(passLogin, "%s", json_object_get_string(val));
				}else if (!strcmp(acao, "checkLastSeen") || !strcmp(acao, "updateLastSeen") || !strcmp(acao, "checkUserInfoConn") || !strcmp(acao, "checkPendingMsg")){
					id = atol(json_object_get_string(val));
				}else if (!strcmp(acao, "insertPendingMsg")){
					if(!strcmp(key, "userId")) id = atol(json_object_get_string(val));
					if(!strcmp(key, "userIdDest")) id_dest = atol(json_object_get_string(val));
					if(!strcmp(key, "msg")) strcpy(msg, json_object_get_string(val));
				}else if (!strcmp(acao, "editInfoConn")){
					if(!strcmp(key, "userId")) id = atol(json_object_get_string(val));
					if(!strcmp(key, "IP")) ip = atoi(json_object_get_string(val));
					if(!strcmp(key, "porta")) porta = atoi(json_object_get_string(val));
				}
		}
		/*printf("ACAO %s, usuario %s, senha %s\n", acao, userLogin, passLogin);*/
		if(!strcmp(acao, "login")){
			userEncontrado = false;
			printf("%s tentando login\n", inet_ntoa(remoto.sin_addr));
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
					sprintf(query, "UPDATE status SET last_seen = NOW() WHERE id = %d", atoi(row[1]));
					if(mysql_query(conexao, query)){
						fprintf(stderr, "%s\n", mysql_error(conexao));
						exit(1);
					}
					printf("%s conectou-se a partir de: %s\n", row[1], inet_ntoa(remoto.sin_addr));
					strcpy(jobj_string, "{ \"resposta\" : \"ok\", \"motivo\" : \"ok\" }");
					tamMsg = strlen(jobj_string);
					if((send(socketNovo, jobj_string, tamMsg, 0)) == SOCKET_ERROR){
						perror("socket send");
						exit(0);
					}
				}else{
					printf("%s falha conexao. \n\tMotivo: senha incorreta\n", inet_ntoa(remoto.sin_addr));
					strcpy(jobj_string, "{ \"resposta\" : \"erro\", \"motivo\" : \"Senha incorreta\" }");
					tamMsg = strlen(jobj_string);
					if((send(socketNovo, jobj_string, tamMsg, 0)) == SOCKET_ERROR){
						perror("socket send");
						exit(0);
					}
				}
			}
			if(!userEncontrado){
				printf("Usuario nao cadastrado\n");
				strcpy(jobj_string, "{ \"resposta\" : \"erro\", \"motivo\" : \"Usuario nao cadastrado\" }");
				tamMsg = strlen(jobj_string);
				if((send(socketNovo, jobj_string, tamMsg, 0)) == SOCKET_ERROR){
					perror("socket send");
					exit(0);
				}
			}
			mysql_free_result(res);
		}else 
		if(!strcmp(acao, "cadastrar")){
			printf("%s cadastrando-se\n", inet_ntoa(remoto.sin_addr));
			sprintf(query, "SELECT COUNT(*) FROM cadastro WHERE usuario = '%s'", userLogin);
			if(mysql_query(conexao, query)){
				fprintf(stderr, "%s\n", mysql_error(conexao));
				exit(1);
			}
			res = mysql_use_result(conexao);
			row = mysql_fetch_row(res);
			if (atoi(row[0]) > 0){
				userEncontrado = true;
				strcpy(jobj_string, "{ \"resposta\" : \"erro\", \"motivo\" : \"Usuario ja cadastrado\" }");
				tamMsg = strlen(jobj_string);
				if((send(socketNovo, jobj_string, tamMsg, 0)) == SOCKET_ERROR){
					perror("socket send");
					exit(0);
				}
			}
			mysql_free_result(res);
			if(!userEncontrado){
				sprintf(query, "INSERT INTO cadastro (usuario, senha) VALUES ('%s', '%s')", userLogin, passLogin); /* obs: a porra da senha vai em plain text mesmo, to nem ai */
				if(mysql_query(conexao, query)){
					fprintf(stderr, "%s\n", mysql_error(conexao));
					exit(1);
				}else{
					perror("Usuario cadastrado");
				}

				strcpy(jobj_string, "{ \"resposta\" : \"ok\", \"motivo\" : \"ok\" }");
				tamMsg = strlen(jobj_string);
				if((send(socketNovo, jobj_string, tamMsg, 0)) == SOCKET_ERROR){
					perror("socket send");
					exit(0);
				}
				id = mysql_insert_id(conexao);
				sprintf(query, "INSERT INTO status (id_usuario, last_seen) VALUES ('%lu', NOW() )", id);
				if(mysql_query(conexao, query)){
					fprintf(stderr, "Atualizacao status: %s\n", mysql_error(conexao));
					exit(1);
				}
				

			}
			userEncontrado = false;
		}else 
		if(!strcmp(acao, "listarUsuarios")){
			if(mysql_query(conexao, "SELECT id, usuario FROM cadastro")){
				fprintf(stderr, "%s\n", mysql_error(conexao));
				exit(1);
			}
			res = mysql_use_result(conexao);
			mysql_num_rows(res);
		}
	}
	
	free(msg);
	free(acao);

	return 0;
}
