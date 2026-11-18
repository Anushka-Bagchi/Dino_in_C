#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 40
#define HEIGHT 7
#define DINO_X 5
#define MIN_DELAY 20
#define MIN_CACTUS_GAP 3
#define MAX_CACTUS_GAP 8
#define MAX_CACTUSES 4
#define CACTUS_CLEARANCE 12

int randomCactusPosition(int rightmost) {
    return rightmost + CACTUS_CLEARANCE
           + rand() % (MAX_CACTUS_GAP - MIN_CACTUS_GAP + 1)
           + MIN_CACTUS_GAP;
}

void gotoTop() {
    COORD pos = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void hideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 100;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

void showCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 100;
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

void clearScreen() {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screenInfo;
    DWORD cellsWritten;
    COORD origin = {0, 0};
    DWORD cellCount;

    if (!GetConsoleScreenBufferInfo(console, &screenInfo))
        return;

    cellCount = (DWORD)screenInfo.dwSize.X * (DWORD)screenInfo.dwSize.Y;
    FillConsoleOutputCharacter(console, ' ', cellCount, origin, &cellsWritten);
    FillConsoleOutputAttribute(console, screenInfo.wAttributes, cellCount,
                               origin, &cellsWritten);
    SetConsoleCursorPosition(console, origin);
}

void drawFrame(int dinoY, const int cactusX[], int cactusCount, int score) {
    char frame[HEIGHT][WIDTH];
    int row, col, cactus;

    for (row = 0; row < HEIGHT; row++)
        for (col = 0; col < WIDTH; col++)
            frame[row][col] = ' ';

    if (dinoY >= 0 && dinoY < HEIGHT)
        frame[dinoY][DINO_X] = 'D';
    for (cactus = 0; cactus < cactusCount; cactus++) {
        if (cactusX[cactus] >= 0 && cactusX[cactus] < WIDTH)
            frame[0][cactusX[cactus]] = 'Y';
    }

    for (row = HEIGHT - 1; row >= 0; row--) {
        for (col = 0; col < WIDTH; col++)
            printf("%c", frame[row][col]);
        printf("\n");
    }
    printf("________________________________________\n");
    printf("Score: %-3d  SPACE = jump  ESC = quit\n", score);
}

int playGame(const char *name) {
    int score = 0, dinoY = 0, jumping = 0, goingUp = 0;
    int cactusX[MAX_CACTUSES] = {WIDTH - 1, -1, -1, -1};
    int cactusCount = 1, delay, cactus;

    clearScreen();
    hideCursor();

    while (1) {
        if (_kbhit()) {
            char key = _getch();
            if (key == ' ' && !jumping) {
                jumping = 1;
                goingUp = 1;
            } else if (key == 27) {
                showCursor();
                return -1;
            }
        }

        if (jumping) {
            if (goingUp) {
                dinoY++;
                if (dinoY >= 5)
                    goingUp = 0;
            } else {
                dinoY--;
                if (dinoY <= 0) {
                    dinoY = 0;
                    jumping = 0;
                }
            }
        }

        if (score >= 15)
            cactusCount = 4;
        else if (score >= 10)
            cactusCount = 3;
        else if (score >= 5)
            cactusCount = 2;

        for (cactus = 0; cactus < cactusCount; cactus++) {
            if (cactusX[cactus] < 0) {
                int rightmost = WIDTH - 1, i;
                for (i = 0; i < cactusCount; i++) {
                    if (cactusX[i] > rightmost)
                        rightmost = cactusX[i];
                }
                cactusX[cactus] = randomCactusPosition(rightmost);
            }
        }

        for (cactus = 0; cactus < cactusCount; cactus++)
            cactusX[cactus]--;
        gotoTop();
        drawFrame(dinoY, cactusX, cactusCount, score);

        for (cactus = 0; cactus < cactusCount; cactus++) {
            if (cactusX[cactus] == DINO_X && dinoY == 0) {
                showCursor();
                printf("\nGame over, %s! Final score: %d\n", name, score);
                return score;
            }

            if (cactusX[cactus] < 0) {
                int rightmost = WIDTH - 1, i;

                score++;
                for (i = 0; i < cactusCount; i++) {
                    if (cactusX[i] > rightmost)
                        rightmost = cactusX[i];
                }
                cactusX[cactus] = randomCactusPosition(rightmost);
            }
        }

        delay = 70 - 3 * score;
        if (delay < MIN_DELAY)
            delay = MIN_DELAY;
        Sleep(delay);
    }
}

int main() {
    char name[30], bestName[30] = "nobody";
    int bestScore = 0;
    int score, i;
    FILE *fp;

    srand((unsigned int)time(NULL));

    fp = fopen("highscore.txt", "r");
    if (fp != NULL) {
        if (fscanf(fp, "%29s %d", bestName, &bestScore) != 2) {
            strcpy(bestName, "nobody");
            bestScore = 0;
        }
        fclose(fp);
    }
    printf("Enter your name: ");
    if (fgets(name, sizeof(name), stdin) == NULL) {
        strcpy(name, "Player");
    } else {
        name[strcspn(name, "\r\n")] = '\0';
        name[strcspn(name, " \t")] = '\0';
        if (name[0] == '\0')
            strcpy(name, "Player");
    }
    printf("\nWelcome, %s!\n", name);
    for (i = 0; i < (int)strlen(name) + 9; i++)
        printf("=");
    printf("\nHigh score: %d by %s\n", bestScore, bestName);
    printf("Press SPACE to jump over the cactus.\n");
    printf("Press any key to start...");
    _getch();

    for (;;) {
        score = playGame(name);
        if (score < 0)
            break;

        if (score > bestScore) {
            fp = fopen("highscore.txt", "w");
            if (fp != NULL) {
                fprintf(fp, "%s %d\n", name, score);
                fclose(fp);
                bestScore = score;
                strcpy(bestName, name);
                printf("New high score! Saved to highscore.txt\n");
            } else {
                printf("Could not save the high score.\n");
            }
        } else {
            printf("High score is still %d by %s\n", bestScore, bestName);
        }

        printf("Press R to replay or ESC to quit...");
        char key = _getch();
        if (key == 27)
            break;
        if (key != 'r' && key != 'R') {
            break;
        }
    }

    showCursor();
    return 0;
}