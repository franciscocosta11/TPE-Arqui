#include "./../include/libc.h"

// Constantes del juego
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define BALL_SIZE 8
#define PADDLE_WIDTH 15
#define PADDLE_HEIGHT 60
#define HOLE_SIZE 30
#define MAX_SPEED 8

// Colores
#define COLOR_GREEN   0x00AA00
#define COLOR_WHITE   0xFFFFFF
#define COLOR_GRAY    0x808080
#define COLOR_BLACK   0x000000
#define COLOR_RED     0xFF0000
#define COLOR_YELLOW  0xFFFF00
#define COLOR_BLUE    0x0000FF 

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
} Paddle;

// Estructura para el hoyo
typedef struct {
    int x, y;
    int size;
} Hole;

// Variables globales del juego
static Ball ball;
static Paddle paddle;
static Hole hole;
static int hits = 0;
static int gameRunning = 1;

// Variables para generador de números aleatorios simple
static unsigned int random_seed = 1;

// Tablas trigonométricas simplificadas
static int cos_table[24] = {
    100, 97, 87, 71, 50, 26, 0, -26, -50, -71, -87, -97,
    -100, -97, -87, -71, -50, -26, 0, 26, 50, 71, 87, 97
};

static int sin_table[24] = {
    0, -26, -50, -71, -87, -97, -100, -97, -87, -71, -50, -26,
    0, 26, 50, 71, 87, 97, 100, 97, 87, 71, 50, 26
};

static int aim_angle = 0;

// Declaraciones de funciones
void initGame(void);
void drawGame(void);
void updateGame(void);
void handleInput(void);
void drawRect(int x, int y, int width, int height, uint32_t color);
void drawCircle(int centerX, int centerY, int radius, uint32_t color);
void drawNumber(int number, int x, int y);
void resetBall(void);
void drawLine(int x1, int y1, int x2, int y2, uint32_t color);
void drawAimArrow(void);
void placeHoleRandomly(void);
void showHoleMessage(void);
int simpleRandom(int min, int max);

void startGolfGame(void) {
    clearScreen();
    print("========================================\n");
    print("         BIENVENIDO A PONGIS-GOLF       \n");
    print("========================================\n");
    print("\n");
    print("Instrucciones:\n");
    print("- Usa las flechas para mover el palo\n");
    print("- Golpea la pelota para que entre al hoyo\n");
    print("- El hoyo aparece en lugares aleatorios\n");
    print("- ESC para salir del juego\n");
    print("\n");
    print("Presiona ENTER para comenzar...\n");
    
    char key;
    do {
        key = getchar();
    } while (key != '\n' && key != '\r');
    
    initGame();
    
    while (gameRunning) {
        handleInput();
        updateGame();
        drawGame();
        
        for (volatile int i = 0; i < 50000; i++); 
    }
}

int simpleRandom(int min, int max) {
    random_seed = random_seed * 1103515245 + 12345;
    return min + (random_seed % (max - min + 1));
}

void placeHoleRandomly(void) {
    int margin = HOLE_SIZE + 20;
    
    do {
        hole.x = simpleRandom(margin, SCREEN_WIDTH - margin);
        hole.y = simpleRandom(margin, SCREEN_HEIGHT - margin);
        
        int dx = hole.x - ball.x;
        int dy = hole.y - ball.y;
        int distance_sq = dx*dx + dy*dy;
        int min_distance = 150;
        
        if (distance_sq > min_distance * min_distance) {
            break;
        }
    } while (1);
}

void initGame(void) {
    fillScreen(COLOR_GREEN);
    
    random_seed = 12345;
    
    ball.x = SCREEN_WIDTH / 2;
    ball.y = SCREEN_HEIGHT / 2;
    ball.vx = 0;
    ball.vy = 0;
    ball.size = BALL_SIZE;
    
    paddle.x = 50;
    paddle.y = SCREEN_HEIGHT / 2 - (BALL_SIZE * 3);
    paddle.width = BALL_SIZE * 6;
    paddle.height = BALL_SIZE * 6;
    
    hole.size = HOLE_SIZE;
    placeHoleRandomly();
    
    hits = 0;
    gameRunning = 1;
    aim_angle = 0;
}

