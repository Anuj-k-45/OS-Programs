#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#define SHIP 'S'
#define EMPTY '.'
#define HIT 'X'
#define MISS 'O'

void initializeGrid(char grid[2][2]) {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            grid[i][j] = EMPTY;
        }
    }
}

void positionShip(char grid[2][2]) {
    int row, col;
    do {
        row = rand() % 2;
        col = rand() % 2;
    } while (grid[row][col] == SHIP);
    grid[row][col] = SHIP;
}

void displayGrid(char grid[2][2]) {
    printf("  A B\n");
    for (int i = 0; i < 2; i++) {
        printf("%d ", i + 1);
        for (int j = 0; j < 2; j++) {
            printf("%c ", (grid[i][j] == SHIP) ? EMPTY : grid[i][j]);
        }
        printf("\n");
    }
}

void extractCoordinates(char *input, int *row, int *col) {
    *col = input[0] - 'A';
    *row = input[1] - '1';
}

int checkValidity(int row, int col) {
    return row >= 0 && row < 2 && col >= 0 && col < 2;
}

void handlePlayer(int readEnd, int writeEnd) {
    char grid[2][2];

    srand(time(NULL) ^ getpid());
    initializeGrid(grid);
    positionShip(grid);

    while (1) {
        char attack[3];
        if (read(readEnd, attack, sizeof(attack)) <= 0) break;
        
        int row, col;
        extractCoordinates(attack, &row, &col);
        int successfulHit = (checkValidity(row, col) && grid[row][col] == SHIP);
        grid[row][col] = successfulHit ? HIT : MISS;

        write(writeEnd, &successfulHit, sizeof(successfulHit));
        write(writeEnd, grid, sizeof(grid));

        if (successfulHit) break;
    }
}

void startGame(int p1Read, int p1Write, int p2Read, int p2Write) {
    int currentPlayer = 1;
    char attack[3];
    int hitStatus;
    char p1Grid[2][2], p2Grid[2][2];
    initializeGrid(p1Grid);
    initializeGrid(p2Grid);
    positionShip(p1Grid);
    positionShip(p2Grid);

    while (1) {
        printf("Player %d, enter attack coordinate (e.g., A1): ", currentPlayer);
        scanf("%2s", attack);
        if (attack[0] >= 'a') attack[0] -= 32;

        int targetWrite = (currentPlayer == 1) ? p2Write : p1Write;
        int targetRead = (currentPlayer == 1) ? p2Read : p1Read;
        char (*targetGrid)[2] = (currentPlayer == 1) ? p2Grid : p1Grid;

        write(targetWrite, attack, sizeof(attack));
        read(targetRead, &hitStatus, sizeof(hitStatus));
        read(targetRead, targetGrid, sizeof(char) * 2 * 2);

        printf("Player %d's Board:\n", currentPlayer);
        displayGrid(targetGrid);

        if (hitStatus) {
            printf("HIT! Player %d wins!\n", currentPlayer);
            displayGrid(targetGrid);
            break;
        }
        
        currentPlayer = 3 - currentPlayer;
    }
}

int main() {
    srand(time(NULL));
    int p1p[2], p1c[2], p2p[2], p2c[2];

    if (pipe(p1p) || pipe(p1c) || pipe(p2p) || pipe(p2c)) {
        perror("Pipe error");
        return 1;
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        close(p1p[1]); close(p1c[0]);
        handlePlayer(p1p[0], p1c[1]);
        exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        close(p2p[1]); close(p2c[0]);
        handlePlayer(p2p[0], p2c[1]);
        exit(0);
    }

    startGame(p1c[0], p1p[1], p2c[0], p2p[1]);

    kill(pid1, SIGTERM);
    kill(pid2, SIGTERM);
    wait(NULL);
    wait(NULL);

    return 0;
}