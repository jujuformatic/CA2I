/**
 * @authors Julien Martinez
 * @date 28/03/25
 * @file comm.c
 * @brief Librairie contenant les fonctions de communication
 * @version en dev
 */

#include "comm.h"
#include <string.h>

socket_t rejoindreSalon(socket_t exchangeSocket, char *hostName, char *playerName)
{
    requete_t requete;
    char hostIP[INET_ADDRSTRLEN];
    socket_t roomExchangeSocket;
    
    // Envoie le nom du joueur donc on veut rejoindre la partie
    requete.id = JOIN_ROOM;
    strcpy(requete.cont, hostName);
    envoyer(&exchangeSocket, &requete, (pFct *)req2str);
    
    // Réception de l'adresse IP de l'hôte
    recevoir(&exchangeSocket, &requete, (pFct *)str2req);
    sscanf(requete.cont, "%s", hostIP);
    
    // Connection à l'hôte de la partie (automatiquement déconnecté du serveur)
    roomExchangeSocket = connectCltToSrv(hostIP, PORT_CLT_SVC, SOCK_STREAM);
    
    // Envoi du nom du joueur à l'hôte
    requete.id = JOIN_ROOM;
    strcpy(requete.cont, playerName);
    envoyer(&roomExchangeSocket, &requete, (pFct *)req2str);
    
    // Renvoie la socket d'échange à jour
    return roomExchangeSocket;
}

int creerSalon(room_t *room, char *hostname, socket_t exchangeSocket)
{
    requete_t requete;
    int i;
    // Rentre les données initiales du salon
    room->playerCount=0;
    strcpy(room->host, hostname);
    strcpy(room->gameMaster, hostname);

    for(i=0; i<MAX_PLAYERS; i++){
        strcpy(room->players[i].nickname, "");
        room->players[i].score = 0;
    }

    // Envoie la requête au serveur avec les données du salon 
    //      (le serveur connaît l'IP et le port, on n'envoie donc que le nom)
    requete.id = CREATE_ROOM;
    sprintf(requete.cont, "%s", 
        room->host
    );
    envoyer(&exchangeSocket, &requete, (pFct *)req2str);

    // Réception du résultat de la création
    recevoir(&exchangeSocket, &requete, (pFct *)str2req);

    // Renvoie le résultat
    if (requete.id == NOK)
        return -1;

    return 1;
}

int attendreDebut(deck_t deck, question_t question, room_t *room, socket_t exchangeSocket)
{
    requete_t requete;
    int j;

    // Attend le début de la partie (comm du salon)
    // Première comm : question + réponses
    recevoir(&exchangeSocket, &requete, (pFct *)str2req);

    // Enregistre les informations transmises
    sscanf(requete.cont, "%[^|]||%[^|]||%[^|]||%[^|]||%[^|]||%[^|]||%[^|]||%[^|]||%[^|]",
        question,
        deck[0],deck[1],deck[2],deck[3],deck[4],deck[5],deck[6],deck[7]
    );

    // Envoi de l'acquitement
    requete.id = OK;
    strcpy(requete.cont, "");
    envoyer(&exchangeSocket, &requete, (pFct *)req2str);

    // Deuxième comm : liste des joueurs
    recevoir(&exchangeSocket, &requete, (pFct *)str2req);

    // Enregistre les informations transmises
    sscanf(requete.cont, "%hd||%[^|]||%[^|]||%[^|]||%[^|]||%[^|]||%[^|]||%[^|]||%[^|]",
        &room->playerCount,
        room->players[0].nickname, room->players[1].nickname, 
        room->players[2].nickname, room->players[3].nickname,
        room->players[4].nickname, room->players[5].nickname,
        room->players[6].nickname, room->players[7].nickname
    );

    strcpy(room->gameMaster, room->players[0].nickname);
    strcpy(room->host, room->players[0].nickname);

    // Envoie le résultat
    return 1;
}

void envoyerDebutAuSalon(room_t *room, socket_t exchangeSocket)
{
    deck_t deck;
    requete_t requete;
    question_t question;
    int i, j;

    // Tirer carte question
    getRandomCards(QUESTIONS, 1, &question);
    
    // Pour chaque joueur : 
    for (i=0; i<room->playerCount; i++){
        // Tirer 8 cartes réponses 
        getRandomCards(ANSWERS, NB_ANSWERS, deck);

        // [3] Envoyer toutes les cartes (réponses + questions)1
        sprintf(requete.cont, "%s||%s||%s||%s||%s||%s||%s||%s||%s",
            question,
            deck[0],deck[1],deck[2],deck[3],deck[4],deck[5],deck[6],deck[7]
            );

        requete.id = MSG_CODE;
        envoyer(&exchangeSocket, &requete, (pFct *)req2str);

        // Attendre acquitement
        recevoir(&exchangeSocket, &requete, (pFct *)str2req);
    }
}

int envoyerCarte()
{
    // Attend le début de la partie (comm de l'hôte)
    // Enregistre les informations transmises
    // Envoie le résultat
}

int attendreFinManche()
{
  // Attend les informations suivantes de l'hôte : gagnant de la manche, nouvelle carte du joueur et nouvelle carte question
  // Met à jour les scores
  // Met à jour les données du joueur
  // Renvoie le résultat
}


void req2str(requete_t *req, buffer_t buff){
    CHECK(sprintf(buff, "%i:%s", req->id, req->cont), "sprintf");
}

void str2req(buffer_t buff, requete_t *req) {
    int offset = 0;

    // Lire l'identifiant et l'offset après le ':'
    CHECK(sscanf(buff, "%d:%n", &req->id, &offset), "Erreur lecture identifiant");

    // Copier le reste de la chaîne après le ':'
    strncpy(req->cont, buff + offset, sizeof(req->cont) - 1);
    req->cont[sizeof(req->cont) - 1] = '\0';  // Assurer la terminaison
}