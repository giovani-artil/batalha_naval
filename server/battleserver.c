
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include "../common/protocol.h"

#define PORT 8080
#define TAM 8

typedef struct{
    int sock;
    int id;
    char nome[100];
    char tabuleiroJogador[TAM][TAM];
    char tabuleiroJogo[TAM][TAM];
    int navios_vivos;
    int coordFrag1[4];
    int coordFrag2[4];
    int coordDestr[6];
}InfoJogador;

InfoJogador jogadores[2];
int jogadoresConectados = 0;
int jogadoresProntos = 0;
int turno = 0; // jogador 1 = 0, jogador 2 = 1
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condConectados = PTHREAD_COND_INITIALIZER;
pthread_cond_t condProntos = PTHREAD_COND_INITIALIZER;
pthread_cond_t condTurno = PTHREAD_COND_INITIALIZER;

void imprimeTabuleiro(char tabuleiro[][TAM], char mensagem[]) {
    snprintf(mensagem, 19, "  0 1 2 3 4 5 6 7\n");

    char linha[32];
    for (int i = 0; i < TAM; i++) {
        snprintf(linha, sizeof(linha), "%d ", i);  // linha começa com o número da linha
        strcat(mensagem, linha);

        for (int j = 0; j < TAM; j++) {
            snprintf(linha, sizeof(linha), "%c ", tabuleiro[i][j]);
            strcat(mensagem, linha);
        }

        strcat(mensagem, "\n");
    }
}


void preencheTabuleiro(char tabuleiro[][TAM]){
    for(int i = 0; i < TAM; i++){
        for(int j = 0; j < TAM; j++){
            tabuleiro[i][j] = '~';
        }
    }
}

void* conectaJogador(void* arg){
    InfoJogador* jogador = (InfoJogador*)arg;
    char buffer[MAX_MSG] = {0};

    recv(jogador->sock, buffer, MAX_MSG, 0);

    if(strncmp(buffer, CMD_JOIN, strlen(CMD_JOIN)) == 0){
        sscanf(buffer + strlen(CMD_JOIN), "%s", jogador->nome);

        pthread_mutex_lock(&mutex);
        jogadoresConectados++;

        if(jogadoresConectados == 2){
            pthread_cond_broadcast(&condConectados);
        }else{
            while(jogadoresConectados < 2){
                pthread_cond_wait(&condConectados, &mutex);
            }
        }

        char mensagem[MAX_MSG];
        snprintf(mensagem, MAX_MSG, "JOGO INICIADO!\nVoce e o jogador %d", jogador->id + 1);
        send(jogador->sock, mensagem, strlen(mensagem), 0);
        pthread_mutex_unlock(&mutex);
    }else{
        send(jogador->sock, "Comando inválido!", 18, 0);
        close(jogador->sock);
    }

    return NULL;
}

