#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>


double epsilon = 1e-6;

typedef struct _simplex_t{
    int         m;       //Constraints
    int         n;       //Decition variables
    int*        var;     //var[n+m+1]    0..n-1 non-basic
    double**    a;       //a[m][n+1]
    double*     b;       //b[m]
    double*     x;       //x[n+1]
    double*     c;       //c[n]
    double      y;

} simplex_t;

typedef struct _node_t{
    int m;
    int n;
    int k;
    int h;
    double xh;
    double ak;
    double bk;
    double* min; //min[n]
    double* max; //max[n]
    double** a; //a[m][n]
    double* b; //b[m]
    double* x; //x[n]
    double* c; //c[n]
    double z;
} node_t;

typedef struct list_node_t list_node_t;

struct list_node_t{
    list_node_t* next;
    list_node_t* prev;
    node_t* data;
};


double** make_matrix(int m, int n)
{
    double** a;
    int i;

    a = calloc(m, sizeof(double*));
    for (i = 0; i < m; i+=1) {
        a[i] = calloc(n+1, sizeof(double));
    }
    return a;
}
node_t* initial_node(int m, int n, double** a, double* b, double* c) {
    node_t* p = calloc(1,sizeof(node_t));
    p->m = m;
    p->n = n;
    p->a = calloc(m + 1, sizeof(double*));
    for (int i = 0; i < m + 1; i += 1) {
        p->a[i] = calloc(n + 1, sizeof(double));
    }
    p->b = calloc(m+1, sizeof(double));
    p->c = calloc(n+1, sizeof(double));
    p->x = calloc(n+1, sizeof(double));
    p->min = calloc(n, sizeof(double));
    p->max = calloc(n, sizeof(double));

    for(int i=0; i<m; i++){
        p->b[i] = b[i];
    }

    for(int i=0; i<n; i++){
        p->c[i] = c[i];
    }

    for(int i=0; i<m; i++){
        for (int j=0; j<n;j++){
            p->a[i][j] = a[i][j];
        }
    }

    for (int i=0; i<n; i++) {
        p->min[i] = -INFINITY;
        p->max[i] = INFINITY;
    }

    return p;
}

double xsimplex(int m, int n, double** a, double* b, double* c, double* x, double y, int* var, int h);

void print_matrix(simplex_t* s)
{
 //PRINT
    int m = s->m;
    int n = s->n;
    printf("\tmax z = ");
    for (int i = 0; i<n; i++) {
        printf("%1.3lfx%d", s->c[i], i);
        if (i != n-1) {
            printf(" + ");
        } 
    }
    printf("\n");

    for (int i = 0; i<m; i++) {
        for (int j = 0; j<n; j++) {
            printf("%10.3lfx%d\t", s->a[i][j], j);
            if (j == n-1) {
                printf("\u2264\t%1.3lf", s->b[i]);
                printf("\n");
            } else {
                printf("+");
            }
        }
    }
}

void free_simplex(simplex_t* s)
{
    free(s->b);
    free(s->c);
    free(s->x);
    free(s->var);
    for (int i = 0; i < s->m; i+=1) {
        free(s->a[i]);
    }
    free(s->a); 
}

void free_node(node_t* p)
{
    free(p->b);
    free(p->c);
    free(p->min);
    free(p->max);
    free(p->x); 
    for (int i = 0; i < p->m+1; i+=1) {
        free(p->a[i]);
    }
    free(p->a);
}



int init(simplex_t* s,int m, int n, double** a, double* b, double* c, double* x, double y, int* var)
{
    s->m = m;
    s->n = n;
    s->a = a;
    s->b = b;
    s->c = c;
    s->x = x;
    s->y = y;
    s->var = var;
    
    int i;
    int k = 0;

    if(s->var == NULL){
        s->var = calloc(m+n+1,sizeof(int));
        for (i = 0; i < m + n; i++){
            s->var[i] = i;
        }
    }
    for(i = 1, k = 0; i < m; i++){
        if (b[i] < b[k]){
            k = i;
        }
    }
    //printf("k-value: %d\n", k);
    return k;
}

int select_nonbasic(simplex_t* s) {
    for (int i=0; i<s->n; i++) {
        if (s->c[i]>epsilon) {
            return i;
        }
    }
    return -1;
}

