#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "error.h"
#include "poly.h"
// #include <math.h> // If needed

typedef struct poly_t{
    int* coef;
    int* exp;
    int size;
} poly_t;

void bubbleSort(poly_t* poly) {
    if (poly == NULL || poly->size <= 1) {
        return; // No need to sort if the polynomial is null or has only one term
    }

    int i, j, tempCoef, tempExp;
    bool swapped;
    for (i = 0; i < poly->size - 1; i++) {
        swapped = false;
        for (j = 0; j < poly->size - i - 1; j++) {
            if (poly->exp[j] < poly->exp[j + 1]) {
                // Swap the terms
                tempCoef = poly->coef[j];
                tempExp = poly->exp[j];
                poly->coef[j] = poly->coef[j + 1];
                poly->exp[j] = poly->exp[j + 1];
                poly->coef[j + 1] = tempCoef;
                poly->exp[j + 1] = tempExp;
                swapped = true;
            }
        }

        // If no two elements were swapped by inner loop, then break
        if (!swapped) {
            break;
        }
    }
}


//INPUT: "x^10000000 + 2" or "2x^2 + 3x + 4"
inline __attribute__((always_inline)) poly_t *new_poly_from_string(const char* s){
    int c, i, size, x;
    size = 0;
    bool exponent = false;
    bool minus = false;
    bool num = false;
    x = 0;

    poly_t* poly = calloc(1, sizeof(poly_t));
    poly->coef = calloc(10000001, sizeof(int));
    poly->exp = calloc(10000001, sizeof(int));

    i = 0;
    while ((c = s[i]) != '\0'){
        i++;
        if (isdigit(c)){
            x = x * 10 + c - '0';
			num = true;
			continue;
        } else if (num){
            if (!exponent){
                if (minus){
                    poly->coef[size] = -x;
                } else {
                    poly->coef[size] = x;
                }
                poly->exp[size] = 0;
            } else {
                    poly->exp[size] = x;
            }
            num = false;
            exponent = false;
            minus = false;
            x = 0;
        }
        
        if (c == 'x'){
            if (poly->coef[size] == 0){
                poly->coef[size] = minus ? -1: 1;
                minus = false;
            }
            poly->exp[size] = 1;
        } else if (c == '^'){
            exponent = true;
        } else if (c == 32){
            // blank space
            continue;
        } else {
            // c = - OR +
            if (c == '-'){
                minus = true;
            } else {
                minus = false;
            }
            size++;
        }
    }

    if (num){
        if (!exponent){
            if (minus){
                poly->coef[size] = -x;
            } else {
                poly->coef[size] = x;
            }
            poly->exp[size] = 0;
        } else {
            poly->exp[size] = x;
        }
    }
    poly->size = size+1;
    /*
    for (i = 0; i <= size; i++){
        printf("%d:%d\n", poly->coef[i], poly->exp[i]);
    }*/
    return poly;
}

inline __attribute__((always_inline)) void free_poly(poly_t* poly){
    free(poly->coef);
    free(poly->exp);
    free(poly);
}

inline __attribute__((always_inline)) poly_t *mul(poly_t* a, poly_t* b){
    int coef;
    int exp;

    poly_t *res = calloc(1, sizeof(poly_t));
    res->coef = calloc(10000001, sizeof(int));
    res->exp = calloc(10000001, sizeof(int));
    int idx = 0;

    for (int x = 0; x < a->size; x++){
        for (int y = 0; y < b->size; y++){
            coef = a->coef[x] * b->coef[y];
            exp = a->exp[x] + b->exp[y];

            res->coef[idx] += coef;
            res->exp[idx] = exp;
            
            bool found = false;
            //printf("%d:%d\n", coef, exp);

            for (int i = 0; i < idx; i++){
                if (res->exp[i] == exp){
                    res->coef[i] += coef;
                    found = true;
                    break; 
                }
            }

            if (!found && coef != 0){
                res->coef[idx] = coef;
                res->exp[idx] = exp;
                idx++;
            }
        }
    }
    res->size = idx;
    bubbleSort(res);
    return res;
}




inline __attribute__((always_inline)) void print_poly(poly_t* poly){
    //printf("size: %d --> ", poly->size);
    bool first = true;
    
    for (int i = 0; i < poly->size; i++){
        int coef = poly->coef[i];
        if (!first && coef != 0){

            if (coef > 0){
                printf(" + ");
            } else{
                coef = -coef;
                printf(" - ");
            }
        }

        if (coef == 0){
            continue;
        } else {
            first = false;
            if (coef == 1 && poly->exp[i] == 0 ){
                printf("1");
                
            } else if (coef != 1 && poly->exp[i] == 0){
                printf("%d", coef);

            } else if (coef != 1 && poly->exp[i] == 1){
                printf("%dx", coef);

            } else if (coef != 1 && poly->exp[i] > 1){
                printf("%dx^%d", coef, poly->exp[i]);

            } else if (coef == 1 && poly->exp[i] > 1){
                printf("x^%d", poly->exp[i]);
            }
        }     
        
    }
    printf("\n");

}
/*

static void poly_test(const char* a, const char* b)
{
	poly_t*		p;
	poly_t*		q;
	poly_t*		r;

	printf("Begin polynomial test of (%s) * (%s)\n", a, b);

	p = new_poly_from_string(a);
	q = new_poly_from_string(b);

	print_poly(p);
	print_poly(q);

	r = mul(p, q);

	print_poly(r);

	free_poly(p);
	free_poly(q);
	free_poly(r);

	printf("End polynomial test of (%s) * (%s)\n", a, b);
}

int main(void)
{   
    poly_test("2", "3x + 2");
	putchar('\n');
	poly_test("x^2 - 7x + 1", "3x + 2");
	putchar('\n');
	poly_test("x^10000000 + 2", "2x^2 + 3x + 4");

	return 0;
}
*/