int posicionaNavio(InfoJogador* j, int x, int y, char tipo[MAX_MSG], char orientacao, char direcao, int tam){
    if(orientacao == 'H'){
        if(direcao == 'D'){
            if(y + tam > TAM) return 0;
            for(int i = 0; i < tam; i++){
                if(j->tabuleiroJogador[x][y + i] != '~') return 0;
            }
            for(int i = 0; i < tam; i++){
                if(tam == 2){
                    if(j->coordFrag1[2*i] == -1){
                        j->coordFrag1[2*i] = x;
                        j->coordFrag1[2*i + 1] = y + i;
                        printf("Fragata 1 do Jogador %d: x: %d, y: %d\n", j->id + 1, j->coordFrag1[2*i], j->coordFrag1[2*i + 1]);
                    }else{
                        j->coordFrag2[2*i] = x;
                        j->coordFrag2[2*i + 1] = y + i;
                        printf("Fragata 2 do Jogador %d: x: %d, y: %d\n", j->id + 1, j->coordFrag2[2*i], j->coordFrag2[2*i + 1]);
                    }
                }else if(tam == 3){
                    j->coordDestr[2*i] = x;
                    j->coordDestr[2*i + 1] = y + i;
                }
                j->tabuleiroJogador[x][y + i] = tipo[0];
            }
        }else if(direcao == 'E'){
            if(y - tam < -1) return 0;
            for(int i = 0; i < tam; i++){
                if(j->tabuleiroJogador[x][y - i] != '~') return 0;
            }
            for(int i = 0; i < tam; i++){
                if(tam == 2){
                    if(j->coordFrag1[2*i] == -1){
                        j->coordFrag1[2*i] = x;
                        j->coordFrag1[2*i + 1] = y - i;
                        printf("Fragata 1 do Jogador %d: x: %d, y: %d\n", j->id + 1, j->coordFrag1[2*i], j->coordFrag1[2*i + 1]);
                    }else{
                        j->coordFrag2[2*i] = x;
                        j->coordFrag2[2*i + 1] = y - i;
                        printf("Fragata 2 do Jogador %d: x: %d, y: %d\n", j->id + 1, j->coordFrag2[2*i], j->coordFrag2[2*i + 1]);
                    }
                }else if(tam == 3){
                    j->coordDestr[2*i] = x;
                    j->coordDestr[2*i + 1] = y - i;
                }
                j->tabuleiroJogador[x][y - i] = tipo[0];
            }
        }else{
            return 0;
        }
    } else if(orientacao == 'V'){
        if(direcao == 'B'){
            if(x + tam > TAM) return 0;
            for(int i = 0; i < tam; i++){
                if(j->tabuleiroJogador[x + i][y] != '~') return 0;
            }
            for(int i = 0; i < tam; i++){
                if(tam == 2){
                    if(j->coordFrag1[2*i] == -1){
                        j->coordFrag1[2*i] = x + i;
                        j->coordFrag1[2*i + 1] = y;
                        printf("Fragata 1 do Jogador %d: x: %d, y: %d\n", j->id + 1, j->coordFrag1[2*i], j->coordFrag1[2*i + 1]);
                    }else{
                        j->coordFrag2[2*i] = x + i;
                        j->coordFrag2[2*i + 1] = y;
                        printf("Fragata 2 do Jogador %d: x: %d, y: %d\n", j->id + 1, j->coordFrag2[2*i], j->coordFrag2[2*i + 1]);
                    }
                }else if(tam == 3){
                    j->coordDestr[2*i] = x + i;
                    j->coordDestr[2*i + 1] = y;
                }
                j->tabuleiroJogador[x + i][y] = tipo[0];
            }
        }else if(direcao == 'C'){
            if(x - tam < -1) return 0;
            for(int i = 0; i < tam; i++){
                if(j->tabuleiroJogador[x - i][y] != '~') return 0;
            }
            for(int i = 0; i < tam; i++){
                if(tam == 2){
                    if(j->coordFrag1[2*i] == -1){
                        j->coordFrag1[2*i] = x - i;
                        j->coordFrag1[2*i + 1] = y;
                        printf("Fragata 1 do Jogador %d: x: %d, y: %d\n", j->id + 1, j->coordFrag1[2*i], j->coordFrag1[2*i + 1]);
                    }else{
                        j->coordFrag2[2*i] = x - i;
                        j->coordFrag2[2*i + 1] = y;
                        printf("Fragata 2 do Jogador %d: x: %d, y: %d\n", j->id + 1, j->coordFrag2[2*i], j->coordFrag2[2*i + 1]);
                    }
                }else if(tam == 3){
                    j->coordDestr[2*i] = x - i;
                    j->coordDestr[2*i + 1] = y;
                }
                j->tabuleiroJogador[x - i][y] = tipo[0];
            }
        }else{
            return 0;
        }
    }else{
        return 0;
    }
    
    return 1;
}

