#include "comm_matriceBtn.h"

// Etat des boutons (1 = relaché, 0 = appuyé)
int buttonState[4][4] = {
	{1, 1, 1, 1},
	{1, 1, 1, 1},
	{1, 1, 1, 1},
	{1, 1, 1, 1}
};

// Initialisation des broches
void initPins() {
    // Initialiser les broches WiringPi
    wiringPiSetup();

    // Configurer les lignes comme sorties
    pinMode(ROW1, OUTPUT);
    pinMode(ROW2, OUTPUT);
    pinMode(ROW3, OUTPUT);
    pinMode(ROW4, OUTPUT);

    // Configurer les colonnes comme entrées avec pull-up
    pinMode(COL1, INPUT);
    pullUpDnControl(COL1, PUD_UP);
    pinMode(COL2, INPUT);
    pullUpDnControl(COL2, PUD_UP);
    pinMode(COL3, INPUT);
    pullUpDnControl(COL3, PUD_UP);
    pinMode(COL4, INPUT);
    pullUpDnControl(COL4, PUD_UP);
}

// Fonction pour détecter quel bouton est appuyé
int detectButton() {
    int rows[4] = {ROW1, ROW2, ROW3, ROW4};
    int cols[4] = {COL1, COL2, COL3, COL4};
	int i, j;
    int buttonPressed = 0;

    for (i = 0; i < 2; i++) {
        // Activer une ligne à la fois (mettre à LOW)
        digitalWrite(rows[i], LOW);

        for (j = 0; j < 4; j++) {
            // Lire l'état de chaque colonne
            if (digitalRead(cols[j]) == LOW) {
                // Bouton appuyé
                buttonPressed = (4 * i) + j + 1; 
                printf("Bouton appuyé : Ligne %d, Colonne %d\n", i + 1, j + 1);
                delay(200); // Anti-rebond
            }
        }

        // Désactiver la ligne (remettre à HIGH)
        digitalWrite(rows[i], HIGH);
    }
    return buttonPressed;
}