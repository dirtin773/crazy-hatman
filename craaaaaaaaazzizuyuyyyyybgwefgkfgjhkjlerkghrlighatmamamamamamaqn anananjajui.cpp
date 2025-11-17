#include "raylib.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define WIDTH 800
#define HEIGHT 600
#define MAX_BULLETS 100
#define MAX_ENEMIES 50
#define MAX_BONUSES 10
#define MAX_KNIVES 50
#define MAX_BOSS_BULLETS 50
#define MAX_ENEMY_BULLETS 30  // Пули обычных врагов

// Структура кнопки
typedef struct {
    Rectangle rect;
    const char* text;
    Color color;
    Color hoverColor;
    Color currentColor;
} Button;

// Структура ножа
typedef struct {
    float x, y;
    float speed;
    float radius;
    Color color;
    float directionAngle;
    float distanceTraveled;
    float maxDistance;
} Knife;

// Структура бонуса
typedef struct {
    float x, y;
    float width, height;
    Color color;
    float speed;
    bool collected;
    int lifetime;
    char bonusType[20];
} HatBonus;

// Структура игрока
typedef struct {
    float x, y;
    float radius;
    float speed;
    Color color;
    int health;
    int lives;
    bool invincible;
    int invincibleTimer;
    int damageMultiplier;
    int bonusTimer;
    bool hasKnifeBonus;
    int shootCooldown;
} Player;

// Структура пули
typedef struct {
    float x, y;
    float radius;
    float speed;
    Color color;
    int damage;
    float dx, dy;
} Bullet;

// Структура пули босса
typedef struct {
    float x, y;
    float radius;
    float speed;
    Color color;
    int damage;
    float dx, dy;
} BossBullet;

// Структура пули врага
typedef struct {
    float x, y;
    float radius;
    float speed;
    Color color;
    int damage;
    float dx, dy;
} EnemyBullet;

// Структура врага
typedef struct {
    float x, y;
    float radius;
    float speed;
    Color color;
    int health;
    int damage;
    int scoreValue;
    int level;
    bool isElite;
    bool isBoss;
    bool isShooter;   // Стреляющий враг
    bool isTank;      // Танк
    bool isRunner;    // Бегун
    float dx, dy;
    int attackCooldown;
    int shootCooldown;
    bool hasStopped;
    int attackPattern;
    int attackTimer;
} Enemy;

// Структура игры
typedef struct {
    char state[20];
    int level;
    int maxLevel;
    int score;
    int enemiesToDefeat;
    int enemiesDefeated;
    Player player;
    Bullet bullets[MAX_BULLETS];
    int bulletCount;
    BossBullet bossBullets[MAX_BOSS_BULLETS];
    int bossBulletCount;
    EnemyBullet enemyBullets[MAX_ENEMY_BULLETS];
    int enemyBulletCount;
    Enemy enemies[MAX_ENEMIES];
    int enemyCount;
    HatBonus bonuses[MAX_BONUSES];
    int bonusCount;
    Knife knives[MAX_KNIVES];
    int knifeCount;
    int enemySpawnTimer;
    int levelCompleteTimer;
    int bonusSpawnTimer;
    bool bossSpawned;
    bool bossDefeated;

    Button newGameButton;
    Button continueButton;
    Button quitButton;
} Game;

// Функции для кнопок
Button CreateButton(float x, float y, float width, float height, const char* text, Color color, Color hoverColor) {
    Button button;
    button.rect = { x - width / 2, y - height / 2, width, height };
    button.text = text;
    button.color = color;
    button.hoverColor = hoverColor;
    button.currentColor = color;
    return button;
}

void DrawButton(Button button) {
    DrawRectangleRec(button.rect, button.currentColor);
    DrawRectangleLinesEx(button.rect, 2, WHITE);

    int fontSize = 20;
    Vector2 textSize = MeasureTextEx(GetFontDefault(), button.text, fontSize, 1);
    Vector2 textPos = {
        button.rect.x + button.rect.width / 2 - textSize.x / 2,
        button.rect.y + button.rect.height / 2 - textSize.y / 2
    };

    DrawText(button.text, textPos.x, textPos.y, fontSize, WHITE);
}

bool IsButtonHovered(Button button) {
    return CheckCollisionPointRec(GetMousePosition(), button.rect);
}

void UpdateButton(Button* button) {
    if (IsButtonHovered(*button)) {
        button->currentColor = button->hoverColor;
    }
    else {
        button->currentColor = button->color;
    }
}

// Функции для ножей
Knife CreateKnife(float startX, float startY, float angle) {
    Knife knife;
    knife.x = startX;
    knife.y = startY;
    knife.speed = 8;
    knife.radius = 5;
    knife.color = RED;
    knife.directionAngle = angle;
    knife.distanceTraveled = 0;
    knife.maxDistance = 200;
    return knife;
}

void UpdateKnife(Knife* knife) {
    knife->x += cos(knife->directionAngle) * knife->speed;
    knife->y += sin(knife->directionAngle) * knife->speed;
    knife->distanceTraveled += knife->speed;
}

void DrawKnife(Knife knife) {
    Vector2 points[3] = {
        {knife.x + 10, knife.y},
        {knife.x - 3, knife.y + 3},
        {knife.x - 3, knife.y - 3}
    };

    for (int i = 0; i < 3; i++) {
        Vector2 rotated = {
            knife.x + (points[i].x - knife.x) * cos(knife.directionAngle) - (points[i].y - knife.y) * sin(knife.directionAngle),
            knife.y + (points[i].x - knife.x) * sin(knife.directionAngle) + (points[i].y - knife.y) * cos(knife.directionAngle)
        };
        points[i] = rotated;
    }

    DrawTriangle(points[0], points[1], points[2], RED);
    DrawTriangleLines(points[0], points[1], points[2], BLACK);
}

bool IsKnifeOffScreen(Knife knife) {
    return (knife.x < -20 || knife.x > WIDTH + 20 ||
        knife.y < -20 || knife.y > HEIGHT + 20 ||
        knife.distanceTraveled >= knife.maxDistance);
}

// Функции для бонусов
HatBonus CreateHatBonus(float startX, float startY, const char* type) {
    HatBonus bonus;
    bonus.x = startX;
    bonus.y = startY;
    bonus.width = 30;
    bonus.height = 25;
    bonus.speed = 2;
    bonus.collected = false;
    bonus.lifetime = 300;
    strcpy(bonus.bonusType, type);

    if (strcmp(type, "damage") == 0) {
        bonus.color = GOLD;
    }
    else {
        bonus.color = RED;
    }
    return bonus;
}

void UpdateHatBonus(HatBonus* bonus) {
    bonus->y += bonus->speed;
    bonus->lifetime--;
}

