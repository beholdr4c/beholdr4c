#include <GL/glut.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <unistd.h>
#include <linux/limits.h>

// Размеры окна
const int WINDOW_WIDTH = 1400;
const int WINDOW_HEIGHT = 800;

// Параметры игрового поля
const float GAME_RECT_X = -0.75;
const float GAME_RECT_Y = -0.75;
const float GAME_RECT_WIDTH = 0.8;
const float GAME_RECT_HEIGHT = 1.25;

// Параметры меню
const float MENU_RECT_X = 0.1;
const float MENU_RECT_Y = -0.75;
const float MENU_RECT_WIDTH = 0.4;
const float MENU_RECT_HEIGHT = 1.25;

// Координаты и размеры кнопок меню
const float BUTTON_WIDTH = 0.3;
const float BUTTON_HEIGHT = 0.1;
const float NEXT_BUTTON_X = MENU_RECT_X + 0.05;
const float NEXT_BUTTON_Y = MENU_RECT_Y + 0.85;
const float PREV_BUTTON_X = MENU_RECT_X + 0.05;
const float PREV_BUTTON_Y = MENU_RECT_Y + 0.65;
const float LEVEL_BUTTON_X = MENU_RECT_X + 0.05;
const float LEVEL_BUTTON_Y = MENU_RECT_Y + 0.45;
const float RESTART_BUTTON_X = MENU_RECT_X + 0.05;
const float RESTART_BUTTON_Y = MENU_RECT_Y + 0.25;
const float LEADERBOARD_BUTTON_X = MENU_RECT_X + 0.05;
const float LEADERBOARD_BUTTON_Y = MENU_RECT_Y + 0.15;
const float CONTROLS_BUTTON_X = MENU_RECT_X + 0.05;
const float CONTROLS_BUTTON_Y = MENU_RECT_Y + 0.05;
const float QUIT_BUTTON_X = MENU_RECT_X + 0.05;
const float QUIT_BUTTON_Y = MENU_RECT_Y - 0.05;

// Параметры кнопок выбора уровня
const float LEVEL_SELECT_BUTTON_WIDTH = 0.1;
const float LEVEL_SELECT_BUTTON_HEIGHT = 0.1;
const float LEVEL_SELECT_BASE_X = -0.8;
const float LEVEL_SELECT_BASE_Y = 0.7;

// Параметры индикатора надежности пароля
const float PASSWORD_STRENGTH_X = -0.4;
const float PASSWORD_STRENGTH_Y = -0.1;
const float PASSWORD_STRENGTH_WIDTH = 0.4;
const float PASSWORD_STRENGTH_HEIGHT = 0.05;

// Координаты кнопок авторизации
const float TOGGLE_BUTTON_X = -0.4;
const float TOGGLE_BUTTON_Y = -0.2;
const float TOGGLE_BUTTON_WIDTH = 0.4;
const float TOGGLE_BUTTON_HEIGHT = 0.1;

const float QUIT_LOGIN_BUTTON_X = -0.4;
const float QUIT_LOGIN_BUTTON_Y = -0.4;
const float QUIT_LOGIN_BUTTON_WIDTH = 0.2;
const float QUIT_LOGIN_BUTTON_HEIGHT = 0.1;

// Определение размеров уровня
#define ROWS 8
#define COLS 8
#define MAX_USERNAME 20
#define MIN_USERNAME 4
#define MAX_PASSWORD 65
#define MIN_PASSWORD 8

// Перечисление для режимов отображения
typedef enum {
    LOGIN,
    GAME,
    LEVEL_SELECTOR,
    LEADERBOARD,
    TUTORIAL,
    CONTROLS
} DisplayMode;

// Структура для хранения уровней
typedef struct Level {
    char grid[ROWS][COLS];
} Level;

// Структура для хранения данных пользователя
typedef struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    int completedLevels[10];
    int levelTimes[10];
    int levelMoves[10];
    float totalScore;
} User;

// Структура для узла связного списка пользователей
typedef struct UserNode {
    User user;
    struct UserNode* next;
} UserNode;

// Исходные уровни
const Level originalLevels[] = {
    {{"########", "#      #", "# @ $  #", "#   .  #", "#      #", "#      #", "#      #", "########"}},
    {{"########", "#      #", "# @ $  #", "#  $ . #", "#   .  #", "#      #", "#      #", "########"}},
    {{"########", "#      #", "# @ $ .#", "#   $  #", "# .  . #", "#      #", "#      #", "########"}},
    {{"########", "#      #", "#  @$  #", "#  $$  #", "#  . . #", "#  .   #", "#      #", "########"}},
    {{"########", "#   #  #", "# @$$  #", "#   .# #", "#  ..  #", "#      #", "#      #", "########"}},
    {{"########", "# @ #  #", "# $$.$ #", "#   .# #", "#  ..  #", "#    $ #", "#      #", "########"}},
    {{"########", "#  @#  #", "# $$   #", "#  #.# #", "# . .  #", "#   $  #", "#      #", "########"}},
    {{"########", "#  @   #", "# $ $ ##", "# #.#. #", "# .#. ##", "#  $$  #", "#      #", "########"}},
    {{"########", "#  @   #", "# $$   #", "#  #.# #", "#  . . #", "# $    #", "#      #", "########"}},
    {{"########", "#  @   #", "# $$$  #", "# #.#. #", "# .. ..#", "# $$$  #", "#  #   #", "########"}}
};

// Демонстрационный уровень для анимации
const Level tutorialLevel = {
    {
        "########",
        "#      #",
        "# @ $ .#",
        "#      #",
        "#      #",
        "#      #",
        "#      #",
        "########"
    }
};

// Последовательность ходов для анимации
const char tutorialMoves[] = {'d', 'd', 's'};
const int tutorialMoveCount = 3;
int currentTutorialMove = 0;
int tutorialPlayerX, tutorialPlayerY;
Level tutorialLevelState;

// Глобальные переменные
Level currentLevelState;
int currentLevel = 0;
int totalLevels = sizeof(originalLevels) / sizeof(Level);
int playerX, playerY;
bool isLevelCompleted = false;
int startTime = 0;
int endTime = 0;
int moveCount = 0;

char currentUsername[MAX_USERNAME + 1] = "";
char currentPassword[MAX_PASSWORD + 1] = "";
char loggedInUser[MAX_USERNAME + 1] = "";
char errorMessage[100] = "";
bool isLoginScreen = true;
bool isRegisterMode = false;
bool isUsernameEntered = false;
bool isFirstGame = false;
int passwordStrength = 0;
DisplayMode currentMode = LOGIN;

// Ключ для шифрования (в продакшене должен быть защищён)
static const unsigned char aes_key[] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};