void pivot(simplex_t* s, int row, int col) {
    double** a = s->a;
    double* b = s->b;
    double* c = s->c;
    int m = s->m;
    int n = s->n;

    int t = s->var[col];
    s->var[col] = s->var[n+row];
    s->var[n+row] = t;
    s->y = s->y + c[col] * b[row] / a[row][col];
 
    for(int i = 0; i < n; i++){                                 //c1
        if (i != col){
            c[i] = c[i] - c[col] * a[row][i] / a[row][col];
        }
    }
    c[col] = -c[col] / a[row][col];                             //

    for(int i=0; i<m; i++){
        if (i != row){
            b[i] = b[i] - a[i][col] * b[row] / a[row][col];
        }
    }

    for(int i=0; i<m; i++){
        if (i != row){
            for(int j=0; j<n; j++){
                if (j!=col){
                    a[i][j] = a[i][j] - a[i][col] * a[row][j] / a[row][col];
                }
            }
        }
    }

    for(int i=0; i<m; i++){
        if (i != row){
            a[i][col] = -a[i][col] / a[row][col];
        }
    }

    for(int i=0; i<n; i++){
        if (i != col){
            a[row][i] = a[row][i] / a[row][col];
        }
    }
    b[row] = b[row] / a[row][col];
    a[row][col] = 1 / a[row][col];
}

void prepare(simplex_t* s, int k){
    int m = s-> m;
    int n = s-> n;


    for (int i = m + n; i > n; i--){
        s->var[i] = s->var[i-1];
    }
    
    s->var[n] = m + n;
    n++;
    for (int i = 0; i < m; i++){
        s->a[i][n-1] = -1;
    }

    s->x = calloc(m+n, sizeof(double));
    s->c = calloc(n, sizeof(double));
    s->c[n-1] = -1;
    s->n = n;
    pivot(s, k, n-1);
 }

int initial(simplex_t* s, int m, int n, double** a, double* b, double* c, double* x, double y, int* var){
    int k = init(s, m, n, a, b, c, x, y, var);
    int i;
    int j;
    double w;
    
    
    if (b[k]>=0) {
        //printf("b-value when true: %lf\n", b[k]);
        //printf("return true! k is: %d\n", k);
        return 1;
    }
    
    prepare(s,k);
    n = s->n;
    s->y = xsimplex(m, n, s->a, s->b, s->c, s->x, 0, s->var, 1);
    for (i=0; i<m+n; i++) {
        if (s->var[i]==m+n-1) {
            //printf("\nx[i] larger than epsilon, x[i]: %lf\n", s->x[i]);
            if (fabs(s->x[i])>epsilon) {
                free(s->x);
                free(s->c);
                return 0;
            } else {
                break;
            }
        }
    }

    if (i>=n){
        for (j = k = 0; k < n; k++){
            if (fabs(s->a[i-n][k]) > fabs(s->a[i-n][j])){
                j = k;
            }
        }
        pivot(s, i-n, j);
        i = j;
    }

    if (i < n-1){
        k = s->var[i];
        s->var[i] = s->var[n-1];
        s->var[n-1] = k;
        for (k = 0; k < m; k++){
            w = s->a[k][n-1];
            s->a[k][n-1] = s->a[k][i];
            s->a[k][i] = w;
        }
    }

    free(s->c);
    s->c = c;
    s->y = y;
    for (k = n-1; k < n+m-1; k++){
        s->var[k] = s->var[k+1];
    }
    
    n = s->n = s->n-1;
    double* t = calloc(n, sizeof(double));
    for (k = 0; k < n; k++){
        for (j = 0; j < n; j++){
            if (k == s->var[j]){
                t[j] = t[j] + s->c[k];
                goto next_k;
            }
        }
        for(j = 0; j < m; j++){
            if(s->var[n+j] == k){
                break;
            }
        }
        s->y = s->y + s->c[k] * s->b[j];
        for(i = 0; i < n; i++){
            t[i] = t[i] - s->c[k] * s->a[j][i];
        }
    next_k:;
    }
    for(i = 0; i<n; i++){
        s->c[i] = t[i];
    }
    free(t);
    free(s->x);
    //printf("\nReached end");
    return 1;
}

