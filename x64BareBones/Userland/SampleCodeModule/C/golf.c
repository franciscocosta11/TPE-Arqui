#include "./../include/libc.h"

// Constantes del juego
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define BALL_SIZE 8
#define PADDLE_WIDTH 15
#define PADDLE_HEIGHT 60
#define MAX_SPEED 8

// Colores
#define COLOR_GREEN   0x00AA00
#define COLOR_WHITE   0xFFFFFF
#define COLOR_GRAY    0x808080
#define COLOR_BLACK   0x000000
#define COLOR_RED     0xFF0000
#define COLOR_YELLOW  0xFFFF00
#define COLOR_BLUE    0x0000FF
#define COLOR_CYAN    0x00FFFF
#define COLOR_MAGENTA 0xFF00FF

// Estados del juego
#define GAME_MENU 0
#define GAME_PLAYING 1
#define GAME_LEVEL_COMPLETE 2
#define GAME_OVER 3

// Modos de juego
#define MODE_SINGLE 1
#define MODE_MULTIPLAYER 2

// Estructura para la pelota
typedef struct {
    int x, y;      
    int vx, vy;    
    int size;
} Ball;

// Estructura para el palo
typedef struct {
    int x, y;
    int width, height;
    uint32_t color;
    int aim_angle;
} Paddle;

// Estructura para el hoyo
typedef struct {
    int x, y;
    int size;
} Hole;

// Variables globales del juego
static Ball ball;
static Paddle paddle1, paddle2;
static Hole hole;
static int hits = 0;
static int gameRunning = 1;
static int gameState = GAME_MENU;
static int gameMode = MODE_SINGLE;
static int currentLevel = 1;
static int maxLevels = 3;
static int currentPlayer = 1; // Para modo multijugador

// Variables para generador de números aleatorios
static unsigned int random_seed = 1;

// Tablas trigonométricas
static int cos_table[24] = {
    100, 97, 87, 71, 50, 26, 0, -26, -50, -71, -87, -97,
    -100, -97, -87, -71, -50, -26, 0, 26, 50, 71, 87, 97
};

static int sin_table[24] = {
    0, -26, -50, -71, -87, -97, -100, -97, -87, -71, -50, -26,
    0, 26, 50, 71, 87, 97, 100, 97, 87, 71, 50, 26
};

// Declaraciones de funciones
void showMenu(void);
void initGame(void);
void drawGame(void);
void updateGame(void);
void handleInput(void);
void drawRect(int x, int y, int width, int height, uint32_t color);
void drawCircle(int centerX, int centerY, int radius, uint32_t color);
void drawNumber(int number, int x, int y);
void resetBall(void);
void drawLine(int x1, int y1, int x2, int y2, uint32_t color);
void drawAimArrow(Paddle* paddle);
void placeHoleRandomly(void);
void showHoleMessage(void);
void showLevelComplete(void);
void drawUI(void);
int simpleRandom(int min, int max);
int getHoleSize(int level);

void startGolfGame(void) {
    clearScreen();
    gameState = GAME_MENU;
    gameRunning = 1;
    
    while (gameRunning) {
        switch (gameState) {
            case GAME_MENU:
                showMenu();
                break;
            case GAME_PLAYING:
                handleInput();
                updateGame();
                drawGame();
                break;
            case GAME_LEVEL_COMPLETE:
                showLevelComplete();
                break;
            case GAME_OVER:
                gameRunning = 0;
                break;
        }
        
        for (volatile int i = 0; i < 50000; i++); 
    }
}

void showMenu(void) {
    static int menuInitialized = 0;
    
    if (!menuInitialized) {
        fillScreen(COLOR_BLACK);
        
        // Título
        drawRect(200, 100, 624, 80, COLOR_BLUE);
        drawRect(210, 110, 604, 60, COLOR_WHITE);
        
        // Dibujar "PONGIS-GOLF" en el título
        print("PONGIS-GOLF\n");
        
        // Instrucciones
        drawRect(150, 250, 724, 200, COLOR_GREEN);
        drawRect(160, 260, 704, 180, COLOR_WHITE);
        
        menuInitialized = 1;
    }
    
    // Leer input del menú
    char key = getKeyNonBlocking();
    
    if (key == '1') {
        gameMode = MODE_SINGLE;
        currentLevel = 1;
        gameState = GAME_PLAYING;
        playBeep();
        initGame();
    } else if (key == '2') {
        gameMode = MODE_MULTIPLAYER;
        currentLevel = 1;
        gameState = GAME_PLAYING;
        playBeep();
        initGame();
    } else if (key == 27) { // ESC
        gameRunning = 0;
    }
}

