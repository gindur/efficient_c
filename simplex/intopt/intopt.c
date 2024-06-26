#include <stddef.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

double epsilon = 1e-6;

double xsimplex (int m,int n,double** a, double* b, double* c, double* x, double y, int* var, int h);
double simplex(int m, int n, double** a, double* b, double* c,double* x, double y);

double** make_matrix(int m, int n)
{
    double** a;

    a = calloc(m, sizeof(double*));
    for (int i = 0; i < m; i += 1){
        a[i] = calloc(n+1, sizeof(double));
    }   
    return a;
}



typedef struct {
    int m ; /* Constraints. */
    int n ; /* Decision variables. */
    int *var ; /* 0..n  1 are nonbasic. */
    double **a; /* A. */
    double *b ; /* b. */
    double *x ; /* x. */
    double *c ; /* c. */
    double y ; /* y. */
} simplex_t;

typedef struct {
    int m; /* Constraints. */
    int n; /* Decision variables. */
    int k; /* Parent branches on xk. */
    int h; /* Branch on xh. */
    double xh; /* xh. */
    double ak; /* Parent ak. */
    double bk; /* Parent bk. */
    double* min; /* Lower bounds. */
    double* max; /* Upper bounds. */
    double** a; /* A. */
    double* b; /* b. */
    double* x; /* x. */
    double* c; /* c. */
    double z ; /* z. */
} node_t;

typedef struct list_node_t list_node_t;

struct list_node_t {
    list_node_t* next;
    list_node_t* prev;
    node_t* data;
};

node_t* initial_node(int m, int n, double** a, double* b, double* c)
{   
    int i;
    node_t* p = calloc(1, sizeof(node_t));
    p->a = make_matrix(m+1,n+1);
    p->b = calloc(m+1, sizeof(double));
    p->c = calloc(n+1, sizeof(double));
    p->x = calloc(n+1, sizeof(double));
    p->min = calloc(n, sizeof(double));
    p->max = calloc(n, sizeof(double));

    p->m = m;
    p->n = n;

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

    for (i = 0; i < n; i++){
        p->min[i] = -INFINITY;
        p->max[i] = INFINITY;   
    }
    return p;
}


node_t* extend (node_t* p, int m, int n, double** a, double* b, double* c, int k, double ak, double bk){
    node_t* q = calloc(1, sizeof(node_t));
    int i,j;

    q->k = k;
    q->ak = ak;
    q->bk = bk;

    if (ak > 0 && p->max[k] < INFINITY){
        q->m = p->m;
    } else if (ak < 0 && p->min[k] > 0){
        q->m = p->m;
    } else {
        q->m = p->m + 1;
    }
    q->n = p->n;
    q->h = -1;
    q->a = make_matrix(q->m+1, q->n+1); // note normally q.m > m
    q->b = calloc(q->m+1, sizeof(double));
    q->c = calloc(q->n+1, sizeof(double));
    q->x = calloc(q->n+1, sizeof(double));
    q->min = calloc(n, sizeof(double));
    q->max = calloc(n, sizeof(double));

    for (i = 0; i<p->n; i++){
        q->min[i] = p->min[i];
        q->max[i] = p->max[i];
    }

    for (i = 0; i < m; i++){
        for (j = 0; j < n+1; j++){
            q->a[i][j] = a[i][j];
        }
        q->b[i] = b[i];
    }

    for (i = 0; i<n; i++){
        q->c[i] = c[i];
    }

    if (ak > 0){
        if (q->max[k] == INFINITY || bk < q->max[k]){
            q->max[k] = bk;
        }
    } else if (q->min[k] == -INFINITY || -bk > q->min[k]){
        q->min[k] = -bk;
    } 
    for (i = m, j = 0; j < n; j++) {
        if (q->min[j] > -INFINITY){
            q->a[i][j] = -1;
            q->b[i] = -q->min[j];
            i++;
        }
        if (q->max[j] < INFINITY){
            q->a[i][j] = 1;
            q->b[i] = q->max[j];
            i++;
        }
    }
    return q;
}



int is_integer(double* xp){
    double x = *xp;
    double r = lround(x);
    if (fabs(r-x) < epsilon){
        *xp =  r;
        return 1;
    } else {
        return 0;
    }
}

int integer(node_t* p){
    int i;
    for (i=0; i<p->n; i++){
        if (!is_integer(&p->x[i])){
            return 0;
        }
    }
    return 1;
}


