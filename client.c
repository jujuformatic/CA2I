#include "libCA2I/comm.h"
#include "libCA2I/affichage.h"
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/select.h>
#include "libWPI/comm_7segments.h"
#include "libWPI/comm_matriceLed.h"
#include "libWPI/comm_matriceBtn.h"

#define NUM_THREAD_MAIN 1
#define NUM_THREAD_MINUTEUR 2
#define NUM_THREAD_LED 3
#define NUM_THREAD_BOUTONS 4

#ifndef INADDR_SVC
  #define INADDR_SVC "127.0.0.1" // Base server IP address (to send to the players)
#endif

typedef struct {
    char playerName[NAME_MAX_LENGTH];
    socket_t socket;
} thread_args_t;

void bye();
void salon();
void jeu(socket_t, char *);
void gestionClient(void *args);
int winner(room_t *room);
pthread_t create_thread(pthread_t *thread, void *(*start_routine)(void *), thread_args_t *args);

void *timer(void *arg);
void *getButton(void *args);

room_t myRoom;
pthread_t roomThread;
pthread_t timerThread;
pthread_t buttonThread;
socket_t servExchangeSocket;
socket_t roomExchangeSocket;

volatile bool stopSalon = false;
volatile bool remainingTime = true;
volatile int buttonPressed = -1;
volatile int gameMasterIndex = 0;
volatile bool killTimer = false;

