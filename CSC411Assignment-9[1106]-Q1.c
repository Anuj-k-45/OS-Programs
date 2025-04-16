#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE 9

typedef struct {
    int row;
    int col;
} Params;

int board[SIZE][SIZE] = {
    {5,3,4,6,7,8,9,1,2},
    {6,7,2,1,9,5,3,4,8},
    {1,9,8,3,4,2,5,6,7},
    {8,5,9,7,6,1,4,2,3},
    {4,2,6,8,5,3,7,9,1},
    {7,1,3,9,2,4,8,5,6},
    {9,6,1,5,3,7,2,8,4},
    {2,8,7,4,1,9,6,3,5},
    {3,4,5,2,8,6,1,7,9}
};

int status[11] = {0};

void* check_rows(void* arg) {
    int seen[SIZE] = {0};
    int r = ((Params*)arg)->row;

    for (int c = 0; c < SIZE; ++c) {
        int val = board[r][c];
        if (val < 1 || val > 9 || seen[val - 1]++) {
            pthread_exit(NULL);
        }
    }

    status[0] = 1;
    pthread_exit(NULL);
}

void* check_columns(void* arg) {
    int seen[SIZE] = {0};
    int c = ((Params*)arg)->col;

    for (int r = 0; r < SIZE; ++r) {
        int val = board[r][c];
        if (val < 1 || val > 9 || seen[val - 1]++) {
            pthread_exit(NULL);
        }
    }

    status[1] = 1;
    pthread_exit(NULL);
}

void* check_box(void* arg) {
    int seen[SIZE] = {0};
    int r_start = ((Params*)arg)->row;
    int c_start = ((Params*)arg)->col;

    for (int r = r_start; r < r_start + 3; ++r) {
        for (int c = c_start; c < c_start + 3; ++c) {
            int val = board[r][c];
            if (val < 1 || val > 9 || seen[val - 1]++) {
                pthread_exit(NULL);
            }
        }
    }

    int box_id = (r_start / 3) * 3 + (c_start / 3);
    status[2 + box_id] = 1;
    pthread_exit(NULL);
}

int main() {
    pthread_t th[11];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    Params* row_arg = malloc(sizeof(Params));
    row_arg->row = 0;
    pthread_create(&th[0], &attr, check_rows, row_arg);

    Params* col_arg = malloc(sizeof(Params));
    col_arg->col = 0;
    pthread_create(&th[1], &attr, check_columns, col_arg);

    int idx = 2;
    for (int r = 0; r < SIZE; r += 3) {
        for (int c = 0; c < SIZE; c += 3) {
            Params* box_arg = malloc(sizeof(Params));
            box_arg->row = r;
            box_arg->col = c;
            pthread_create(&th[idx++], &attr, check_box, box_arg);
        }
    }

    for (int i = 0; i < 11; ++i) {
        pthread_join(th[i], NULL);
    }

    pthread_attr_destroy(&attr);

    int all_good = 1;
    for (int i = 0; i < 11; ++i) {
        if (!status[i]) {
            all_good = 0;
            break;
        }
    }

    if (all_good) {
        printf("Valid Sudoku\n");
    } else {
        printf("Invalid Sudoku\n");
    }

    return 0;
}