double xsimplex(int m, int n, double** a, double* b, double* c, double* x, double y, int* var, int h) {
    simplex_t s;
    s.m = m;
    s.n = n;
    s.var = var;
    s.a = a;
    s.b = b;
    s.x = x;
    s.c = c;
    s.y = y;
    int col;    
    if(!initial(&s, m, n, a, b, c, x, y, var)){
        //printf("\nb value, initial false! b value is (supposed to be negative): %lf", s->b[2]);
        free(s.var);
        return NAN;
    }
    while ((col = select_nonbasic(&s)) >=0) {        //algoritmen kör tills alla c-värden är negativa => max z
        int row = -1;
        for (int i = 0; i<m; i++) {
            if ((a[i][col]>epsilon) && (row<0 || (b[i]/a[i][col])<(b[row]/a[row][col]))) {
                row = i;
            }
        }
        if (row<0) { // 0 or epsilon?
            free(s.var);
            return INFINITY;
        }
        pivot(&s, row, col);
    }
    if (h == 0){
        for(int i=0; i<n; i++){
            if (s.var[i] < n){
                x[s.var[i]] = 0;
            }
        }

        for(int i=0; i<m; i++){
            if (s.var[n+i] < n){
                x[s.var[n+i]] = s.b[i];
            }
        }
        free(s.var);
    } else {
        for(int i=0; i<n; i++) {
            x[i] = 0;
        }
        for(int i=n; i<m+n; i++) {
            x[i] = s.b[i-n];
        }
    }
    return s.y;
}

double simplex(int m, int n, double** a, double* b, double* c, double* x, double y){
    return xsimplex(m,n,a,b,c,x,y,NULL,0);
}

int is_integer(double* xp){
    double x = *xp;
    double r = lround(x);
    if (fabs(r-x) < epsilon) {
      *xp = r;
      return 1;
    } else {
        return 0;
    }
}

int integer(node_t* p){
    for(int i=0; i<p->n; i++){
        if (!is_integer(&p->x[i])) return 0;
    }
    return 1;
}

int branch(node_t *q, double z){
    double min;
    double max;

   // printf("BRANCH: min: %lf max: %lf", *q->min, *q->max);

    if (q->z<z) return 0;

    for (int h=0; h<q->n; h++) {
        if (!is_integer(&q->x[h])) {
            if (q->min[h] == -INFINITY) {
                min = 0;
            } else {
                min = q->min[h];
            }
            max = q->max[h];
            if ((floor(q->x[h])<min)||(ceil(q->x[h])>max)) {
                continue;
            }
            q->h = h;
            q->xh = q->x[h];

            for (int i=0; i< q->m + 1;i++) {
                 free(q->a[i]);
            }
            free(q->a);
            free(q->b);
            free(q->c);
            free(q->x);
            return 1;
        }
    }
    return 0;
}

node_t* extend(node_t* p, int m, int n, double** a, double*b, double* c, int k, double ak, double bk){
    node_t* q = calloc(1,sizeof(node_t));
    int i;
    int j;
    q->k = k;
    q->ak = ak;
    q->bk = bk;
    if (ak > 0 && p->max[k] < INFINITY){
        q->m = p->m;
    }
    else if (ak < 0 && p->min[k] > 0){
        q->m = p->m;
    }
    else{
        q->m = p->m + 1;
    }
    q->n = p->n;
    q->h = -1;
    q->a = calloc(q->m + 1, sizeof(double*));
    for (i = 0; i < q->m + 1; i += 1) {
        q->a[i] = calloc(q->n + 1, sizeof(double));
    }
    q->b = calloc(q->m+1, sizeof(double));
    q->c = calloc(q->n+1, sizeof(double));
    q->x = calloc(q->n+1, sizeof(double));
    q->min = calloc(n, sizeof(double));
    q->max = calloc(n, sizeof(double));
    for(int i=0; i < p->n; i++){
        q->max[i] = p->max[i];
        q->min[i] = p->min[i];
    }
    for (int i=0; i<m;i++) {
            for (int j = 0; j < n; j += 1) {
                q->a[i][j] = a[i][j];
            }
            q->b[i] = b[i];
        }
    

    for (int i=0; i<n;i++){
        q->c[i] = c[i];
    }

    if (ak > 0){
        if (q->max[k] == INFINITY || bk < q->max[k]) {
            q->max[k] = bk;
        }
    }
    else if (q->min[k] == -INFINITY || -bk > q->min[k]){
        q->min[k] = -bk;
    }
    
    for(i = m, j=0; j<n; j++) {
        if (q->min[j] > - INFINITY) {
            q->a[i][j]= -1;
            q->b[i] = -q->min[j];
            i++;
        }
        if (q->max[j]<INFINITY) {
            q->a[i][j] = 1;
            q->b[i]= q->max[j];
            i++;
        }
    }
    return q; 
}