int main(int argc, char *argv[])
{
    pthread_t tempThread;
    atexit(bye);

    if (argc < 3)
    {
        printf("Arguments : pseudo, salon à rejoindre (=pseudo pour une création)\n");
        exit(1);
    }
    printf("Lancement du programme\n");
    printf("Nom du joueur : %s\n", argv[1]);
    printf("Nom de l'hôte du salon : %s\n", argv[2]);
    printf("Adresse du serveur : %s\n", INADDR_SVC);
    servExchangeSocket = connectCltToSrv(INADDR_SVC, PORT_SVC, SOCK_STREAM);

    wiringPiSetup();
    initPins(); // Initialise la matrice de boutons
    initMatriceLed(); // Initialise la matrice de LED
    initHT16K33(); // Initialise le 7-segment (minuteur)

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
    deck_t answers;
    deck_t deck;
    question_t question;
    int gameMasterIndex;

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
            strcpy(myRoom.playerNames[myRoom.playerCount], requete.cont);
            myRoom.playerCount++;

            // Annonce d'un nouveau joueur 
            printf("%s a rejoint le salon\n", requete.cont);
        }
    }
    
    // Après le lancement du jeu, demande les données au serveur
    // Le message signale aussi la fin de l'acceptation de nouveaux joueurs
    printf("Lancement de la partie\n");
    requete.id = MSG_CODE;
    strcpy(requete.cont, " ");
    envoyer(&servExchangeSocket, &requete, (pFct *)req2str);

    // La réception des decks se fait un à un
    for (i=0; i<myRoom.playerCount; i++){
        // Reçoit le deck du joueur i et la question
        recevoir(&servExchangeSocket, (generic)&requete, (pFct *)str2req);
        
        // Envoie le deck au joueur i (on garde le même contenu de requête)
        envoyer(&sd[i], &requete, (pFct *)req2str);

        //TODO: Traiter acquitement
        recevoir(&sd[i], (generic)&requete, (pFct *)str2req);

        // Envoie la liste des joueurs 
        sprintf(requete.cont, "%hd||%s||%s||%s||%s||%s||%s||%s||%s",
            myRoom.playerCount,
            myRoom.playerNames[0], myRoom.playerNames[1], 
            myRoom.playerNames[2], myRoom.playerNames[3],
            myRoom.playerNames[4], myRoom.playerNames[5],
            myRoom.playerNames[6], myRoom.playerNames[7]
        );

        envoyer(&sd[i], &requete, (pFct *)req2str);

        requete.id = MSG_CODE;
        strcpy(requete.cont, " ");
        envoyer(&servExchangeSocket, &requete, (pFct *)req2str);
    }

    while (winner(&myRoom)==-1){
        // Attendre que les joueurs (sauf GM) envoient leurs cartes
        for (i=0; i<myRoom.playerCount; i++){
            if(strcmp(myRoom.playerNames[i], myRoom.gameMaster)!=0){
                recevoir(&sd[i], (generic)&requete, (pFct *)str2req);
                strcpy(answers[i], requete.cont);
            } else {
                strcpy(answers[i], " ");
                gameMasterIndex = i;
            }
        }
        for (i=myRoom.playerCount; i<MAX_PLAYERS; i++){
            strcpy(answers[i], " ");
        }

        // Transmettre les cartes à tous les joueurs
        requete.id = CARTE;
        sprintf(requete.cont, "%s||%s||%s||%s||%s||%s||%s||%s",  
            answers[0], answers[1], answers[2], answers[3], answers[4], answers[5], answers[6], answers[7]
        );

        for (i=0; i<myRoom.playerCount; i++)
            envoyer(&sd[i], &requete, (pFct *)req2str);

        // Attendre le choix du GM
        recevoir(&sd[gameMasterIndex], (generic)&requete, (pFct *)str2req);

        // Changer de GM suivant le choix du dernier et ajouter le point
        strcpy(myRoom.gameMaster, requete.cont);
        for (i=0; i<myRoom.playerCount; i++){
            if(strcmp(myRoom.playerNames[i], requete.cont)!=0){
                gameMasterIndex=i;
            }
        }
                    
        // Demander les nouvelles cartes au serveur
        requete.id = CARTE;
        sprintf(requete.cont, "%d", MAX_PLAYERS); // On demande 8 mais utilise moins après
        envoyer(&servExchangeSocket, &requete, (pFct *)req2str);

        recevoir(&servExchangeSocket, (generic)&requete, (pFct *)str2req);
        sscanf(requete.cont, "%[^|]||%[^|]||%[^|]||%[^|]||%[^|]||%[^|]||%[^|]||%[^|]||%[^|]", 
            question, deck[0], deck[1], deck[2], deck[3], deck[4], deck[5], deck[6], deck[7]);
        
        // Envoyer le choix du GM aux joueurs
        // Envoyer la nouvelle question
        // Envoyer une carte à chaque joueur ayant joué une carte (et message vide aux autres)
        requete.id = CARTE;
        for (i=0; i<myRoom.playerCount; i++){
            if(strlen(answers[i])<=1){
                sprintf(requete.cont, "%s||%s|| ", myRoom.gameMaster, question);
            } else {
                sprintf(requete.cont, "%s||%s||%s", myRoom.gameMaster, question, deck[i]);
            }
            envoyer(&sd[i], &requete, (pFct *)req2str);
        }
    }
    // Clôture des dialogue
    for (i=0; i<myRoom.playerCount; i++)
        CHECK(close(sd[i].fd), "close()");

    requete.id = BYE_CODE;
    strcpy(requete.cont, "");
    envoyer(&servExchangeSocket, &requete, (pFct *)req2str);

    CHECK(close(servExchangeSocket.fd), "close()");

    CHECK(close(se.fd), "close()");

    pthread_exit(NULL);
}

