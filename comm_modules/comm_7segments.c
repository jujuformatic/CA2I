#include <stdio.h>
#include <wiringPiI2C.h>
#include "comm_7segments.h"


// Identifiant du périphérique I2C
int i2c_fd;

// Table de correspondance pour les chiffres 0-9 sur un afficheur 7 segments
const unsigned char digitTable[] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F // 9
};

// Initialisation du HT16K33
void initHT16K33() {
    // Initialiser le périphérique I2C
    i2c_fd = wiringPiI2CSetup(HT16K33_ADDR);
    if (i2c_fd == -1) {
        perror("Erreur lors de l'initialisation de l'I2C");
        return;
    }

    // Activer l'oscillateur
    wiringPiI2CWrite(i2c_fd, 0x21);

    // Activer l'affichage et désactiver le clignotement
    wiringPiI2CWrite(i2c_fd, 0x81);

    // Régler la luminosité (0xE0 à 0xEF, 0xEF étant le maximum)
    wiringPiI2CWrite(i2c_fd, 0xE0 | 0x0F);
}

// Fonction pour afficher un nombre sur l'afficheur 7 segments
void displayNumber(int number) {
    if (number < 0 || number > 9999) {
        printf("Nombre hors limites (0-9999) : %d\n", number);
        return;
    }

    // Extraire les chiffres des milliers, centaines, dizaines et unités
    int thousands = (number / 1000) % 10;
    int hundreds = (number / 100) % 10;
    int tens = (number / 10) % 10;
    int units = number % 10;
    int separator = 0x3F; // Code pour le séparateur (point décimal)

    // Écrire les chiffres sur les segments correspondants
    wiringPiI2CWriteReg8(i2c_fd, 0x08, digitTable[units]);      // Unités
    wiringPiI2CWriteReg8(i2c_fd, 0x06, digitTable[tens]);       // Dizaines
    wiringPiI2CWriteReg8(i2c_fd, 0x04, digitTable[separator]);  // Séparateur
    wiringPiI2CWriteReg8(i2c_fd, 0x02, digitTable[hundreds]);   // Centaines
    wiringPiI2CWriteReg8(i2c_fd, 0x00, digitTable[thousands]);  // Milliers
}