void DrawHatBonus(HatBonus bonus) {
    DrawRectangle(bonus.x - bonus.width / 2, bonus.y, bonus.width, 10, bonus.color);
    DrawRectangle(bonus.x - bonus.width / 3, bonus.y - 15, bonus.width / 1.5f, 15, bonus.color);

    const char* bonusName = (strcmp(bonus.bonusType, "damage") == 0) ? "DMG x2" : "KNIVES";
    Color textColor = (strcmp(bonus.bonusType, "damage") == 0) ? BLACK : WHITE;
    int textWidth = MeasureText(bonusName, 12);
    DrawText(bonusName, bonus.x - textWidth / 2, bonus.y - 30, 12, textColor);
}

bool ShouldRemoveBonus(HatBonus bonus) {
    return bonus.lifetime <= 0 || bonus.y > HEIGHT;
}

bool BonusCollidesWithPlayer(HatBonus bonus, Player player) {
    float distance = sqrt(pow(bonus.x - player.x, 2) + pow(bonus.y - player.y, 2));
    return distance < (bonus.width / 2 + player.radius);
}

// Функции для игрока
Player CreatePlayer() {
    Player player;
    player.x = WIDTH / 2;
    player.y = HEIGHT - 50;
    player.radius = 20;
    player.speed = 5;
    player.color = BLUE;
    player.health = 10;
    player.lives = 1;
    player.invincible = false;
    player.invincibleTimer = 0;
    player.damageMultiplier = 1;
    player.bonusTimer = 0;
    player.hasKnifeBonus = false;
    player.shootCooldown = 0;
    return player;
}

void DrawPlayer(Player player) {
    DrawLine(player.x - 8, player.y + player.radius, player.x - 12, player.y + player.radius + 15, BLACK);
    DrawLine(player.x + 8, player.y + player.radius, player.x + 12, player.y + player.radius + 15, BLACK);

    DrawLine(player.x - 12, player.y, player.x - 12 - 18, player.y + 5, BLACK);
    DrawLine(player.x + 12, player.y, player.x + 12 + 18, player.y + 5, BLACK);

    Color bodyColor = player.invincible ? RED : player.color;
    DrawCircle(player.x, player.y, player.radius, bodyColor);

    Color hatColor;
    if (player.hasKnifeBonus) {
        hatColor = RED;
    }
    else if (player.damageMultiplier > 1) {
        hatColor = GOLD;
    }
    else {
        hatColor = DARKBLUE;
    }

    DrawRectangle(player.x - 25, player.y - player.radius - 20, 50, 15, hatColor);
    DrawRectangle(player.x - 15, player.y - player.radius - 35, 30, 20, hatColor);

    DrawCircle(player.x - 8, player.y - 5, 5, WHITE);
    DrawCircle(player.x + 8, player.y - 5, 5, WHITE);

    Vector2 mousePos = GetMousePosition();
    float dx = mousePos.x - player.x;
    float dy = mousePos.y - player.y;
    float distance = sqrt(dx * dx + dy * dy);
    if (distance > 0) {
        dx /= distance;
        dy /= distance;
    }

    DrawCircle(player.x - 8 + dx * 2, player.y - 5 + dy * 2, 2, BLACK);
    DrawCircle(player.x + 8 + dx * 2, player.y - 5 + dy * 2, 2, BLACK);

    if (player.health <= 3) {
        Vector2 mouthPoints[3] = {
            {player.x - 8, player.y + 3},
            {player.x + 8, player.y + 3},
            {player.x, player.y + 13}
        };
        DrawTriangle(mouthPoints[0], mouthPoints[1], mouthPoints[2], BLACK);
    }
    else if (player.invincible) {
        Vector2 mouthPoints[3] = {
            {player.x - 8, player.y + 13},
            {player.x + 8, player.y + 13},
            {player.x, player.y + 3}
        };
        DrawTriangle(mouthPoints[0], mouthPoints[1], mouthPoints[2], RED);
    }
    else {
        DrawLine(player.x - 6, player.y + 5, player.x + 6, player.y + 5, BLACK);
    }

    DrawRectangle(player.x - 20, player.y - player.radius - 10, 40, 6, RED);
    DrawRectangle(player.x - 20, player.y - player.radius - 10, 40 * (player.health / 10.0f), 6, GREEN);
}

void MovePlayer(Player* player) {
    if (IsKeyDown(KEY_LEFT) && player->x - player->radius > 0) {
        player->x -= player->speed;
    }
    if (IsKeyDown(KEY_RIGHT) && player->x + player->radius < WIDTH) {
        player->x += player->speed;
    }
    if (IsKeyDown(KEY_UP) && player->y - player->radius > 0) {
        player->y -= player->speed;
    }
    if (IsKeyDown(KEY_DOWN) && player->y + player->radius < HEIGHT) {
        player->y += player->speed;
    }
}

bool PlayerTakeDamage(Player* player, int damage) {
    if (!player->invincible) {
        player->health -= damage;
        player->invincible = true;
        player->invincibleTimer = 90;

        if (player->health <= 0) {
            player->lives--;
            player->health = 0;
            player->damageMultiplier = 1;
            player->bonusTimer = 0;
            player->hasKnifeBonus = false;
            return true;
        }
    }
    return false;
}

void UpdatePlayer(Player* player) {
    if (player->invincible) {
        player->invincibleTimer--;
        if (player->invincibleTimer <= 0) {
            player->invincible = false;
        }
    }

    if (player->damageMultiplier > 1) {
        player->bonusTimer--;
        if (player->bonusTimer <= 0) {
            player->damageMultiplier = 1;
        }
    }

    if (player->shootCooldown > 0) {
        player->shootCooldown--;
    }
}

void AddDamageBonus(Player* player) {
    player->damageMultiplier = 2;
    player->bonusTimer = 600;
    player->hasKnifeBonus = false;
}

void AddKnifeBonus(Player* player) {
    player->hasKnifeBonus = true;
    player->damageMultiplier = 1;
    player->bonusTimer = 0;
}

bool IsPlayerAlive(Player player) {
    return player.lives > 0 && player.health > 0;
}

void CreatePlayerKnives(Player player, Knife knives[], int* knifeCount) {
    for (int i = 0; i < 10; i++) {
        if (*knifeCount < MAX_KNIVES) {
            float angle = i * 36 * PI / 180.0f;
            knives[*knifeCount] = CreateKnife(player.x, player.y, angle);
            (*knifeCount)++;
        }
    }
}

// Функции для пуль
Bullet CreateBullet(float startX, float startY, float targetX, float targetY, int level, int damageMultiplier) {
    Bullet bullet;
    bullet.x = startX;
    bullet.y = startY;
    bullet.radius = 8;
    bullet.speed = 7 + level;

    if (level == 1) bullet.color = GREEN;
    else if (level == 2) bullet.color = YELLOW;
    else bullet.color = PURPLE;

    bullet.damage = 3 * damageMultiplier;

    float diffX = targetX - startX;
    float diffY = targetY - startY;
    float distance = sqrt(diffX * diffX + diffY * diffY);
    if (distance > 0) {
        bullet.dx = diffX / distance * bullet.speed;
        bullet.dy = diffY / distance * bullet.speed;
    }
    else {
        bullet.dx = 0;
        bullet.dy = -bullet.speed;
    }

    return bullet;
}