int simpleRandom(int min, int max) {
    random_seed = random_seed * 1103515245 + 12345;
    return min + (random_seed % (max - min + 1));
}

int getHoleSize(int level) {
    switch (level) {
        case 1: return 30;  // Fácil
        case 2: return 20;  // Medio
        case 3: return 15;  // Difícil
        default: return 15;
    }
}

void placeHoleRandomly(void) {
    hole.size = getHoleSize(currentLevel);
    int margin = hole.size + 30;
    
    do {
        hole.x = simpleRandom(margin, SCREEN_WIDTH - margin);
        hole.y = simpleRandom(margin, SCREEN_HEIGHT - margin);
        
        int dx = hole.x - ball.x;
        int dy = hole.y - ball.y;
        int distance_sq = dx*dx + dy*dy;
        int min_distance = 200;
        
        if (distance_sq > min_distance * min_distance) {
            break;
        }
    } while (1);
}

void initGame(void) {
    fillScreen(COLOR_GREEN);
    
    random_seed = 12345 + currentLevel * 1000;
    
    // Pelota en el centro
    ball.x = SCREEN_WIDTH / 2;
    ball.y = SCREEN_HEIGHT / 2;
    ball.vx = 0;
    ball.vy = 0;
    ball.size = BALL_SIZE;
    
    // Paddle 1 (Jugador 1)
    paddle1.x = 50;
    paddle1.y = SCREEN_HEIGHT / 2 - (BALL_SIZE * 3);
    paddle1.width = BALL_SIZE * 6;
    paddle1.height = BALL_SIZE * 6;
    paddle1.color = COLOR_GRAY;
    paddle1.aim_angle = 0;
    
    // Paddle 2 (Jugador 2) - solo en modo multijugador
    if (gameMode == MODE_MULTIPLAYER) {
        paddle2.x = SCREEN_WIDTH - 100;
        paddle2.y = SCREEN_HEIGHT / 2 - (BALL_SIZE * 3);
        paddle2.width = BALL_SIZE * 6;
        paddle2.height = BALL_SIZE * 6;
        paddle2.color = COLOR_CYAN;
        paddle2.aim_angle = 180; // Apuntando hacia la izquierda
        currentPlayer = 1;
    }
    
    placeHoleRandomly();
    
    hits = 0;
}

