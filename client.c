#include "libCA2I/comm.h"
#include "libCA2I/affichage.h"
#include <pthread.h>
#include <signal.h>
#include <sys/select.h>

typedef struct {
    char playerName[NAME_MAX_LENGTH];
    socket_t socket;
} thread_args_t;

void bye();
void salon();
void jeu(socket_t, char *);
void gestionClient(void *args);
pthread_t create_thread(pthread_t *thread, void *(*start_routine)(void *), thread_args_t *args);

room_t myRoom;
pthread_mutex_t terminalMutex;
pthread_t roomThread;
socket_t servExchangeSocket;
socket_t roomExchangeSocket;

bool stopSalon = false;

int main(int argc, char *argv[])
{
    atexit(bye);

    if (argc < 3)
    {
        printf("Arguments : pseudo, salon à rejoindre (=pseudo pour une création)\n");
        exit(1);
    }
    printf("Lancement du programme\n");
    printf("Nom du joueur : %s\n", argv[1]);
    printf("Nom de l'hôte du salon : %s\n", argv[2]);
    servExchangeSocket = connectCltToSrv(INADDR_SVC, PORT_SVC, SOCK_STREAM);

    strcpy(myRoom.host, argv[2]);

    pthread_mutex_init(&terminalMutex, NULL);

    // Nom = Hôte => création de salon
    if (strcmp(argv[1], argv[2])==0){
        printf("Demande de création de salon\n");
        creerSalon(&myRoom, argv[1], servExchangeSocket);
        printf("Salon créé\n");

        // Lancement du thread salon
        create_thread(&roomThread, (void *)salon, NULL);
        usleep(100000);
    }

    roomExchangeSocket = connectCltToSrv(INADDR_SVC, PORT_SVC, SOCK_STREAM);

    // // L'hôte lance le jeu en tant que client de son salon
    printf("Demande pour rejoindre un salon\n");
    roomExchangeSocket = rejoindreSalon(roomExchangeSocket, argv[2], argv[1]);
    printf("Salon rejoint\n");

    jeu(roomExchangeSocket, argv[1]);

    return 0;
}

// Gestion du salon : Entrées des utilisateurs 
void salon(){
    socket_t se, sd[MAX_PLAYERS];
    pthread_t client;
    struct sockaddr_in svc, clt;
    socklen_t cltLen;
    requete_t requete;
    thread_args_t *thread_args = malloc(sizeof(thread_args_t));
    int i, j;
    fd_set readfds;
    struct timeval timeout;

    se = creerSocketEcoute(INADDR_ANY_SOURCE, PORT_CLT_SVC);
    
    // Attend l'arrivée de clients jusqu'au démarrage de la partie (décidé par l'hôte)
    while (!stopSalon) { 
        // Initialiser l'ensemble de descripteurs de fichiers
        FD_ZERO(&readfds);
        FD_SET(se.fd, &readfds);  // Ajouter la socket d'écoute

        // Timeout de 1 seconde
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        // Vérifier si la socket est prête à accepter une connexion
        int ret = select(se.fd + 1, &readfds, NULL, NULL, &timeout);

        if (ret > 0 && FD_ISSET(se.fd, &readfds)) { // Si la socket est prête
            // New player
            sd[myRoom.playerCount] = acceptClt(se);
            if (stopSalon) break; // Vérifier si on doit arrêter

            // Reçoit le nom du nouveau joueur
            recevoir(&sd[myRoom.playerCount], (generic)&requete, (pFct *)str2req);
            strcpy(myRoom.players[myRoom.playerCount++].nickname, requete.cont);

            // Annonce d'un nouveau joueur 
            pthread_mutex_lock(&terminalMutex); // L'hôte partage le terminal avec le salon
            printf("%s a rejoint le salon\n", requete.cont);
            pthread_mutex_unlock(&terminalMutex);
        }
    }
    
    // [1] Après le lancement du jeu, demande les données au serveur
    // Le message signale aussi la fin de l'acceptation de nouveaux joueurs
    printf("Lancement de la partie\n");
    requete.id = MSG_CODE;
    strcpy(requete.cont, "");
    envoyer(&servExchangeSocket, &requete, (pFct *)req2str);

    // La réception des decks se fait un à un
    for (i=0; i<myRoom.playerCount; i++){
        // [4] Reçoit le deck du joueur i et la question
        recevoir(&servExchangeSocket, (generic)&requete, (pFct *)str2req);
        
        // [5] Envoie le deck au joueur i (on garde le même contenu de requête)
        envoyer(&sd[i], &requete, (pFct *)req2str);

        //TODO: Traiter acquitement
        recevoir(&sd[i], (generic)&requete, (pFct *)str2req);

        // [6] Envoie la liste des joueurs 
        sprintf(requete.cont, "%hd||%s||%s||%s||%s||%s||%s||%s||%s",
            myRoom.playerCount,
            myRoom.players[0].nickname, myRoom.players[1].nickname, 
            myRoom.players[2].nickname, myRoom.players[3].nickname,
            myRoom.players[4].nickname, myRoom.players[5].nickname,
            myRoom.players[6].nickname, myRoom.players[7].nickname
        );

        envoyer(&sd[i], &requete, (pFct *)req2str);

        requete.id = MSG_CODE;
        strcpy(requete.cont, "");
        envoyer(&servExchangeSocket, &requete, (pFct *)req2str);
    }

    //TODO: suite du jeu

    // Clôture du dialogue
    CHECK(close(se.fd), "close()");

    pthread_exit(NULL);
}

// Fonction principale de jeu
void jeu(socket_t sd, char *playerName){
    deck_t deck;
    question_t question;

    if (strcmp(playerName, myRoom.host)!=0){
        printf("Attente du lancement de la partie par l'hôte\n");
    }
    else {
        pthread_mutex_lock(&terminalMutex); // L'hôte partage le terminal avec le salon
        printf("Appuyez sur la touche entrée pour lancer la partie\n");
        pthread_mutex_unlock(&terminalMutex);

        while (getchar()!='\n'){}

        // Envoi du "signal" de démarrage de partie
        stopSalon = 1;
    }
    
    // [7] Reçoit les données de la partie par le salon 
    attendreDebut(deck, question, &myRoom, sd);

    // On passe en mode graphique pour jouer
    print_ncurses_game(myRoom, question, deck, playerName);

    // Clôture du dialogue
    CHECK(close(sd.fd), "close()");

    pthread_exit(NULL);
}

void bye(){
}

/**
 * @brief Fonction qui créer un thread avec un numéro
 * @details La fonction créer un thread
 * @param thread destination de l'identifiant de thread
 * @param start_routine fonction principale du thread
 * @param
 */
pthread_t create_thread(pthread_t *thread, void *(*start_routine)(void *), thread_args_t *args){
    pthread_attr_t attr;
    pthread_t tid;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // Passer le pointeur du socket alloué au thread
    CHECK_T(tid = pthread_create(thread, &attr, start_routine, (void *) args), "pthread_create");
    pthread_attr_destroy(&attr);

    return tid;
}