// Функция хеширования пароля
void hash_password(const char *password, char *hashed) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char *)password, strlen(password), hash);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        snprintf(hashed + i * 2, 3, "%02x", hash[i]);
    }
    hashed[SHA256_DIGEST_LENGTH * 2] = '\0';
}

// Функция шифрования данных
int encrypt_data(const unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;

    unsigned char iv[16] = {0};
    int len, ciphertext_len;

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, aes_key, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    ciphertext_len = len;

    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}

// Функция дешифрования данных
int decrypt_data(const unsigned char *ciphertext, int ciphertext_len, unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;

    unsigned char iv[16] = {0};
    int len, plaintext_len;

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, aes_key, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plaintext_len = len;

    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return plaintext_len;
}

// Функция оценки надежности пароля
int evaluatePasswordStrength(const char* password) {
    int length = strlen(password);
    int score = 0;
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
    char uniqueChars[256] = {0};
    int uniqueCount = 0;

    for (int i = 0; i < length; i++) {
        if (!uniqueChars[(unsigned char)password[i]]) {
            uniqueChars[(unsigned char)password[i]] = 1;
            uniqueCount++;
        }
    }

    if (length >= 20) score += 4;
    else if (length >= 16) score += 3;
    else if (length >= 12) score += 2;
    else if (length >= 8) score += 1;

    if (uniqueCount >= 12) score += 2;
    else if (uniqueCount >= 8) score += 1;

    for (int i = 0; i < length; i++) {
        if (islower(password[i])) hasLower = true;
        else if (isupper(password[i])) hasUpper = true;
        else if (isdigit(password[i])) hasDigit = true;
        else hasSpecial = true;
    }

    if (hasLower) score++;
    if (hasUpper) score++;
    if (hasDigit) score++;
    if (hasSpecial) score++;

    return (score > 5) ? 5 : score;
}

// Функция получения обратной связи по надежности пароля
void getPasswordStrengthFeedback(int strength, char* feedback) {
    switch (strength) {
        case 0:
        case 1:
            strcpy(feedback, "Weak password! Use more characters and variety.");
            break;
        case 2:
            strcpy(feedback, "Fair password. Add unique symbols or length.");
            break;
        case 3:
            strcpy(feedback, "Moderate password. More variety or length helps.");
            break;
        case 4:
            strcpy(feedback, "Good password. Add a few more unique chars.");
            break;
        case 5:
            strcpy(feedback, "Strong password!");
            break;
    }
}

// Прототипы функций
void keyboard(unsigned char key, int x, int y);
void specialKeys(int key, int x, int y);
void mouse(int button, int state, int x, int y);
void startGame();
float recalculateTotalScore(User *user);
void saveUserData(User user);
bool checkUserExists(char *username, User *foundUser);
void updateUserData(User user);
void tutorialUpdate(int value);
void tutorialDisplay();
void freeUserList(UserNode* head);

// Функция освобождения связного списка
void freeUserList(UserNode* head) {
    UserNode* current = head;
    while (current != NULL) {
        UserNode* temp = current;
        current = current->next;
        free(temp);
    }
}

// Функция сохранения данных пользователя
void saveUserData(User user) {
    char data[512];
    snprintf(data, sizeof(data), "%s %s", user.username, user.password);
    for (int i = 0; i < 10; i++) {
        char level_data[64];
        snprintf(level_data, sizeof(level_data), " %d %d %d",
                 user.completedLevels[i], user.levelTimes[i], user.levelMoves[i]);
        strncat(data, level_data, sizeof(data) - strlen(data) - 1);
    }
    char score_data[32];
    snprintf(score_data, sizeof(score_data), " %.2f", user.totalScore);
    strncat(data, score_data, sizeof(data) - strlen(data) - 1);

    unsigned char ciphertext[1024];
    int ciphertext_len = encrypt_data((unsigned char *)data, strlen(data), ciphertext);
    if (ciphertext_len < 0) {
        printf("Ошибка шифрования данных\n");
        return;
    }

    FILE *file = fopen("users.bin", "ab");
    if (file) {
        fwrite(&ciphertext_len, sizeof(int), 1, file);
        fwrite(ciphertext, 1, ciphertext_len, file);
        fwrite("\n", 1, 1, file);
        fclose(file);
    } else {
        printf("Ошибка: Не удалось открыть файл users.bin для записи!\n");
    }
}

// Проверка существования пользователя
bool checkUserExists(char *username, User *foundUser) {
    FILE *file = fopen("users.bin", "rb");
    if (!file) return false;

    char cwd[65535];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s", cwd);
    }

    unsigned char buffer[1024];
    int ciphertext_len;

    while (fread(&ciphertext_len, sizeof(int), 1, file) == 1) {
        if (ciphertext_len > 1024 || ciphertext_len <= 0) {
            fseek(file, ciphertext_len + 1, SEEK_CUR);
            continue;
        }
        fread(buffer, 1, ciphertext_len, file);
        char newline;
        fread(&newline, 1, 1, file);

        unsigned char plaintext[1024];
        int plaintext_len = decrypt_data(buffer, ciphertext_len, plaintext);
        if (plaintext_len < 0) continue;
        plaintext[plaintext_len] = '\0';

        User tempUser = {{0}, {0}, {0}, {0}, {0}, 0};
        char tempPassword[MAX_PASSWORD];
        int items = sscanf((char *)plaintext, "%s %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %f",
                           tempUser.username, tempPassword,
                           &tempUser.completedLevels[0], &tempUser.levelTimes[0], &tempUser.levelMoves[0],
                           &tempUser.completedLevels[1], &tempUser.levelTimes[1], &tempUser.levelMoves[1],
                           &tempUser.completedLevels[2], &tempUser.levelTimes[2], &tempUser.levelMoves[2],
                           &tempUser.completedLevels[3], &tempUser.levelTimes[3], &tempUser.levelMoves[3],
                           &tempUser.completedLevels[4], &tempUser.levelTimes[4], &tempUser.levelMoves[4],
                           &tempUser.completedLevels[5], &tempUser.levelTimes[5], &tempUser.levelMoves[5],
                           &tempUser.completedLevels[6], &tempUser.levelTimes[6], &tempUser.levelMoves[6],
                           &tempUser.completedLevels[7], &tempUser.levelTimes[7], &tempUser.levelMoves[7],
                           &tempUser.completedLevels[8], &tempUser.levelTimes[8], &tempUser.levelMoves[8],
                           &tempUser.completedLevels[9], &tempUser.levelTimes[9], &tempUser.levelMoves[9],
                           &tempUser.totalScore);
        if (items < 2) continue;
        strncpy(tempUser.password, tempPassword, MAX_PASSWORD - 1);
        tempUser.password[MAX_PASSWORD - 1] = '\0';

        if (strcmp(tempUser.username, username) == 0) {
            if (foundUser) *foundUser = tempUser;
            fclose(file);
            return true;
        }
    }
    fclose(file);
    return false;
}