void UpdateBullet(Bullet* bullet) {
    bullet->x += bullet->dx;
    bullet->y += bullet->dy;
}

void DrawBullet(Bullet bullet) {
    DrawCircle(bullet.x, bullet.y, bullet.radius, bullet.color);
    DrawCircleLines(bullet.x, bullet.y, bullet.radius, BLACK);
}

bool IsBulletOffScreen(Bullet bullet) {
    return (bullet.x < -bullet.radius || bullet.x > WIDTH + bullet.radius ||
        bullet.y < -bullet.radius || bullet.y > HEIGHT + bullet.radius);
}

// Функции для пуль босса
BossBullet CreateBossBullet(float startX, float startY, float dx, float dy) {
    BossBullet bullet;
    bullet.x = startX;
    bullet.y = startY;
    bullet.radius = 8;
    bullet.speed = 5;
    bullet.color = { 255, 50, 50, 255 };
    bullet.damage = 2;

    bullet.dx = dx;
    bullet.dy = dy;

    return bullet;
}

void UpdateBossBullet(BossBullet* bullet) {
    bullet->x += bullet->dx * bullet->speed;
    bullet->y += bullet->dy * bullet->speed;
}

void DrawBossBullet(BossBullet bullet) {
    DrawCircle(bullet.x, bullet.y, bullet.radius, bullet.color);
    DrawCircleLines(bullet.x, bullet.y, bullet.radius, BLACK);
}

bool IsBossBulletOffScreen(BossBullet bullet) {
    return (bullet.x < -bullet.radius || bullet.x > WIDTH + bullet.radius ||
        bullet.y < -bullet.radius || bullet.y > HEIGHT + bullet.radius);
}

// Функции для пуль врагов
EnemyBullet CreateEnemyBullet(float startX, float startY, float targetX, float targetY) {
    EnemyBullet bullet;
    bullet.x = startX;
    bullet.y = startY;
    bullet.radius = 6;
    bullet.speed = 4;
    bullet.color = ORANGE;
    bullet.damage = 1;

    float diffX = targetX - startX;
    float diffY = targetY - startY;
    float distance = sqrt(diffX * diffX + diffY * diffY);
    if (distance > 0) {
        bullet.dx = diffX / distance * bullet.speed;
        bullet.dy = diffY / distance * bullet.speed;
    }
    else {
        bullet.dx = 0;
        bullet.dy = bullet.speed;
    }

    return bullet;
}

void UpdateEnemyBullet(EnemyBullet* bullet) {
    bullet->x += bullet->dx;
    bullet->y += bullet->dy;
}

void DrawEnemyBullet(EnemyBullet bullet) {
    DrawCircle(bullet.x, bullet.y, bullet.radius, bullet.color);
    DrawCircleLines(bullet.x, bullet.y, bullet.radius, BLACK);
}

bool IsEnemyBulletOffScreen(EnemyBullet bullet) {
    return (bullet.x < -bullet.radius || bullet.x > WIDTH + bullet.radius ||
        bullet.y < -bullet.radius || bullet.y > HEIGHT + bullet.radius);
}

bool BossBulletCollidesWithPlayer(BossBullet bullet, Player player) {
    float distance = sqrt(pow(bullet.x - player.x, 2) + pow(bullet.y - player.y, 2));
    return distance < bullet.radius + player.radius;
}

bool EnemyBulletCollidesWithPlayer(EnemyBullet bullet, Player player) {
    float distance = sqrt(pow(bullet.x - player.x, 2) + pow(bullet.y - player.y, 2));
    return distance < bullet.radius + player.radius;
}

// Функции для врагов
Enemy CreateEnemy(int enemyLevel, bool elite, bool boss, bool shooter, bool tank, bool runner, Player player) {
    Enemy enemy;
    enemy.level = enemyLevel;
    enemy.isElite = elite;
    enemy.isBoss = boss;
    enemy.isShooter = shooter;
    enemy.isTank = tank;
    enemy.isRunner = runner;
    enemy.attackCooldown = 0;
    enemy.shootCooldown = 0;
    enemy.hasStopped = false;
    enemy.attackPattern = 0;
    enemy.attackTimer = 0;

    if (boss) {
        enemy.radius = 50;
        enemy.speed = 1;
        enemy.color = PURPLE;
        enemy.health = 200;
        enemy.damage = 3;
        enemy.scoreValue = 500;
        enemy.x = WIDTH / 2;
        enemy.y = -100;
        enemy.shootCooldown = 30;
    }
    else if (shooter) {
        enemy.radius = 18;
        enemy.speed = 1.5f;
        enemy.color = { 0, 255, 255, 255 }; // Голубой
        enemy.health = 15;
        enemy.damage = 1;
        enemy.scoreValue = 25;
        enemy.x = GetRandomValue(enemy.radius, WIDTH - enemy.radius);
        enemy.y = -enemy.radius;
        enemy.shootCooldown = 90; // Стреляет раз в 1.5 секунды
    }
    else if (tank) {
        enemy.radius = 35;
        enemy.speed = 0.8f;
        enemy.color = { 100, 100, 100, 255 }; // Серый
        enemy.health = 50;
        enemy.damage = 2;
        enemy.scoreValue = 40;
        enemy.x = GetRandomValue(enemy.radius, WIDTH - enemy.radius);
        enemy.y = -enemy.radius;
    }
    else if (runner) {
        enemy.radius = 12;
        enemy.speed = 4.0f;
        enemy.color = { 255, 105, 180, 255 }; // Розовый
        enemy.health = 10;
        enemy.damage = 1;
        enemy.scoreValue = 15;
        enemy.x = GetRandomValue(enemy.radius, WIDTH - enemy.radius);
        enemy.y = -enemy.radius;
    }
    else if (elite) {
        enemy.radius = 30;
        enemy.speed = 1.5;
        enemy.color = GOLD;
        enemy.health = 25;
        enemy.damage = 2;
        enemy.scoreValue = 50;
        enemy.x = GetRandomValue(enemy.radius, WIDTH - enemy.radius);
        enemy.y = -enemy.radius;
    }
    else if (enemyLevel == 1) {
        enemy.radius = 15;
        enemy.speed = 2;
        enemy.color = RED;
        enemy.health = 15;
        enemy.damage = 1;
        enemy.scoreValue = 10;
        enemy.x = GetRandomValue(enemy.radius, WIDTH - enemy.radius);
        enemy.y = -enemy.radius;
    }
    else if (enemyLevel == 2) {
        enemy.radius = 20;
        enemy.speed = 2.5;
        enemy.color = ORANGE;
        enemy.health = 20;
        enemy.damage = 1;
        enemy.scoreValue = 20;
        enemy.x = GetRandomValue(enemy.radius, WIDTH - enemy.radius);
        enemy.y = -enemy.radius;
    }
    else {
        enemy.radius = 25;
        enemy.speed = 3;
        enemy.color = { 139, 0, 0, 255 };
        enemy.health = 25;
        enemy.damage = 2;
        enemy.scoreValue = 30;
        enemy.x = GetRandomValue(enemy.radius, WIDTH - enemy.radius);
        enemy.y = -enemy.radius;
    }

    float dx = player.x - enemy.x;
    float dy = player.y - enemy.y;
    float distance = sqrt(dx * dx + dy * dy);
    if (distance > 0) {
        enemy.dx = dx / distance * enemy.speed;
        enemy.dy = dy / distance * enemy.speed;
    }
    else {
        enemy.dx = 0;
        enemy.dy = enemy.speed;
    }

    return enemy;
}