void* posicionaNavios(void* arg){
    InfoJogador* jogador = (InfoJogador*)arg;
    jogador->coordFrag1[0] = -1;
    jogador->coordFrag1[2] = -1;
    int x, y, s = 1, f = 2, d = 1;
    char comando[MAX_MSG], tipo[MAX_MSG], orientacao, direcao;
    
    send(jogador->sock, "Posicione seus navios utilizando o seguinte comando 'POS <tipo> <x> <y> <H|V> <direcao>'\nTipos disponiveis: 1 SUBMARINO, 2 FRAGATAS, 1 DESTROYER", 144, 0);

    while(s + f + d > 0){
        memset(comando, 0, sizeof(comando));
        recv(jogador->sock, comando, MAX_MSG, 0);

        if(strncmp(comando, CMD_POS, strlen(CMD_POS)) == 0){
            if(sscanf(comando + strlen(CMD_POS), "%s %d %d %c %c", tipo, &x, &y, &orientacao, &direcao) == 5){
                if((x < 0 || x >= TAM) || (y < 0 || y >= TAM)){
                    send(jogador->sock, "Coordenadas invalidas!", 22, 0);
                }else{
                   if(strncmp(tipo, "SUBMARINO", 9) == 0 && s > 0){
                    if(posicionaNavio(jogador, x, y, tipo, orientacao, direcao, 1)){
                        s--;

                        char mensagem[MAX_MSG], tabuleiro[300];
                        imprimeTabuleiro(jogador->tabuleiroJogador, tabuleiro);
                        snprintf(mensagem, sizeof(mensagem), "\n%sNavio posicionado!", tabuleiro);
                        send(jogador->sock, mensagem, sizeof(mensagem), 0);
                    }else{
                        send(jogador->sock, "Posicionamento inválido!", 25, 0);
                    }
                   }else if(strncmp(tipo, "FRAGATA", 7) == 0 && f > 0){
                    if(posicionaNavio(jogador, x, y, tipo, orientacao, direcao, 2)){
                        f--;
                        
                        char mensagem[MAX_MSG], tabuleiro[300];
                        imprimeTabuleiro(jogador->tabuleiroJogador, tabuleiro);
                        snprintf(mensagem, sizeof(mensagem), "\n%sNavio posicionado!", tabuleiro);
                        send(jogador->sock, mensagem, sizeof(mensagem), 0);
                    }else{
                        send(jogador->sock, "Posicionamento inválido!", 25, 0);
                    }
                   }else if(strncmp(tipo, "DESTROYER", 9) == 0 && d > 0){
                    if(posicionaNavio(jogador, x, y, tipo, orientacao, direcao, 3)){
                        d--;
                        
                        char mensagem[MAX_MSG], tabuleiro[300];
                        imprimeTabuleiro(jogador->tabuleiroJogador, tabuleiro);
                        snprintf(mensagem, sizeof(mensagem), "\n%sNavio posicionado!", tabuleiro);
                        send(jogador->sock, mensagem, sizeof(mensagem), 0);
                    }else{
                        send(jogador->sock, "Posicionamento inválido!", 25, 0);
                    }
                   }else{
                    send(jogador->sock, "Erro ao posicionar. Tipo digitado incorreto ou limite de navios desse tipo atingido.", 84, 0);
                   }
                }
            }else{
                send(jogador->sock, "Formato inválido. Utilize 'POS <tipo> <x> <y> <H|V> <direcao>'", 63, 0);
            }
        }else{
            send(jogador->sock, "Comando invalido. Use POS.", 26, 0);
        }
    }

    send(jogador->sock, "Todos os navios posicionados", 28, 0);

    return NULL;
}

