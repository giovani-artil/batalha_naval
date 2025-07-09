
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include "../common/protocol.h"

#define PORT 8080

int main() {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in serv_addr;
        char buffer[MAX_MSG] = {0};
        char comando[MAX_MSG];

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);
        inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

        printf("Digite o comando 'JOIN <seu_nome>' para se conectar ao servidor\n");
        fgets(comando, sizeof(comando), stdin);
        comando[strcspn(comando, "\n")] = 0;

        connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

        send(sock, comando, strlen(comando), 0);

        // Recebe mensagem jogo iniciado e indica id do jogador
        recv(sock, buffer, MAX_MSG, 0);
        printf("\n%s\n", buffer);

        // Recebe mensagem para posicionar jogador
        int cont = 0;
        recv(sock, buffer, MAX_MSG, 0);
        printf("%s\n", buffer);
        while(cont < 4){
            memset(buffer, 0, sizeof(buffer));
            memset(comando, 0, sizeof(comando));
            fgets(comando, sizeof(comando), stdin);
            comando[strcspn(comando, "\n")] = 0;
            send(sock, comando, strlen(comando), 0);

            recv(sock, buffer, MAX_MSG, 0);
            printf("%s\n\n", buffer);

            if(strstr(buffer, "Navio posicionado!") != NULL){
                cont++;
            }
        }

        close(sock);
        return 0;
}