void DrawEnemy(Enemy enemy) {
    // Ноги для всех врагов
    DrawLine(enemy.x - 5, enemy.y + enemy.radius, enemy.x - 8, enemy.y + enemy.radius + 10, BLACK);
    DrawLine(enemy.x + 5, enemy.y + enemy.radius, enemy.x + 8, enemy.y + enemy.radius + 10, BLACK);

    // Руки для всех врагов
    DrawLine(enemy.x - 8, enemy.y, enemy.x - 8 - 12, enemy.y + 3, BLACK);
    DrawLine(enemy.x + 8, enemy.y, enemy.x + 8 + 12, enemy.y + 3, BLACK);

    // Основное тело
    DrawCircle(enemy.x, enemy.y, enemy.radius, enemy.color);
    DrawCircleLines(enemy.x, enemy.y, enemy.radius, BLACK);

    if (enemy.isBoss) {
        Vector2 crownPoints[7] = {
            {enemy.x - 25, enemy.y - enemy.radius + 5},
            {enemy.x - 15, enemy.y - enemy.radius - 10},
            {enemy.x - 5, enemy.y - enemy.radius + 2},
            {enemy.x, enemy.y - enemy.radius - 15},
            {enemy.x + 5, enemy.y - enemy.radius + 2},
            {enemy.x + 15, enemy.y - enemy.radius - 10},
            {enemy.x + 25, enemy.y - enemy.radius + 5}
        };

        for (int i = 0; i < 6; i++) {
            DrawLine(crownPoints[i].x, crownPoints[i].y, crownPoints[i + 1].x, crownPoints[i + 1].y, YELLOW);
        }

        DrawText("BOSS", enemy.x - 25, enemy.y - enemy.radius - 40, 16, YELLOW);

        float eyeOffset = enemy.radius * 0.3;
        DrawCircle(enemy.x - eyeOffset, enemy.y - 10, 6, RED);
        DrawCircle(enemy.x + eyeOffset, enemy.y - 10, 6, RED);
        DrawCircle(enemy.x - eyeOffset, enemy.y - 10, 3, BLACK);
        DrawCircle(enemy.x + eyeOffset, enemy.y - 10, 3, BLACK);

        DrawLine(enemy.x - 10, enemy.y + 10, enemy.x + 10, enemy.y + 10, BLACK);
        DrawLine(enemy.x - 10, enemy.y + 10, enemy.x - 8, enemy.y + 5, BLACK);
        DrawLine(enemy.x + 10, enemy.y + 10, enemy.x + 8, enemy.y + 5, BLACK);

    }
    else if (enemy.isShooter) {
        // Стреляющий враг - с пистолетом
        DrawRectangle(enemy.x - 15, enemy.y - 5, 10, 5, DARKGRAY);
        DrawText("S", enemy.x - 5, enemy.y - enemy.radius - 15, 14, WHITE);

        float eyeOffset = enemy.radius * 0.4;
        DrawCircle(enemy.x - eyeOffset, enemy.y - 5, 3, WHITE);
        DrawCircle(enemy.x + eyeOffset, enemy.y - 5, 3, WHITE);
        DrawCircle(enemy.x - eyeOffset, enemy.y - 5, 1, BLACK);
        DrawCircle(enemy.x + eyeOffset, enemy.y - 5, 1, BLACK);

    }
    else if (enemy.isTank) {
        // Танк - с броней
        DrawRectangle(enemy.x - enemy.radius, enemy.y - enemy.radius, enemy.radius * 2, 8, DARKGRAY);
        DrawRectangle(enemy.x - enemy.radius, enemy.y + enemy.radius - 8, enemy.radius * 2, 8, DARKGRAY);
        DrawText("T", enemy.x - 5, enemy.y - enemy.radius - 15, 14, WHITE);

        float eyeOffset = enemy.radius * 0.3;
        DrawCircle(enemy.x - eyeOffset, enemy.y - 5, 4, WHITE);
        DrawCircle(enemy.x + eyeOffset, enemy.y - 5, 4, WHITE);
        DrawCircle(enemy.x - eyeOffset, enemy.y - 5, 2, BLACK);
        DrawCircle(enemy.x + eyeOffset, enemy.y - 5, 2, BLACK);

    }
    else if (enemy.isRunner) {
        // Бегун - маленький и быстрый
        DrawText("R", enemy.x - 5, enemy.y - enemy.radius - 12, 12, WHITE);

        float eyeOffset = enemy.radius * 0.5;
        DrawCircle(enemy.x - eyeOffset, enemy.y - 3, 2, WHITE);
        DrawCircle(enemy.x + eyeOffset, enemy.y - 3, 2, WHITE);
        DrawCircle(enemy.x - eyeOffset, enemy.y - 3, 1, BLACK);
        DrawCircle(enemy.x + eyeOffset, enemy.y - 3, 1, BLACK);

    }
    else if (enemy.isElite) {
        Vector2 crownPoints[7] = {
            {enemy.x - 15, enemy.y - enemy.radius + 5},
            {enemy.x - 10, enemy.y - enemy.radius - 5},
            {enemy.x - 5, enemy.y - enemy.radius + 2},
            {enemy.x, enemy.y - enemy.radius - 8},
            {enemy.x + 5, enemy.y - enemy.radius + 2},
            {enemy.x + 10, enemy.y - enemy.radius - 5},
            {enemy.x + 15, enemy.y - enemy.radius + 5}
        };

        for (int i = 0; i < 6; i++) {
            DrawLine(crownPoints[i].x, crownPoints[i].y, crownPoints[i + 1].x, crownPoints[i + 1].y, YELLOW);
        }

        DrawText("ELITE", enemy.x - 25, enemy.y - enemy.radius - 25, 12, YELLOW);
    }
    else {
        float eyeOffset = enemy.radius * 0.4;
        Color eyeColor = BLACK;

        DrawCircle(enemy.x - enemy.radius + eyeOffset + 2, enemy.y - enemy.radius + eyeOffset + 2, 4, WHITE);
        DrawCircle(enemy.x + enemy.radius - eyeOffset - 2, enemy.y - enemy.radius + eyeOffset + 2, 4, WHITE);

        DrawCircle(enemy.x - enemy.radius + eyeOffset + 2, enemy.y - enemy.radius + eyeOffset + 2, 2, eyeColor);
        DrawCircle(enemy.x + enemy.radius - eyeOffset - 2, enemy.y - enemy.radius + eyeOffset + 2, 2, eyeColor);

        float mouthY = enemy.y + eyeOffset;
        Vector2 mouthPoints[3] = {
            {enemy.x - 5, mouthY - 2},
            {enemy.x + 5, mouthY - 2},
            {enemy.x, mouthY + 3}
        };
        DrawTriangle(mouthPoints[0], mouthPoints[1], mouthPoints[2], BLACK);
    }

    char healthText[10];
    sprintf(healthText, "%d", enemy.health);
    int textWidth = MeasureText(healthText, 16);
    DrawText(healthText, enemy.x - textWidth / 2, enemy.y - 25, 16, WHITE);
}

