/**
 * @file cards.c
 * @version 1.0
 * @date 22/03/2025
 * @authors Julien Martinez
 * @brief Ce fichier permet de tester le tirage aléatoire des cartes
 * @result Fonctionnel
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 
 #define ANSWERS "../Answers.txt"
 #define QUESTIONS "../Questions.txt"
 #define NB_ANSWERS 8
 #define QUESTION_MAX_LENGTH 100
 #define ANSWER_MAX_LENGTH 50

 void getRandomCards(char *file, int cardsNumber, char **destination) {
    char command[128];
    FILE *fp;
    int i;

    snprintf(command, sizeof(command), "awk 'length<=50' %s | shuf -n %d", file, cardsNumber);
    fp = popen(command, "r");

    for (i = 0; i < cardsNumber; i++) {
        fgets(destination[i], __INT_MAX__, fp);

        // Retire le caractère de retour à la ligne
        destination[i][strcspn(destination[i], "\n")] = '\0';
    }

    pclose(fp);
}
  
int main() {
    int i;
    char *answers[NB_ANSWERS];
    for (i=0; i<NB_ANSWERS; i++)
        answers[i] = malloc(ANSWER_MAX_LENGTH*sizeof(char));

    char *question = malloc(QUESTION_MAX_LENGTH*sizeof(char));
    
    getRandomCards(ANSWERS, NB_ANSWERS, answers);
    getRandomCards(QUESTIONS, 1, &question);

    printf("Generated question:\n");
    printf("%s\n", question);
    
    printf("\nGenerated answers:\n");
    for (i = 0; i < NB_ANSWERS; i++)
        printf("%s\n", answers[i]);
    
    return 0;
}
 