// Fonction principale de jeu
void jeu(socket_t sd, char *playerName){
    deck_t deck;
    deck_t answer;
    question_t question;
    requete_t requete;
    int i, tempButton;
    char gagnantManche[NAME_MAX_LENGTH];

    if (strcmp(playerName, myRoom.playerNames[0])!=0){
        printf("[Client] Attente du lancement de la partie par l'hôte\n");
    }
    else {
        printf("[Hôte] Appuyez sur la touche entrée pour lancer la partie\n");

        while (getchar()!='\n'){}

        // Envoi du "signal" de démarrage de partie
        stopSalon = 1;
    }
    
    // Reçoit les données de la partie par le salon 
    attendreDebut(deck, question, &myRoom, sd);

    // Boucle principale de jeu
    while(winner(&myRoom)==-1){
        buttonPressed = -1;
        gameMasterIndex = -1;
        remainingTime = true;
        killTimer = false;

        for (i=0; i<NB_ANSWERS; i++){
            strcpy(answer[i], " ");
        }

        if(strcmp(playerName, myRoom.gameMaster)!=0){ // Manche en tant que joueur
            // Affiche la partie avec ncurses
            print_ncurses_game(myRoom, question, deck, playerName);

            // Lance le minuteur pour décider d'une carte à jouer
            create_thread(&timerThread, (void *)timer, NULL);

            // Lance la détection d'appui de bouton pour choisir une carte à jouer
            create_thread(&buttonThread, (void *)getButton, NULL);

            while(remainingTime && buttonPressed == -1){usleep(10000);}
            pthread_join(buttonThread, NULL);
            
            if(buttonPressed>=0){ // Le joueur a choisi une carte avant la fin du minuteur

                // Réafficher l'interface avec uniquement la carte jouée
                for (i=0; i<NB_ANSWERS; i++){
                    strcpy(answer[i], " ");
                    if (i==buttonPressed){
                        strcpy(answer[i], deck[i]);
                    }
                }
                print_ncurses_game(myRoom, question, answer, playerName);
                
                // Envoyer la carte jouée au salon
                requete.id = CARTE;
                strcpy(requete.cont, deck[buttonPressed]);
                envoyer(&sd, &requete, (pFct *)req2str);

                // Retirer la carte de la main (remplacer par "")
                strcpy(deck[buttonPressed], " ");

                // Recevoir les cartes jouées par chaque joueur et les afficher
                recevoir(&sd, (generic)&requete, (pFct *)str2req);
                sscanf(requete.cont, "%[^|]||%[^|]||%[^|]||%[^|]||%[^|]||%[^|]||%[^|]||%[^|]", 
                    answer[0], answer[1], answer[2], answer[3], answer[4], answer[5], answer[6], answer[7]);
                print_ncurses_game(myRoom, question, answer, playerName);
                killTimer = true; 
                pthread_join(timerThread, NULL);
                
                tempButton = buttonPressed;

                // Lance le minuteur pour le choix du game master
                create_thread(&timerThread, (void *) timer, NULL);
                
                // Attendre gagnant de la manche, nouvelle carte et nouvelle question
                recevoir(&sd, (generic)&requete, (pFct *)str2req);
                sscanf(requete.cont, "%[^|]||%[^|]||%[^|]", gagnantManche, question, deck[tempButton]);
                killTimer = true;
                pthread_join(timerThread, NULL);
            }
            else if(buttonPressed==-1){ // Le minuteur s'est arrêté avant que le joueur ne choisisse une carte

                // Réafficher l'interface avec uniquement la carte jouée (aucune donc)
                for (i=0; i<NB_ANSWERS; i++){
                    strcpy(answer[i], " ");
                }
                print_ncurses_game(myRoom, question, answer, playerName);
                
                // Envoyer la carte jouée au salon (toujours aucune)
                requete.id = CARTE;
                strcpy(requete.cont, " ");
                envoyer(&sd, &requete, (pFct *)req2str);

                // Recevoir les cartes jouées par chaque joueur et les afficher
                recevoir(&sd, (generic)&requete, (pFct *)str2req);
                sscanf(requete.cont, "%[^|]||%[^|]||%[^|]||%[^|]||%[^|]||%[^|]||%[^|]||%[^|]", 
                    answer[0], answer[1], answer[2], answer[3], answer[4], answer[5], answer[6], answer[7]);
                print_ncurses_game(myRoom, question, answer, playerName);
                killTimer = true; 
                pthread_join(timerThread, NULL);

                // Lance le minuteur pour le choix du game master
                create_thread(&timerThread, (void *) timer, NULL);
                
                // Attendre gagnant de la manche, nouvelle carte (ignorée) et nouvelle question
                recevoir(&sd, (generic)&requete, (pFct *)str2req);
                sscanf(requete.cont, "%[^|]||%[^|]||", gagnantManche, question);
                killTimer = true;
                pthread_join(timerThread, NULL);
            }
            strcpy(myRoom.gameMaster, gagnantManche);
            for (i=0; i<myRoom.playerCount; i++)
                if (strcmp(myRoom.playerNames[i], gagnantManche) == 0)
                    myRoom.playersScores[i]++;

        } else { // Manche en tant que GM
            print_ncurses_game(myRoom, question, answer, playerName); // Le game master doit attenndre les cartes des autres joueurs

            // Lance le minuteur pour le choix des joueurs
            create_thread(&timerThread, (void *) timer, NULL);

            // Enregistre l'index du game master pour savoir quelle carte ignorer dans le choix
            for (i=0; i<myRoom.playerCount; i++)
                if (strcmp(myRoom.playerNames[i], playerName) == 0)
                    gameMasterIndex = i;

            // Recevoir les cartes par le salon
            recevoir(&sd, (generic)&requete, (pFct *)str2req);
            killTimer = true;
            pthread_join(timerThread, NULL);
            
            sscanf(requete.cont, "%[^|]||%[^|]||%[^|]||%[^|]||%[^|]||%[^|]||%[^|]||%[^|]", 
                answer[0], answer[1], answer[2], answer[3], answer[4], answer[5], answer[6], answer[7]);
            // Afficher les cartes reçues
            print_ncurses_game(myRoom, question, answer, playerName);

            // Lance le minuteur pour le choix du game master
            create_thread(&timerThread, (void *) timer, NULL);

            // Lance la détection d'appui de bouton pour décider du gagnant de la manche
            create_thread(&buttonThread, (void *) getButton, NULL);

            //TODO: empêcher le GM de choisir une carte vide
            while(remainingTime && buttonPressed == -1){usleep(10000);}
            pthread_join(buttonThread, NULL);

            if(buttonPressed>=0){ // Le game master a choisi une carte avant la fin du minuteur
                // Envoyer le nom du joueur gagnant au salon
                requete.id = GAGNANT;
                strcpy(requete.cont, myRoom.playerNames[buttonPressed]);
                envoyer(&sd, &requete, (pFct *)req2str);

                // Changer le GM en local et ajouter les points
                strcpy(myRoom.gameMaster, myRoom.playerNames[buttonPressed]);
                myRoom.playersScores[buttonPressed]++;
            }
            else if(buttonPressed==-1){ // Le minuteur s'est arrêté avant que le game master ne choisisse une carte
                // Envoyer automatiquement la première carte -> à changer plus tard pour carte vide = pas de changement de points et GM=hôte
                requete.id = GAGNANT;
                strcpy(requete.cont, myRoom.playerNames[0]);
                envoyer(&sd, &requete, (pFct *)req2str);

                // Changer le GM en local et ajouter les points
                strcpy(myRoom.gameMaster, myRoom.playerNames[0]);
                myRoom.playersScores[0]++;
            }
            killTimer = true; 
            pthread_join(timerThread, NULL);

            // Recevoir gagnant de la manche (ignoré), nouvelle carte (ignorée) et nouvelle question
            recevoir(&sd, (generic)&requete, (pFct *)str2req);            
            sscanf(requete.cont, "%*[^|]||%[^|]||", question);
        }
        afficherPoints(myRoom.playersScores);
    }
    endwin();
    printf("Partie finie, victoire de %s\n", myRoom.gameMaster);

    // Clôture du dialogue
    CHECK(close(sd.fd), "close()");

    pthread_exit(NULL);
}

void bye(){
}

int winner(room_t *room){
    int i;
    for (i=0; i<room->playerCount; i++){
        if(room->playersScores[i] == MAX_POINTS)
            return i;
    }
    return -1;
}

void *timer(void *arg) {
    int i = 3000;

    killTimer = false;
    remainingTime = true;

    while (i > 0 && !killTimer) {
        displayNumber(i--); // Afficher le temps restant
        usleep(8000); // Attendre 8 ms
    }

    killTimer = false;

    displayNumber(0); // Afficher un temps nul
    
    remainingTime = (i!=0); 
    pthread_exit(NULL);
}

void *getButton(void *args) {

    delay(100); // Attendre un peu pour éviter les faux positifs

    while (remainingTime && ( buttonPressed==-1 || buttonPressed==gameMasterIndex )) {
        buttonPressed = detectButton(); // Détecter les boutons
        delay(100); // Petite pause pour éviter une surcharge CPU
    }

    pthread_exit(NULL);
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