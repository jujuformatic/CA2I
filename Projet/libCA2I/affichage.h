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

void print_text_in_window(WINDOW *win, int start_y, int start_x, const char *text, int width);
void print_ncurses_game(room_t, question_t, deck_t, player_t);