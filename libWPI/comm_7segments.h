#ifndef COMM_7SEGMENTS_H
#define COMM_7SEGMENTS_H

#include <wiringPi.h>
#include <softTone.h>

/**
 * @file comm_7segments.h
 * @brief Gestion d'un afficheur 7 segments via le contrôleur HT16K33.
 * 
 * Ce fichier contient les définitions et déclarations nécessaires pour gérer
 * un afficheur 7 segments connecté via I2C au contrôleur HT16K33.
 */

/**
 * @brief Adresse I2C du contrôleur HT16K33.
 */
#define HT16K33_ADDR 0x70

/**
 * @brief Table de correspondance pour les chiffres 0-9 sur un afficheur 7 segments.
 * 
 * Chaque élément de ce tableau correspond au motif binaire permettant d'afficher
 * un chiffre sur un afficheur 7 segments.
 */
extern const unsigned char digitTable[];

/**
 * @brief Identifiant du périphérique I2C.
 * 
 * Cet identifiant est utilisé pour communiquer avec le contrôleur HT16K33 via I2C.
 */
extern int i2c_fd;

/**
 * @brief Initialise le contrôleur HT16K33.
 * 
 * Configure le contrôleur HT16K33 pour un fonctionnement normal, active l'oscillateur,
 * désactive le clignotement et règle la luminosité.
 */
void initHT16K33();

/**
 * @brief Affiche un nombre sur l'afficheur 7 segments.
 * 
 * @param number Nombre entier à afficher (compris entre 0 et 9999).
 * 
 * Cette fonction décompose le nombre en chiffres individuels et les affiche
 * sur les segments correspondants.
 */
void displayNumber(int number);

#endif // COMM_7SEGMENTS_H