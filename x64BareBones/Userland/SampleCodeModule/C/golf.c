#include "./../include/libc.h"
#include "./../include/golf.h"

int SCREEN_HEIGHT = 768;
int SCREEN_WIDTH = 1024;

static Ball ball;
static Paddle paddle1, paddle2;
static Hole hole;
static int hits = 0;
static int gameRunning = 1;
static int gameState = GAME_MENU;
static int gameMode = MODE_SINGLE;
static int currentLevel = 1;
static int maxLevels = 3;

static unsigned int random_seed = 1;

// Tablas trigonométricas de alta precisión (cada 10 grados) - 36 posiciones
static int cos_table_fine[36] = {
    100, 98, 94, 87, 77, 64, 50, 34, 17, 0, -17, -34, -50, -64, -77, -87, -94, -98,
    -100, -98, -94, -87, -77, -64, -50, -34, -17, 0, 17, 34, 50, 64, 77, 87, 94, 98
};

static int sin_table_fine[36] = {
    0, 17, 34, 50, 64, 77, 87, 94, 98, 100, 98, 94, 87, 77, 64, 50, 34, 17,
    0, -17, -34, -50, -64, -77, -87, -94, -98, -100, -98, -94, -87, -77, -64, -50, -34, -17
};

// Variables de control de movimiento mejoradas
static int p1_moving = 0;
static int p2_moving = 0;
static int p1_rotate_cooldown = 0;
static int p2_rotate_cooldown = 0;

// Variables para movimiento continuo y fluido
static int p1_forward_key_held = 0;
static int p2_forward_key_held = 0;

// Declaraciones de funciones
void drawCharPattern(int* pattern, int x, int y, int width, int height);
void drawSimpleText(const char* text, int x, int y);
void handleInput(void);
void processMovement(void);
void updateGame(void);
void drawGame(void);
void drawUI(void);

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
                processMovement();
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
        
        // Optimizado para máxima fluidez
        for (volatile int i = 0; i < 5000; i++);
    }
}

// Función auxiliar para dibujar patrones de caracteres de forma eficiente
void drawCharPattern(int* pattern, int x, int y, int width, int height) {
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            if (pattern[row * width + col]) {
                putPixel(COLOR_BLACK, x + col, y + row);
            }
        }
    }
}

void showMenu(void) {
    static int menuInitialized = 0;
    
    if (!menuInitialized) {
        fillScreen(COLOR_BLACK);
        
                print("========================================\n");
                print("         BIENVENIDO A PONGIS-GOLF       \n");
                print("========================================\n");
                print("\n");
                print("Selecciona el modo de juego:\n");
                print("\n");
                print("  [1] - Un Jugador\n");
                print("       Controles: Flechas\n");
                print("       Flechas laterales: Rotar palo\n");
                print("       Flecha superior   : Mover palo\n");
                print("\n");
                print("  [2] - Dos Jugadores\n");
                print("       Jugador 1: Flechas \n");
                print("       Jugador 2: WASD (A D W)\n");
                print("       ESPACIO: Cambiar jugador\n");
                print("\n");
                print("  [ESC] - Volver al menu\n");
                print("\n");
                print("Niveles: 3 (hoyo cada vez mas pequeno)\n");
                print("Objetivo: Meter la pelota en el hoyo\n");
                print("\n");
        
        menuInitialized = 1;
    }
    
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
    } else if (key == 27) {
        gameRunning = 0;
    }
}

// Generador de números aleatorios mejorado
int simpleRandom(int min, int max) {
    random_seed = random_seed * 1664525 + 1013904223; // Mejores constantes
    return min + (random_seed % (max - min + 1));
}

// Tamaños de hoyo más progresivos
int getHoleSize(int level) {
    switch (level) {
        case 1: return 35;  // Más grande para empezar
        case 2: return 25;  // Tamaño intermedio
        case 3: return 18;  // Más pequeño pero no imposible
        default: return 18;
    }
}

void placeHoleRandomly(void) {
    hole.size = getHoleSize(currentLevel);
    int margin = hole.size + 50;
    
    do {
        hole.x = simpleRandom(margin, SCREEN_WIDTH - margin);
        hole.y = simpleRandom(margin + 45, SCREEN_HEIGHT - margin);
        
        // Verificar distancia mínima de la pelota
        int dx = hole.x - ball.x;
        int dy = hole.y - ball.y;
        int distance_sq = dx*dx + dy*dy;
        int min_distance = 250;
        
        // Verificar distancia de los paddles (proporcional a pantalla)
        int dx1 = hole.x - (paddle1.x + paddle1.width/2);
        int dy1 = hole.y - (paddle1.y + paddle1.height/2);
        int dist1_sq = dx1*dx1 + dy1*dy1;
        
        int dx2 = 999, dy2 = 999, dist2_sq = 999999;
        if (gameMode == MODE_MULTIPLAYER) {
            dx2 = hole.x - (paddle2.x + paddle2.width/2);
            dy2 = hole.y - (paddle2.y + paddle2.height/2);
            dist2_sq = dx2*dx2 + dy2*dy2;
        }
        
        // Verificar conflictos con UI (proporcional a pantalla)
        int ui_conflict = 0;
        if (hole.y < 50) { // Área de UI
            // Calcular áreas de UI proporcionales
            int left_ui_width = SCREEN_WIDTH / 6;   // ~17% para UI izquierda
            int right_ui_width = SCREEN_WIDTH / 4;  // ~25% para UI derecha
            
            if ((hole.x < left_ui_width) || (hole.x > SCREEN_WIDTH - right_ui_width)) {
                ui_conflict = 1;
            }
        }
        
        if (distance_sq > min_distance * min_distance && 
            dist1_sq > 150*150 && dist2_sq > 150*150 && !ui_conflict) {
            break;
        }
    } while (1);
}

