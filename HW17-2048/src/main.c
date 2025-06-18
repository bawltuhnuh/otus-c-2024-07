#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#define SIZE 4

int board[SIZE][SIZE];

void printBoard() {
    system("clear");
    printf("\n2048 Game\n");
    for (int i = 0; i < SIZE; i++) {
        printf("+------+------+------+------+\n");
        for (int j = 0; j < SIZE; j++) {
            if (board[i][j] == 0)
                printf("|      ");
            else
                printf("|%6d", board[i][j]);
        }
        printf("|\n");
    }
    printf("+------+------+------+------+\n");
}

void addRandomTile() {
    int empty[SIZE * SIZE][2];
    int count = 0;
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            if (board[i][j] == 0) {
                empty[count][0] = i;
                empty[count][1] = j;
                count++;
            }
    if (count > 0) {
        int r = rand() % count;
        int val = (rand() % 10 == 0) ? 4 : 2;
        board[empty[r][0]][empty[r][1]] = val;
    }
}

void initBoard() {
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            board[i][j] = 0;
    addRandomTile();
    addRandomTile();
}

int canMove() {
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++) {
            if (board[i][j] == 0)
                return 1;
            if (j < SIZE - 1 && board[i][j] == board[i][j + 1])
                return 1;
            if (i < SIZE - 1 && board[i][j] == board[i + 1][j])
                return 1;
        }
    return 0;
}

int checkWin() {
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            if (board[i][j] == 2048)
                return 1;
    return 0;
}

int slideAndMergeRow(int *row) {
    int moved = 0;
    int temp[SIZE] = {0};
    int t = 0;

    for (int i = 0; i < SIZE; i++) {
        if (row[i] != 0) {
            temp[t++] = row[i];
        }
    }

    for (int i = 0; i < SIZE - 1; i++) {
        if (temp[i] != 0 && temp[i] == temp[i + 1]) {
            temp[i] *= 2;
            temp[i + 1] = 0;
            moved = 1;
        }
    }

    int final[SIZE] = {0};
    t = 0;
    for (int i = 0; i < SIZE; i++) {
        if (temp[i] != 0)
            final[t++] = temp[i];
    }

    for (int i = 0; i < SIZE; i++) {
        if (row[i] != final[i]) {
            row[i] = final[i];
            moved = 1;
        }
    }

    return moved;
}

int moveLeft() {
    int moved = 0;
    for (int i = 0; i < SIZE; i++) {
        if (slideAndMergeRow(board[i]))
            moved = 1;
    }
    return moved;
}

int moveRight() {
    int moved = 0;
    for (int i = 0; i < SIZE; i++) {
        int temp[SIZE];
        for (int j = 0; j < SIZE; j++)
            temp[j] = board[i][SIZE - 1 - j];
        if (slideAndMergeRow(temp))
            moved = 1;
        for (int j = 0; j < SIZE; j++)
            board[i][SIZE - 1 - j] = temp[j];
    }
    return moved;
}

int moveUp() {
    int moved = 0;
    for (int j = 0; j < SIZE; j++) {
        int temp[SIZE];
        for (int i = 0; i < SIZE; i++)
            temp[i] = board[i][j];
        if (slideAndMergeRow(temp))
            moved = 1;
        for (int i = 0; i < SIZE; i++)
            board[i][j] = temp[i];
    }
    return moved;
}

int moveDown() {
    int moved = 0;
    for (int j = 0; j < SIZE; j++) {
        int temp[SIZE];
        for (int i = 0; i < SIZE; i++)
            temp[i] = board[SIZE - 1 - i][j];
        if (slideAndMergeRow(temp))
            moved = 1;
        for (int i = 0; i < SIZE; i++)
            board[SIZE - 1 - i][j] = temp[i];
    }
    return moved;
}

void gameLoop() {
    initBoard();
    char input;
    while (1) {
        printBoard();
        if (checkWin()) {
            printf("🎉 Вы выиграли! 2048 достигнута!\n");
            break;
        }
        if (!canMove()) {
            printf("💀 Игра окончена! Нет доступных ходов.\n");
            break;
        }

        printf("Ход (WASD): ");
        scanf(" %c", &input);
        input = tolower(input);

        int moved = 0;
        if (input == 'w') moved = moveUp();
        else if (input == 'a') moved = moveLeft();
        else if (input == 's') moved = moveDown();
        else if (input == 'd') moved = moveRight();
        else continue;

        if (moved)
            addRandomTile();
    }
}

void mainMenu() {
    int choice;
    do {
        printf("\n===== 2048 =====\n");
        printf("1. Начать игру\n");
        printf("2. Выход\n");
        printf("Ваш выбор: ");
        scanf("%d", &choice);
        if (choice == 1) {
            gameLoop();
        }
    } while (choice != 2);
    printf("Выход из игры. До свидания!\n");
}

int main() {
    srand(time(NULL));
    mainMenu();
    return 0;
}