// Обновление данных пользователя
void updateUserData(User user) {
    FILE *file = fopen("users.bin", "rb");
    FILE *tempFile = fopen("temp.bin", "wb");
    if (!file || !tempFile) {
        printf("Ошибка: Не удалось открыть файл для обновления данных!\n");
        if (file) fclose(file);
        if (tempFile) fclose(tempFile);
        return;
    }

    unsigned char buffer[1024];
    int ciphertext_len;
    bool user_updated = false;

    while (fread(&ciphertext_len, sizeof(int), 1, file) == 1) {
        if (ciphertext_len > 1024 || ciphertext_len <= 0) {
            fseek(file, ciphertext_len + 1, SEEK_CUR);
            continue;
        }
        fread(buffer, 1, ciphertext_len, file);
        char newline;
        fread(&newline, 1, 1, file);

        unsigned char plaintext[1024];
        int plaintext_len = decrypt_data(buffer, ciphertext_len, plaintext);
        if (plaintext_len < 0) continue;
        plaintext[plaintext_len] = '\0';

        char tempUsername[MAX_USERNAME];
        sscanf((char *)plaintext, "%s", tempUsername);

        if (strcmp(tempUsername, user.username) == 0) {
            char data[512];
            snprintf(data, sizeof(data), "%s %s", user.username, user.password);
            for (int i = 0; i < 10; i++) {
                char level_data[64];
                snprintf(level_data, sizeof(level_data), " %d %d %d",
                         user.completedLevels[i], user.levelTimes[i], user.levelMoves[i]);
                strncat(data, level_data, sizeof(data) - strlen(data) - 1);
            }
            char score_data[32];
            snprintf(score_data, sizeof(score_data), " %.2f", user.totalScore);
            strncat(data, score_data, sizeof(data) - strlen(data) - 1);

            unsigned char ciphertext[1024];
            ciphertext_len = encrypt_data((unsigned char *)data, strlen(data), ciphertext);
            if (ciphertext_len < 0) continue;

            fwrite(&ciphertext_len, sizeof(int), 1, tempFile);
            fwrite(ciphertext, 1, ciphertext_len, tempFile);
            fwrite("\n", 1, 1, tempFile);
            user_updated = true;
        } else {
            fwrite(&ciphertext_len, sizeof(int), 1, tempFile);
            fwrite(buffer, 1, ciphertext_len, tempFile);
            fwrite("\n", 1, 1, tempFile);
        }
    }

    if (!user_updated) {
        char data[512];
        snprintf(data, sizeof(data), "%s %s", user.username, user.password);
        for (int i = 0; i < 10; i++) {
            char level_data[64];
            snprintf(level_data, sizeof(level_data), " %d %d %d",
                     user.completedLevels[i], user.levelTimes[i], user.levelMoves[i]);
            strncat(data, level_data, sizeof(data) - strlen(data) - 1);
        }
        char score_data[32];
        snprintf(score_data, sizeof(score_data), " %.2f", user.totalScore);
        strncat(data, score_data, sizeof(data) - strlen(data) - 1);

        unsigned char ciphertext[1024];
        ciphertext_len = encrypt_data((unsigned char *)data, strlen(data), ciphertext);
        if (ciphertext_len < 0) {
            fclose(file);
            fclose(tempFile);
            return;
        }

        fwrite(&ciphertext_len, sizeof(int), 1, tempFile);
        fwrite(ciphertext, 1, ciphertext_len, tempFile);
        fwrite("\n", 1, 1, tempFile);
    }

    fclose(file);
    fclose(tempFile);
    remove("users.bin");
    rename("temp.bin", "users.bin");
}

// Пересчет общего счета
float recalculateTotalScore(User *user) {
    float totalScore = 0.0;
    for (int i = 0; i < totalLevels; i++) {
        if (user->levelMoves[i] > 0 && user->levelTimes[i] > 0) {
            totalScore += ((100.0 / (float)user->levelMoves[i]) +
                          (100.0 / (float)user->levelTimes[i])) * (i + 1);
        }
    }
    return totalScore;
}

// Отрисовка прямоугольника
void drawRectangle(float x, float y, float width, float height, float r, float g, float b) {
    glBegin(GL_QUADS);
    glColor3f(r, g, b);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

// Отрисовка текста
void drawText(float x, float y, const char* text, float r, float g, float b) {
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *text);
        text++;
    }
}

// Отрисовка ячейки уровня
void drawCell(float x, float y, float cellWidth, float cellHeight, char type) {
    switch (type) {
        case '#': glColor3f(153.0 / 255.0, 153.0 / 255.0, 153.0 / 255.0); break;
        case '@': glColor3f(255.0 / 255.0, 204.0 / 255.0, 0.0 / 255.0); break;
        case '$': glColor3f(204.0 / 255.0, 153.0 / 255.0, 102.0 / 255.0); break;
        case '.': glColor3f(153.0 / 255.0, 204.0 / 255.0, 153.0 / 255.0); break;
        case '*': glColor3f(255.0 / 255.0, 153.0 / 255.0, 51.0 / 255.0); break;
        case '+': glColor3f(153.0 / 255.0, 255.0 / 255.0, 255.0 / 255.0); break;
        default: glColor3f(102.0 / 255.0, 76.0 / 255.0, 51.0 / 255.0); break;
    }
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + cellWidth, y);
    glVertex2f(x + cellWidth, y + cellHeight);
    glVertex2f(x, y + cellHeight);
    glEnd();
}

// Проверка победы
bool checkWin() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (currentLevelState.grid[i][j] == '$') return false;
        }
    }
    return true;
}

