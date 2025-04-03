#include "comm_matriceLed.h"

unsigned char matrix[8] = {0};

void initMatriceLed() {
    if (wiringPiSetup() == -1) {
        printf("Erreur lors de l'initialisation de WiringPi\n");
    }
    
    if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1) {
        printf("Erreur lors de l'initialisation SPI\n");
    }

    initMAX7219();
}

// Envoi d'une commande au MAX7219
void sendCommand(unsigned char address, unsigned char value) {
    unsigned char buffer[2];
    buffer[0] = address;
    buffer[1] = value;
    wiringPiSPIDataRW(SPI_CHANNEL, buffer, 2);
}

// Initialisation du MAX7219
void initMAX7219() {
    sendCommand(0x09, 0x00);  // Pas de mode décodeur
    sendCommand(0x0A, 0x03);  // Luminosité moyenne (3 sur 15)
    sendCommand(0x0B, 0x07);  // Affichage sur les 8 colonnes
    sendCommand(0x0C, 0x01);  // Sortie en mode normal (pas en veille)
    sendCommand(0x0F, 0x00);  // Pas de test d'affichage
    clearall(); // Efface l'affichage au démarrage
    delay(100); // Attendre 100 ms pour stabiliser
}

void setLED(int row, int col, int state) {
    if (state) {
        matrix[row] |= (1 << col);  // Active le bit correspondant
    } else {
        matrix[row] &= ~(1 << col); // Désactive le bit correspondant
    }
    sendCommand(row + 1, matrix[row]); // Met à jour l'affichage
}

void clearall() {
    int i;
    for (i = 0; i < 8; i++) {
        matrix[i] = 0; // Réinitialise la matrice
        sendCommand(i + 1, matrix[i]); // Met à jour l'affichage
    }
}

void afficherPoints(int *scores) {
    int i, j;
    for (i = 0; i < LED_MATRIX_ROWS; i++) {
        for (j = 7; j > 7 - scores[i]; j--) {
            setLED(i, j, 1);
        }
    }
}