void drawGame(void) {
    static int last_ball_x = -1, last_ball_y = -1;
    static int last_paddle1_x = -1, last_paddle1_y = -1;
    static int last_paddle2_x = -1, last_paddle2_y = -1;
    static int last_hole_x = -1, last_hole_y = -1;
    static int last_angle1 = -1, last_angle2 = -1;
    static int initialized = 0;
    
    if (!initialized) {
        fillScreen(COLOR_GREEN);
        initialized = 1;
    }
    
    // Limpiar posiciones anteriores
    if (last_hole_x != -1 && (last_hole_x != hole.x || last_hole_y != hole.y)) {
        drawCircle(last_hole_x, last_hole_y, getHoleSize(currentLevel) + 2, COLOR_GREEN);
    }
    
    if (last_ball_x != -1 && (last_ball_x != ball.x || last_ball_y != ball.y)) {
        drawCircle(last_ball_x, last_ball_y, ball.size + 2, COLOR_GREEN);
    }
    
    if (last_paddle1_x != -1 && (last_paddle1_x != paddle1.x || last_paddle1_y != paddle1.y || last_angle1 != paddle1.aim_angle)) {
        int old_center_x = last_paddle1_x + paddle1.width/2;
        int old_center_y = last_paddle1_y + paddle1.height/2;
        drawRect(old_center_x - 40, old_center_y - 40, 80, 80, COLOR_GREEN);
    }
    
    if (gameMode == MODE_MULTIPLAYER && last_paddle2_x != -1 && 
        (last_paddle2_x != paddle2.x || last_paddle2_y != paddle2.y || last_angle2 != paddle2.aim_angle)) {
        int old_center_x = last_paddle2_x + paddle2.width/2;
        int old_center_y = last_paddle2_y + paddle2.height/2;
        drawRect(old_center_x - 40, old_center_y - 40, 80, 80, COLOR_GREEN);
    }
    
    // Dibujar elementos del juego
    drawCircle(hole.x, hole.y, hole.size, COLOR_BLACK);
    drawCircle(ball.x, ball.y, ball.size, COLOR_WHITE);
    
    // Dibujar paddle 1
    int paddle1_center_x = paddle1.x + paddle1.width/2;
    int paddle1_center_y = paddle1.y + paddle1.height/2;
    uint32_t p1_color = (gameMode == MODE_SINGLE || currentPlayer == 1) ? paddle1.color : COLOR_GRAY;
    drawCircle(paddle1_center_x, paddle1_center_y, paddle1.width/2, p1_color);
    
    if (gameMode == MODE_SINGLE || currentPlayer == 1) {
        drawAimArrow(&paddle1);
    }
    
    // Dibujar paddle 2 (solo en multijugador)
    if (gameMode == MODE_MULTIPLAYER) {
        int paddle2_center_x = paddle2.x + paddle2.width/2;
        int paddle2_center_y = paddle2.y + paddle2.height/2;
        uint32_t p2_color = (currentPlayer == 2) ? paddle2.color : COLOR_GRAY;
        drawCircle(paddle2_center_x, paddle2_center_y, paddle2.width/2, p2_color);
        
        if (currentPlayer == 2) {
            drawAimArrow(&paddle2);
        }
    }
    
    drawUI();
    
    // Actualizar posiciones
    last_ball_x = ball.x;
    last_ball_y = ball.y;
    last_paddle1_x = paddle1.x;
    last_paddle1_y = paddle1.y;
    last_angle1 = paddle1.aim_angle;
    
    if (gameMode == MODE_MULTIPLAYER) {
        last_paddle2_x = paddle2.x;
        last_paddle2_y = paddle2.y;
        last_angle2 = paddle2.aim_angle;
    }
    
    last_hole_x = hole.x;
    last_hole_y = hole.y;
}

void drawUI(void) {
    // Limpiar área de UI
    drawRect(0, 0, SCREEN_WIDTH, 40, COLOR_YELLOW);
    
    // Mostrar nivel
    drawRect(10, 5, 120, 30, COLOR_BLACK);
    drawRect(15, 10, 110, 20, COLOR_WHITE);
    drawNumber(currentLevel, 80, 15);
    
    // Mostrar golpes
    drawRect(SCREEN_WIDTH - 150, 5, 140, 30, COLOR_BLACK);
    drawRect(SCREEN_WIDTH - 145, 10, 130, 20, COLOR_WHITE);
    drawNumber(hits, SCREEN_WIDTH - 100, 15);
    
    // En modo multijugador, mostrar jugador actual
    if (gameMode == MODE_MULTIPLAYER) {
        drawRect(SCREEN_WIDTH/2 - 60, 5, 120, 30, COLOR_BLACK);
        drawRect(SCREEN_WIDTH/2 - 55, 10, 110, 20, COLOR_WHITE);
        drawNumber(currentPlayer, SCREEN_WIDTH/2, 15);
    }
}