// Обновление уровня
void updateLevel(int dx, int dy) {
    int newX = playerX + dx;
    int newY = playerY + dy;

    if (newX < 0 || newX >= COLS || newY < 0 || newY >= ROWS) return;

    char nextCell = currentLevelState.grid[newY][newX];

    if (nextCell == ' ' || nextCell == '.') {
        currentLevelState.grid[playerY][playerX] = (currentLevelState.grid[playerY][playerX] == '+') ? '.' : ' ';
        currentLevelState.grid[newY][newX] = (nextCell == '.') ? '+' : '@';
        playerX = newX;
        playerY = newY;
        moveCount++;
    } else if (nextCell == '$' || nextCell == '*') {
        int boxX = newX + dx;
        int boxY = newY + dy;
        if (boxX >= 0 && boxX < COLS && boxY >= 0 && boxY < ROWS) {
            char nextBoxCell = currentLevelState.grid[boxY][boxX];
            if (nextBoxCell == ' ' || nextBoxCell == '.') {
                currentLevelState.grid[boxY][boxX] = (nextBoxCell == '.') ? '*' : '$';
                currentLevelState.grid[newY][newX] = (nextCell == '*') ? '.' : ' ';
                currentLevelState.grid[playerY][playerX] = (currentLevelState.grid[playerY][playerX] == '+') ? '.' : ' ';
                currentLevelState.grid[newY][newX] = (nextCell == '*') ? '+' : '@';
                playerX = newX;
                playerY = newY;
                moveCount++;
            }
        }
    }

    if (checkWin() && !isLevelCompleted) {
        printf("Уровень %d пройден! Ходов: %d\n", currentLevel + 1, moveCount);
        isLevelCompleted = true;
        endTime = glutGet(GLUT_ELAPSED_TIME);
        User user;
        if (checkUserExists(loggedInUser, &user)) {
            user.completedLevels[currentLevel] = 1;
            int timeTaken = (endTime - startTime) / 1000;
            if (timeTaken == 0) timeTaken = 1;
            float levelScore = ((100.0 / (float)moveCount) + (100.0 / (float)timeTaken)) * (currentLevel + 1);

            bool updateScore = false;
            if (user.levelMoves[currentLevel] == 0 || moveCount < user.levelMoves[currentLevel] ||
                (moveCount == user.levelMoves[currentLevel] && timeTaken < user.levelTimes[currentLevel])) {
                user.levelMoves[currentLevel] = moveCount;
                user.levelTimes[currentLevel] = timeTaken;
                updateScore = true;
            }

            if (updateScore) {
                user.totalScore = recalculateTotalScore(&user);
                updateUserData(user);
                printf("Очки за уровень %d: %.2f, Общие очки: %.2f\n",
                       currentLevel + 1, levelScore, user.totalScore);
            }
        }
    }
}

// Обновление демонстрационного уровня
void updateTutorialLevel(int dx, int dy) {
    int newX = tutorialPlayerX + dx;
    int newY = tutorialPlayerY + dy;

    if (newX < 0 || newX >= COLS || newY < 0 || newY >= ROWS) return;

    char nextCell = tutorialLevelState.grid[newY][newX];

    if (nextCell == ' ' || nextCell == '.') {
        tutorialLevelState.grid[tutorialPlayerY][tutorialPlayerX] = (tutorialLevelState.grid[tutorialPlayerY][tutorialPlayerX] == '+') ? '.' : ' ';
        tutorialLevelState.grid[newY][newX] = (nextCell == '.') ? '+' : '@';
        tutorialPlayerX = newX;
        tutorialPlayerY = newY;
    } else if (nextCell == '$' || nextCell == '*') {
        int boxX = newX + dx;
        int boxY = newY + dy;
        if (boxX >= 0 && boxX < COLS && boxY >= 0 && boxY < ROWS) {
            char nextBoxCell = tutorialLevelState.grid[boxY][boxX];
            if (nextBoxCell == ' ' || nextBoxCell == '.') {
                tutorialLevelState.grid[boxY][boxX] = (nextBoxCell == '.') ? '*' : '$';
                tutorialLevelState.grid[newY][newX] = (nextCell == '*') ? '.' : ' ';
                tutorialLevelState.grid[tutorialPlayerY][tutorialPlayerX] = (tutorialLevelState.grid[tutorialPlayerY][tutorialPlayerX] == '+') ? '.' : ' ';
                tutorialLevelState.grid[newY][newX] = (nextCell == '*') ? '+' : '@';
                tutorialPlayerX = newX;
                tutorialPlayerY = newY;
            }
        }
    }
}

// Загрузка демонстрационного уровня
void loadTutorialLevel() {
    memcpy(tutorialLevelState.grid, tutorialLevel.grid, sizeof(tutorialLevelState.grid));
    tutorialPlayerX = -1;
    tutorialPlayerY = -1;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (tutorialLevelState.grid[i][j] == '@' || tutorialLevelState.grid[i][j] == '+') {
                tutorialPlayerX = j;
                tutorialPlayerY = i;
                break;
            }
        }
        if (tutorialPlayerX != -1) break;
    }
    if (tutorialPlayerX == -1 || tutorialPlayerY == -1) {
        printf("Ошибка: Игрок не найден в демонстрационном уровне\n");
        tutorialPlayerX = 2;
        tutorialPlayerY = 2;
    }
    currentTutorialMove = 0;
    printf("Демонстрационный уровень загружен: playerX=%d, playerY=%d\n", tutorialPlayerX, tutorialPlayerY);
}

// Загрузка уровня
void loadLevel(int levelIndex) {
    if (levelIndex < 0 || levelIndex >= totalLevels) {
        printf("Ошибка: Уровень %d вне диапазона, загрузка уровня 0\n", levelIndex + 1);
        levelIndex = 0;
    }

    currentLevel = levelIndex;
    isLevelCompleted = false;
    memcpy(currentLevelState.grid, originalLevels[levelIndex].grid, sizeof(currentLevelState.grid));

    playerX = -1;
    playerY = -1;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (currentLevelState.grid[i][j] == '@' || currentLevelState.grid[i][j] == '+') {
                playerX = j;
                playerY = i;
                break;
            }
        }
        if (playerX != -1) break;
    }

    if (playerX == -1 || playerY == -1) {
        printf("Ошибка: Игрок не найден на уровне %d, установка позиции по умолчанию\n", levelIndex + 1);
        currentLevel = 0;
        memcpy(currentLevelState.grid, originalLevels[0].grid, sizeof(currentLevelState.grid));
        playerX = 2;
        playerY = 2;
    }

    startTime = glutGet(GLUT_ELAPSED_TIME);
    endTime = 0;
    moveCount = 0;
    printf("Уровень %d загружен: playerX=%d, playerY=%d\n", currentLevel + 1, playerX, playerY);
}

// Отрисовка меню игры
void drawMenu() {
    drawRectangle(MENU_RECT_X, MENU_RECT_Y, MENU_RECT_WIDTH, MENU_RECT_HEIGHT, 0.0, 0.0, 0.0);

    drawRectangle(NEXT_BUTTON_X, NEXT_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 0.5, 0.5, 0.5);
    drawText(NEXT_BUTTON_X + 0.05, NEXT_BUTTON_Y + 0.05, "Next Level", 1.0, 1.0, 1.0);

    drawRectangle(PREV_BUTTON_X, PREV_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 0.5, 0.5, 0.5);
    drawText(PREV_BUTTON_X + 0.05, PREV_BUTTON_Y + 0.05, "Previous Level", 1.0, 1.0, 1.0);

    drawRectangle(LEVEL_BUTTON_X, LEVEL_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 0.5, 0.5, 0.5);
    drawText(LEVEL_BUTTON_X + 0.05, LEVEL_BUTTON_Y + 0.05, "Choose Level", 1.0, 1.0, 1.0);

    drawRectangle(RESTART_BUTTON_X, RESTART_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 0.5, 0.5, 0.5);
    drawText(RESTART_BUTTON_X + 0.05, RESTART_BUTTON_Y + 0.05, "Restart Level", 1.0, 1.0, 1.0);

    drawRectangle(LEADERBOARD_BUTTON_X, LEADERBOARD_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 0.5, 0.5, 0.5);
    drawText(LEADERBOARD_BUTTON_X + 0.05, LEADERBOARD_BUTTON_Y + 0.05, "Leaderboard", 1.0, 1.0, 1.0);

    drawRectangle(CONTROLS_BUTTON_X, CONTROLS_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 0.5, 0.5, 0.5);
    drawText(CONTROLS_BUTTON_X + 0.05, CONTROLS_BUTTON_Y + 0.05, "Controls", 1.0, 1.0, 1.0);

    drawRectangle(QUIT_BUTTON_X, QUIT_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 0.5, 0.5, 0.5);
    drawText(QUIT_BUTTON_X + 0.05, QUIT_BUTTON_Y + 0.05, "Quit", 1.0, 1.0, 1.0);
}