void ShooterEnemyAttack(Enemy* shooter, EnemyBullet enemyBullets[], int* enemyBulletCount, Player player) {
    shooter->shootCooldown--;
    if (shooter->shootCooldown <= 0) {
        if (*enemyBulletCount < MAX_ENEMY_BULLETS) {
            enemyBullets[*enemyBulletCount] = CreateEnemyBullet(shooter->x, shooter->y, player.x, player.y);
            (*enemyBulletCount)++;
        }
        shooter->shootCooldown = 90; // Стреляет раз в 1.5 секунды
    }
}

void BossAttackPattern(Enemy* boss, BossBullet bossBullets[], int* bossBulletCount) {
    boss->attackTimer++;

    if (boss->attackTimer >= 180) {
        boss->attackPattern = (boss->attackPattern + 1) % 3;
        boss->attackTimer = 0;
    }

    boss->shootCooldown--;
    if (boss->shootCooldown <= 0) {
        switch (boss->attackPattern) {
        case 0: // Веерная атака
            for (int i = 0; i < 12; i++) {
                if (*bossBulletCount < MAX_BOSS_BULLETS) {
                    float angle = i * 30 * PI / 180.0f;
                    float dx = cos(angle);
                    float dy = sin(angle);
                    bossBullets[*bossBulletCount] = CreateBossBullet(boss->x, boss->y, dx, dy);
                    (*bossBulletCount)++;
                }
            }
            boss->shootCooldown = 40;
            break;

        case 1: // Спиральная атака
            for (int i = 0; i < 8; i++) {
                if (*bossBulletCount < MAX_BOSS_BULLETS) {
                    float angle = (boss->attackTimer * 10 + i * 45) * PI / 180.0f;
                    float dx = cos(angle);
                    float dy = sin(angle);
                    bossBullets[*bossBulletCount] = CreateBossBullet(boss->x, boss->y, dx, dy);
                    (*bossBulletCount)++;
                }
            }
            boss->shootCooldown = 20;
            break;

        case 2: // Прицельная атака + веер
            for (int i = -1; i <= 1; i++) {
                if (*bossBulletCount < MAX_BOSS_BULLETS) {
                    float spread = i * 0.2f;
                    bossBullets[*bossBulletCount] = CreateBossBullet(boss->x, boss->y, 0 + spread, 1);
                    (*bossBulletCount)++;
                }
            }
            for (int i = 0; i < 8; i++) {
                if (*bossBulletCount < MAX_BOSS_BULLETS) {
                    float angle = (i * 45 - 20) * PI / 180.0f;
                    float dx = cos(angle);
                    float dy = sin(angle);
                    bossBullets[*bossBulletCount] = CreateBossBullet(boss->x, boss->y, dx, dy);
                    (*bossBulletCount)++;
                }
            }
            boss->shootCooldown = 50;
            break;
        }
    }
}

void UpdateEnemy(Enemy* enemy, Player player, BossBullet bossBullets[], int* bossBulletCount, EnemyBullet enemyBullets[], int* enemyBulletCount) {
    if (enemy->isBoss) {
        if (!enemy->hasStopped && enemy->y >= 100) {
            enemy->hasStopped = true;
            enemy->dy = 0;
        }

        if (enemy->hasStopped) {
            BossAttackPattern(enemy, bossBullets, bossBulletCount);
        }
        else {
            enemy->x += enemy->dx;
            enemy->y += enemy->dy;
        }
    }
    else if (enemy->isShooter) {
        // Стреляющий враг останавливается и стреляет
        if (!enemy->hasStopped && enemy->y >= 150) {
            enemy->hasStopped = true;
            enemy->dy = 0;
        }

        if (enemy->hasStopped) {
            ShooterEnemyAttack(enemy, enemyBullets, enemyBulletCount, player);
        }
        else {
            enemy->x += enemy->dx;
            enemy->y += enemy->dy;
        }
    }
    else {
        // Обычные враги, танки и бегуны просто двигаются к игроку
        float dx = player.x - enemy->x;
        float dy = player.y - enemy->y;
        float distance = sqrt(dx * dx + dy * dy);
        if (distance > 0) {
            enemy->dx = dx / distance * enemy->speed;
            enemy->dy = dy / distance * enemy->speed;
        }

        enemy->x += enemy->dx;
        enemy->y += enemy->dy;
    }

    if (enemy->attackCooldown > 0) {
        enemy->attackCooldown--;
    }
}

bool IsEnemyOffScreen(Enemy enemy) {
    return enemy.y > HEIGHT + enemy.radius;
}

bool EnemyTakeDamage(Enemy* enemy, int damage) {
    enemy->health -= damage;
    return enemy->health <= 0;
}

bool EnemyCollidesWithPlayer(Enemy enemy, Player player) {
    float distance = sqrt(pow(enemy.x - player.x, 2) + pow(enemy.y - player.y, 2));
    bool collided = distance < enemy.radius + player.radius;

    if (collided && enemy.attackCooldown == 0) {
        enemy.attackCooldown = 30;
        return true;
    }
    return false;
}

// Функции игры
Game CreateGame() {
    Game game;
    strcpy(game.state, "menu");
    game.level = 1;
    game.maxLevel = 3;
    game.score = 0;
    game.enemiesToDefeat = 10;
    game.enemiesDefeated = 0;
    game.player = CreatePlayer();
    game.bulletCount = 0;
    game.bossBulletCount = 0;
    game.enemyBulletCount = 0;
    game.enemyCount = 0;
    game.bonusCount = 0;
    game.knifeCount = 0;
    game.enemySpawnTimer = 0;
    game.levelCompleteTimer = 0;
    game.bonusSpawnTimer = 0;
    game.bossSpawned = false;
    game.bossDefeated = false;

    game.newGameButton = CreateButton(WIDTH / 2, 250, 200, 50, "NEW GAME", BLUE, DARKBLUE);
    game.continueButton = CreateButton(WIDTH / 2, 320, 200, 50, "CONTINUE", BLUE, DARKBLUE);
    game.quitButton = CreateButton(WIDTH / 2, 390, 200, 50, "QUIT", BLUE, DARKBLUE);

    return game;
}