void handleInput(void) {
    char key = getKeyNonBlocking();
    Paddle* activePaddle = (gameMode == MODE_SINGLE || currentPlayer == 1) ? &paddle1 : &paddle2;
    
    switch (key) {
        case 27: // ESC
            gameState = GAME_MENU;
            return;
            
        // Controles Jugador 1 (Flechas)
        case 75: // Flecha IZQUIERDA
            if (gameMode == MODE_SINGLE || currentPlayer == 1) {
                paddle1.aim_angle = (paddle1.aim_angle - 15 + 360) % 360;
            }
            break;
            
        case 77: // Flecha DERECHA
            if (gameMode == MODE_SINGLE || currentPlayer == 1) {
                paddle1.aim_angle = (paddle1.aim_angle + 15) % 360;
            }
            break;
            
        case 72: // Flecha ARRIBA
            if (gameMode == MODE_SINGLE || currentPlayer == 1) {
                int angle_x = cos_table[paddle1.aim_angle / 15];
                int angle_y = sin_table[paddle1.aim_angle / 15];
                
                int new_x = paddle1.x + (angle_x * 8) / 100;
                int new_y = paddle1.y + (angle_y * 8) / 100;
                
                if (new_x >= 0 && new_x + paddle1.width <= SCREEN_WIDTH &&
                    new_y >= 0 && new_y + paddle1.height <= SCREEN_HEIGHT) {
                    paddle1.x = new_x;
                    paddle1.y = new_y;
                }
            }
            break;
            
        // Controles Jugador 2 (WASD) - solo en multijugador
        case 'a': case 'A': // A - rotar izquierda
            if (gameMode == MODE_MULTIPLAYER && currentPlayer == 2) {
                paddle2.aim_angle = (paddle2.aim_angle - 15 + 360) % 360;
            }
            break;
            
        case 'd': case 'D': // D - rotar derecha
            if (gameMode == MODE_MULTIPLAYER && currentPlayer == 2) {
                paddle2.aim_angle = (paddle2.aim_angle + 15) % 360;
            }
            break;
            
        case 'w': case 'W': // W - mover hacia donde apunta
            if (gameMode == MODE_MULTIPLAYER && currentPlayer == 2) {
                int angle_x = cos_table[paddle2.aim_angle / 15];
                int angle_y = sin_table[paddle2.aim_angle / 15];
                
                int new_x = paddle2.x + (angle_x * 8) / 100;
                int new_y = paddle2.y + (angle_y * 8) / 100;
                
                if (new_x >= 0 && new_x + paddle2.width <= SCREEN_WIDTH &&
                    new_y >= 0 && new_y + paddle2.height <= SCREEN_HEIGHT) {
                    paddle2.x = new_x;
                    paddle2.y = new_y;
                }
            }
            break;
            
        // Cambiar de jugador en multijugador (Barra espaciadora)
        case ' ':
            if (gameMode == MODE_MULTIPLAYER) {
                currentPlayer = (currentPlayer == 1) ? 2 : 1;
                playBeep();
            }
            break;
    }
}