void bound(node_t* p, list_node_t** h, double* zp, double* x){
    if(p->z > *zp){
        *zp = p->z;
        for (int i=0; i<p->n; i++) {
            x[i] = p->x[i];
        }
        list_node_t* current = *h;
        while(current != NULL){
            list_node_t* next = current->next;
            if(current->data->z < p->z) {
                if(current->prev != NULL){
                    current->prev->next = current->next;
                } else{
                    *h = current->next;
                }
                if (current->next != NULL){
                    current->next->prev = current->prev;
                }
                free(current->data->min);
                free(current->data->max);
                free(current->data);
                free(current);
            }
            current = next;
        }
        // list_node_t* current = h;
        // while(current->next != h){
        //     if (current->data->z < p->z) {
        //         current->prev->next = current->next;
        //         current->next->prev = current->prev;
        //     }
        //     current = current->next;
        // }
        // if (current->data->z < p->z) {
        //         current->prev->next = current->next;
        //         current->next->prev = current->prev;
        //     }
        // if ((current->next == current->prev && current->next == current) && current->data->z < p->z){
        //     printf("\n här");
        //     h = NULL;
        // }
    }
}

void succ(node_t* p, list_node_t** h, int m, int n, double** a, double* b, double* c, int k, double ak, double bk, double* zp, double* x){
    node_t* q = extend(p,m,n,a,b,c,k,ak,bk);
    if (q==NULL) return;
    q->z = simplex(q->m, q->n, q->a, q->b, q->c, q->x, 0);
    if (isfinite(q->z)) {
        if (integer(q)) {
            bound(q, h, zp, x);    
        } else if(branch(q, *zp)){
            list_node_t* new_elem = calloc(1, sizeof(list_node_t));
            new_elem->data = q;
            new_elem->next = *h;
            if (*h != NULL) {
                (*h)->prev = new_elem;
            }
            *h = new_elem;
            return;
        }
    }
    for (int i = 0; i < q->m + 1; i += 1) {
        free(q->a[i]);
    }
    free(q->a);
    free(q->b);
    free(q->c);
    free(q->x);
    free(q->min);
    free(q->max);
    free(q);
}

double intopt(int m, int n, double** a, double* b, double* c, double* x){
    
    node_t* p = initial_node(m, n, a, b, c);
    list_node_t* h = calloc(1, sizeof(list_node_t));
    h->data = p;

    double z = -INFINITY;
    //printf("z-value %lf", z);
    p->z = simplex(p->m, p->n, p->a, p->b, p->c, p->x, 0);

    if (integer(p) || !isfinite(p->z)) {
        z = p->z;
        if (integer(p)) {
            for (int i = 0; i < p->n; i += 1) {
                x[i] = p->x[i];
            }
        }
        for (int i = 0; i < p->m + 1; i += 1) {
            free(p->a[i]);
        }
        free(p->a);
        free(p->b);
        free(p->c);
        free(p->x);
        free(p->min);
        free(p->max);
        free(p);
        free(h);
        return z;
    }
    branch(p,z);
    while (h != NULL) {
        p = h->data;
        list_node_t* prev = h;
        if(h->next != NULL){
            h->next->prev = NULL;
        }
        h = h->next;
        free(prev);

        succ(p, &h, m, n, a, b, c, p->h, 1, floor(p->xh), &z, x);
        succ(p, &h, m, n, a, b, c, p->h, -1, -ceil(p->xh), &z, x);
        free(p->min);
        free(p->max);
        free(p);
    }
    
    if (z== -INFINITY) {
        return NAN;
    }
    else{
        return z;
    }
}

int main(int argc, char** argv) 
{
    simplex_t* s = calloc(1,sizeof(simplex_t));

    scanf("%d %d\n", &s->m, &s->n);

    s->a = make_matrix(s->m, s->n+1); //size sets to [m][n+1] in the function.
    s->b = calloc(s->m, sizeof(double));
    s->c = calloc(s->n, sizeof(double));
    s->x = calloc(s->n+1, sizeof(double));

    for (int i = 0; i<s->n; i++) {
        scanf("%lf\n", &s->c[i]);
    }

    for (int i = 0; i<s->m; i++) {
        for (int j = 0; j<s->n; j++) {
            scanf("%lf\n", &s->a[i][j]);
        }
    }

    for (int i = 0; i<s->m; i++) {
        scanf("%lf\n", &s->b[i]);
    }
    //printf("Inital matrix: \n");
    //print_matrix(s);
    double y = 0;
    y = intopt(s->m,s->n,s->a,s->b,s->c,s->x);

    printf("\nY is %lf", y);
    printf("\n");

    //Free
    free_simplex(s);
    free(s);
    return 0;
}