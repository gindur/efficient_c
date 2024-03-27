typedef struct node_t node_t;
struct node_t {
    node_t* next;
    /* other declarations. */
};

void f(node_t** h, node_t* p)
{
    p->next = (*h);
    *h = p;
}
void f2(node_t** h, node_t* p)
{
    if (*h == NULL){
        *h = p;
        return
    }

    node_t* temp = *h;
    while (temp->next != NULL){
        temp = temp->next;
    }
    temp->next = p;
    return
}

double g(void)
{
    node_t* h;
    node_t* q;
    /* more code. */
    h = NULL;
    q = calloc(1, sizeof(node_t));


    /* call f somehow. */
    f2(&h, q);
}