// Отрисовка игры
void displayGame() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (currentLevel < 0 || currentLevel >= totalLevels) {
        printf("Ошибка: Неверный уровень %d, перезагрузка уровня 0\n", currentLevel + 1);
        loadLevel(0);
    }
    if (playerX < 0 || playerY < 0 || playerX >= COLS || playerY >= ROWS) {
        printf("Ошибка: Неверная позиция игрока (playerX=%d, playerY=%d), перезагрузка уровня %d\n",
               playerX, playerY, currentLevel + 1);
        loadLevel(currentLevel);
    }

    drawRectangle(GAME_RECT_X, GAME_RECT_Y, GAME_RECT_WIDTH, GAME_RECT_HEIGHT, 0.999, 0.844, 0.600);

    float cellWidth = GAME_RECT_WIDTH / COLS;
    float cellHeight = GAME_RECT_HEIGHT / ROWS;

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            float x = GAME_RECT_X + j * cellWidth;
            float y = GAME_RECT_Y + i * cellHeight;
            drawCell(x, y, cellWidth, cellHeight, currentLevelState.grid[i][j]);
        }
    }

    if (isLevelCompleted) {
        drawText(-0.2, 0.0, "LEVEL COMPLETE!", 0.0, 1.0, 0.0);
    }

    int displayTime = (isLevelCompleted && endTime != 0) ? (endTime - startTime) / 1000 : (glutGet(GLUT_ELAPSED_TIME) - startTime) / 1000;
    int hours = displayTime / 3600;
    int minutes = (displayTime % 3600) / 60;
    int seconds = displayTime % 60;
    char timerText[20];
    snprintf(timerText, sizeof(timerText), "Time: %02d:%02d:%02d", hours, minutes, seconds);
    drawText(-0.2, 0.8, timerText, 1.0, 1.0, 1.0);

    char movesText[20];
    snprintf(movesText, sizeof(movesText), "Moves: %d", moveCount);
    drawText(-0.2, 0.75, movesText, 1.0, 1.0, 1.0);

    if (strlen(loggedInUser) > 0) {
        User currentUser;
        if (checkUserExists(loggedInUser, &currentUser)) {
            currentUser.totalScore = recalculateTotalScore(&currentUser);
            char scoreText[30];
            snprintf(scoreText, sizeof(scoreText), "Score: %.2f", currentUser.totalScore);
            drawText(-0.2, 0.70, scoreText, 1.0, 1.0, 1.0);
        }
    }

    drawText(0.6, 0.8, loggedInUser, 1.0, 1.0, 0.0);

    drawMenu();
    glutSwapBuffers();
}

// Отрисовка окна авторизации
void displayLogin() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawRectangle(-0.5, -0.5, 1.0, 1.0, 0.2, 0.2, 0.2);

    drawText(-0.4, 0.4, isRegisterMode ? "Register" : "Login", 1.0, 1.0, 1.0);
    drawText(-0.4, 0.2, "ENTER YOUR USERNAME (just enter):", 1.0, 1.0, 1.0);
    drawText(-0.1, 0.2, currentUsername, 1.0, 1.0, 1.0);
    drawText(-0.4, 0.0, isUsernameEntered ? "ENTER YOUR PASSWORD (just enter):" : "", 1.0, 1.0, 1.0);
    char maskedPassword[MAX_PASSWORD];
    memset(maskedPassword, '*', strlen(currentPassword));
    maskedPassword[strlen(currentPassword)] = '\0';
    drawText(-0.1, 0.0, isUsernameEntered ? maskedPassword : "", 1.0, 1.0, 1.0);

    if (isUsernameEntered && isRegisterMode) {
        float r, g;
        switch (passwordStrength) {
            case 0:
            case 1: r = 1.0; g = 0.0; break;
            case 2: r = 1.0; g = 0.5; break;
            case 3: r = 1.0; g = 1.0; break;
            case 4:
            case 5: r = 0.0; g = 1.0; break;
        }
        drawRectangle(PASSWORD_STRENGTH_X, PASSWORD_STRENGTH_Y, PASSWORD_STRENGTH_WIDTH, PASSWORD_STRENGTH_HEIGHT, r, g, 0.0);
        char strengthText[20];
        snprintf(strengthText, sizeof(strengthText), "Strength: %d/5", passwordStrength);
        drawText(PASSWORD_STRENGTH_X + 0.05, PASSWORD_STRENGTH_Y + 0.03, strengthText, 1.0, 1.0, 1.0);
    }

    drawRectangle(TOGGLE_BUTTON_X, TOGGLE_BUTTON_Y, TOGGLE_BUTTON_WIDTH, TOGGLE_BUTTON_HEIGHT, 0.5, 0.5, 0.5);
    drawText(TOGGLE_BUTTON_X + 0.05, TOGGLE_BUTTON_Y + 0.05, "Toggle Mode", 1.0, 1.0, 1.0);

    drawRectangle(QUIT_LOGIN_BUTTON_X, QUIT_LOGIN_BUTTON_Y, QUIT_LOGIN_BUTTON_WIDTH, QUIT_LOGIN_BUTTON_HEIGHT, 0.5, 0.5, 0.5);
    drawText(QUIT_LOGIN_BUTTON_X + 0.05, QUIT_LOGIN_BUTTON_Y + 0.05, "Quit", 1.0, 1.0, 1.0);

    drawText(-0.4, -0.3, "Enter: Next/Submit", 0.8, 0.8, 0.8);
    drawText(-0.4, -0.6, "Username: 4-20 chars, Password: 8-20 chars", 0.8, 0.8, 0.8);

    if (strlen(errorMessage) > 0) {
        drawText(-0.4, -0.5, errorMessage, 1.0, 0.0, 0.0);
    }

    glutSwapBuffers();
}

