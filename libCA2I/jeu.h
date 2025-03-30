/**
 * @authors Julien Martinez, Simon Pernet
 * @date 23/03/25
 * @file jeu.h
 * @brief Fichier d'en-tête contenant les structures et constantes de jeu
 * @version en dev
 */

#include "../libMCS/session.h"

#ifndef JEU
    #define JEU
    #define MAX_PLAYERS 8
    #define NAME_MAX_LENGTH 10
    #define CARD_MAX_LENGTH 50
    #define NB_ANSWERS 8
    #define MAX_ROOMS 10
    
    #define ANSWERS "Answers.txt"
    #define QUESTIONS "Questions.txt"

    typedef struct
    {
        char nickname[NAME_MAX_LENGTH];
        unsigned char score;
    } player_t;

    typedef struct
    {
        char host[NAME_MAX_LENGTH];
        char IP[INET_ADDRSTRLEN];
        player_t players[MAX_PLAYERS];
        short playerCount;
        char gameMaster[NAME_MAX_LENGTH]; 
        short closed;
    } room_t;

    typedef char deck_t[NB_ANSWERS][CARD_MAX_LENGTH];
    typedef char question_t[CARD_MAX_LENGTH];

    void getRandomCards(char *file, int cardsNumber, deck_t destination);
    /*
    void getRandomCards(char *file, int cardsNumber, question_t *destination);
    Autre déclaration utilisant la carte question seule (exemple d'utilisation)
    */
#endif