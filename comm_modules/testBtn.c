#include "comm_matriceBtn.h"

int main(int argc, char *argv[]) {
    // Initialiser les broches
    initPins();

    // Boucle principale pour détecter les boutons
    while (1) {
        detectButton(); // Détecter les boutons
        delay(100); // Petite pause pour éviter une surcharge CPU
    }

    return 0;
}