// Отрисовка выбора уровня
void levelSelectorDisplay() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawText(-0.9, 0.9, "SELECT LEVEL:", 1.0, 1.0, 1.0);

    for (int i = 0; i < totalLevels; i++) {
        float x = LEVEL_SELECT_BASE_X + (i % 5) * (LEVEL_SELECT_BUTTON_WIDTH + 0.05);
        float y = LEVEL_SELECT_BASE_Y - (i / 5) * (LEVEL_SELECT_BUTTON_HEIGHT + 0.05);
        char levelText[4];
        snprintf(levelText, sizeof(levelText), "%d", i + 1);
        drawRectangle(x, y, LEVEL_SELECT_BUTTON_WIDTH, LEVEL_SELECT_BUTTON_HEIGHT, 0.5, 0.5, 0.5);
        drawText(x + 0.03, y + 0.05, levelText, 1.0, 1.0, 1.0);
    }

    drawRectangle(-0.1, -0.8, 0.2, 0.1, 0.5, 0.5, 0.5);
    drawText(-0.05, -0.75, "Back", 1.0, 1.0, 1.0);

    glutSwapBuffers();
}

// Отрисовка таблицы лидеров
void leaderboardDisplay() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawText(-0.9, 0.9, "LEADERBOARD:", 1.0, 1.0, 1.0);

    UserNode* head = NULL;
    UserNode* tail = NULL;
    int userCount = 0;

    FILE *file = fopen("users.bin", "rb");
    if (file) {
        unsigned char buffer[1024];
        int ciphertext_len;
        while (fread(&ciphertext_len, sizeof(int), 1, file) == 1) {
            if (ciphertext_len > 1024 || ciphertext_len <= 0) {
                fseek(file, ciphertext_len + 1, SEEK_CUR);
                continue;
            }
            fread(buffer, 1, ciphertext_len, file);
            char newline;
            fread(&newline, 1, 1, file);

            unsigned char plaintext[1024];
            int plaintext_len = decrypt_data(buffer, ciphertext_len, plaintext);
            if (plaintext_len < 0) continue;
            plaintext[plaintext_len] = '\0';

            User tempUser = {{0}, {0}, {0}, {0}, {0}, 0};
            char tempPassword[MAX_PASSWORD];
            int items = sscanf((char *)plaintext, "%s %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %f",
                               tempUser.username, tempPassword,
                               &tempUser.completedLevels[0], &tempUser.levelTimes[0], &tempUser.levelMoves[0],
                               &tempUser.completedLevels[1], &tempUser.levelTimes[1], &tempUser.levelMoves[1],
                               &tempUser.completedLevels[2], &tempUser.levelTimes[2], &tempUser.levelMoves[2],
                               &tempUser.completedLevels[3], &tempUser.levelTimes[3], &tempUser.levelMoves[3],
                               &tempUser.completedLevels[4], &tempUser.levelTimes[4], &tempUser.levelMoves[4],
                               &tempUser.completedLevels[5], &tempUser.levelTimes[5], &tempUser.levelMoves[5],
                               &tempUser.completedLevels[6], &tempUser.levelTimes[6], &tempUser.levelMoves[6],
                               &tempUser.completedLevels[7], &tempUser.levelTimes[7], &tempUser.levelMoves[7],
                               &tempUser.completedLevels[8], &tempUser.levelTimes[8], &tempUser.levelMoves[8],
                               &tempUser.completedLevels[9], &tempUser.levelTimes[9], &tempUser.levelMoves[9],
                               &tempUser.totalScore);
            if (items < 2) continue;
            strncpy(tempUser.password, tempPassword, MAX_PASSWORD - 1);
            tempUser.password[MAX_PASSWORD - 1] = '\0';

            UserNode* newNode = (UserNode*)malloc(sizeof(UserNode));
            if (!newNode) {
                printf("Ошибка выделения памяти для UserNode\n");
                continue;
            }
            newNode->user = tempUser;
            newNode->next = NULL;

            if (head == NULL) {
                head = newNode;
                tail = newNode;
            } else {
                tail->next = newNode;
                tail = newNode;
            }
            userCount++;
        }
        fclose(file);
    }

    // Сортировка пользователей по убыванию очков
    if (userCount > 1) {
        for (UserNode* i = head; i != NULL; i = i->next) {
            for (UserNode* j = i->next; j != NULL; j = j->next) {
                if (i->user.totalScore < j->user.totalScore) {
                    User temp = i->user;
                    i->user = j->user;
                    j->user = temp;
                }
            }
        }
    }

    // Отрисовка топ-10
    int rank = 0;
    for (UserNode* current = head; current != NULL && rank < 10; current = current->next, rank++) {
        char entry[50];
        snprintf(entry, sizeof(entry), "%d. %s - %.2f", rank + 1, current->user.username, current->user.totalScore);
        drawText(-0.8, 0.7 - rank * 0.1, entry, 1.0, 1.0, 1.0);
    }

    drawRectangle(-0.1, -0.8, 0.2, 0.1, 0.5, 0.5, 0.5);
    drawText(-0.05, -0.75, "Back", 1.0, 1.0, 1.0);

    // Освобождение памяти
    freeUserList(head);

    glutSwapBuffers();
}

// Отрисовка демонстрационного уровня
void tutorialDisplay() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawRectangle(GAME_RECT_X, GAME_RECT_Y, GAME_RECT_WIDTH, GAME_RECT_HEIGHT, 0.999, 0.844, 0.600);

    float cellWidth = GAME_RECT_WIDTH / COLS;
    float cellHeight = GAME_RECT_HEIGHT / ROWS;

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            float x = GAME_RECT_X + j * cellWidth;
            float y = GAME_RECT_Y + i * cellHeight;
            drawCell(x, y, cellWidth, cellHeight, tutorialLevelState.grid[i][j]);
        }
    }

    char hint[50];
    switch (currentTutorialMove) {
        case 0:
            snprintf(hint, sizeof(hint), "Press D or Right Arrow to move right");
            break;
        case 1:
            snprintf(hint, sizeof(hint), "Press D or Right Arrow to push the box");
            break;
        case 2:
            snprintf(hint, sizeof(hint), "Press S or Down Arrow to push the box to the target");
            break;
        default:
            snprintf(hint, sizeof(hint), "Tutorial Complete!");
            break;
    }
    drawText(-0.4, 0.8, "Welcome to Sokoban! Watch how to play:", 1.0, 1.0, 1.0);
    drawText(-0.4, 0.7, hint, 0.0, 1.0, 0.0);

    glutSwapBuffers();
}

