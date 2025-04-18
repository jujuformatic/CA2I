/**
 * @file affichage.c
 * @version 1.0
 * @date 23/03/2025
 * @author Julien Martinez
 * @brief Librairie contenant les fonctions pour afficher la partie avec ncurses
 * @todo \
 * Retour à la ligne à améliorer \
 * - Retirer espaces et ponctuation \
 * - Couper mot avec un tiret \
 * Prendre en compte les caractères ASCII étendu (voir noms)
 */

#include "affichage.h"

void print_text_in_window(WINDOW *win, int start_y, int start_x, const char *text, int width)
{
    int len = strlen(text);
    int line = start_y;
    int col = start_x;
    int i;

    for (i = 0; i < len; i++)
    {
        if (text[i] == '\n' || col - start_x >= width - 2)
        {
            line++;
            col = start_x;
            if (text[i] == '\n')
                continue;
        }
        mvwaddch(win, line, col, text[i]);
        col++;
    }
}

void print_ncurses_game(room_t room, question_t question, deck_t deck, char *player)
{
    int i, j;

    // Initialisation de ncurses
    initscr();
    noecho();
    cbreak();
    curs_set(0);

#ifndef THEME_NOIR
    // Teste si le terminal gère les couleurs
    if (has_colors())
    {
        start_color();
        // Définition d'un couple de couleurs (texte, fond)
        init_pair(1, COLOR_BLUE, COLOR_WHITE);
        init_pair(2, COLOR_BLACK, COLOR_WHITE);
        init_pair(3, COLOR_WHITE, COLOR_BLACK);
    }

    // On change la couleur du terminal en blanc
    wbkgd(stdscr, COLOR_PAIR(2));
#else
    // Teste si le terminal gère les couleurs
    if (has_colors())
    {
        start_color();
        // Définition d'un couple de couleurs (texte, fond)
        init_pair(1, COLOR_WHITE, COLOR_BLUE);  // Bloc gauche
        init_pair(2, COLOR_BLACK, COLOR_WHITE); // Cartes bas
        init_pair(3, COLOR_WHITE, COLOR_BLACK); // Carte millieu
    }

    // On change la couleur du terminal en noir
    wbkgd(stdscr, COLOR_PAIR(3));
#endif

    // Récupère la taille du terminal
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    // printf("%d, %d", max_x, max_y);
    // exit(0);

    // Calcul des dimensions
    // int leftRectWidth    = (int)(max_x * 0.15); // 15% de la largeur
    int leftRectWidth = NAME_MAX_LENGTH + 6;    // Taille fixée suivant la taille maximale du nom d'un joueur
    int leftRectHeight = max_y;                 // 100% de la hauteur
    int bottomRectHeight = (int)(max_y * 0.40); // 40% de la hauteur
    int regionWidth = max_x - leftRectWidth;
    int regionHeight = max_y - bottomRectHeight;

    // On s'assure que rien ne tombe à zéro ou négatif
    if (leftRectWidth < 1)
        leftRectWidth = 1;
    if (leftRectHeight < 1)
        leftRectHeight = 1;
    if (bottomRectHeight < 1)
        bottomRectHeight = 1;
    if (regionWidth < 1)
        regionWidth = 1;
    if (regionHeight < 1)
        regionHeight = 1;

    refresh();

    // --- 1) Rectangle de gauche ---
    /* Problème : les caractères ascii étendu ne sont pas pris en compte par ncurses
    Sur RPI la bordure de la fenêtre est décalée et sur PC l'affichage est chaotique
    -> Ne pas autoriser le joueur à rentrer des caractères ascii étendu

    https://stackoverflow.com/questions/35162938/cannot-correctly-print-extended-ascii-characters-in-ncurses
    */
    WINDOW *winLeft = newwin(leftRectHeight, leftRectWidth, 0, 0);

    // Si on a les couleurs, on applique le thème 1 (blanc sur bleu)
    if (has_colors())
    {
        wbkgd(winLeft, COLOR_PAIR(1));
    }

    DRAW_BORDER(winLeft);

    // Pour mieux le repérer, on place l'étiquette en haut à gauche
    // (plutôt qu'au milieu vertical, pour être sûr de la voir)
    for (i = 0; i < 8; i++)
    {
        char name1[NAME_MAX_LENGTH + 2];
        char name2[NAME_MAX_LENGTH + 2];
        if (!strcmp(room.playerNames[i], player))
        {
            sprintf(name1, "[%s]", room.playerNames[i]);
        }
        else
        {
            sprintf(name1, " %s", room.playerNames[i]);
        }
        if (!strcmp(room.playerNames[i], room.gameMaster))
        {
            sprintf(name2, ">%s", name1);
        }
        else
        {
            sprintf(name2, " %s", name1);
        }

        mvwprintw(winLeft, 2 + i * 2, 1, "%s", name2);
    }
    wrefresh(winLeft);

    // --- 2) Les 8 blocs en bas ---
    int blockWidth = regionWidth / 4 - 1;       // Largeur de chaque bloc
    int blockHeight = bottomRectHeight / 2 - 1; // Hauteur de chaque bloc

    // Sécurité pour éviter blockWidth = 0
    if (blockWidth < 1)
        blockWidth = 1;
    if (blockHeight < 1)
        blockHeight = 1;

    int startY = max_y - bottomRectHeight; // On démarre en bas
    int startX = leftRectWidth;            // Juste à droite du bloc gauche

    for (j = 0; j < 2; j++)
    {
        for (i = 0; i < 4; i++)
        {
            WINDOW *winBlock = newwin(2 * blockHeight, blockWidth,
                                      startY + j, startX + i * (blockWidth + 1) + 1);

            // Si on a les couleurs, on applique le thème 2 (noir sur blanc)
            if (has_colors())
            {
                wbkgd(winBlock, COLOR_PAIR(2));
            }

            DRAW_BORDER(winBlock);

            char label[32];
            print_text_in_window(winBlock, 1, 1, deck[i + 4 * j], blockWidth);
            wrefresh(winBlock);
        }
        startY += blockHeight;
    }

    // --- 3) Rectangle du milieu ---
    // Même largeur que les blocs du bas, double de leur hauteur
    int midRectWidth = blockWidth;
    int midRectHeight = 3 * blockHeight;

    // On le centre dans la zone au-dessus des blocs (regionHeight)
    int midRectTop = (regionHeight - midRectHeight) / 2;
    int midRectLeft = leftRectWidth + (regionWidth - midRectWidth) / 2;

    // Sécurités si le terminal est petit
    if (midRectTop < 0)
        midRectTop = 0;
    if (midRectLeft < leftRectWidth)
        midRectLeft = leftRectWidth;
    if (midRectWidth < 1)
        midRectWidth = 1;
    if (midRectHeight < 1)
        midRectHeight = 1;

    WINDOW *winMid = newwin(midRectHeight, midRectWidth, midRectTop, midRectLeft);

    // Si on a les couleurs, on applique le thème 3 (blanc sur noir)
    if (has_colors())
    {
        wbkgd(winMid, COLOR_PAIR(3));
    }

    DRAW_BORDER(winMid);

    print_text_in_window(winMid, 1, 1, question, midRectWidth);
    wrefresh(winMid);
    
    return;
}