void updateGame(void) {
    static int last_paddle1_x = -1, last_paddle1_y = -1;
    static int last_paddle2_x = -1, last_paddle2_y = -1;
    static int collision_cooldown = 0;
    static int was_far = 1;
    
    if (collision_cooldown > 0) {
        collision_cooldown--;
    }
    
    // Comprobar colisión con paddle 1
    int paddle1_center_x = paddle1.x + paddle1.width/2;
    int paddle1_center_y = paddle1.y + paddle1.height/2;
    int dx1 = ball.x - paddle1_center_x;
    int dy1 = ball.y - paddle1_center_y;
    int distance1_sq = dx1*dx1 + dy1*dy1;
    int collision_radius = (ball.size + paddle1.width/2 - 8);
    
    // Comprobar colisión con paddle 2 (si existe)
    int distance2_sq = 999999;
    int dx2 = 0, dy2 = 0;
    if (gameMode == MODE_MULTIPLAYER) {
        int paddle2_center_x = paddle2.x + paddle2.width/2;
        int paddle2_center_y = paddle2.y + paddle2.height/2;
        dx2 = ball.x - paddle2_center_x;
        dy2 = ball.y - paddle2_center_y;
        distance2_sq = dx2*dx2 + dy2*dy2;
    }
    
    int far_distance = (collision_radius + 25) * (collision_radius + 25);
    if (distance1_sq > far_distance && distance2_sq > far_distance) {
        was_far = 1;
    }
    
    // Calcular velocidades de paddles
    int paddle1_speed = 0, paddle2_speed = 0;
    if (last_paddle1_x != -1) {
        int dx = paddle1.x - last_paddle1_x;
        int dy = paddle1.y - last_paddle1_y;
        paddle1_speed = isqrt(dx*dx + dy*dy);
    }
    if (gameMode == MODE_MULTIPLAYER && last_paddle2_x != -1) {
        int dx = paddle2.x - last_paddle2_x;
        int dy = paddle2.y - last_paddle2_y;
        paddle2_speed = isqrt(dx*dx + dy*dy);
    }
    
    // Verificar colisión con paddle 1
    if (distance1_sq < (collision_radius * collision_radius) && 
        collision_cooldown == 0 && was_far == 1 && paddle1_speed > 0) {
        
        int dist = isqrt(distance1_sq);
        if (dist > 1) {
            int base_impact = 150;
            int speed_multiplier = paddle1_speed * 80;
            int final_speed = base_impact + speed_multiplier;
            
            if (final_speed < 150) final_speed = 150;
            if (final_speed > 1200) final_speed = 1200;
            
            ball.vx = (dx1 * final_speed) / dist;
            ball.vy = (dy1 * final_speed) / dist;
            
            hits++;
            
            int separation = collision_radius + 15;
            ball.x = paddle1_center_x + (dx1 * separation) / dist;
            ball.y = paddle1_center_y + (dy1 * separation) / dist;
            
            collision_cooldown = 30;
            was_far = 0;
        }
    }
    
    // Verificar colisión con paddle 2 (modo multijugador)
    if (gameMode == MODE_MULTIPLAYER && 
        distance2_sq < (collision_radius * collision_radius) && 
        collision_cooldown == 0 && was_far == 1 && paddle2_speed > 0) {
        
        int dist = isqrt(distance2_sq);
        if (dist > 1) {
            int base_impact = 150;
            int speed_multiplier = paddle2_speed * 80;
            int final_speed = base_impact + speed_multiplier;
            
            if (final_speed < 150) final_speed = 150;
            if (final_speed > 1200) final_speed = 1200;
            
            ball.vx = (dx2 * final_speed) / dist;
            ball.vy = (dy2 * final_speed) / dist;
            
            hits++;
            
            int paddle2_center_x = paddle2.x + paddle2.width/2;
            int paddle2_center_y = paddle2.y + paddle2.height/2;
            int separation = collision_radius + 15;
            ball.x = paddle2_center_x + (dx2 * separation) / dist;
            ball.y = paddle2_center_y + (dy2 * separation) / dist;
            
            collision_cooldown = 30;
            was_far = 0;
        }
    }
    
    last_paddle1_x = paddle1.x;
    last_paddle1_y = paddle1.y;
    if (gameMode == MODE_MULTIPLAYER) {
        last_paddle2_x = paddle2.x;
        last_paddle2_y = paddle2.y;
    }
    
    // Física de la pelota
    ball.x += ball.vx / 100;
    ball.y += ball.vy / 100;
    
    ball.vx = (ball.vx * 995) / 1000;
    ball.vy = (ball.vy * 995) / 1000;
    
    if (abs(ball.vx) < 5 && abs(ball.vy) < 5) {
        ball.vx = 0;
        ball.vy = 0;
    }
    
    // Rebotes en paredes
    if (ball.x - ball.size <= 0) {
        ball.x = ball.size;
        ball.vx = -ball.vx * 80 / 100;
    }
    if (ball.x + ball.size >= SCREEN_WIDTH) {
        ball.x = SCREEN_WIDTH - ball.size;
        ball.vx = -ball.vx * 80 / 100;
    }
    if (ball.y - ball.size <= 40) { // Dejar espacio para UI
        ball.y = 40 + ball.size;
        ball.vy = -ball.vy * 80 / 100;
    }
    if (ball.y + ball.size >= SCREEN_HEIGHT) {
        ball.y = SCREEN_HEIGHT - ball.size;
        ball.vy = -ball.vy * 80 / 100;
    }
    
    // Verificar si la pelota entró en el hoyo
    int dx = ball.x - hole.x;
    int dy = ball.y - hole.y;
    int distance_sq = dx*dx + dy*dy;
    int min_distance_sq = (hole.size - ball.size) * (hole.size - ball.size);
    
    if (distance_sq < min_distance_sq) {
        if (currentLevel < maxLevels) {
            currentLevel++;
            gameState = GAME_LEVEL_COMPLETE;
        } else {
            showHoleMessage();
            gameState = GAME_MENU;
        }
    }
}

void showLevelComplete(void) {
    playWinSound();
    
    fillScreen(COLOR_YELLOW);
    
    drawRect(200, 300, 600, 150, COLOR_BLUE);
    drawRect(210, 310, 580, 130, COLOR_WHITE);
    
    // Mostrar "NIVEL COMPLETO"
    drawRect(300, 350, 400, 50, COLOR_BLACK);
    
    // Mostrar número de golpes
    drawNumber(hits, 450, 280);
    
    for (volatile int i = 0; i < 10000000; i++);
    
    // Inicializar siguiente nivel
    resetBall();
    placeHoleRandomly();
    hits = 0;
    gameState = GAME_PLAYING;
    fillScreen(COLOR_GREEN);
}