// Обновление анимации
void tutorialUpdate(int value) {
    if (currentTutorialMove >= tutorialMoveCount) {
        currentMode = GAME;
        isFirstGame = false;
        loadLevel(0);
        glutPostRedisplay();
        return;
    }

    char move = tutorialMoves[currentTutorialMove];
    switch (move) {
        case 'w': case 'W': updateTutorialLevel(0, 1); break;
        case 's': case 'S': updateTutorialLevel(0, -1); break;
        case 'a': case 'A': updateTutorialLevel(-1, 0); break;
        case 'd': case 'D': updateTutorialLevel(1, 0); break;
    }

    currentTutorialMove++;
    glutPostRedisplay();
    glutTimerFunc(1500, tutorialUpdate, 0);
}

// Отрисовка окна управления
void controlsDisplay() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawRectangle(-0.8, -0.8, 1.6, 1.6, 0.2, 0.2, 0.2);

    drawText(-0.4, 0.8, "Controls", 1.0, 1.0, 1.0);

    drawRectangle(-0.6, 0.5, 0.2, 0.1, 0.0, 0.5, 1.0);
    drawText(-0.55, 0.55, "W / Up Arrow", 1.0, 1.0, 1.0);
    drawText(-0.3, 0.55, ": Move Up", 1.0, 1.0, 1.0);

    drawRectangle(-0.6, 0.3, 0.2, 0.1, 0.0, 0.5, 1.0);
    drawText(-0.55, 0.35, "S / Down Arrow", 1.0, 1.0, 1.0);
    drawText(-0.3, 0.35, ": Move Down", 1.0, 1.0, 1.0);

    drawRectangle(-0.6, 0.1, 0.2, 0.1, 0.0, 0.5, 1.0);
    drawText(-0.55, 0.15, "A / Left Arrow", 1.0, 1.0, 1.0);
    drawText(-0.3, 0.15, ": Move Left", 1.0, 1.0, 1.0);

    drawRectangle(-0.6, -0.1, 0.2, 0.1, 0.0, 0.5, 1.0);
    drawText(-0.55, -0.05, "D / Right Arrow", 1.0, 1.0, 1.0);
    drawText(-0.3, -0.05, ": Move Right", 1.0, 1.0, 1.0);

    drawRectangle(-0.6, -0.3, 0.2, 0.1, 0.0, 0.8, 0.0);
    drawText(-0.55, -0.25, "Enter", 1.0, 1.0, 1.0);
    drawText(-0.3, -0.25, ": Confirm / Submit", 1.0, 1.0, 1.0);

    drawRectangle(-0.1, -0.8, 0.2, 0.1, 0.5, 0.5, 0.5);
    drawText(-0.05, -0.75, "Back", 1.0, 1.0, 1.0);

    glutSwapBuffers();
}

// Главная функция отображения
void display() {
    switch (currentMode) {
        case LOGIN:
            displayLogin();
            break;
        case GAME:
            displayGame();
            break;
        case LEVEL_SELECTOR:
            levelSelectorDisplay();
            break;
        case LEADERBOARD:
            leaderboardDisplay();
            break;
        case TUTORIAL:
            tutorialDisplay();
            break;
        case CONTROLS:
            controlsDisplay();
            break;
    }
}

// Обработчик изменения размеров
void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Обработчик мыши
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        float glX = (float)x / glutGet(GLUT_WINDOW_WIDTH) * 2.0 - 1.0;
        float glY = -((float)y / glutGet(GLUT_WINDOW_HEIGHT) * 2.0 - 1.0);

        if (currentMode == LOGIN) {
            if (glX >= TOGGLE_BUTTON_X && glX <= TOGGLE_BUTTON_X + TOGGLE_BUTTON_WIDTH &&
                glY >= TOGGLE_BUTTON_Y && glY <= TOGGLE_BUTTON_Y + TOGGLE_BUTTON_HEIGHT) {
                isRegisterMode = !isRegisterMode;
                strcpy(errorMessage, "");
                glutPostRedisplay();
            }
            else if (glX >= QUIT_LOGIN_BUTTON_X && glX <= QUIT_LOGIN_BUTTON_X + QUIT_LOGIN_BUTTON_WIDTH &&
                     glY >= QUIT_LOGIN_BUTTON_Y && glY <= QUIT_LOGIN_BUTTON_Y + QUIT_LOGIN_BUTTON_HEIGHT) {
                exit(0);
            }
        }
        else if (currentMode == GAME) {
            if (glX >= NEXT_BUTTON_X && glX <= NEXT_BUTTON_X + BUTTON_WIDTH &&
                glY >= NEXT_BUTTON_Y && glY <= NEXT_BUTTON_Y + BUTTON_HEIGHT) {
                loadLevel(currentLevel + 1);
                glutPostRedisplay();
            }
            else if (glX >= PREV_BUTTON_X && glX <= PREV_BUTTON_X + BUTTON_WIDTH &&
                     glY >= PREV_BUTTON_Y && glY <= PREV_BUTTON_Y + BUTTON_HEIGHT) {
                loadLevel(currentLevel - 1);
                glutPostRedisplay();
            }
            else if (glX >= LEVEL_BUTTON_X && glX <= LEVEL_BUTTON_X + BUTTON_WIDTH &&
                     glY >= LEVEL_BUTTON_Y && glY <= LEVEL_BUTTON_Y + BUTTON_HEIGHT) {
                currentMode = LEVEL_SELECTOR;
                glutPostRedisplay();
            }
            else if (glX >= RESTART_BUTTON_X && glX <= RESTART_BUTTON_X + BUTTON_WIDTH &&
                     glY >= RESTART_BUTTON_Y && glY <= RESTART_BUTTON_Y + BUTTON_HEIGHT) {
                loadLevel(currentLevel);
                glutPostRedisplay();
            }
            else if (glX >= LEADERBOARD_BUTTON_X && glX <= LEADERBOARD_BUTTON_X + BUTTON_WIDTH &&
                     glY >= LEADERBOARD_BUTTON_Y && glY <= LEADERBOARD_BUTTON_Y + BUTTON_HEIGHT) {
                currentMode = LEADERBOARD;
                glutPostRedisplay();
            }
            else if (glX >= CONTROLS_BUTTON_X && glX <= CONTROLS_BUTTON_X + BUTTON_WIDTH &&
                     glY >= CONTROLS_BUTTON_Y && glY <= CONTROLS_BUTTON_Y + BUTTON_HEIGHT) {
                currentMode = CONTROLS;
                glutPostRedisplay();
            }
            else if (glX >= QUIT_BUTTON_X && glX <= QUIT_BUTTON_X + BUTTON_WIDTH &&
                     glY >= QUIT_BUTTON_Y && glY <= QUIT_BUTTON_Y + BUTTON_HEIGHT) {
                exit(0);
            }
        }
        else if (currentMode == LEVEL_SELECTOR) {
            for (int i = 0; i < totalLevels; i++) {
                float btnX = LEVEL_SELECT_BASE_X + (i % 5) * (LEVEL_SELECT_BUTTON_WIDTH + 0.05);
                float btnY = LEVEL_SELECT_BASE_Y - (i / 5) * (LEVEL_SELECT_BUTTON_HEIGHT + 0.05);
                if (glX >= btnX && glX <= btnX + LEVEL_SELECT_BUTTON_WIDTH &&
                    glY >= btnY && glY <= btnY + LEVEL_SELECT_BUTTON_HEIGHT) {
                    loadLevel(i);
                    currentMode = GAME;
                    glutPostRedisplay();
                    break;
                }
            }
            if (glX >= -0.1 && glX <= 0.1 && glY >= -0.8 && glY <= -0.7) {
                currentMode = GAME;
                glutPostRedisplay();
            }
        }
        else if (currentMode == LEADERBOARD) {
            if (glX >= -0.1 && glX <= 0.1 && glY >= -0.8 && glY <= -0.7) {
                currentMode = GAME;
                glutPostRedisplay();
            }
        }
        else if (currentMode == CONTROLS) {
            if (glX >= -0.1 && glX <= 0.1 && glY >= -0.8 && glY <= -0.7) {
                currentMode = GAME;
                glutPostRedisplay();
            }
        }
    }
}

