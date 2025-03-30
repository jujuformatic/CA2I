/**
 * @file affichage.h
 * @version 1.0
 * @date 23/03/2025
 * @author Julien Martinez
 * @brief En-tête contenant les prototypes des fonctions pour afficher la partie avec ncurses
 * @result en dev
 */

#include <locale.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>

#include "jeu.h"

// Macro pour dessiner la bordure selon le mode choisi
#ifdef ASCII_BORDER
#define DRAW_BORDER(win) wborder(win, '|', '|', '-', '-', '+', '+', '+', '+')
#else
#define DRAW_BORDER(win) box(win, ACS_VLINE, ACS_HLINE)
#endif

/**
 * @fn void print_text_in_window(WINDOW *win, int start_y, int start_x, const char *text, int width)
 * @brief imprime du texte dans une fenêtre 
 * @param win fenêtre dans laquelle on affiche
 * @param start_x début du texte dans la fenêtre sur l'axe x
 * @param start_y début du texte dans la fenêtre sur l'axe y
 * @param text texte à afficher
 * @param width largeur de l'espace libre pour le texte, pour faire des retours à la ligne
 */
void print_text_in_window(WINDOW *win, int start_y, int start_x, const char *text, int width);

/**
 * @fn void print_ncurses_game(room_t room, question_t question, deck_t deck, player_t player)
 * @brief affiche la partie dans la fenêtre d'un joueur grâce à ncurses
 * @param room structure de données room_t contenant les informations communes à tous les joueurs
 * @param question structure question_t (char*) contenant la question posée à tous les joueurs
 * @param deck structure deck_t (char**) contenant les cartes du joueur
 * @param player structure player_t contenant les informations du joueur
 */
void print_ncurses_game(room_t room, question_t question, deck_t deck, char *player);