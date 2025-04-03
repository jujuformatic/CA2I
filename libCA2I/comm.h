/**
 * @authors Julien Martinez
 * @date 24/03/25
 * @file comm.h
 * @brief Fichier d'en-tête contenant les structures et constantes de communication
 * @version en dev
 */

#include "../libMCS/data.h"
#include "jeu.h"

#define PORT_SVC 5000 // Server listening port (to send to the players)
#define PORT_CLT_SVC 5001 // Host listening port
#define INADDR_ANY_SOURCE "0.0.0.0" // Origin IP address of players (any)

// Communication codes
#define MSG_CODE 100
#define ERR_CODE 200
#define BYE_CODE 0

// Choices available to the client in the "lobby"
#define CREATE_ROOM 101
#define JOIN_ROOM 102
#define LIST_ROOM 103

// Communications durant la partie
#define CARTE 110
#define GAGNANT 111

// Messages du serveur
#define OK 1
#define NOK 0

#define MAX_ATTEMPTS 5

#define MAX_BUFF 1024

#define CHECK_T(status, msg)                        \
  if (0 != (status))   {                            \
    fprintf(stderr, "pthread/sem erreur : %s\n", msg);  \
    exit (EXIT_FAILURE);                            \
  }  

#define CHECK_NULL(status, msg)                        \
  if (NULL == (status))   {                            \
    fprintf(stderr, "erreur lecture : %s\n", msg);  \
    exit (EXIT_FAILURE);                            \
  }

  typedef struct{
    int id;
    char cont[MAX_BUFF-4]; //3 chiffres + :
} requete_t;

/**
 * @fn socket_t rejoindreSalon(socket_t exchangeSocket, char *hostName, char *playerName)
 * @brief Fonction établissant la connection entre le joueur et un salon 
 * @param servExchangeSocket Socket de discussion avec le serveur
 * @param playerName Nom de l'hôte du salon
 * @param hostName Nom du joueur voulant rejoindre
 * @returns socket connectée à l'hôte
 */
socket_t rejoindreSalon(socket_t exchangeSocket, char *hostName, char *playerName);

/**
 * @fn int creerSalon(room_t *room, char *hostname, socket_t exchangeSocket)
 * @brief Fonction demandant au serveur la création d'un salon
 * @param room structure room_t initialisée
 * @param hostname nom de l'utilisateur créant le salon
 * @param exchangeSocket socket d'échange avec le serveur
 * @returns 1 si création, -1 sinon
 */
int creerSalon(room_t *room, char *hostname, socket_t exchangeSocket);

/**
 * @fn int attendreDebut(deck_t deck, question_t question, room_t *room, socket_t exchangeSocket)
 * @brief Fonction traitant les informations envoyés par le salon au début de la partie 
 * @param deck main du joueur
 * @param question carte question de la manche
 * @param room structure de données du salon
 * @param exchangeSocket socket d'échange avec l'hôte
 * @returns -1 si le salon est dissout
 * @returns 1 si la partie commence
 */
int attendreDebut(deck_t deck, question_t question, room_t *room, socket_t exchangeSocket);

void envoyerDebutAuSalon(room_t *room, socket_t exchangeSocket, char *questionsFile, char *answersFile);

/**
 * @fn int lancerPartie()
 * @brief Envoie les informations à tous les joueurs ayant rejoint le serveur
 */
void lancerPartie();

/**
 * @fn int envoyerCarte(char *carte)
 * @brief Fonction envoyant à l'hôte le choix du joueur : 
 *  - Carte jouée si joueur
 *  - Carte gagnante si game master
 * @returns char*, nom du gagnant si le jeu est fini
 * @returns char*, nouvelle carte du joueur
 */
int envoyerCarte();

/**
 * @fn int attendreFinManche()
 * @brief Après avoir envoyé la carte, on attend des informations envoyées par l'hôte
 * @returns -1 si le salon est dissout
 * @returns 1 si on passe à la manche suivante
 * @returns 2 si la partie est finie
 */
int attendreFinManche();


void req2str(requete_t *req, buffer_t buff);

void str2req(buffer_t buff, requete_t *req);