// Запуск игры
void startGame() {
    printf("Запуск игры для пользователя: %s\n", loggedInUser);
    if (isFirstGame) {
        currentMode = TUTORIAL;
        loadTutorialLevel();
        glutTimerFunc(1500, tutorialUpdate, 0);
    } else {
        currentMode = GAME;
        loadLevel(0);
    }
    glutPostRedisplay();
}

// Обработчик клавиш для авторизации
void loginKeyboard(unsigned char key, int x, int y) {
    if (key == 13) {
        if (!isUsernameEntered) {
            int len = strlen(currentUsername);
            if (len < MIN_USERNAME || len > MAX_USERNAME - 1) {
                snprintf(errorMessage, sizeof(errorMessage), "Username must be %d-%d characters!", MIN_USERNAME, MAX_USERNAME - 1);
            } else {
                isUsernameEntered = true;
                strcpy(errorMessage, "");
            }
        } else {
            int passLen = strlen(currentPassword);
            if (passLen < MIN_PASSWORD || passLen > MAX_PASSWORD - 1) {
                snprintf(errorMessage, sizeof(errorMessage), "Password must be %d-%d characters!", MIN_PASSWORD, MAX_PASSWORD - 1);
            } else if (isRegisterMode) {
                passwordStrength = evaluatePasswordStrength(currentPassword);
                if (passwordStrength <= 2) {
                    char feedback[100];
                    getPasswordStrengthFeedback(passwordStrength, feedback);
                    snprintf(errorMessage, sizeof(errorMessage), "%s", feedback);
                } else if (checkUserExists(currentUsername, NULL)) {
                    snprintf(errorMessage, sizeof(errorMessage), "Username already exists!");
                } else {
                    User newUser = {{0}, {0}, {0}, {0}, {0}, 0};
                    strncpy(newUser.username, currentUsername, MAX_USERNAME - 1);
                    newUser.username[MAX_USERNAME - 1] = '\0';
                    char hashed_password[SHA256_DIGEST_LENGTH * 2 + 1];
                    hash_password(currentPassword, hashed_password);
                    strncpy(newUser.password, hashed_password, MAX_PASSWORD - 1);
                    newUser.password[MAX_PASSWORD - 1] = '\0';
                    newUser.totalScore = 0.0;
                    saveUserData(newUser);
                    strncpy(loggedInUser, currentUsername, MAX_USERNAME - 1);
                    loggedInUser[MAX_USERNAME - 1] = '\0';
                    isLoginScreen = false;
                    isFirstGame = true;
                    printf("Регистрация успешна: %s\n", currentUsername);
                    startGame();
                }
            } else {
                User user;
                if (!checkUserExists(currentUsername, &user)) {
                    snprintf(errorMessage, sizeof(errorMessage), "Username does not exist!");
                } else {
                    char hashed_input[SHA256_DIGEST_LENGTH * 2 + 1];
                    hash_password(currentPassword, hashed_input);
                    if (strcmp(user.password, hashed_input) != 0) {
                        snprintf(errorMessage, sizeof(errorMessage), "Incorrect password!");
                    } else {
                        strncpy(loggedInUser, currentUsername, MAX_USERNAME - 1);
                        loggedInUser[MAX_USERNAME - 1] = '\0';
                        isLoginScreen = false;
                        printf("Вход успешен: %s\n", currentUsername);
                        startGame();
                    }
                }
            }
        }
        glutPostRedisplay();
    } else if (key == 8) {
        if (isUsernameEntered && strlen(currentPassword) > 0) {
            currentPassword[strlen(currentPassword) - 1] = '\0';
            passwordStrength = evaluatePasswordStrength(currentPassword);
        } else if (!isUsernameEntered && strlen(currentUsername) > 0) {
            currentUsername[strlen(currentUsername) - 1] = '\0';
        }
        strcpy(errorMessage, "");
        glutPostRedisplay();
    } else if (key >= 32 && key <= 126) {
        if (!isUsernameEntered && strlen(currentUsername) < MAX_USERNAME - 1) {
            currentUsername[strlen(currentUsername)] = key;
            currentUsername[strlen(currentUsername)] = '\0';
            strcpy(errorMessage, "");
        } else if (isUsernameEntered && strlen(currentPassword) < MAX_PASSWORD - 1) {
            currentPassword[strlen(currentPassword)] = key;
            currentPassword[strlen(currentPassword)] = '\0';
            passwordStrength = evaluatePasswordStrength(currentPassword);
            strcpy(errorMessage, "");
        }
        glutPostRedisplay();
    }
}

// Обработчик клавиш для игры
void keyboard(unsigned char key, int x, int y) {
    if (currentMode == LOGIN) {
        loginKeyboard(key, x, y);
    } else if (currentMode == GAME) {
        switch (key) {
            case 'w': case 'W': updateLevel(0, 1); break;
            case 's': case 'S': updateLevel(0, -1); break;
            case 'a': case 'A': updateLevel(-1, 0); break;
            case 'd': case 'D': updateLevel(1, 0); break;
        }
        glutPostRedisplay();
    }
}

// Обработчик специальных клавиш
void specialKeys(int key, int x, int y) {
    if (currentMode == GAME) {
        switch (key) {
            case GLUT_KEY_UP:    updateLevel(0, 1); break;
            case GLUT_KEY_DOWN:  updateLevel(0, -1); break;
            case GLUT_KEY_LEFT:  updateLevel(-1, 0); break;
            case GLUT_KEY_RIGHT: updateLevel(1, 0); break;
        }
        glutPostRedisplay();
    }
}

// Главная функция
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Sokoban by NP");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glutMainLoop();
    return 0;
}