void drawGame(void) {
    static int last_ball_x = -1, last_ball_y = -1;
    static int last_paddle_x = -1, last_paddle_y = -1;
    static int last_hole_x = -1, last_hole_y = -1;
    static int last_angle = -1;
    static int initialized = 0;
    
    if (!initialized) {
        fillScreen(COLOR_GREEN);
        initialized = 1;
    }
    
    if (last_hole_x != -1 && (last_hole_x != hole.x || last_hole_y != hole.y)) {
        drawCircle(last_hole_x, last_hole_y, hole.size + 2, COLOR_GREEN);
    }
    
    drawCircle(hole.x, hole.y, hole.size, COLOR_BLACK);
    
    if (last_ball_x != -1 && (last_ball_x != ball.x || last_ball_y != ball.y)) {
        drawCircle(last_ball_x, last_ball_y, ball.size + 2, COLOR_GREEN);
    }
    
    if (last_paddle_x != -1 && (last_paddle_x != paddle.x || last_paddle_y != paddle.y || last_angle != aim_angle)) {
        int old_center_x = last_paddle_x + paddle.width/2;
        int old_center_y = last_paddle_y + paddle.height/2;
        drawRect(old_center_x - 40, old_center_y - 40, 80, 80, COLOR_GREEN);
    }
    
    drawCircle(ball.x, ball.y, ball.size, COLOR_WHITE);
    
    int paddle_center_x = paddle.x + paddle.width/2;
    int paddle_center_y = paddle.y + paddle.height/2;
    drawCircle(paddle_center_x, paddle_center_y, paddle.width/2, COLOR_GRAY);
    drawAimArrow();
    
    last_ball_x = ball.x;
    last_ball_y = ball.y;
    last_paddle_x = paddle.x;
    last_paddle_y = paddle.y;
    last_hole_x = hole.x;
    last_hole_y = hole.y;
    last_angle = aim_angle;
    
    static int last_hits = -1;
    if (hits != last_hits) {
        drawRect(SCREEN_WIDTH - 150, 10, 140, 40, COLOR_YELLOW);
        drawNumber(hits, SCREEN_WIDTH - 100, 25);
        last_hits = hits;
    }
}

void handleInput(void) {
    char key = getKeyNonBlocking();
    
    switch (key) {
        case 27: // ESC
            gameRunning = 0;
            break;
            
        case 75: // Flecha IZQUIERDA
            aim_angle = (aim_angle - 15 + 360) % 360;
            break;
            
        case 77: // Flecha DERECHA
            aim_angle = (aim_angle + 15) % 360;
            break;
            
        case 72: // Flecha ARRIBA
            {
                int angle_x = cos_table[aim_angle / 15];
                int angle_y = sin_table[aim_angle / 15];
                
                int new_x = paddle.x + (angle_x * 8) / 100;
                int new_y = paddle.y + (angle_y * 8) / 100;
                
                if (new_x >= 0 && new_x + paddle.width <= SCREEN_WIDTH &&
                    new_y >= 0 && new_y + paddle.height <= SCREEN_HEIGHT) {
                    paddle.x = new_x;
                    paddle.y = new_y;
                }
            }
            break;
    }
}

