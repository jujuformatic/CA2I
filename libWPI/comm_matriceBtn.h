#ifndef COMM_MATRICEBTN_H
#define COMM_MATRICEBTN_H

#include <stdio.h>
#include <wiringPi.h>
#include <softTone.h>

/**
 * @file comm_matriceBtn.h
 * @brief Gestion d'une matrice de boutons via GPIO.
 * 
 * Ce fichier contient les définitions et déclarations nécessaires pour gérer
 * une matrice de boutons connectée à un microcontrôleur via les broches GPIO.
 */

/**
 * @brief Broches GPIO utilisées pour les lignes de la matrice.
 */
#define ROW1 2 ///< Ligne 1
#define ROW2 3 ///< Ligne 2
#define ROW3 21 ///< Ligne 3
#define ROW4 22 ///< Ligne 4

/**
 * @brief Broches GPIO utilisées pour les colonnes de la matrice.
 */
#define COL1 6 ///< Colonne 1
#define COL2 25 ///< Colonne 2
#define COL3 24 ///< Colonne 3
#define COL4 23 ///< Colonne 4

/**
 * @brief État des boutons de la matrice.
 * 
 * Tableau 4x4 représentant l'état des boutons :
 * - `1` : Bouton relâché.
 * - `0` : Bouton appuyé.
 */
extern int buttonState[4][4];

/**
 * @brief Initialise les broches GPIO pour la matrice de boutons.
 * 
 * Configure les broches des lignes comme sorties et celles des colonnes comme entrées.
 */
void initPins();

/**
 * @brief Détecte quel bouton est appuyé dans la matrice.
 * 
 * Met à jour le tableau `buttonState` en fonction des boutons détectés comme appuyés.
 */
int detectButton();

#endif // COMM_MATRICEBTN_H