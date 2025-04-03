#ifndef COMM_MATRICELED_H
#define COMM_MATRICELED_H

#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>

#define LED_MATRIX_ROWS 8

/**
 * @brief Canal SPI utilisé pour la communication avec le MAX7219.
 */
#define SPI_CHANNEL 1

/**
 * @brief Vitesse de communication SPI en Hz.
 */
#define SPI_SPEED 1000000

/**
 * @brief Tableau représentant l'état des LEDs de la matrice (8x8).
 * 
 * Chaque élément du tableau correspond à une ligne de la matrice.
 * Les bits de chaque élément représentent les colonnes (1 = LED allumée, 0 = LED éteinte).
 */
extern unsigned char matrix[8];

/**
 * @brief Initialise la matrice LED.
 * 
 * Configure le canal SPI et initialise le MAX7219 pour contrôler la matrice LED.
 */
void initMatriceLed();

/**
 * @brief Envoie une commande au contrôleur MAX7219.
 * 
 * @param address Adresse du registre à modifier (1 à 8 pour les lignes).
 * @param value Valeur à écrire dans le registre.
 */
void sendCommand(unsigned char address, unsigned char value);

/**
 * @brief Initialise le contrôleur MAX7219.
 * 
 * Configure le MAX7219 pour un fonctionnement normal, désactive le mode décodeur,
 * règle la luminosité, active l'affichage sur 8 colonnes et efface l'écran.
 */
void initMAX7219();

/**
 * @brief Allume ou éteint une LED spécifique dans la matrice.
 * 
 * @param row Ligne de la LED (0 à 7).
 * @param col Colonne de la LED (0 à 7).
 * @param state État de la LED : 1 pour allumer, 0 pour éteindre.
 */
void setLED(int row, int col, int state);

/**
 * @brief Éteint toutes les LEDs de la matrice.
 * 
 * Réinitialise le tableau `matrix` et met à jour l'affichage pour éteindre toutes les LEDs.
 */
void clearall();

/**
 * @brief Affiche un tableau de points sur la matrice LED.
 * 
 * Chaque élément du tableau représente le nombre de LEDs allumées dans la colonne correspondante.
 * 
 * @param room Structure de données contenant chaque joueur avec ses points
 */
void afficherPoints(int *scores) ;

#endif // COMM_MATRICELED_H