void updateGame(void) {
    static int last_paddle_x = -1, last_paddle_y = -1;
    static int collision_cooldown = 0;
    static int was_far = 1;
    
    if (collision_cooldown > 0) {
        collision_cooldown--;
    }
    
    int paddle_center_x = paddle.x + paddle.width/2;
    int paddle_center_y = paddle.y + paddle.height/2;
    int dx_collision = ball.x - paddle_center_x;
    int dy_collision = ball.y - paddle_center_y;
    int collision_distance_sq = dx_collision*dx_collision + dy_collision*dy_collision;
    int collision_radius = (ball.size + paddle.width/2 - 8);
    
    int far_distance = (collision_radius + 25) * (collision_radius + 25);
    if (collision_distance_sq > far_distance) {
        was_far = 1;
    }
    
    int paddle_speed = 0;
    if (last_paddle_x != -1) {
        int paddle_dx = paddle.x - last_paddle_x;
        int paddle_dy = paddle.y - last_paddle_y;
        paddle_speed = isqrt(paddle_dx*paddle_dx + paddle_dy*paddle_dy);
    }
    
    if (collision_distance_sq < (collision_radius * collision_radius) && 
        collision_cooldown == 0 &&
        was_far == 1 &&
        paddle_speed > 0) {
        
        int dist = isqrt(collision_distance_sq);
        if (dist > 1) {
            int base_impact = 150;
            int speed_multiplier = paddle_speed * 80;
            int final_speed = base_impact + speed_multiplier;
            
            if (final_speed < 150) final_speed = 150;
            if (final_speed > 1200) final_speed = 1200;
            
            ball.vx = (dx_collision * final_speed) / dist;
            ball.vy = (dy_collision * final_speed) / dist;
            
            hits++;
            // SIN SONIDO AL GOLPEAR - solo incrementar hits
            
            int separation = collision_radius + 15;
            ball.x = paddle_center_x + (dx_collision * separation) / dist;
            ball.y = paddle_center_y + (dy_collision * separation) / dist;
            
            collision_cooldown = 30;
            was_far = 0;
        }
    }
    
    last_paddle_x = paddle.x;
    last_paddle_y = paddle.y;
    
    ball.x += ball.vx / 100;
    ball.y += ball.vy / 100;
    
    ball.vx = (ball.vx * 995) / 1000;
    ball.vy = (ball.vy * 995) / 1000;
    
    if (abs(ball.vx) < 5 && abs(ball.vy) < 5) {
        ball.vx = 0;
        ball.vy = 0;
    }
    
    if (ball.x - ball.size <= 0) {
        ball.x = ball.size;
        ball.vx = -ball.vx * 80 / 100;
    }
    if (ball.x + ball.size >= SCREEN_WIDTH) {
        ball.x = SCREEN_WIDTH - ball.size;
        ball.vx = -ball.vx * 80 / 100;
    }
    if (ball.y - ball.size <= 0) {
        ball.y = ball.size;
        ball.vy = -ball.vy * 80 / 100;
    }
    if (ball.y + ball.size >= SCREEN_HEIGHT) {
        ball.y = SCREEN_HEIGHT - ball.size;
        ball.vy = -ball.vy * 80 / 100;
    }
    
    // VERIFICAR GOL
    int dx = ball.x - hole.x;
    int dy = ball.y - hole.y;
    int distance_sq = dx*dx + dy*dy;
    int min_distance_sq = (hole.size - ball.size) * (hole.size - ball.size);
    
    if (distance_sq < min_distance_sq) {
        // ¡HOYO! - ÚNICO SONIDO EN EL JUEGO
        showHoleMessage();
        resetBall();
        placeHoleRandomly();
        hits = 0;
    }
}

void showHoleMessage(void) {
    // SONIDO DE VICTORIA AL ANOTAR
    playWinSound();
    
    fillScreen(COLOR_YELLOW);
    
    drawRect(200, 300, 600, 150, COLOR_BLUE);
    drawRect(210, 310, 580, 130, COLOR_WHITE);
    
    int start_x = 300;
    int start_y = 350;
    
    // H
    drawRect(start_x, start_y, 10, 50, COLOR_BLACK);
    drawRect(start_x + 20, start_y, 10, 50, COLOR_BLACK);
    drawRect(start_x, start_y + 20, 30, 10, COLOR_BLACK);
    
    // O
    start_x += 50;
    drawRect(start_x, start_y, 30, 10, COLOR_BLACK);
    drawRect(start_x, start_y + 40, 30, 10, COLOR_BLACK);
    drawRect(start_x, start_y, 10, 50, COLOR_BLACK);
    drawRect(start_x + 20, start_y, 10, 50, COLOR_BLACK);
    
    // Y
    start_x += 50;
    drawRect(start_x, start_y, 10, 25, COLOR_BLACK);
    drawRect(start_x + 20, start_y, 10, 25, COLOR_BLACK);
    drawRect(start_x + 10, start_y + 20, 10, 30, COLOR_BLACK);
    
    // O
    start_x += 50;
    drawRect(start_x, start_y, 30, 10, COLOR_BLACK);
    drawRect(start_x, start_y + 40, 30, 10, COLOR_BLACK);
    drawRect(start_x, start_y, 10, 50, COLOR_BLACK);
    drawRect(start_x + 20, start_y, 10, 50, COLOR_BLACK);
    
    // !
    start_x += 50;
    drawRect(start_x + 10, start_y, 10, 35, COLOR_BLACK);
    drawRect(start_x + 10, start_y + 40, 10, 10, COLOR_BLACK);
    
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

void drawAimArrow(void) {
    int paddle_center_x = paddle.x + paddle.width/2;
    int paddle_center_y = paddle.y + paddle.height/2;
    
    int angle_index = aim_angle / 15;
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