#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct word_t {
    char* word;
    int freq;
} word_t;

int isPrime(int num){
    int i;
    if (num == 0 || num == 1)
       return 0;

    for (i = 2; i <= num / 2; ++i) {
        if (num % i == 0) {
        return 0;
        }
    }

    return 1;
}

int main(){
    char* line = calloc(1048, sizeof(char));
    unsigned int used = 0, found = 0, linenbr = 1, i;

    word_t* words = calloc(10000, sizeof(word_t));
    for (int i = 0; i<10000; i++){
        words[i].word = calloc(1048, sizeof(char));
    }

    while(fgets(line, 1048, stdin) != NULL){
        line[strlen(line)-1] = '\0';

        for (i = 0; i<used; i++){
            if (!strcmp(line, words[i].word)){
                if (words[i].freq == 0){
                    break;
                }
                found = 1;
                break;
            }
        }

        if (!found){
            if (!isPrime(linenbr)){
                strcpy(words[i].word, line);
                words[i].freq = 1;
                used++;
                printf("added %s\n", line);
            } else {
                printf("trying to delete %s: not found\n", line);
            }
        } else if (isPrime(linenbr)){
            words[i].freq = 0;
            printf("trying to delete %s: deleted\n", line);
        } else {
            words[i].freq++;
            printf("counted %s\n", line);
        }

        found = 0;
        linenbr++;
    }

    word_t best = words[0];
    for (i = 1; i < used; i++) {
        if (words[i].freq > best.freq)
            best = words[i];
    }
    //printf("array: \n(%s %d)\n(%s %d)\n(%s %d)\n(%s %d)\n", words[0].word, words[0].freq, words[1].word, words[1].freq, words[2].word, words[2].freq, words[3].word, words[3].freq);
    printf("result: %s %d\n", best.word, best.freq);

    for (int i = 0; i<10001; i++){
        free(words[i].word);
    }
    free(words);
    free(line);
    
    return 0;
}


