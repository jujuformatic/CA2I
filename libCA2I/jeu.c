/**
 * @file jeu.c
 * @version 1.0
 * @date 22/03/2025
 * @author Julien Martinez
 * @brief Fichier contenant l'application des fonctions de jeu
 * @result en dev
 * @todo \
 * Retour à la ligne à améliorer \
 * - Retirer espaces et ponctuation \
 * - Couper mot avec un tiret \
 * Prendre en compte les caractères ASCII étendu (voir noms)
 */

#include "jeu.h"
#include <string.h>

void getRandomCards(char *file, int cardsNumber, deck_t destination)
{
    char command[128];
    FILE *fp;
    int i;

    snprintf(command, sizeof(command), "awk 'length<=50' %s | shuf -n %d", file, cardsNumber);
    fp = popen(command, "r");

    for (i = 0; i < cardsNumber; i++)
    {
        fgets(destination[i], __INT_MAX__, fp);

        // Retire le caractère de retour à la ligne
        destination[i][strcspn(destination[i], "\n")] = '\0';
    }

    pclose(fp);
}