int processaJogada(InfoJogador* j, InfoJogador* a, int x, int y){
    if(j->tabuleiroJogo[x][y] == 'O' || j->tabuleiroJogo[x][y] == 'X') return 0;

    if(a->tabuleiroJogador[x][y] == 'S'){
        j->tabuleiroJogo[x][y] = 'O';
        a->navios_vivos -= 1;
        return 2;
    }else if(a->tabuleiroJogador[x][y] == 'F'){
        int contAcerto = 0;
        j->tabuleiroJogo[x][y] = 'O';
        
        if((a->coordFrag1[0] == x && a->coordFrag1[1] == y) || (a->coordFrag1[2] == x && a->coordFrag1[3] == y)){
            for(int i = 0; i < 2; i++){
                if(j->tabuleiroJogo[a->coordFrag1[2*i]][a->coordFrag1[2*i + 1]] == 'O') contAcerto++;
            }
        }else{
            for(int i = 0; i < 2; i++){
                if(j->tabuleiroJogo[a->coordFrag2[2*i]][a->coordFrag2[2*i + 1]] == 'O') contAcerto++;
            }
        }

        if(contAcerto == 2){
            a->navios_vivos -= 1;
            return 2;
        }else{
            return 1;
        }
    }else if(a->tabuleiroJogador[x][y] == 'D'){
        int contAcerto = 0;
        j->tabuleiroJogo[x][y] = 'O';

        for(int i = 0; i < 3; i++){
            if(j->tabuleiroJogo[a->coordDestr[2*i]][a->coordDestr[2*i + 1]] == 'O') contAcerto++;
        }

        if(contAcerto == 3){
            a->navios_vivos -= 1;
            return 2;
        }else{
            return 1;
        }
    }else{
        j->tabuleiroJogo[x][y] = 'X';
        return -1;
    }
}

void* inicioJogo(void* arg){
    InfoJogador* jogador = (InfoJogador*)arg;
    InfoJogador* adversario = &jogadores[1 - jogador->id];
    char comando[MAX_MSG], buffer[MAX_MSG], tabuleiro[300];
    int x, y;

    imprimeTabuleiro(jogador->tabuleiroJogo, tabuleiro);

    send(jogador->sock, "Quando estiver pronto digite 'READY'!", 37, 0);
    recv(jogador->sock, comando, MAX_MSG, 0);

    pthread_mutex_lock(&mutex);
    jogadoresProntos++;

    if(jogadoresProntos == 2){
        pthread_cond_broadcast(&condProntos);
    }else{
        while(jogadoresProntos < 2){
            pthread_cond_wait(&condProntos, &mutex);
        }
    }

    pthread_mutex_unlock(&mutex);

    snprintf(buffer, sizeof(buffer), "Jogadores Prontos!\nTurno do jogador %d!", turno + 1);
    send(jogador->sock, buffer, strlen(buffer), 0);
    sleep(1);

    while(jogador->navios_vivos > 0 && adversario->navios_vivos > 0){
        pthread_mutex_lock(&mutex);
        while(turno != jogador->id){
            pthread_cond_wait(&condTurno, &mutex);
        }
        if(jogador->navios_vivos == 0 || adversario->navios_vivos == 0) break;
        sleep(1);

        memset(buffer, 0, MAX_MSG);
        snprintf(buffer, sizeof(buffer), "%s", "Seu turno!\nUtilize o comando 'FIRE <x> <y> para disparar no adversario!\n");
        strncat(buffer, tabuleiro, sizeof(buffer) - strlen(buffer) - 1);
        send(jogador->sock, buffer, strlen(buffer), 0);
        send(adversario->sock, "Turno do adversario!", 21, 0);
        memset(buffer, 0, MAX_MSG);

        memset(comando, 0, sizeof(comando));
        recv(jogador->sock, comando, MAX_MSG, 0);

        if(strncmp(comando, CMD_FIRE, strlen(CMD_FIRE)) == 0){
            if(sscanf(comando + strlen(CMD_FIRE), "%d %d", &x, &y) == 2){
                if((x < 0 || x >= TAM) || (y < 0 || y >= TAM)){
                    send(jogador->sock, "Coordenadas invalidas!", 22, 0);
                }else{
                    int resul = processaJogada(jogador, adversario, x, y);
                    imprimeTabuleiro(jogador->tabuleiroJogo, tabuleiro);
                    if(resul != 0){
                        if(resul == 1){
                            snprintf(buffer, sizeof(buffer), "%s", tabuleiro);
                            strncat(buffer, CMD_HIT, sizeof(buffer) - strlen(buffer) - 1);
                            send(jogador->sock, buffer, strlen(buffer), 0);
                            send(adversario->sock, CMD_HIT, strlen(CMD_HIT), 0);
                        }else if(resul == 2){
                            snprintf(buffer, sizeof(buffer), "%s", tabuleiro);
                            strncat(buffer, CMD_SUNK, sizeof(buffer) - strlen(buffer) - 1);
                            send(jogador->sock, buffer, strlen(buffer), 0);
                            send(adversario->sock, CMD_SUNK, strlen(CMD_SUNK), 0);
                        }else if(resul == -1){
                            snprintf(buffer, sizeof(buffer), "%s", tabuleiro);
                            strncat(buffer, CMD_MISS, sizeof(buffer) - strlen(buffer) - 1);
                            send(jogador->sock, buffer, strlen(buffer), 0);
                            send(adversario->sock, CMD_MISS, strlen(CMD_MISS), 0);
                        }
                        
                        printf("JOGADOR: %d\tJOGADA: %d %d\n", jogador->id, x, y);
                        turno = 1 - jogador->id;
                        pthread_cond_broadcast(&condTurno);
                    }else{
                        send(jogador->sock, "Jogada Invalida! Digite outra coordenada.", 42, 0);
                        send(adversario->sock, "Jogada Invalida! O adversario tentou uma jogada invalida.", 58, 0);
                        printf("JOGADOR: %d\tJOGADA: %d %d\n", jogador->id + 1, x, y);
                        pthread_mutex_unlock(&mutex);
                        continue; 
                    }
                }
            }
        }else{
            send(jogador->sock, "Digite o comando correto!", 25, 0);
            pthread_mutex_unlock(&mutex);
            continue;   
        }

        pthread_mutex_unlock(&mutex);
    }

    if(jogador->navios_vivos > 0){
        send(jogador->sock, CMD_WIN, strlen(CMD_WIN), 0);
    }else{
        send(jogador->sock, CMD_LOSE, strlen(CMD_LOSE), 0);
    }

    return NULL;
}