void showHoleMessage(void) {
    playWinSound();
    
    fillScreen(COLOR_YELLOW);
    
    drawRect(200, 300, 600, 150, COLOR_BLUE);
    drawRect(210, 310, 580, 130, COLOR_WHITE);
    
    // Dibujar "¡COMPLETADO!"
    int start_x = 250;
    int start_y = 350;
    drawRect(start_x, start_y, 500, 50, COLOR_BLACK);
    
    drawNumber(hits, 450, 280);
    
    for (volatile int i = 0; i < 15000000; i++);
    
    fillScreen(COLOR_GREEN);
}

void resetBall(void) {
    ball.x = SCREEN_WIDTH / 2;
    ball.y = SCREEN_HEIGHT / 2;
    ball.vx = 0;
    ball.vy = 0;
}

void drawRect(int x, int y, int width, int height, uint32_t color) {
    drawRectangle(color, x, y, x + width, y + height);
}

void drawCircle(int centerX, int centerY, int radius, uint32_t color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                putPixel(color, centerX + x, centerY + y);
            }
        }
    }
}

void drawNumber(int number, int x, int y) {
    char numStr[10];
    int i = 0;
    
    if (number == 0) {
        numStr[0] = '0';
        numStr[1] = '\0';
    } else {
        while (number > 0) {
            numStr[i++] = '0' + (number % 10);
            number /= 10;
        }
        numStr[i] = '\0';
        
        for (int j = 0; j < i/2; j++) {
            char temp = numStr[j];
            numStr[j] = numStr[i-1-j];
            numStr[i-1-j] = temp;
        }
    }
    
    for (int digit = 0; digit < strlen(numStr); digit++) {
        char c = numStr[digit];
        int digit_x = x + (digit * 20);
        
        int patterns[10][15] = {
            {1,1,1,1,0,1,1,0,1,1,0,1,1,1,1}, // 0
            {0,1,0,0,1,0,0,1,0,0,1,0,0,1,0}, // 1
            {1,1,1,0,0,1,1,1,1,1,0,0,1,1,1}, // 2
            {1,1,1,0,0,1,1,1,1,0,0,1,1,1,1}, // 3
            {1,0,1,1,0,1,1,1,1,0,0,1,0,0,1}, // 4
            {1,1,1,1,0,0,1,1,1,0,0,1,1,1,1}, // 5
            {1,1,1,1,0,0,1,1,1,1,0,1,1,1,1}, // 6
            {1,1,1,0,0,1,0,0,1,0,0,1,0,0,1}, // 7
            {1,1,1,1,0,1,1,1,1,1,0,1,1,1,1}, // 8
            {1,1,1,1,0,1,1,1,1,0,0,1,1,1,1}  // 9
        };
        
        int num = c - '0';
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 3; col++) {
                if (patterns[num][row * 3 + col]) {
                    drawRect(digit_x + col * 3, y + row * 3, 2, 2, COLOR_BLACK);
                }
            }
        }
    }
}

void drawAimArrow(Paddle* paddle) {
    int paddle_center_x = paddle->x + paddle->width/2;
    int paddle_center_y = paddle->y + paddle->height/2;
    
    int angle_index = paddle->aim_angle / 15;
    int dir_x = cos_table[angle_index];
    int dir_y = sin_table[angle_index];
    
    int tip_x = paddle_center_x + (dir_x * 25) / 100;
    int tip_y = paddle_center_y + (dir_y * 25) / 100;
    
    for (int t = 0; t < 25; t += 2) {
        int line_x = paddle_center_x + (dir_x * t) / 100;
        int line_y = paddle_center_y + (dir_y * t) / 100;
        
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                putPixel(COLOR_RED, line_x + dx, line_y + dy);
            }
        }
    }
    
    int perp_x = -dir_y;
    int perp_y = dir_x;
    
    int head1_x = tip_x - (dir_x * 8) / 100 + (perp_x * 6) / 100;
    int head1_y = tip_y - (dir_y * 8) / 100 + (perp_y * 6) / 100;
    int head2_x = tip_x - (dir_x * 8) / 100 - (perp_x * 6) / 100;
    int head2_y = tip_y - (dir_y * 8) / 100 - (perp_y * 6) / 100;
    
    drawLine(tip_x, tip_y, head1_x, head1_y, COLOR_RED);
    drawLine(tip_x, tip_y, head2_x, head2_y, COLOR_RED);
}

void drawLine(int x1, int y1, int x2, int y2, uint32_t color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    int x = x1, y = y1;
    
    while (1) {
        putPixel(color, x, y);
        
        if (x == x2 && y == y2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}