void SpawnEnemy(Game* game) {
    if (game->enemyCount < MAX_ENEMIES) {
        // НА 3 УРОВНЕ СПАВНИМ ТОЛЬКО БОССА
        if (game->level == 3) {
            if (!game->bossSpawned) {
                game->enemies[game->enemyCount] = CreateEnemy(3, false, true, false, false, false, game->player);
                game->enemyCount++;
                game->bossSpawned = true;
                printf("BOSS SPAWNED!\n");
            }
            return;
        }

        // Шансы спавна разных типов врагов в зависимости от уровня
        int spawnType = GetRandomValue(0, 100);

        if (game->level == 1) {
            // На 1 уровне только обычные враги
            if (spawnType < 70) {
                game->enemies[game->enemyCount] = CreateEnemy(1, false, false, false, false, false, game->player);
            }
            else if (spawnType < 85) {
                game->enemies[game->enemyCount] = CreateEnemy(1, false, false, false, false, true, game->player); // Бегун
            }
            else {
                game->enemies[game->enemyCount] = CreateEnemy(1, false, false, true, false, false, game->player); // Стрелок
            }
        }
        else if (game->level == 2) {
            // На 2 уровне появляются все типы
            if (spawnType < 40) {
                game->enemies[game->enemyCount] = CreateEnemy(2, false, false, false, false, false, game->player);
            }
            else if (spawnType < 60) {
                game->enemies[game->enemyCount] = CreateEnemy(2, false, false, false, false, true, game->player); // Бегун
            }
            else if (spawnType < 75) {
                game->enemies[game->enemyCount] = CreateEnemy(2, false, false, true, false, false, game->player); // Стрелок
            }
            else if (spawnType < 85) {
                game->enemies[game->enemyCount] = CreateEnemy(2, false, false, false, true, false, game->player); // Танк
            }
            else if (spawnType < 92) {
                game->enemies[game->enemyCount] = CreateEnemy(2, true, false, false, false, false, game->player); // Элитный
            }
            else {
                game->enemies[game->enemyCount] = CreateEnemy(3, false, false, false, false, false, game->player); // Сильный обычный
            }
        }

        game->enemyCount++;
    }
}

void SpawnHatBonus(Game* game, float x, float y) {
    if (game->bonusCount < MAX_BONUSES) {
        const char* bonusType = (GetRandomValue(0, 100) < 50) ? "damage" : "knife";
        game->bonuses[game->bonusCount] = CreateHatBonus(x, y, bonusType);
        game->bonusCount++;
    }
}

void StartNewGame(Game* game) {
    strcpy(game->state, "playing");
    game->level = 1;
    game->score = 0;
    game->enemiesDefeated = 0;
    game->enemiesToDefeat = 10;
    game->player = CreatePlayer();
    game->bulletCount = 0;
    game->bossBulletCount = 0;
    game->enemyBulletCount = 0;
    game->enemyCount = 0;
    game->bonusCount = 0;
    game->knifeCount = 0;
    game->bossSpawned = false;
    game->bossDefeated = false;
}

void StartNextLevel(Game* game) {
    if (game->level < game->maxLevel) {
        game->level++;
        game->enemiesDefeated = 0;
        game->bulletCount = 0;
        game->bossBulletCount = 0;
        game->enemyBulletCount = 0;
        game->enemyCount = 0;
        game->bonusCount = 0;
        game->knifeCount = 0;
        game->enemiesToDefeat = (game->level == 3) ? 1 : 15;
        game->bossSpawned = false;
        game->bossDefeated = false;
    }
    else {
        strcpy(game->state, "victory");
    }
}