int main() {
    pthread_t c1, c2, p1, p2, j1, j2;
    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 2); // até dois jogadores

    printf("Servidor aguardando jogadores na porta %d...\n", PORT);

    jogadores[0].id = 0;
    jogadores[1].id = 1;

    preencheTabuleiro(jogadores[0].tabuleiroJogador);
    preencheTabuleiro(jogadores[0].tabuleiroJogo);
    preencheTabuleiro(jogadores[1].tabuleiroJogador);
    preencheTabuleiro(jogadores[1].tabuleiroJogo);

    jogadores[0].navios_vivos = 4;
    jogadores[1].navios_vivos = 4;

    jogadores[0].sock = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    jogadores[1].sock = accept(server_fd, (struct sockaddr*)&address, &addrlen);

    pthread_create(&c1, NULL, conectaJogador, &jogadores[0]);
    pthread_create(&c2, NULL, conectaJogador, &jogadores[1]);

    pthread_join(c1, NULL);
    pthread_join(c2, NULL);

    printf("Jogadores Conectados!\n");

    pthread_create(&p1, NULL, posicionaNavios, &jogadores[0]);
    pthread_create(&p2, NULL, posicionaNavios, &jogadores[1]);

    pthread_join(p1, NULL);
    pthread_join(p2, NULL);

    unsigned int seed = time(NULL);
    turno = rand_r(&seed) % 2;

    pthread_create(&j1, NULL, inicioJogo, &jogadores[0]);
    pthread_create(&j2, NULL, inicioJogo, &jogadores[1]);

    pthread_join(j1, NULL);
    pthread_join(j2, NULL);

    send(jogadores[0].sock, "FIM", 3, 0);
    send(jogadores[1].sock, "FIM", 3, 0);
    printf("FIM DE JOGO\n");

    close(server_fd);
    return 0;
}