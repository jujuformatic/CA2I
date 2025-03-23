/**
 * @authors Julien Martinez
 * @date 23/03/25
 * @file jeu.h
 * @brief Fichier d'en-tête contenant les structures et constantes de jeu et de communication
 * @version 1.0
 */

#include "../libMCS/session.h"

#define MAX_PLAYERS 8
#define NAME_MAX_LENGTH 10
#define QUESTION_MAX_LENGTH 100
#define ANSWER_MAX_LENGTH 50
#define NB_ANSWERS 8

typedef struct
{
    char nickname[NAME_MAX_LENGTH];
    unsigned char score;
} player_t;

typedef struct
{
    player_t host;
    socket_t listenSocket;
    player_t players[MAX_PLAYERS];
    unsigned char playerCount;
    player_t gameMaster;
} room_t;

typedef char *deck_t[NB_ANSWERS];
typedef char *question_t;

void getRandomCards(char *file, int cardsNumber, deck_t destination);
/*
 void getRandomCards(char *file, int cardsNumber, question_t *destination);
 Autre déclaration utilisant la carte question seule (exemple d'utilisation)
 */

 