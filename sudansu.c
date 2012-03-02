#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define D 3
#define N (D*D)

struct node {
    union {
        char size;      // headers exclusively use size (to find most interesting column)
        uint32_t name;  // cells exclusively use name (to print solution)
    };
    struct node *left, *up, *right, *down;
    struct node *column;
};

enum {CST_CELL, CST_ROW, CST_COL, CST_BOX, CST_N};

#define NODES_HEIGHT (N+1) /* +1 for header */
#define NODES_WIDTH  (CST_N * N*N)

struct sudansu {
    uint32_t solution[N*N];
    struct node h;
    struct node nodes[NODES_HEIGHT * NODES_WIDTH];
    struct node *rows[N*N*N];
};

static unsigned get_node_position(unsigned type, unsigned r, unsigned c, unsigned z)
{
    switch (type) {
    case CST_CELL:  return c + r*N;
    case CST_ROW:   return z + r*N;
    case CST_COL:   return z + c*N;
    case CST_BOX:   return z + c/D*N + r/D*D*N;
    default: abort();
    }
}

#define CELL_NAME(row, col, numchr) ((row)<<16 | (col)<<8 | (numchr))

static void init(struct sudansu *ctx)
{
    unsigned i, j, r, c, z, cst_type;
    struct node *x, *row_nodes[CST_N]; // one node per line per constraint type

    memset(ctx, 0, sizeof(*ctx));

    /* vertical links */
    x = ctx->nodes;
    for (j = 0; j < NODES_HEIGHT; j++) {
        for (i = 0; i < NODES_WIDTH; i++) {
            x->up     = j ==              0 ? &ctx->nodes[(NODES_HEIGHT-1)*NODES_WIDTH + i] : x-NODES_WIDTH;
            x->down   = j == NODES_HEIGHT-1 ? &ctx->nodes[               0*NODES_WIDTH + i] : x+NODES_WIDTH;
            x->column = &ctx->nodes[i];
            x++;
        }
    }

    /* header horizontal links */
    ctx->h.right = ctx->nodes;
    ctx->h.left  = ctx->nodes + NODES_WIDTH-1;
    for (i = 0; i < NODES_WIDTH; i++) {
        ctx->nodes[i].size  = N;
        ctx->nodes[i].left  = i ==             0 ? &ctx->h : &ctx->nodes[i - 1];
        ctx->nodes[i].right = i == NODES_WIDTH-1 ? &ctx->h : &ctx->nodes[i + 1];
    }

    /* nodes horizontal links */
    i = 0;
    for (r = 0; r < N; r++) {
        for (c = 0; c < N; c++) {
            for (z = 0; z < N; z++) {

                /* locate the CST_N nodes per row */
                for (cst_type = 0; cst_type < CST_N; cst_type++) {
                    unsigned col = get_node_position(cst_type, r, c, z);

                    x = ctx->nodes[cst_type*N*N + col].down; // skip the header
                    while (x->name) // locate first unused node of the column
                        x = x->down;
                    x->name = CELL_NAME(r, c, z+'1');
                    row_nodes[cst_type] = x;
                }

                /* link them together */
                for (cst_type = 0; cst_type < CST_N; cst_type++) {
                    row_nodes[cst_type]->left  = row_nodes[cst_type == 0 ? CST_N-1 : cst_type-1];
                    row_nodes[cst_type]->right = row_nodes[cst_type == CST_N-1 ? 0 : cst_type+1];
                }

                /* index the row since the row positions in ctx->nodes are not linear */
                ctx->rows[i++] = row_nodes[0];
            }
        }
    }
}

static void cover(struct node *c)
{
    struct node *i, *j;

    c->right->left = c->left;
    c->left->right = c->right;
    for (i = c->down; i != c; i = i->down) {
        for (j = i->right; j != i; j = j->right) {
            j->down->up = j->up;
            j->up->down = j->down;
            j->column->size--;
        }
    }
}

static void uncover(struct node *c)
{
    struct node *i, *j;

    for (i = c->up; i != c; i = i->up) {
        for (j = i->left; j != i; j = j->left) {
            j->column->size++;
            j->down->up = j->up->down = j;
        }
    }
    c->right->left = c->left->right = c;
}

static int print_solution(const uint32_t *solution)
{
    unsigned i, j, k;
    char grid[N*N] = {0};
    const char *p = grid;

    for (i = 0; i < N*N; i++) {
        const char row = solution[i]>>16 & 0xff;
        const char col = solution[i]>> 8 & 0xff;
        const char num = solution[i]     & 0xff;
        grid[row*N + col] = num;
    }

    for (j = 0; j < N; j++) {
        for (i = 0; i < N; i++) {
            printf(" %c", *p ? *p : '?');
            if (i+1 != N && (i+1) % D == 0)
                printf(" |");
            p++;
        }
        if (j+1 != N && (j+1) % D == 0) {
            printf("\n ");
            for (k = 0; k < (N+D-1)*2 - 1; k++)
                printf("-");
        }
        printf("\n");
    }
    printf("\n");
    return 1;
}

static struct node *choose_a_column(struct node *h)
{
    int size = INT_MAX;
    struct node *j, *c = h->right;

    if (c == h)
        return NULL;
    for (j = h->right; j != h; j = j->right)
        if (j->size < size)
            c = j, size = j->size;
    return c;
}

static int search(struct sudansu *ctx, unsigned k)
{
    struct node *c, *r, *j;

    c = choose_a_column(&ctx->h);
    if (!c)
        return print_solution(ctx->solution);
    cover(c);
    for (r = c->down; r != c; r = r->down) {
        ctx->solution[k] = r->name;
        for (j = r->right; j != r; j = j->right)
            cover(j->column);
        if (search(ctx, k + 1))
            return 1;
        ctx->solution[k] = 0;
        for (j = r->left; j != r; j = j->left)
            uncover(j->column);
    }
    uncover(c);
    return 0;
}

static int parse_grid(struct sudansu *ctx, FILE *stream)
{
    char buf[16];
    unsigned i, j, k = 0;

    for (j = 0; j < N; j++) {
        if (!fgets(buf, sizeof buf, stream))
            return -1;
        for (i = 0; i < N; i++) {
            if (buf[i] >= '1' && buf[i] <= '1'+N) {
                struct node *x, *r = ctx->rows[j*N*N + i*N + buf[i]-'1'];

                ctx->solution[k++] = CELL_NAME(j, i, buf[i]);
                cover(r->column);
                for (x = r->right; x != r; x = x->right)
                    cover(x->column);
            }
        }
    }
    return k;
}

int main(int ac, char **av)
{
    int k = 0;
    struct sudansu ctx;

    init(&ctx);
    if (ac == 2 && strcmp(av[1], "-") == 0) {
        k = parse_grid(&ctx, stdin);
        if (k < 0)
            k = 0;
    }
    search(&ctx, k);
    return 0;
}