void bound(node_t* p, list_node_t** h, double* zp, double* x){
    int i;
    if (p->z > *zp){
        *zp = p->z;
        for (i = 0; i < p->n; i++){
            x[i] = p->x[i];
        }
        list_node_t* current = *h;
        while(current != NULL){
            list_node_t* next = current->next;
            if (current->data->z < p->z){
                if (current->prev != NULL){
                    current->prev->next = current->next;
                } else {
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
    }
}


int branch(node_t* q, double z){
    double min, max;
    int h;
    int i, j;
    if (q->z < z){
        return 0 ;
    }
    for (h = 0; h < q->n; h++){
        if (!is_integer(&q->x[h])){
            if (q->min[h] == -INFINITY){
                min = 0;
            } else {
                min = q->min[h];
            }
            max = q->max[h];
            if ((floor(q->x[h]) < min) || (ceil(q->x[h]) > max)){
                continue;
            }
            q->h = h;
            q->xh = q->x[h];
            for (i = 0; i<q->m + 1; i++){
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


void succ(node_t* p, list_node_t** h, int m, int n, double** a, double* b, double* c, int k, double ak, double bk, double* zp, double* x){
    node_t* q = extend(p,m,n,a,b,c,k,ak,bk);
    if ( q == NULL){
        return;
    }
    q->z = simplex(q->m, q->n, q->a, q->b, q->c, q->x, 0);
    if (isfinite(q->z)){
        if (integer(q)){
            bound(q, h, zp, x);
        } else if (branch(q, *zp)){
            //add q to h
            list_node_t* new_elem = calloc(1, sizeof(list_node_t));
            new_elem->data = q;
            new_elem->next = *h;
            if (*h != NULL){
                (*h)->prev = new_elem;
            }
            *h = new_elem;
            return;
        }
    }
    //delete q
    for (int i = 0; i<q->m+1; i++){
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
    p->z = simplex(p->m, p->n, p->a, p->b, p->c, p->x, 0);

    if (integer(p) || !isfinite(p->z)){
        z = p->z;
        if (integer(p)){
            for (int i = 0; i < p->n; i++){
                x[i] = p->x[i];
            }
        }
        for (int i = 0; i < p->m+1; i++){
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
    while (h != NULL){
        //take p from h
        p = h->data;
        list_node_t* prev = h;
        if (h->next != NULL){
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
    if ( z == -INFINITY){
        return NAN;
    } else{
        return z;
    }
}



int init(simplex_t* s, int m, int n, double** a, double* b, double* c, double* x, double y, int* var){
    int i, k;

    s->m = m;
    s->n = n;
    s->var = var;
    s->a = a;
    s->b = b;
    s->x = x;
    s->c = c;
    s->y = y;

    if (var == NULL){
        s->var = calloc(m + n + 1, sizeof(int));
        for ( i = 0; i < m+n; i++){
            s->var[i] = i;
        }
    }
    for (k = 0, i = 1; i < m; i++){
        if (b[i] < b[k]){
            k = i;
        }
    }
    return k;
}

int select_nonbasic(simplex_t* s){
    int i;
    for (i = 0; i < s->n; i++){
        if (s->c[i] > epsilon){
        return i;
        }    
    }
    return -1;
}





void pivot(simplex_t* s, int row, int col){
    double** a = s->a;
    double* b = s->b;
    double* c = s->c;
    int m = s->m;
    int n = s->n;
    int i, j, t;
    
    t = s->var[col];
    s->var[col] = s->var[n+row];
    s->var[n+row] = t;
    double a_rc = 1 / a[row][col];
    s->y = s->y + c[col] * b[row] * a_rc;


    for (i = 0; i < n; i++){
        if (i != col){
            c[i] = c[i] - c[col] * a[row][i] * a_rc;
        }
    }
    c[col] = - c[col] * a_rc;

    for (i=0; i < m; i++){
        if (i != row){
            b[i] = b[i] - a[i][col] * b[row] * a_rc;
        }
    }
    for (i=0; i<m; i++){
        if (i!=row){
            for (j=0; j<n; j++){
                if (j!=col){
                    a[i][j] = a[i][j] - a[i][col] * a[row][j] * a_rc;
                }
            }
        }
    }
    for (i = 0; i < m; i++){
        if (i != row){
            a[i][col] = -a[i][col] * a_rc;
        }
    }
    for (i=0;i<n; i++){
        if (i!=col){
            a[row][i] = a[row][i] * a_rc;
        }
    }   
    b[row] = b[row] * a_rc;
    a[row][col] = a_rc;
}


void prepare(simplex_t* s, int k){
    int m = s->m;
    int n = s->n;
    int i;

    for (i = m+n; i>n; i--){
        s->var[i] = s->var[i-1];
    }
    s->var[n] = m + n;
    n++;
    for (i=0;i<m; i++){
        s->a[i][n-1] = -1;
    }
    s->x = calloc(m+n, sizeof(double));
    s->c = calloc(n, sizeof(double));
    s->c[n-1] = -1;
    s->n = n;
    pivot(s, k, n-1);
}


int initial(simplex_t* s, int m, int n, double** a, double* b,double* c,double* x, double y, int* var){
    int i, j;
    double w;
    int k = init(s, m, n, a, b, c, x, y, var);
    if (b[k] >= 0){
        return 1;
    }
    prepare(s,k);
    n = s->n;
    s->y = xsimplex(m, n, s->a, s->b, s->c, s->x, 0, s->var,1);
    for (i = 0; i<m+n; i++){
        if (s->var[i] == m + n -1){
            if (fabs(s->x[i]) > epsilon){
                free(s->x);
                free(s->c);
                return 0;
            } else {
                break;
            }
        }
    }
    if (i >= n){
        for(j=k=0; k<n; k++){
            if (fabs(s->a[i-n][k]) > fabs(s->a[i-n][j])){
                j = k;
            }
        }
        pivot(s, i-n, j);
        i = j;
    }
    if (i< n-1){
        k = s->var[i];
        s->var[i] =  s->var[n-1];
        s->var[n-1] = k;
        for (k = 0; k<m; k++){
            w = s->a[k][n-1];
            s->a[k][n-1] = s->a[k][i];
            s->a[k][i] = w;
        }
    }
    free(s->c);
    s->c = c;
    s->y = y;
    for (k = n-1; k < n+m-1;k++){
        s->var[k] = s->var[k+1];
    }
    n = s->n = s->n-1;
    double* t = calloc(n, sizeof(double));
    for (k=0; k<n; k++){
        for (j=0; j<n; j++){
            if(k == s->var[j]){
                t[j] = t[j] + s->c[k];
                goto next_k;
            }           
        }
        for (j=0; j<m; j++){
            if(s->var[n+j] == k){
                break;
            }
        }
        s->y = s->y + s->c[k] * s->b[j];
        for (i=0; i<n; i++){
            t[i] = t[i] - s->c[k] * s->a[j][i];
        }
        next_k:;
    }
    for (i=0; i<n; i++){
        s->c[i] = t[i];
    }
    free(t);
    free(s->x);
    return 1;
}


//Helper func
double sumArrayElements(double* array, int size) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += array[i];
    }
    return sum;
}

double xsimplex (int m,int n,double** a, double* b, double* c, double* x, double y, int* var, int h){
    simplex_t s;
    int i, row, col;

    if (!initial(&s, m, n, a, b, c, x, y, var)){
        free(s.var);
        return NAN;
    }
    while ((col = select_nonbasic(&s))>=0){
        row = -1;
        for (i = 0; i<m; i++){
            if ((a[i][col] > epsilon)  && (row < 0 || b[i] / a[i][col] < b[row] / a[row][col])){
                row = i;
            }
        }
        if (row < 0 ){
            free(s.var);
            return INFINITY;
        }
        pivot(&s, row, col);
    }
    if (h==0){
        for (i=0; i<n; i++){
            if (s.var[i] < n){
                x[s.var[i]] = 0;
            }
        }
        for (i=0; i<m; i++){
            if (s.var[n+i] < n){
                x[s.var[n+i]] = s.b[i];
            }
        }
        free(s.var);
    } else{
        for (i = 0; i < n; i++){
            x[i] = 0;
        }
        for (i = n; i < n+m; i++){
            x[i] = s.b[i-n];
        }
    }
    return s.y;
}




double simplex(int m, int n, double** a, double* b, double* c,double* x, double y){
    return xsimplex(m, n, a, b, c, x, y, NULL, 0);
}

void free_simplex(simplex_t* s){
    free(s->b);
    free(s->c);
    free(s->x);
    free(s->var);
    for (int i = 0; i< s->m; i++){
        free(s->a[i]);
    }
    free(s->a);
}




//#define MAIN
#ifdef MAIN
int main(int argc, char** argv)
{

    simplex_t* s = calloc(1, sizeof(*s));

    scanf("%d %d", &s->m, &s->n);

    int i, j;

    s->a = make_matrix(s->m, s->n+1);
    s->b = calloc(s->m, sizeof(double));
    s->c = calloc(s->n, sizeof(double));
    s->x = calloc(s->n+1, sizeof(double));

    for (i = 0; i < s->n; i++){
        scanf("%lf", &s->c[i]);
    }

    for (i = 0; i < s->m; i++){
        for (j = 0; j < s->n; j++){
            scanf("%lf", &s->a[i][j]); 
        }
    }

    for (i = 0; i < s->m; i++){
        scanf("%lf", &s->b[i]); 
    }

    double res = intopt(s->m, s->n, s->a, s->b, s->c, s->x);
    printf("\n%lf", res);
    // Free allocated memory
    free_simplex(s);
    free(s);
    return 0;

}
#endif