void initGame(void) {
    
    // Obtener valores
    SCREEN_WIDTH = getScreenWidth();
    SCREEN_HEIGHT = getScreenHeight();
    
    fillScreen(COLOR_GREEN);
    
    // Semilla random...
    random_seed = 12345 + currentLevel * 1000 + hits * 100;
    
    // Pelota en el centro (usando dimensiones dinámicas)
    ball.x = SCREEN_WIDTH / 2;
    ball.y = SCREEN_HEIGHT / 2;
    ball.vx = 0;
    ball.vy = 0;
    ball.size = BALL_SIZE;
    
    // Paddle 1 - posicionado proporcionalmente
    paddle1.x = SCREEN_WIDTH / 20;  // 5% del ancho desde el borde izquierdo
    paddle1.y = SCREEN_HEIGHT / 2 - (BALL_SIZE * 3);
    if (paddle1.y < 45) paddle1.y = 45; // Asegurar que no esté en la UI
    paddle1.width = BALL_SIZE * 6;
    paddle1.height = BALL_SIZE * 6;
    paddle1.color = COLOR_BLUE;
    paddle1.aim_angle = 0;
    
    // Paddle 2 - posicionado proporcionalmente desde el borde derecho
    if (gameMode == MODE_MULTIPLAYER) {
        paddle2.x = SCREEN_WIDTH - (SCREEN_WIDTH / 10);  // 10% del ancho desde el borde derecho
        paddle2.y = SCREEN_HEIGHT / 2 - (BALL_SIZE * 3);
        if (paddle2.y < 45) paddle2.y = 45; // Asegurar que no esté en la UI
        paddle2.width = BALL_SIZE * 6;
        paddle2.height = BALL_SIZE * 6;
        paddle2.color = COLOR_RED;
        paddle2.aim_angle = 180;
    }
    
    // Colocar hoyo usando dimensiones dinámicas
    placeHoleRandomly();
    
    hits = 0;
    p1_moving = 0;
    p2_moving = 0;
    p1_rotate_cooldown = 0;
    p2_rotate_cooldown = 0;
    p1_forward_key_held = 0;
    p2_forward_key_held = 0;
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
    
    // Limpiar posición anterior del hoyo SOLO si cambió
    if (last_hole_x != -1 && (last_hole_x != hole.x || last_hole_y != hole.y)) {
        drawCircle(last_hole_x, last_hole_y, getHoleSize(currentLevel) + 3, COLOR_GREEN);
    }
    
    // Limpiar posición anterior de la pelota SOLO si se movió
    if (last_ball_x != -1 && (last_ball_x != ball.x || last_ball_y != ball.y)) {
        drawCircle(last_ball_x, last_ball_y, ball.size + 2, COLOR_GREEN);
    }
    
    // Limpiar paddle 1 SOLO si se movió o rotó
    if (last_paddle1_x != -1 && (last_paddle1_x != paddle1.x || last_paddle1_y != paddle1.y || last_angle1 != paddle1.aim_angle)) {
        int old_center_x = last_paddle1_x + paddle1.width/2;
        int old_center_y = last_paddle1_y + paddle1.height/2;
        // Limpiar área más grande para incluir la flecha
        drawRect(old_center_x - 60, old_center_y - 60, 120, 120, COLOR_GREEN);
    }
    
    // Limpiar paddle 2 SOLO si se movió o rotó
    if (gameMode == MODE_MULTIPLAYER && last_paddle2_x != -1 && 
        (last_paddle2_x != paddle2.x || last_paddle2_y != paddle2.y || last_angle2 != paddle2.aim_angle)) {
        int old_center_x = last_paddle2_x + paddle2.width/2;
        int old_center_y = last_paddle2_y + paddle2.height/2;
        drawRect(old_center_x - 60, old_center_y - 60, 120, 120, COLOR_GREEN);
    }
    
    // Dibujar hoyo con borde negro
    drawCircle(hole.x, hole.y, hole.size + 2, COLOR_BLACK);
    drawCircle(hole.x, hole.y, hole.size, COLOR_BLACK);
    
    // Dibujar pelota blanca
    drawCircle(ball.x, ball.y, ball.size, COLOR_WHITE);
    
    // Dibujar paddles con sus flechas
    int paddle1_center_x = paddle1.x + paddle1.width/2;
    int paddle1_center_y = paddle1.y + paddle1.height/2;
    drawCircle(paddle1_center_x, paddle1_center_y, paddle1.width/2, paddle1.color);
    drawAimArrow(&paddle1);
    
    if (gameMode == MODE_MULTIPLAYER) {
        int paddle2_center_x = paddle2.x + paddle2.width/2;
        int paddle2_center_y = paddle2.y + paddle2.height/2;
        drawCircle(paddle2_center_x, paddle2_center_y, paddle2.width/2, paddle2.color);
        drawAimArrow(&paddle2);
    }
    
    // Dibujar UI siempre al final
    drawUI();
    
    // Actualizar posiciones anteriores
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

// UI sin fondo amarillo - solo recuadros flotantes
void drawUI(void) {
    // Calcular posiciones proporcionales a la pantalla
    int left_box_width = SCREEN_WIDTH / 8;      // ~12.5% del ancho
    int right_box_width = SCREEN_WIDTH / 5;     // ~20% del ancho
    
    // Caja del nivel (izquierda) - proporcional
    drawRect(10, 5, left_box_width, 35, COLOR_BLACK);
    drawRect(12, 7, left_box_width - 4, 31, COLOR_WHITE);
    
    // Texto "NIVEL:" ajustado al tamaño de caja
    drawSimpleText("NIVEL: ", 18, 15);
    drawNumber(currentLevel, 18 + 77, 15); // Posición relativa al texto
    
    // Caja de golpes (derecha) - proporcional
    int right_box_x = SCREEN_WIDTH - right_box_width - 10;
    drawRect(right_box_x, 5, right_box_width, 35, COLOR_BLACK);
    drawRect(right_box_x + 2, 7, right_box_width - 4, 31, COLOR_WHITE);
    
    // Texto "GOLPES:" ajustado al tamaño de caja
    drawSimpleText("GOLPES: ", right_box_x + 8, 15);
    drawNumber(hits, right_box_x + 8 + 88, 15); // Posición relativa al texto
}

// Función para dibujar texto
void drawSimpleText(const char* text, int x, int y) {
    for (int i = 0; text[i] != '\0'; i++) {
        char c = text[i];
        int char_x = x + (i * 10); // Espaciado entre caracteres
        
        // Patrones 8x12 
        if (c == 'N') {
            int n_pattern[] = {
                1,0,0,0,0,0,1,0,
                1,1,0,0,0,0,1,0,
                1,1,0,0,0,0,1,0,
                1,0,1,0,0,0,1,0,
                1,0,1,0,0,0,1,0,
                1,0,0,1,0,0,1,0,
                1,0,0,1,0,0,1,0,
                1,0,0,0,1,0,1,0,
                1,0,0,0,1,0,1,0,
                1,0,0,0,0,1,1,0,
                1,0,0,0,0,1,1,0,
                1,0,0,0,0,0,1,0
            };
            drawCharPattern(n_pattern, char_x, y, 8, 12);
        } else if (c == 'I' || c == 'i') {
            int i_pattern[] = {
                0,1,1,1,1,1,0,0,
                0,0,0,1,0,0,0,0,
                0,0,0,1,0,0,0,0,
                0,0,0,1,0,0,0,0,
                0,0,0,1,0,0,0,0,
                0,0,0,1,0,0,0,0,
                0,0,0,1,0,0,0,0,
                0,0,0,1,0,0,0,0,
                0,0,0,1,0,0,0,0,
                0,0,0,1,0,0,0,0,
                0,0,0,1,0,0,0,0,
                0,1,1,1,1,1,0,0
            };
            drawCharPattern(i_pattern, char_x, y, 8, 12);
        } else if (c == 'V' || c == 'v') {
            int v_pattern[] = {
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                0,1,0,0,0,1,0,0,
                0,1,0,0,0,1,0,0,
                0,1,0,0,0,1,0,0,
                0,0,1,0,1,0,0,0,
                0,0,1,0,1,0,0,0,
                0,0,0,1,0,0,0,0,
                0,0,0,1,0,0,0,0,
                0,0,0,0,0,0,0,0
            };
            drawCharPattern(v_pattern, char_x, y, 8, 12);
        } else if (c == 'E' || c == 'e') {
            int e_pattern[] = {
                1,1,1,1,1,1,1,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,1,1,1,1,1,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,1,1,1,1,1,1,0
            };
            drawCharPattern(e_pattern, char_x, y, 8, 12);
        } else if (c == 'L' || c == 'l') {
            int l_pattern[] = {
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,1,1,1,1,1,1,0
            };
            drawCharPattern(l_pattern, char_x, y, 8, 12);
        } else if (c == ':') {
            int colon_pattern[] = {
                0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,
                0,0,1,1,0,0,0,0,
                0,0,1,1,0,0,0,0,
                0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,
                0,0,1,1,0,0,0,0,
                0,0,1,1,0,0,0,0,
                0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0
            };
            drawCharPattern(colon_pattern, char_x, y, 8, 12);
        } else if (c == 'C' || c == 'c') {

            int c_pattern[] = {
                0,1,1,1,1,1,0,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,1,0,
                0,1,1,1,1,1,0,0
            };
            drawCharPattern(c_pattern, char_x, y, 8, 12);
        } else if (c == 'O' || c == 'o') {
            int o_pattern[] = {
                0,1,1,1,1,1,0,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                0,1,1,1,1,1,0,0
            };
            drawCharPattern(o_pattern, char_x, y, 8, 12);
        } else if (c == 'M' || c == 'm') {
            int m_pattern[] = {
                1,0,0,0,0,0,1,0,
                1,1,0,0,0,1,1,0,
                1,1,0,0,0,1,1,0,
                1,0,1,0,1,0,1,0,
                1,0,1,0,1,0,1,0,
                1,0,0,1,0,0,1,0,
                1,0,0,1,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0
            };
            drawCharPattern(m_pattern, char_x, y, 8, 12);
        } else if (c == 'P' || c == 'p') {
            int p_pattern[] = {
                1,1,1,1,1,1,0,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,1,1,1,1,1,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0
            };
            drawCharPattern(p_pattern, char_x, y, 8, 12);
        } else if (c == 'T' || c == 't') {
            int t_pattern[] = {
                1,1,1,1,1,1,1,0,
                0,0,0,1,0,0,0,0,
                0,0,0,1,0,0,0,0,
                0,0,0,1,0,0,0,0,
                0,0,0,1,0,0,0,0,
                0,0,0,1,0,0,0,0,
                0,0,0,1,0,0,0,0,
                0,0,0,1,0,0,0,0,
                0,0,0,1,0,0,0,0,
                0,0,0,1,0,0,0,0,
                0,0,0,1,0,0,0,0,
                0,0,0,1,0,0,0,0
            };
            drawCharPattern(t_pattern, char_x, y, 8, 12);
        } else if (c == 'A' || c == 'a') {
            int a_pattern[] = {
                0,0,1,1,0,0,0,0,
                0,1,0,0,1,0,0,0,
                0,1,0,0,1,0,0,0,
                1,0,0,0,0,1,0,0,
                1,0,0,0,0,1,0,0,
                1,0,0,0,0,1,0,0,
                1,1,1,1,1,1,0,0,
                1,0,0,0,0,1,0,0,
                1,0,0,0,0,1,0,0,
                1,0,0,0,0,1,0,0,
                1,0,0,0,0,1,0,0,
                1,0,0,0,0,1,0,0
            };
            drawCharPattern(a_pattern, char_x, y, 8, 12);
        } else if (c == 'D' || c == 'd') {
            int d_pattern[] = {
                1,1,1,1,1,0,0,0,
                1,0,0,0,0,1,0,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,1,0,0,
                1,1,1,1,1,0,0,0
            };
            drawCharPattern(d_pattern, char_x, y, 8, 12);
        } else if (c == 'G' || c == 'g') {
            int g_pattern[] = {
                0,1,1,1,1,1,0,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                1,0,0,1,1,1,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                0,1,1,1,1,1,0,0
            };
            drawCharPattern(g_pattern, char_x, y, 8, 12);
        } else if (c == 'S' || c == 's') {
            int s_pattern[] = {
                0,1,1,1,1,1,0,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,
                0,1,1,1,1,0,0,0,
                0,0,0,0,0,1,0,0,
                0,0,0,0,0,0,1,0,
                0,0,0,0,0,0,1,0,
                0,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                1,0,0,0,0,0,1,0,
                0,1,1,1,1,1,0,0
            };
            drawCharPattern(s_pattern, char_x, y, 8, 12);
        } else if (c == '!') {
            int excl_pattern[] = {
                0,0,1,1,0,0,0,0,
                0,0,1,1,0,0,0,0,
                0,0,1,1,0,0,0,0,
                0,0,1,1,0,0,0,0,
                0,0,1,1,0,0,0,0,
                0,0,1,1,0,0,0,0,
                0,0,1,1,0,0,0,0,
                0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,
                0,0,1,1,0,0,0,0,
                0,0,1,1,0,0,0,0,
                0,0,0,0,0,0,0,0
            };
            drawCharPattern(excl_pattern, char_x, y, 8, 12);
        } else if (c == ' ') {
        }
    }
}

void handleInput(void) {
    char key;
    
    // Flags para detectar múltiples teclas en este frame
    int p1_left_pressed = 0, p1_right_pressed = 0, p1_up_pressed = 0;
    int p2_left_pressed = 0, p2_right_pressed = 0, p2_up_pressed = 0;
    
    // Sin cooldown para rotación ultra fluida
    if (p1_rotate_cooldown > 0) p1_rotate_cooldown--;
    if (p2_rotate_cooldown > 0) p2_rotate_cooldown--;
    
    // Procesar TODAS las teclas presionadas en este frame
    while ((key = getKeyNonBlocking()) != 0) {
        if (key == 27) {
            gameState = GAME_MENU;
            return;
        }
        
        // Detectar teclas del jugador 1
        if (key == 75) { // Izquierda
            p1_left_pressed = 1;
        }
        else if (key == 77) { // Derecha
            p1_right_pressed = 1;
        }
        else if (key == 72) { // Arriba
            p1_up_pressed = 1;
        }
        
        // Detectar teclas del jugador 2
        if (gameMode == MODE_MULTIPLAYER) {
            if (key == 'a' || key == 'A') {
                p2_left_pressed = 1;
            }
            else if (key == 'd' || key == 'D') {
                p2_right_pressed = 1;
            }
            else if (key == 'w' || key == 'W') {
                p2_up_pressed = 1;
            }
        }
    }
    
    // Procesar rotación RÁPIDA del jugador 1 (independiente del movimiento)
    if (p1_left_pressed && p1_rotate_cooldown == 0) {
        paddle1.aim_angle = (paddle1.aim_angle - 8 + 360) % 360; // Rotación más rápida
        p1_rotate_cooldown = 0; // Sin cooldown para fluidez máxima
    }
    if (p1_right_pressed && p1_rotate_cooldown == 0) {
        paddle1.aim_angle = (paddle1.aim_angle + 8) % 360; // Rotación más rápida
        p1_rotate_cooldown = 0;
    }
    
    // Control de movimiento INMEDIATO para jugador 1
    if (p1_up_pressed) {
        p1_moving = 1; // Activar movimiento
        p1_forward_key_held = 1; // Marcar como presionada
    } else {
        p1_moving = 0; // PARAR INMEDIATAMENTE cuando se suelta
        p1_forward_key_held = 0; // Resetear flag
    }
    
    // Procesar rotación RÁPIDA del jugador 2 (independiente del movimiento)
    if (gameMode == MODE_MULTIPLAYER) {
        if (p2_left_pressed && p2_rotate_cooldown == 0) {
            paddle2.aim_angle = (paddle2.aim_angle - 8 + 360) % 360; // Rotación más rápida
            p2_rotate_cooldown = 0;
        }
        if (p2_right_pressed && p2_rotate_cooldown == 0) {
            paddle2.aim_angle = (paddle2.aim_angle + 8) % 360; // Rotación más rápida
            p2_rotate_cooldown = 0;
        }
        
        // Control de movimiento INMEDIATO para jugador 2
        if (p2_up_pressed) {
            p2_moving = 1; // Activar movimiento
            p2_forward_key_held = 1; // Marcar como presionada
        } else {
            p2_moving = 0; // PARAR INMEDIATAMENTE cuando se suelta
            p2_forward_key_held = 0; // Resetear flag
        }
    }
}

void processMovement(void) {
    // Movimiento continuo y fluido para jugador 1
    if (p1_moving > 0) {
        int angle_index = (paddle1.aim_angle / 10) % 36;
        int angle_x = cos_table_fine[angle_index];
        int angle_y = sin_table_fine[angle_index];
        
        int new_x = paddle1.x + (angle_x * 10) / 100;
        int new_y = paddle1.y + (angle_y * 10) / 100;
        
        // Usar límites dinámicos
        if (new_x >= 0 && new_x + paddle1.width <= SCREEN_WIDTH &&
            new_y >= 45 && new_y + paddle1.height <= SCREEN_HEIGHT) {
            paddle1.x = new_x;
            paddle1.y = new_y;
        }
    }
    
    // Movimiento continuo y fluido para jugador 2
    if (gameMode == MODE_MULTIPLAYER && p2_moving > 0) {
        int angle_index = (paddle2.aim_angle / 10) % 36;
        int angle_x = cos_table_fine[angle_index];
        int angle_y = sin_table_fine[angle_index];
        
        int new_x = paddle2.x + (angle_x * 10) / 100;
        int new_y = paddle2.y + (angle_y * 10) / 100;
        
        // Usar límites dinámicos
        if (new_x >= 0 && new_x + paddle2.width <= SCREEN_WIDTH &&
            new_y >= 45 && new_y + paddle2.height <= SCREEN_HEIGHT) {
            paddle2.x = new_x;
            paddle2.y = new_y;
        }
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
    
    // Detectar colisiones con paddle 1
    int paddle1_center_x = paddle1.x + paddle1.width/2;
    int paddle1_center_y = paddle1.y + paddle1.height/2;
    int dx1 = ball.x - paddle1_center_x;
    int dy1 = ball.y - paddle1_center_y;
    int distance1_sq = dx1*dx1 + dy1*dy1;
    int collision_radius = (ball.size + paddle1.width/2 - 3);
    
    // Detectar colisiones con paddle 2
    int distance2_sq = 999999;
    int dx2 = 0, dy2 = 0;
    if (gameMode == MODE_MULTIPLAYER) {
        int paddle2_center_x = paddle2.x + paddle2.width/2;
        int paddle2_center_y = paddle2.y + paddle2.height/2;
        dx2 = ball.x - paddle2_center_x;
        dy2 = ball.y - paddle2_center_y;
        distance2_sq = dx2*dx2 + dy2*dy2;
    }
    
    // Verificar si está lejos para resetear el cooldown
    int far_distance = (collision_radius + 40) * (collision_radius + 40);
    if (distance1_sq > far_distance && distance2_sq > far_distance) {
        was_far = 1;
    }
    
    // Calcular velocidad de los paddles
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
    
    // Colisión con paddle 1
    if (distance1_sq < (collision_radius * collision_radius) && 
        collision_cooldown == 0 && was_far == 1 && paddle1_speed > 0) {
        
        int dist = isqrt(distance1_sq);
        if (dist > 1) {
            // Física ajustada para la nueva mecánica
            int base_impact = 300; // Más impulso inicial
            int speed_multiplier = paddle1_speed * 100;
            int final_speed = base_impact + speed_multiplier;
            
            if (final_speed < 300) final_speed = 300;
            if (final_speed > 2000) final_speed = 2000; // Límite más alto
            
            ball.vx = (dx1 * final_speed) / dist;
            ball.vy = (dy1 * final_speed) / dist;
            
            hits++;
            playBeep(); // Sonido al golpear
            
            // Separar la pelota del paddle
            int separation = collision_radius + 25;
            ball.x = paddle1_center_x + (dx1 * separation) / dist;
            ball.y = paddle1_center_y + (dy1 * separation) / dist;
            
            collision_cooldown = 20;
            was_far = 0;
        }
    }
    
    // Colisión con paddle 2
    if (gameMode == MODE_MULTIPLAYER && 
        distance2_sq < (collision_radius * collision_radius) && 
        collision_cooldown == 0 && was_far == 1 && paddle2_speed > 0) {
        
        int dist = isqrt(distance2_sq);
        if (dist > 1) {
            int base_impact = 300; // Más impulso inicial
            int speed_multiplier = paddle2_speed * 100;
            int final_speed = base_impact + speed_multiplier;
            
            if (final_speed < 300) final_speed = 300;
            if (final_speed > 2000) final_speed = 2000; // Límite más alto
            
            ball.vx = (dx2 * final_speed) / dist;
            ball.vy = (dy2 * final_speed) / dist;
            
            hits++;
            playBeep();
            
            int paddle2_center_x = paddle2.x + paddle2.width/2;
            int paddle2_center_y = paddle2.y + paddle2.height/2;
            int separation = collision_radius + 25;
            ball.x = paddle2_center_x + (dx2 * separation) / dist;
            ball.y = paddle2_center_y + (dy2 * separation) / dist;
            
            collision_cooldown = 20;
            was_far = 0;
        }
    }
    
    // Actualizar posiciones anteriores
    last_paddle1_x = paddle1.x;
    last_paddle1_y = paddle1.y;
    if (gameMode == MODE_MULTIPLAYER) {
        last_paddle2_x = paddle2.x;
        last_paddle2_y = paddle2.y;
    }
    
    // Física de la pelota mejorada - más fluida y con fricción optimizada
    ball.x += ball.vx / 100; // Movimiento más fluido
    ball.y += ball.vy / 100;
    
    // Fricción optimizada para fluidez
    ball.vx = (ball.vx * 996) / 1000; // Fricción balanceada
    ball.vy = (ball.vy * 996) / 1000;
    
    // Parar cuando la velocidad es baja
    if (abs(ball.vx) < 6 && abs(ball.vy) < 6) { // Umbral optimizado
        ball.vx = 0;
        ball.vy = 0;
    }
    
    // Rebotes en las paredes con física ajustada
    if (ball.x - ball.size <= 0) {
        ball.x = ball.size;
        ball.vx = -ball.vx * 87 / 100;
        playSound(400, 60);
    }
    if (ball.x + ball.size >= SCREEN_WIDTH) {  // Ya usa SCREEN_WIDTH dinámico
        ball.x = SCREEN_WIDTH - ball.size;
        ball.vx = -ball.vx * 87 / 100;
        playSound(400, 60);
    }
    if (ball.y - ball.size <= 45) { // Evitar UI
        ball.y = 45 + ball.size;
        ball.vy = -ball.vy * 87 / 100;
        playSound(400, 60);
    }
    if (ball.y + ball.size >= SCREEN_HEIGHT) {  // Ya usa SCREEN_HEIGHT dinámico
        ball.y = SCREEN_HEIGHT - ball.size;
        ball.vy = -ball.vy * 87 / 100;
        playSound(400, 60);
    }
    
    // Verificar si la pelota entra en el hoyo
    int dx = ball.x - hole.x;
    int dy = ball.y - hole.y;
    int distance_sq = dx*dx + dy*dy;
    int hole_radius = hole.size - ball.size;
    
    if (distance_sq < (hole_radius * hole_radius) && hole_radius > 0) {
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
    
    int box_width = SCREEN_WIDTH * 3 / 5;  // 60% del ancho de pantalla
    int box_height = 200;
    int box_x = (SCREEN_WIDTH - box_width) / 2;
    int box_y = (SCREEN_HEIGHT - box_height) / 2;
    
    drawRect(box_x, box_y, box_width, box_height, COLOR_BLUE);
    drawRect(box_x + 10, box_y + 10, box_width - 20, box_height - 20, COLOR_WHITE);
    
    // Texto centrado dinámicamente
    int text_x = box_x + (box_width - 190) / 2;  // Centrar "NIVEL COMPLETADO!" 
    drawSimpleText("NIVEL COMPLETADO!", text_x, box_y + 50);
    
    int golpes_x = box_x + (box_width - 100) / 2;  // Centrar "GOLPES:"
    drawSimpleText("GOLPES: ", golpes_x, box_y + 100);
    drawNumber(hits, golpes_x + 88, box_y + 100);
    
    
    // Pausa MUCHO más larga para poder leer tranquilo
    for (volatile int i = 0; i < 50000000; i++); 
    
    resetBall();
    placeHoleRandomly();
    hits = 0;
    gameState = GAME_PLAYING;
    fillScreen(COLOR_GREEN);
}

void showHoleMessage(void) {
    playWinSound();
    
    fillScreen(COLOR_YELLOW);
    
    int box_width = SCREEN_WIDTH * 3 / 5;  // 60% del ancho de pantalla
    int box_height = 200;
    int box_x = (SCREEN_WIDTH - box_width) / 2;
    int box_y = (SCREEN_HEIGHT - box_height) / 2;
    
    drawRect(box_x, box_y, box_width, box_height, COLOR_BLUE);
    drawRect(box_x + 10, box_y + 10, box_width - 20, box_height - 20, COLOR_WHITE);
    
    // Texto centrado dinámicamente
    int text_x = box_x + (box_width - 180) / 2;  // Centrar "JUEGO COMPLETADO!"
    drawSimpleText("JUEGO COMPLETADO!", text_x, box_y + 50);
    
    int golpes_x = box_x + (box_width - 160) / 2;  // Centrar "GOLPES TOTALES:"
    drawSimpleText("GOLPES TOTALES: ", golpes_x, box_y + 100);
    drawNumber(hits, golpes_x + 168, box_y + 100);
    
    // Pausa más larga para el final del juego
    for (volatile int i = 0; i < 60000000; i++); 
    
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
        
        // Invertir string
        for (int j = 0; j < i/2; j++) {
            char temp = numStr[j];
            numStr[j] = numStr[i-1-j];
            numStr[i-1-j] = temp;
        }
    }
    
    // Dibujar cada dígito con patrones más prolijos y compactos
    for (int digit = 0; digit < strlen(numStr); digit++) {
        char c = numStr[digit];
        int digit_x = x + (digit * 10); // Más compacto para alinearse con el texto
        
        // Patrones de números 7x12 más prolijos (igual altura que el texto)
        int patterns[10][84] = {
            // 0
            {0,1,1,1,1,1,0, 1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 1,1,0,0,0,1,1,
             1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 0,1,1,1,1,1,0},
            // 1
            {0,0,1,1,0,0,0, 0,1,1,1,0,0,0, 1,1,1,1,0,0,0, 0,0,1,1,0,0,0, 0,0,1,1,0,0,0, 0,0,1,1,0,0,0,
             0,0,1,1,0,0,0, 0,0,1,1,0,0,0, 0,0,1,1,0,0,0, 0,0,1,1,0,0,0, 0,0,1,1,0,0,0, 1,1,1,1,1,1,0},
            // 2
            {0,1,1,1,1,1,0, 1,1,0,0,0,1,1, 0,0,0,0,0,1,1, 0,0,0,0,0,1,1, 0,0,0,0,1,1,0, 0,0,0,1,1,0,0,
             0,0,1,1,0,0,0, 0,1,1,0,0,0,0, 1,1,0,0,0,0,0, 1,1,0,0,0,0,0, 1,1,0,0,0,1,1, 1,1,1,1,1,1,1},
            // 3
            {0,1,1,1,1,1,0, 1,1,0,0,0,1,1, 0,0,0,0,0,1,1, 0,0,0,0,0,1,1, 0,0,1,1,1,1,0, 0,0,0,0,0,1,1,
             0,0,0,0,0,1,1, 0,0,0,0,0,1,1, 0,0,0,0,0,1,1, 1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 0,1,1,1,1,1,0},
            // 4
            {1,1,0,0,1,1,0, 1,1,0,0,1,1,0, 1,1,0,0,1,1,0, 1,1,0,0,1,1,0, 1,1,0,0,1,1,0, 1,1,1,1,1,1,1,
             1,1,1,1,1,1,1, 0,0,0,0,1,1,0, 0,0,0,0,1,1,0, 0,0,0,0,1,1,0, 0,0,0,0,1,1,0, 0,0,0,0,1,1,0},
            // 5
            {1,1,1,1,1,1,1, 1,1,0,0,0,0,0, 1,1,0,0,0,0,0, 1,1,0,0,0,0,0, 1,1,1,1,1,1,0, 0,0,0,0,0,1,1,
             0,0,0,0,0,1,1, 0,0,0,0,0,1,1, 0,0,0,0,0,1,1, 1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 0,1,1,1,1,1,0},
            // 6
            {0,1,1,1,1,1,0, 1,1,0,0,0,1,1, 1,1,0,0,0,0,0, 1,1,0,0,0,0,0, 1,1,1,1,1,1,0, 1,1,0,0,0,1,1,
             1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 0,1,1,1,1,1,0},
            // 7
            {1,1,1,1,1,1,1, 0,0,0,0,0,1,1, 0,0,0,0,1,1,0, 0,0,0,0,1,1,0, 0,0,0,1,1,0,0, 0,0,0,1,1,0,0,
             0,0,1,1,0,0,0, 0,0,1,1,0,0,0, 0,1,1,0,0,0,0, 0,1,1,0,0,0,0, 1,1,0,0,0,0,0, 1,1,0,0,0,0,0},
            // 8
            {0,1,1,1,1,1,0, 1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 0,1,1,1,1,1,0, 1,1,0,0,0,1,1,
             1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 0,1,1,1,1,1,0},
            // 9
            {0,1,1,1,1,1,0, 1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 0,1,1,1,1,1,1,
             0,0,0,0,0,1,1, 0,0,0,0,0,1,1, 0,0,0,0,0,1,1, 1,1,0,0,0,1,1, 1,1,0,0,0,1,1, 0,1,1,1,1,1,0}
        };
        
        int num = c - '0';
        for (int row = 0; row < 12; row++) {
            for (int col = 0; col < 7; col++) {
                if (patterns[num][row * 7 + col]) {
                    putPixel(COLOR_BLACK, digit_x + col, y + row);
                }
            }
        }
    }
}

void drawAimArrow(Paddle* paddle) {
    int paddle_center_x = paddle->x + paddle->width/2;
    int paddle_center_y = paddle->y + paddle->height/2;
    
    // Usar las nuevas tablas para flechas más precisas
    int angle_index = (paddle->aim_angle / 10) % 36;
    int dir_x = cos_table_fine[angle_index];
    int dir_y = sin_table_fine[angle_index];
    
    // Flecha con longitud optimizada para visibilidad
    int arrow_length = 35; // Longitud perfecta
    int tip_x = paddle_center_x + (dir_x * arrow_length) / 100;
    int tip_y = paddle_center_y + (dir_y * arrow_length) / 100;
    
    // Línea principal más suave y visible
    for (int t = 0; t < arrow_length; t += 1) {
        int line_x = paddle_center_x + (dir_x * t) / 100;
        int line_y = paddle_center_y + (dir_y * t) / 100;
        
        // Línea más suave con mejor antialiasing
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                putPixel(COLOR_RED, line_x + dx, line_y + dy);
            }
        }
    }
    
    // Punta de flecha bien proporcionada
    int perp_x = -dir_y;
    int perp_y = dir_x;
    
    int head1_x = tip_x - (dir_x * 10) / 100 + (perp_x * 7) / 100; // Proporcionada
    int head1_y = tip_y - (dir_y * 10) / 100 + (perp_y * 7) / 100;
    int head2_x = tip_x - (dir_x * 10) / 100 - (perp_x * 7) / 100;
    int head2_y = tip_y - (dir_y * 10) / 100 - (perp_y * 7) / 100;
    
    drawLine(tip_x, tip_y, head1_x, head1_y, COLOR_RED);
    drawLine(tip_x, tip_y, head2_x, head2_y, COLOR_RED);
    
    // Círculo en la punta optimizado
    drawCircle(tip_x, tip_y, 4, COLOR_RED); // Tamaño perfecto
}

void drawLine(int x1, int y1, int x2, int y2, uint32_t color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    int x = x1, y = y1;
    
    while (1) {
        // Línea más gruesa
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                putPixel(color, x + i, y + j);
            }
        }
        
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