void UpdateGame(Game* game) {
    if (strcmp(game->state, "playing") == 0) {
        MovePlayer(&game->player);
        UpdatePlayer(&game->player);

        // АКТИВАЦИЯ НОЖЕЙ ПРАВОЙ КНОПКОЙ МЫШИ
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && game->player.hasKnifeBonus) {
            CreatePlayerKnives(game->player, game->knives, &game->knifeCount);
            game->player.hasKnifeBonus = false;
        }

        // НЕПРЕРЫВНАЯ СТРЕЛЬБА ПРИ ЗАЖАТОЙ ЛКМ
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            if (game->player.shootCooldown <= 0 && game->bulletCount < MAX_BULLETS) {
                Vector2 mousePos = GetMousePosition();
                game->bullets[game->bulletCount] = CreateBullet(
                    game->player.x, game->player.y,
                    mousePos.x, mousePos.y,
                    game->level, game->player.damageMultiplier
                );
                game->bulletCount++;
                game->player.shootCooldown = 10;
            }
        }

        if (!IsPlayerAlive(game->player)) {
            strcpy(game->state, "game_over");
            return;
        }

        // ГАРАНТИРОВАННЫЙ СПАВН БОССА НА 3 УРОВНЕ
        if (game->level == 3 && !game->bossSpawned) {
            SpawnEnemy(game);
        }

        // НА 3 УРОВНЕ НЕ СПАВНИМ ОБЫЧНЫХ ВРАГОВ
        if (game->level < 3) {
            game->enemySpawnTimer++;
            int spawnRate = 70 - game->level * 10;
            if (spawnRate < 30) spawnRate = 30;

            if (game->enemySpawnTimer >= spawnRate) {
                SpawnEnemy(game);
                game->enemySpawnTimer = 0;
            }
        }

        game->bonusSpawnTimer++;
        if (game->bonusSpawnTimer >= 450 && GetRandomValue(0, 100) < 15) {
            SpawnHatBonus(game, GetRandomValue(50, WIDTH - 50), -50);
            game->bonusSpawnTimer = 0;
        }

        // Обновление пуль игрока
        for (int i = 0; i < game->bulletCount; i++) {
            UpdateBullet(&game->bullets[i]);
            if (IsBulletOffScreen(game->bullets[i])) {
                for (int j = i; j < game->bulletCount - 1; j++) {
                    game->bullets[j] = game->bullets[j + 1];
                }
                game->bulletCount--;
                i--;
            }
        }

        // Обновление пуль босса
        for (int i = 0; i < game->bossBulletCount; i++) {
            UpdateBossBullet(&game->bossBullets[i]);
            if (IsBossBulletOffScreen(game->bossBullets[i])) {
                for (int j = i; j < game->bossBulletCount - 1; j++) {
                    game->bossBullets[j] = game->bossBullets[j + 1];
                }
                game->bossBulletCount--;
                i--;
            }
        }

        // Обновление пуль врагов
        for (int i = 0; i < game->enemyBulletCount; i++) {
            UpdateEnemyBullet(&game->enemyBullets[i]);
            if (IsEnemyBulletOffScreen(game->enemyBullets[i])) {
                for (int j = i; j < game->enemyBulletCount - 1; j++) {
                    game->enemyBullets[j] = game->enemyBullets[j + 1];
                }
                game->enemyBulletCount--;
                i--;
            }
        }

        // Обновление ножей
        for (int i = 0; i < game->knifeCount; i++) {
            UpdateKnife(&game->knives[i]);
            if (IsKnifeOffScreen(game->knives[i])) {
                for (int j = i; j < game->knifeCount - 1; j++) {
                    game->knives[j] = game->knives[j + 1];
                }
                game->knifeCount--;
                i--;
            }
        }

        // ПРОВЕРКА СТОЛКНОВЕНИЙ ПУЛЬ С ВРАГАМИ
        for (int i = 0; i < game->enemyCount; i++) {
            UpdateEnemy(&game->enemies[i], game->player, game->bossBullets, &game->bossBulletCount, game->enemyBullets, &game->enemyBulletCount);

            if (IsEnemyOffScreen(game->enemies[i])) {
                for (int j = i; j < game->enemyCount - 1; j++) {
                    game->enemies[j] = game->enemies[j + 1];
                }
                game->enemyCount--;
                i--;
                continue;
            }

            if (EnemyCollidesWithPlayer(game->enemies[i], game->player)) {
                PlayerTakeDamage(&game->player, game->enemies[i].damage);
            }

            for (int j = 0; j < game->bulletCount; j++) {
                float distance = sqrt(pow(game->bullets[j].x - game->enemies[i].x, 2) + pow(game->bullets[j].y - game->enemies[i].y, 2));
                if (distance < game->bullets[j].radius + game->enemies[i].radius) {
                    if (EnemyTakeDamage(&game->enemies[i], game->bullets[j].damage)) {
                        game->score += game->enemies[i].scoreValue;
                        game->enemiesDefeated++;

                        // ПРОВЕРКА ПОБЕДЫ НАД БОССОМ
                        if (game->level == 3 && game->enemies[i].isBoss) {
                            game->bossDefeated = true;
                            strcpy(game->state, "victory");
                            return;
                        }

                        if (GetRandomValue(0, 100) < 10) {
                            SpawnHatBonus(game, game->enemies[i].x, game->enemies[i].y);
                        }

                        for (int k = i; k < game->enemyCount - 1; k++) {
                            game->enemies[k] = game->enemies[k + 1];
                        }
                        game->enemyCount--;
                        i--;
                    }

                    for (int k = j; k < game->bulletCount - 1; k++) {
                        game->bullets[k] = game->bullets[k + 1];
                    }
                    game->bulletCount--;
                    j--;
                    break;
                }
            }
        }

        // ПРОВЕРКА СТОЛКНОВЕНИЙ НОЖЕЙ С ВРАГАМИ
        for (int i = 0; i < game->knifeCount; i++) {
            for (int j = 0; j < game->enemyCount; j++) {
                float distance = sqrt(pow(game->knives[i].x - game->enemies[j].x, 2) + pow(game->knives[i].y - game->enemies[j].y, 2));
                if (distance < game->knives[i].radius + game->enemies[j].radius) {
                    if (EnemyTakeDamage(&game->enemies[j], 3)) {
                        game->score += game->enemies[j].scoreValue;
                        game->enemiesDefeated++;

                        // ПРОВЕРКА ПОБЕДЫ НАД БОССОМ
                        if (game->level == 3 && game->enemies[j].isBoss) {
                            game->bossDefeated = true;
                            strcpy(game->state, "victory");
                            return;
                        }

                        if (GetRandomValue(0, 100) < 15) {
                            SpawnHatBonus(game, game->enemies[j].x, game->enemies[j].y);
                        }

                        for (int k = j; k < game->enemyCount - 1; k++) {
                            game->enemies[k] = game->enemies[k + 1];
                        }
                        game->enemyCount--;
                        j--;
                    }

                    for (int k = i; k < game->knifeCount - 1; k++) {
                        game->knives[k] = game->knives[k + 1];
                    }
                    game->knifeCount--;
                    i--;
                    break;
                }
            }
        }

        // ПРОВЕРКА СТОЛКНОВЕНИЙ ПУЛЬ БОССА С ИГРОКОМ
        for (int i = 0; i < game->bossBulletCount; i++) {
            if (BossBulletCollidesWithPlayer(game->bossBullets[i], game->player)) {
                PlayerTakeDamage(&game->player, game->bossBullets[i].damage);
                for (int j = i; j < game->bossBulletCount - 1; j++) {
                    game->bossBullets[j] = game->bossBullets[j + 1];
                }
                game->bossBulletCount--;
                i--;
            }
        }

        // ПРОВЕРКА СТОЛКНОВЕНИЙ ПУЛЬ ВРАГОВ С ИГРОКОМ
        for (int i = 0; i < game->enemyBulletCount; i++) {
            if (EnemyBulletCollidesWithPlayer(game->enemyBullets[i], game->player)) {
                PlayerTakeDamage(&game->player, game->enemyBullets[i].damage);
                for (int j = i; j < game->enemyBulletCount - 1; j++) {
                    game->enemyBullets[j] = game->enemyBullets[j + 1];
                }
                game->enemyBulletCount--;
                i--;
            }
        }

        for (int i = 0; i < game->bonusCount; i++) {
            UpdateHatBonus(&game->bonuses[i]);
            if (ShouldRemoveBonus(game->bonuses[i])) {
                for (int j = i; j < game->bonusCount - 1; j++) {
                    game->bonuses[j] = game->bonuses[j + 1];
                }
                game->bonusCount--;
                i--;
            }
            else if (BonusCollidesWithPlayer(game->bonuses[i], game->player)) {
                if (strcmp(game->bonuses[i].bonusType, "damage") == 0) {
                    AddDamageBonus(&game->player);
                }
                else {
                    AddKnifeBonus(&game->player);
                }

                for (int j = i; j < game->bonusCount - 1; j++) {
                    game->bonuses[j] = game->bonuses[j + 1];
                }
                game->bonusCount--;
                i--;
            }
        }

        // ПРОВЕРКА ЗАВЕРШЕНИЯ УРОВНЯ ДЛЯ УРОВНЕЙ 1-2
        if (game->level < 3 && game->enemiesDefeated >= game->enemiesToDefeat) {
            strcpy(game->state, "level_complete");
            game->levelCompleteTimer = 0;
        }

    }
    else if (strcmp(game->state, "level_complete") == 0) {
        game->levelCompleteTimer++;
        if (game->levelCompleteTimer > 180) {
            StartNextLevel(game);
            strcpy(game->state, "playing");
        }
    }
}

void DrawGame(Game game) {
    ClearBackground(SKYBLUE);

    if (strcmp(game.state, "menu") == 0) {
        DrawText("CRAZY HATMAN", WIDTH / 2 - 180, 100, 50, DARKBLUE);
        DrawText("Controls:", WIDTH / 2 - 50, 160, 24, DARKBLUE);
        DrawText("Arrows - Move", WIDTH / 2 - 70, 190, 20, DARKBLUE);
        DrawText("LMB (Hold) - Auto Shoot", WIDTH / 2 - 100, 215, 20, DARKBLUE);
        DrawText("RMB - Knives (if bonus)", WIDTH / 2 - 100, 240, 20, DARKBLUE);

        UpdateButton(&game.newGameButton);
        UpdateButton(&game.continueButton);
        UpdateButton(&game.quitButton);

        DrawButton(game.newGameButton);
        DrawButton(game.continueButton);
        DrawButton(game.quitButton);

    }
    else if (strcmp(game.state, "playing") == 0) {
        DrawPlayer(game.player);

        for (int i = 0; i < game.bulletCount; i++) {
            DrawBullet(game.bullets[i]);
        }

        for (int i = 0; i < game.bossBulletCount; i++) {
            DrawBossBullet(game.bossBullets[i]);
        }

        for (int i = 0; i < game.enemyBulletCount; i++) {
            DrawEnemyBullet(game.enemyBullets[i]);
        }

        for (int i = 0; i < game.enemyCount; i++) {
            DrawEnemy(game.enemies[i]);
        }

        for (int i = 0; i < game.bonusCount; i++) {
            DrawHatBonus(game.bonuses[i]);
        }

        for (int i = 0; i < game.knifeCount; i++) {
            DrawKnife(game.knives[i]);
        }

        char scoreText[50];
        sprintf(scoreText, "Score: %d", game.score);
        DrawText(scoreText, 10, 10, 24, DARKBLUE);

        char levelText[50];
        sprintf(levelText, "Level: %d", game.level);
        DrawText(levelText, 10, 40, 24, DARKBLUE);

        char enemiesText[50];
        if (game.level == 3) {
            if (game.bossSpawned) {
                sprintf(enemiesText, "BOSS: ALIVE");
            }
            else {
                sprintf(enemiesText, "BOSS: SPAWNING...");
            }
        }
        else {
            sprintf(enemiesText, "Enemies: %d/%d", game.enemiesDefeated, game.enemiesToDefeat);
        }
        DrawText(enemiesText, 10, 70, 24, DARKBLUE);

        char healthText[50];
        sprintf(healthText, "Health: %d", game.player.health);
        DrawText(healthText, WIDTH - 200, 10, 24, DARKBLUE);

        if (game.player.hasKnifeBonus) {
            DrawText("KNIVES READY! PRESS RMB", WIDTH / 2 - 150, HEIGHT - 30, 20, RED);
        }
        if (game.player.damageMultiplier > 1) {
            DrawText("DAMAGE x2 ACTIVE!", WIDTH / 2 - 100, HEIGHT - 60, 20, GOLD);
        }

        DrawText("HOLD LMB - AUTO SHOOT", WIDTH - 200, HEIGHT - 30, 18, DARKBLUE);
        DrawText("RMB - KNIVES", WIDTH - 150, HEIGHT - 60, 18, DARKBLUE);

        if (game.level == 3 && game.bossSpawned) {
            for (int i = 0; i < game.enemyCount; i++) {
                if (game.enemies[i].isBoss) {
                    DrawText("FINAL BOSS FIGHT!", WIDTH / 2 - 100, 100, 30, RED);
                    char bossHealth[50];
                    sprintf(bossHealth, "BOSS HP: %d", game.enemies[i].health);
                    DrawText(bossHealth, WIDTH / 2 - 60, 130, 24, RED);

                    const char* patternText = "";
                    switch (game.enemies[i].attackPattern) {
                    case 0: patternText = "WAVE ATTACK"; break;
                    case 1: patternText = "SPIRAL ATTACK"; break;
                    case 2: patternText = "TARGETED ATTACK"; break;
                    }
                    DrawText(patternText, WIDTH / 2 - 80, 160, 20, ORANGE);
                    break;
                }
            }
        }

    }
    else if (strcmp(game.state, "level_complete") == 0) {
        DrawText("LEVEL COMPLETE!", WIDTH / 2 - 180, HEIGHT / 2 - 50, 40, GREEN);

        char levelText[50];
        sprintf(levelText, "Level %d completed!", game.level);
        DrawText(levelText, WIDTH / 2 - 120, HEIGHT / 2, 30, DARKBLUE);

        char scoreText[50];
        sprintf(scoreText, "Score: %d", game.score);
        DrawText(scoreText, WIDTH / 2 - 60, HEIGHT / 2 + 50, 30, DARKBLUE);

    }
    else if (strcmp(game.state, "game_over") == 0) {
        DrawText("GAME OVER", WIDTH / 2 - 150, HEIGHT / 2 - 50, 50, RED);

        char scoreText[50];
        sprintf(scoreText, "Final Score: %d", game.score);
        DrawText(scoreText, WIDTH / 2 - 120, HEIGHT / 2, 30, DARKBLUE);

        DrawText("Press ESC to return to menu", WIDTH / 2 - 180, HEIGHT / 2 + 100, 20, DARKBLUE);

    }
    else if (strcmp(game.state, "victory") == 0) {
        DrawText("VICTORY!", WIDTH / 2 - 100, HEIGHT / 2 - 50, 60, GOLD);

        char scoreText[50];
        sprintf(scoreText, "Final Score: %d", game.score);
        DrawText(scoreText, WIDTH / 2 - 100, HEIGHT / 2 + 30, 30, DARKBLUE);

        DrawText("You defeated the FINAL BOSS!", WIDTH / 2 - 180, HEIGHT / 2 + 80, 30, GREEN);
        DrawText("Press ESC to return to menu", WIDTH / 2 - 180, HEIGHT / 2 + 120, 20, DARKBLUE);
    }
}

int main() {
    InitWindow(WIDTH, HEIGHT, "Crazy Hatman - Controls: Arrows - Move, Hold LMB - Auto Shoot, RMB - Knives");
    SetTargetFPS(60);

    Game game = CreateGame();

    while (!WindowShouldClose()) {
        if (strcmp(game.state, "menu") == 0) {
            UpdateButton(&game.newGameButton);
            UpdateButton(&game.continueButton);
            UpdateButton(&game.quitButton);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (IsButtonHovered(game.newGameButton)) {
                    StartNewGame(&game);
                }
                else if (IsButtonHovered(game.quitButton)) {
                    break;
                }
            }
        }
        else if (strcmp(game.state, "playing") == 0) {
            if (IsKeyPressed(KEY_ESCAPE)) {
                strcpy(game.state, "menu");
            }
        }
        else if (strcmp(game.state, "game_over") == 0 || strcmp(game.state, "victory") == 0) {
            if (IsKeyPressed(KEY_ESCAPE)) {
                strcpy(game.state, "menu");
            }
        }

        UpdateGame(&game);

        BeginDrawing();
        DrawGame(game);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}