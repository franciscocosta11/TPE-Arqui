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

// Estado de las teclas
typedef struct {
    int p1_left;
    int p1_right; 
    int p1_up;
    int p2_left;
    int p2_right;
    int p2_up;
    int escape;
} KeyState;

static KeyState keyState = {0};

// Cooldowns
static int p1_rotate_cooldown = 0;
static int p2_rotate_cooldown = 0;

void startGolfGame(void) {
    clearScreen();
    gameState = GAME_MENU;
    gameRunning = 1;
    
    // Inicializar estado de teclas
    keyState = (KeyState){0};
    
    while (gameRunning) {
        switch (gameState) {
            case GAME_MENU:
                showMenu();
                break;
            case GAME_PLAYING:
                handleInputImproved();
                processMovementImproved();
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

void handleInputImproved(void) {
    char key;
    
    // Decrementar cooldowns
    if (p1_rotate_cooldown > 0) p1_rotate_cooldown--;
    if (p2_rotate_cooldown > 0) p2_rotate_cooldown--;
    
    // Leer TODAS las teclas disponibles en este frame
    // jugador 2
    while ((key = getKeyNonBlocking()) != 0) {
        switch (key) {
            case 27: // ESC
                keyState.escape = 1;
                break;
                
            case 75: // Flecha izquierda
                keyState.p1_left = 1;
                break;
            case 77: // Flecha derecha 
                keyState.p1_right = 1;
                break;
            case 72: // Flecha arriba
                keyState.p1_up = 1;
                break;
                
            // jugador 1
            case 'a': case 'A':
                if (gameMode == MODE_MULTIPLAYER) {
                    keyState.p2_left = 1;
                }
                break;
            case 'd': case 'D':
                if (gameMode == MODE_MULTIPLAYER) {
                    keyState.p2_right = 1;
                }
                break;
            case 'w': case 'W':
                if (gameMode == MODE_MULTIPLAYER) {
                    keyState.p2_up = 1;
                }
                break;
        }
    }
    
    // Procesar ESC
    if (keyState.escape) {
        gameState = GAME_MENU;
        keyState.escape = 0;
        return;
    }
}

void processMovementImproved(void) {

    // Rotación del jugador 1 (independiente del movimiento)
    if (keyState.p1_left && p1_rotate_cooldown == 0) {
        paddle1.aim_angle = (paddle1.aim_angle - 10 + 360) % 360;
        p1_rotate_cooldown = 1; // Cooldown 
        keyState.p1_left = 0; // Consumir la tecla
    }
    
    if (keyState.p1_right && p1_rotate_cooldown == 0) {
        paddle1.aim_angle = (paddle1.aim_angle + 10) % 360;
        p1_rotate_cooldown = 1;
        keyState.p1_right = 0; 
    }
    
    // Movimiento del jugador 1 
    if (keyState.p1_up) {
        int angle_index = (paddle1.aim_angle / 10) % 36;
        int angle_x = cos_table_fine[angle_index];
        int angle_y = sin_table_fine[angle_index];
        
        int new_x = paddle1.x + (angle_x * 12) / 100; 
        int new_y = paddle1.y + (angle_y * 12) / 100;
        
        // Verificar límites
        if (new_x >= 0 && new_x + paddle1.width <= SCREEN_WIDTH &&
            new_y >= 45 && new_y + paddle1.height <= SCREEN_HEIGHT) {
            paddle1.x = new_x;
            paddle1.y = new_y;
        }

    } else {
        // Resetear la tecla si no está presionada en este frame
        keyState.p1_up = 0;
    }
    
    // modo multiplayer
    if (gameMode == MODE_MULTIPLAYER) {
        
        // Rotación del jugador 2 (independiente del movimiento)
        if (keyState.p2_left && p2_rotate_cooldown == 0) {
            paddle2.aim_angle = (paddle2.aim_angle - 10 + 360) % 360;
            p2_rotate_cooldown = 1;
            keyState.p2_left = 0; // Consumir la tecla
        }
        
        if (keyState.p2_right && p2_rotate_cooldown == 0) {
            paddle2.aim_angle = (paddle2.aim_angle + 10) % 360;
            p2_rotate_cooldown = 1;
            keyState.p2_right = 0; // Consumir la tecla
        }
        
        // Movimiento del jugador 2
        if (keyState.p2_up) {
            int angle_index = (paddle2.aim_angle / 10) % 36;
            int angle_x = cos_table_fine[angle_index];
            int angle_y = sin_table_fine[angle_index];
            
            int new_x = paddle2.x + (angle_x * 12) / 100; // Velocidad aumentada
            int new_y = paddle2.y + (angle_y * 12) / 100;
            
            // Verificar límites
            if (new_x >= 0 && new_x + paddle2.width <= SCREEN_WIDTH &&
                new_y >= 45 && new_y + paddle2.height <= SCREEN_HEIGHT) {
                paddle2.x = new_x;
                paddle2.y = new_y;
            }
            
        } else {
            // Resetear la tecla si no está presionada 
            keyState.p2_up = 0;
        }
    }
}


// Función auxiliar para dibujar patrones de caracteres 
void drawCharPattern(const unsigned char* pattern, int x, int y) {
    for (int row = 0; row < 12; row++) {
        unsigned char byte = pattern[row];
        for (int col = 0; col < 8; col++) {
            if (byte & (0x80 >> col)) {    // Bit más significativo primero
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
        print("   [1] - Un Jugador\n");
        print("         Controles: Flechas\n");
        print("         Flechas laterales: Rotar palo\n");
        print("         Flecha superior   : Mover palo\n");
        print("\n");
        print("   [2] - Dos Jugadores\n");
        print("         Jugador 1: Flechas \n");
        print("         Jugador 2: WASD (A D W)\n");
        print("         *** MOVIMIENTO SIMULTANEO ***\n");
        print("\n");
        print("   [ESC] - Volver al menu\n");
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

// Generador de números aleatorios 
int simpleRandom(int min, int max) {
    random_seed = random_seed * 1664525 + 1013904223; 
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
        
        // Verificar conflictos con UI 
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
        paddle2.x = SCREEN_WIDTH - (SCREEN_WIDTH / 10);    // 10% del ancho desde el borde derecho
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
    p1_rotate_cooldown = 0;
    p2_rotate_cooldown = 0;
    
    // Limpiar estado de teclas
    keyState = (KeyState){0};
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
    
    // Limpiar paddle 1 SOLO si se movió o rotó - CON VERIFICACIÓN DE LÍMITES
    if (last_paddle1_x != -1 && (last_paddle1_x != paddle1.x || last_paddle1_y != paddle1.y || last_angle1 != paddle1.aim_angle)) {
        int old_center_x = last_paddle1_x + paddle1.width/2;
        int old_center_y = last_paddle1_y + paddle1.height/2;
        
        int clean_x = old_center_x - 60;
        int clean_y = old_center_y - 60;
        int clean_width = 120;
        int clean_height = 120;
        
        // Ajustar coordenadas para que no se salgan de la pantalla
        if (clean_x < 0) {
            clean_width += clean_x; // Reducir ancho
            clean_x = 0;
        }
        if (clean_y < 45) { // No limpiar sobre la UI
            clean_height += (clean_y - 45);
            clean_y = 45;
        }
        if (clean_x + clean_width > SCREEN_WIDTH) {
            clean_width = SCREEN_WIDTH - clean_x;
        }
        if (clean_y + clean_height > SCREEN_HEIGHT) {
            clean_height = SCREEN_HEIGHT - clean_y;
        }
        
        // Solo limpiar si el área es válida
        if (clean_width > 0 && clean_height > 0) {
            drawRect(clean_x, clean_y, clean_width, clean_height, COLOR_GREEN);
        }
    }
    
    // Limpiar paddle 2 SOLO si se movió o rotó 
    if (gameMode == MODE_MULTIPLAYER && last_paddle2_x != -1 && 
        (last_paddle2_x != paddle2.x || last_paddle2_y != paddle2.y || last_angle2 != paddle2.aim_angle)) {
        int old_center_x = last_paddle2_x + paddle2.width/2;
        int old_center_y = last_paddle2_y + paddle2.height/2;
        
        int clean_x = old_center_x - 60;
        int clean_y = old_center_y - 60;
        int clean_width = 120;
        int clean_height = 120;
        
        // Ajustar coordenadas para que no se salgan de la pantalla
        if (clean_x < 0) {
            clean_width += clean_x; // Reducir ancho
            clean_x = 0;
        }
        if (clean_y < 45) { // No limpiar sobre la UI
            clean_height += (clean_y - 45);
            clean_y = 45;
        }
        if (clean_x + clean_width > SCREEN_WIDTH) {
            clean_width = SCREEN_WIDTH - clean_x;
        }
        if (clean_y + clean_height > SCREEN_HEIGHT) {
            clean_height = SCREEN_HEIGHT - clean_y;
        }
        
        // Solo limpiar si el área es válida
        if (clean_width > 0 && clean_height > 0) {
            drawRect(clean_x, clean_y, clean_width, clean_height, COLOR_GREEN);
        }
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


void drawUI(void) {
    // Calcular posiciones proporcionales a la pantalla
    int left_box_width = 120;   // Ancho fijo para "NIVEL: X"
    int right_box_width = 140;  // Ancho fijo para "GOLPES: XX"
    
    // Caja del nivel (izquierda)
    drawRect(10, 5, left_box_width, 25, COLOR_BLACK);
    drawRect(12, 7, left_box_width - 4, 21, COLOR_WHITE);
    
    // Texto centrado en la caja
    int nivel_text_width = getTextWidth("NIVEL: ") + getTextWidth("9"); // Máximo 1 dígito
    int nivel_x = 12 + (left_box_width - 4 - nivel_text_width) / 2;
    drawSimpleText("NIVEL: ", nivel_x, 12);
    drawNumber(currentLevel, nivel_x + getTextWidth("NIVEL: "), 12);
    
    // Caja de golpes (derecha)
    int right_box_x = SCREEN_WIDTH - right_box_width - 10;
    drawRect(right_box_x, 5, right_box_width, 25, COLOR_BLACK);
    drawRect(right_box_x + 2, 7, right_box_width - 4, 21, COLOR_WHITE);
    
    // Texto centrado en la caja
    int golpes_text_width = getTextWidth("GOLPES: ") + getTextWidth("999"); // Máximo 3 dígitos
    int golpes_x = right_box_x + 2 + (right_box_width - 4 - golpes_text_width) / 2;
    drawSimpleText("GOLPES: ", golpes_x, 12);
    drawNumber(hits, golpes_x + getTextWidth("GOLPES: "), 12);
}

// Función para dibujar texto
void drawSimpleText(const char* text, int x, int y) {
    for (int i = 0; text[i] != '\0'; i++) {
        const unsigned char* pattern = getCharPattern(text[i]);
        drawCharPattern(pattern, x + (i * 9), y);   // Espaciado de 9 píxeles
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
            int base_impact = 300; 
            int speed_multiplier = paddle1_speed * 100;
            int final_speed = base_impact + speed_multiplier;
            
            if (final_speed < 300) final_speed = 300;
            if (final_speed > 2000) final_speed = 2000; 
            
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
            int base_impact = 300; // impulso inicial
            int speed_multiplier = paddle2_speed * 100;
            int final_speed = base_impact + speed_multiplier;
            
            if (final_speed < 300) final_speed = 300;
            if (final_speed > 2000) final_speed = 2000; // Límite
            
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
    
    ball.x += ball.vx / 100; 
    ball.y += ball.vy / 100;
    
    ball.vx = (ball.vx * 996) / 1000; 
    ball.vy = (ball.vy * 996) / 1000;
    
    // Parar cuando la velocidad es baja
    if (abs(ball.vx) < 6 && abs(ball.vy) < 6) { 
        ball.vx = 0;
        ball.vy = 0;
    }
    
    // Rebotes en las paredes
    if (ball.x - ball.size <= 0) {
        ball.x = ball.size;
        ball.vx = -ball.vx * 87 / 100;
        playSound(400, 60);
    }
    if (ball.x + ball.size >= SCREEN_WIDTH) {  
        ball.x = SCREEN_WIDTH - ball.size;
        ball.vx = -ball.vx * 87 / 100;
        playSound(400, 60);
    }
    if (ball.y - ball.size <= 45) { 
        ball.y = 45 + ball.size;
        ball.vy = -ball.vy * 87 / 100;
        playSound(400, 60);
    }
    if (ball.y + ball.size >= SCREEN_HEIGHT) {  
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
    
    int box_width = SCREEN_WIDTH * 3 / 5;
    int box_height = 100;
    int box_x = (SCREEN_WIDTH - box_width) / 2;
    int box_y = (SCREEN_HEIGHT - box_height) / 2;
    
    drawRect(box_x, box_y, box_width, box_height, COLOR_BLUE);
    drawRect(box_x + 5, box_y + 5, box_width - 10, box_height - 10, COLOR_WHITE);
    
    // Centrar "NIVEL COMPLETADO!"
    const char* texto1 = "NIVEL COMPLETADO!";
    int text1_width = getTextWidth(texto1);
    int text1_x = box_x + (box_width - text1_width) / 2;
    drawSimpleText(texto1, text1_x, box_y + 20);
    
    // "GOLPES: " + número
    const char* golpes_label = "GOLPES: ";
    int label_width = getTextWidth(golpes_label);
    int number_width = getTextWidth("999"); // Estimación para centrado
    int total_width = label_width + number_width;
    int text2_x = box_x + (box_width - total_width) / 2;
    
    drawSimpleText(golpes_label, text2_x, box_y + 50);
    drawNumber(hits, text2_x + label_width, box_y + 50);
    
    // Pausa
    for (volatile int i = 0; i < 300000000; i++);
    
    resetBall();
    placeHoleRandomly();
    hits = 0;
    gameState = GAME_PLAYING;
    fillScreen(COLOR_GREEN);
}

void showHoleMessage(void) {
    playWinSound();
    
    fillScreen(COLOR_YELLOW);
    
    int box_width = SCREEN_WIDTH * 3 / 5;
    int box_height = 100;
    int box_x = (SCREEN_WIDTH - box_width) / 2;
    int box_y = (SCREEN_HEIGHT - box_height) / 2;
    
    drawRect(box_x, box_y, box_width, box_height, COLOR_BLUE);
    drawRect(box_x + 5, box_y + 5, box_width - 10, box_height - 10, COLOR_WHITE);
    
    // Centrar "JUEGO COMPLETADO!"
    const char* texto1 = "JUEGO COMPLETADO!";
    int text1_width = getTextWidth(texto1);
    int text1_x = box_x + (box_width - text1_width) / 2;
    drawSimpleText(texto1, text1_x, box_y + 20);
    
    // "GOLPES TOTALES: " + número - CORREGIDO
    const char* golpes_label = "GOLPES TOTALES: ";
    int label_width = getTextWidth(golpes_label);
    int number_width = getTextWidth("999"); // Estimación para centrado
    int total_width = label_width + number_width;
    int text2_x = box_x + (box_width - total_width) / 2;
    
    drawSimpleText(golpes_label, text2_x, box_y + 50);
    drawNumber(hits, text2_x + label_width, box_y + 50);
    
    // Pausa
    for (volatile int i = 0; i < 300000000; i++);
    
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
                int pixel_x = centerX + x;
                int pixel_y = centerY + y;
                
                
                if (pixel_x >= 0 && pixel_x < SCREEN_WIDTH && 
                    pixel_y >= 0 && pixel_y < SCREEN_HEIGHT) {
                    putPixel(color, pixel_x, pixel_y);
                }
            }
        }
    }
}

void drawNumber(int number, int x, int y) {
    char numStr[16];
    int i = 0;
    
    if (number == 0) {
        numStr[0] = '0';
        numStr[1] = '\0';
    } else {
        // Manejar números negativos
        int isNegative = 0;
        if (number < 0) {
            isNegative = 1;
            number = -number;
        }
        
        while (number > 0) {
            numStr[i++] = '0' + (number % 10);
            number /= 10;
        }
        
        if (isNegative) {
            numStr[i++] = '-';
        }
        
        numStr[i] = '\0';
        
        // Invertir string
        for (int j = 0; j < i/2; j++) {
            char temp = numStr[j];
            numStr[j] = numStr[i-1-j];
            numStr[i-1-j] = temp;
        }
    }
    
    // Usar drawSimpleText para renderizar
    drawSimpleText(numStr, x, y);
}

void drawAimArrow(Paddle* paddle) {
    int paddle_center_x = paddle->x + paddle->width/2;
    int paddle_center_y = paddle->y + paddle->height/2;
    
    
    int angle_index = (paddle->aim_angle / 10) % 36;
    int dir_x = cos_table_fine[angle_index];
    int dir_y = sin_table_fine[angle_index];
    
    // Flecha con longitud optimizada para visibilidad
    int arrow_length = 35; // Longitud perfecta
    int tip_x = paddle_center_x + (dir_x * arrow_length) / 100;
    int tip_y = paddle_center_y + (dir_y * arrow_length) / 100;
    
    
    if (tip_x < 10 || tip_x >= SCREEN_WIDTH - 10 || 
        tip_y < 55 || tip_y >= SCREEN_HEIGHT - 10) {
        // Si la flecha se saldría de límites, acortar su longitud
        arrow_length = 20; // Longitud más corta para bordes
        tip_x = paddle_center_x + (dir_x * arrow_length) / 100;
        tip_y = paddle_center_y + (dir_y * arrow_length) / 100;
    }
    
    // Línea principal más suave y visible
    for (int t = 0; t < arrow_length; t += 2) { // Menos puntos para mejor rendimiento
        int line_x = paddle_center_x + (dir_x * t) / 100;
        int line_y = paddle_center_y + (dir_y * t) / 100;
        
        
        if (line_x >= 2 && line_x < SCREEN_WIDTH - 2 && 
            line_y >= 47 && line_y < SCREEN_HEIGHT - 2) {
            
            // Línea más suave con mejor antialiasing
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    int pixel_x = line_x + dx;
                    int pixel_y = line_y + dy;
                    
                    if (pixel_x >= 0 && pixel_x < SCREEN_WIDTH && 
                        pixel_y >= 45 && pixel_y < SCREEN_HEIGHT) {
                        putPixel(COLOR_RED, pixel_x, pixel_y);
                    }
                }
            }
        }
    }
    
    // Punta de flecha bien proporcionada - SOLO si está dentro de límites
    if (tip_x >= 15 && tip_x < SCREEN_WIDTH - 15 && 
        tip_y >= 60 && tip_y < SCREEN_HEIGHT - 15) {
        
        int perp_x = -dir_y;
        int perp_y = dir_x;
        
        int head1_x = tip_x - (dir_x * 10) / 100 + (perp_x * 7) / 100;
        int head1_y = tip_y - (dir_y * 10) / 100 + (perp_y * 7) / 100;
        int head2_x = tip_x - (dir_x * 10) / 100 - (perp_x * 7) / 100;
        int head2_y = tip_y - (dir_y * 10) / 100 - (perp_y * 7) / 100;
        
        // Solo dibujar líneas de la punta si están dentro de límites
        if (head1_x >= 0 && head1_x < SCREEN_WIDTH && head1_y >= 45 && head1_y < SCREEN_HEIGHT &&
            head2_x >= 0 && head2_x < SCREEN_WIDTH && head2_y >= 45 && head2_y < SCREEN_HEIGHT) {
            drawLine(tip_x, tip_y, head1_x, head1_y, COLOR_RED);
            drawLine(tip_x, tip_y, head2_x, head2_y, COLOR_RED);
        }
        
        // Círculo en la punta optimizado - con verificación
        drawCircle(tip_x, tip_y, 4, COLOR_RED);
    }
}

void drawLine(int x1, int y1, int x2, int y2, uint32_t color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    int x = x1, y = y1;
    
    while (1) {
        
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                int pixel_x = x + i;
                int pixel_y = y + j;
                
                // Verificar límites antes de dibujar
                if (pixel_x >= 0 && pixel_x < SCREEN_WIDTH && 
                    pixel_y >= 0 && pixel_y < SCREEN_HEIGHT) {
                    putPixel(color, pixel_x, pixel_y);
                }
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

const unsigned char* getCharPattern(char c) {
    if (c >= 32 && c <= 126) {
        return font_8x12[c - 32];   // ASCII 32 es espacio, índice 0
    }
    return font_8x12[0];    // Carácter por defecto (espacio)
}

int getTextWidth(const char* text) {
    int len = strlen(text);
    return len * 9 - 1;   // 8 píxeles + 1 de espaciado, menos 1 al final
}

// #include "./../include/libc.h"
// #include "./../include/golf.h" // Se incluyó el archivo de cabecera que contiene los prototipos de handleInputImproved y processMovementImproved

// int SCREEN_HEIGHT = 768;
// int SCREEN_WIDTH = 1024;

// static Ball ball;
// static Paddle paddle1, paddle2;
// static Hole hole;
// static int hits = 0;
// static int gameRunning = 1;
// static int gameState = GAME_MENU;
// static int gameMode = MODE_SINGLE;
// static int currentLevel = 1;
// static int maxLevels = 3;

// static unsigned int random_seed = 1;

// // Tablas trigonométricas de alta precisión (cada 10 grados) - 36 posiciones
// static int cos_table_fine[36] = {
//     100, 98, 94, 87, 77, 64, 50, 34, 17, 0, -17, -34, -50, -64, -77, -87, -94, -98,
//     -100, -98, -94, -87, -77, -64, -50, -34, -17, 0, 17, 34, 50, 64, 77, 87, 94, 98
// };

// static int sin_table_fine[36] = {
//     0, 17, 34, 50, 64, 77, 87, 94, 98, 100, 98, 94, 87, 77, 64, 50, 34, 17,
//     0, -17, -34, -50, -64, -77, -87, -94, -98, -100, -98, -94, -87, -77, -64, -50, -34, -17
// };

// // ================ ESTADO DE TECLAS MEJORADO ================
// // Estado persistente de las teclas para movimiento simultáneo
// typedef struct {
//     int p1_left;
//     int p1_right; 
//     int p1_up;
//     int p2_left;
//     int p2_right;
//     int p2_up;
//     int escape;
// } KeyState;

// static KeyState keyState = {0};

// // Cooldowns para rotación suave
// static int p1_rotate_cooldown = 0;
// static int p2_rotate_cooldown = 0;

// void startGolfGame(void) {
//     clearScreen();
//     gameState = GAME_MENU;
//     gameRunning = 1;
    
//     // Inicializar estado de teclas
//     keyState = (KeyState){0};
    
//     while (gameRunning) {
//         switch (gameState) {
//             case GAME_MENU:
//                 showMenu();
//                 break;
//             case GAME_PLAYING:
//                 handleInputImproved();
//                 processMovementImproved();
//                 updateGame();
//                 drawGame();
//                 break;
//             case GAME_LEVEL_COMPLETE:
//                 showLevelComplete();
//                 break;
//             case GAME_OVER:
//                 gameRunning = 0;
//                 break;
//         }
        
//         // Optimizado para máxima fluidez
//         for (volatile int i = 0; i < 5000; i++);
//     }
// }

// // ================ INPUT HANDLING MEJORADO ================
// void handleInputImproved(void) {
//     char key;
    
//     // Decrementar cooldowns
//     if (p1_rotate_cooldown > 0) p1_rotate_cooldown--;
//     if (p2_rotate_cooldown > 0) p2_rotate_cooldown--;
    
//     // Primero, asumimos que ninguna tecla de movimiento está presionada
//     // Esto es importante para que los paddles frenen al soltar las teclas
//     keyState.p1_up = 0; 
//     keyState.p2_up = 0;
//     keyState.p1_left = 0; // También reseteamos las rotaciones para un control más preciso
//     keyState.p1_right = 0;
//     keyState.p2_left = 0;
//     keyState.p2_right = 0;


//     // Leer TODAS las teclas disponibles en este frame
//     while ((key = getKeyNonBlocking()) != 0) {
//         switch (key) {
//             case 27: // ESC
//                 keyState.escape = 1;
//                 break;
                
//             // ============ JUGADOR 1 (Flechas) ============
//             case 75: // Flecha izquierda
//                 keyState.p1_left = 1;
//                 break;
//             case 77: // Flecha derecha 
//                 keyState.p1_right = 1;
//                 break;
//             case 72: // Flecha arriba
//                 keyState.p1_up = 1;
//                 break;
                
//             // ============ JUGADOR 2 (WASD) ============
//             case 'a': case 'A':
//                 if (gameMode == MODE_MULTIPLAYER) {
//                     keyState.p2_left = 1;
//                 }
//                 break;
//             case 'd': case 'D':
//                 if (gameMode == MODE_MULTIPLAYER) {
//                     keyState.p2_right = 1;
//                 }
//                 break;
//             case 'w': case 'W':
//                 if (gameMode == MODE_MULTIPLAYER) {
//                     keyState.p2_up = 1;
//                 }
//                 break;
//         }
//     }
    
//     // Procesar ESC
//     if (keyState.escape) {
//         gameState = GAME_MENU;
//         keyState.escape = 0;
//         return;
//     }
// }

// // ================ MOVEMENT PROCESSING MEJORADO ================
// void processMovementImproved(void) {
//     // ============ PROCESAR JUGADOR 1 ============
    
//     // Rotación del jugador 1 (independiente del movimiento)
//     if (keyState.p1_left && p1_rotate_cooldown == 0) {
//         paddle1.aim_angle = (paddle1.aim_angle - 10 + 360) % 360;
//         p1_rotate_cooldown = 1; // Cooldown mínimo para suavidad
//         // keyState.p1_left se resetea en handleInputImproved
//     }
    
//     if (keyState.p1_right && p1_rotate_cooldown == 0) {
//         paddle1.aim_angle = (paddle1.aim_angle + 10) % 360;
//         p1_rotate_cooldown = 1;
//         // keyState.p1_right se resetea en handleInputImproved
//     }
    
//     // Movimiento del jugador 1 (continuo mientras se mantiene presionada)
//     if (keyState.p1_up) {
//         int angle_index = (paddle1.aim_angle / 10) % 36;
//         int angle_x = cos_table_fine[angle_index];
//         int angle_y = sin_table_fine[angle_index];
        
//         int new_x = paddle1.x + (angle_x * 12) / 100; // Velocidad aumentada
//         int new_y = paddle1.y + (angle_y * 12) / 100;
        
//         // Verificar límites
//         if (new_x >= 0 && new_x + paddle1.width <= SCREEN_WIDTH &&
//             new_y >= 45 && new_y + paddle1.height <= SCREEN_HEIGHT) {
//             paddle1.x = new_x;
//             paddle1.y = new_y;
//         }
//     }
    
//     // ============ PROCESAR JUGADOR 2 (si está en modo multiplayer) ============
//     if (gameMode == MODE_MULTIPLAYER) {
        
//         // Rotación del jugador 2 (independiente del movimiento)
//         if (keyState.p2_left && p2_rotate_cooldown == 0) {
//             paddle2.aim_angle = (paddle2.aim_angle - 10 + 360) % 360;
//             p2_rotate_cooldown = 1;
//             // keyState.p2_left se resetea en handleInputImproved
//         }
        
//         if (keyState.p2_right && p2_rotate_cooldown == 0) {
//             paddle2.aim_angle = (paddle2.aim_angle + 10) % 360;
//             p2_rotate_cooldown = 1;
//             // keyState.p2_right se resetea en handleInputImproved
//         }
        
//         // Movimiento del jugador 2 (continuo mientras se mantiene presionada)
//         if (keyState.p2_up) {
//             int angle_index = (paddle2.aim_angle / 10) % 36;
//             int angle_x = cos_table_fine[angle_index];
//             int angle_y = sin_table_fine[angle_index];
            
//             int new_x = paddle2.x + (angle_x * 12) / 100; // Velocidad aumentada
//             int new_y = paddle2.y + (angle_y * 12) / 100;
            
//             // Verificar límites
//             if (new_x >= 0 && new_x + paddle2.width <= SCREEN_WIDTH &&
//                 new_y >= 45 && new_y + paddle2.height <= SCREEN_HEIGHT) {
//                 paddle2.x = new_x;
//                 paddle2.y = new_y;
//             }
//         }
//     }
// }

// // ================ RESTO DE FUNCIONES SIN CAMBIOS ================
// // Función auxiliar para dibujar patrones de caracteres de forma eficiente
// void drawCharPattern(const unsigned char* pattern, int x, int y) {
//     for (int row = 0; row < 12; row++) {
//         unsigned char byte = pattern[row];
//         for (int col = 0; col < 8; col++) {
//             if (byte & (0x80 >> col)) {    // Bit más significativo primero
//                 putPixel(COLOR_BLACK, x + col, y + row);
//             }
//         }
//     }
// }

// void showMenu(void) {
//     static int menuInitialized = 0;
    
//     if (!menuInitialized) {
//         fillScreen(COLOR_BLACK);
        
//         print("========================================\n");
//         print("         BIENVENIDO A PONGIS-GOLF       \n");
//         print("========================================\n");
//         print("\n");
//         print("Selecciona el modo de juego:\n");
//         print("\n");
//         print("   [1] - Un Jugador\n");
//         print("         Controles: Flechas\n");
//         print("         Flechas laterales: Rotar palo\n");
//         print("         Flecha superior   : Mover palo\n");
//         print("\n");
//         print("   [2] - Dos Jugadores\n");
//         print("         Jugador 1: Flechas \n");
//         print("         Jugador 2: WASD (A D W)\n");
//         print("         *** MOVIMIENTO SIMULTANEO ***\n");
//         print("\n");
//         print("   [ESC] - Volver al menu\n");
//         print("\n");
//         print("Niveles: 3 (hoyo cada vez mas pequeno)\n");
//         print("Objetivo: Meter la pelota en el hoyo\n");
//         print("\n");
        
//         menuInitialized = 1;
//     }
    
//     char key = getKeyNonBlocking();
    
//     if (key == '1') {
//         gameMode = MODE_SINGLE;
//         currentLevel = 1;
//         gameState = GAME_PLAYING;
//         playBeep();
//         initGame();
//     } else if (key == '2') {
//         gameMode = MODE_MULTIPLAYER;
//         currentLevel = 1;
//         gameState = GAME_PLAYING;
//         playBeep();
//         initGame();
//     } else if (key == 27) {
//         gameRunning = 0;
//     }
// }

// // Generador de números aleatorios mejorado
// int simpleRandom(int min, int max) {
//     random_seed = random_seed * 1664525 + 1013904223; // Mejores constantes
//     return min + (random_seed % (max - min + 1));
// }

// // Tamaños de hoyo más progresivos
// int getHoleSize(int level) {
//     switch (level) {
//         case 1: return 35;  // Más grande para empezar
//         case 2: return 25;  // Tamaño intermedio
//         case 3: return 18;  // Más pequeño pero no imposible
//         default: return 18;
//     }
// }

// void placeHoleRandomly(void) {
//     hole.size = getHoleSize(currentLevel);
//     int margin = hole.size + 50;
    
//     do {
//         hole.x = simpleRandom(margin, SCREEN_WIDTH - margin);
//         hole.y = simpleRandom(margin + 45, SCREEN_HEIGHT - margin);
        
//         // Verificar distancia mínima de la pelota
//         int dx = hole.x - ball.x;
//         int dy = hole.y - ball.y;
//         int distance_sq = dx*dx + dy*dy;
//         int min_distance = 250;
        
//         // Verificar distancia de los paddles (proporcional a pantalla)
//         int dx1 = hole.x - (paddle1.x + paddle1.width/2);
//         int dy1 = hole.y - (paddle1.y + paddle1.height/2);
//         int dist1_sq = dx1*dx1 + dy1*dy1;
        
//         int dx2 = 999, dy2 = 999, dist2_sq = 999999;
//         if (gameMode == MODE_MULTIPLAYER) {
//             dx2 = hole.x - (paddle2.x + paddle2.width/2);
//             dy2 = hole.y - (paddle2.y + paddle2.height/2);
//             dist2_sq = dx2*dx2 + dy2*dy2;
//         }
        
//         // Verificar conflictos con UI (proporcional a pantalla)
//         int ui_conflict = 0;
//         if (hole.y < 50) { // Área de UI
//             // Calcular áreas de UI proporcionales
//             int left_ui_width = SCREEN_WIDTH / 6;   // ~17% para UI izquierda
//             int right_ui_width = SCREEN_WIDTH / 4;  // ~25% para UI derecha
            
//             if ((hole.x < left_ui_width) || (hole.x > SCREEN_WIDTH - right_ui_width)) {
//                 ui_conflict = 1;
//             }
//         }
        
//         if (distance_sq > min_distance * min_distance && 
//             dist1_sq > 150*150 && dist2_sq > 150*150 && !ui_conflict) {
//             break;
//         }
//     } while (1);
// }

// void initGame(void) {
    
//     // Obtener valores
//     SCREEN_WIDTH = getScreenWidth();
//     SCREEN_HEIGHT = getScreenHeight();
    
//     fillScreen(COLOR_GREEN);
    
//     // Semilla random...
//     random_seed = 12345 + currentLevel * 1000 + hits * 100;
    
//     // Pelota en el centro (usando dimensiones dinámicas)
//     ball.x = SCREEN_WIDTH / 2;
//     ball.y = SCREEN_HEIGHT / 2;
//     ball.vx = 0;
//     ball.vy = 0;
//     ball.size = BALL_SIZE;
    
//     // Paddle 1 - posicionado proporcionalmente
//     paddle1.x = SCREEN_WIDTH / 20;  // 5% del ancho desde el borde izquierdo
//     paddle1.y = SCREEN_HEIGHT / 2 - (BALL_SIZE * 3);
//     if (paddle1.y < 45) paddle1.y = 45; // Asegurar que no esté en la UI
//     paddle1.width = BALL_SIZE * 6;
//     paddle1.height = BALL_SIZE * 6;
//     paddle1.color = COLOR_BLUE;
//     paddle1.aim_angle = 0;
    
//     // Paddle 2 - posicionado proporcionalmente desde el borde derecho
//     if (gameMode == MODE_MULTIPLAYER) {
//         paddle2.x = SCREEN_WIDTH - (SCREEN_WIDTH / 10);    // 10% del ancho desde el borde derecho
//         paddle2.y = SCREEN_HEIGHT / 2 - (BALL_SIZE * 3);
//         if (paddle2.y < 45) paddle2.y = 45; // Asegurar que no esté en la UI
//         paddle2.width = BALL_SIZE * 6;
//         paddle2.height = BALL_SIZE * 6;
//         paddle2.color = COLOR_RED;
//         paddle2.aim_angle = 180;
//     }
    
//     // Colocar hoyo usando dimensiones dinámicas
//     placeHoleRandomly();
    
//     hits = 0;
//     p1_rotate_cooldown = 0;
//     p2_rotate_cooldown = 0;
    
//     // Limpiar estado de teclas
//     keyState = (KeyState){0};
// }

// void drawGame(void) {
//     static int last_ball_x = -1, last_ball_y = -1;
//     static int last_paddle1_x = -1, last_paddle1_y = -1;
//     static int last_paddle2_x = -1, last_paddle2_y = -1;
//     static int last_hole_x = -1, last_hole_y = -1;
//     static int last_angle1 = -1, last_angle2 = -1;
//     static int initialized = 0;
    
//     if (!initialized) {
//         fillScreen(COLOR_GREEN);
//         initialized = 1;
//     }
    
//     // Limpiar posición anterior del hoyo SOLO si cambió
//     if (last_hole_x != -1 && (last_hole_x != hole.x || last_hole_y != hole.y)) {
//         drawCircle(last_hole_x, last_hole_y, getHoleSize(currentLevel) + 3, COLOR_GREEN);
//     }
    
//     // Limpiar posición anterior de la pelota SOLO si se movió
//     if (last_ball_x != -1 && (last_ball_x != ball.x || last_ball_y != ball.y)) {
//         drawCircle(last_ball_x, last_ball_y, ball.size + 2, COLOR_GREEN);
//     }
    
//     // Limpiar paddle 1 SOLO si se movió o rotó
//     if (last_paddle1_x != -1 && (last_paddle1_x != paddle1.x || last_paddle1_y != paddle1.y || last_angle1 != paddle1.aim_angle)) {
//         int old_center_x = last_paddle1_x + paddle1.width/2;
//         int old_center_y = last_paddle1_y + paddle1.height/2;
//         // Limpiar área más grande para incluir la flecha
//         drawRect(old_center_x - 60, old_center_y - 60, 120, 120, COLOR_GREEN);
//     }
    
//     // Limpiar paddle 2 SOLO si se movió o rotó
//     if (gameMode == MODE_MULTIPLAYER && last_paddle2_x != -1 && 
//         (last_paddle2_x != paddle2.x || last_paddle2_y != paddle2.y || last_angle2 != paddle2.aim_angle)) {
//         int old_center_x = last_paddle2_x + paddle2.width/2;
//         int old_center_y = last_paddle2_y + paddle2.height/2;
//         drawRect(old_center_x - 60, old_center_y - 60, 120, 120, COLOR_GREEN);
//     }
    
//     // Dibujar hoyo con borde negro
//     drawCircle(hole.x, hole.y, hole.size + 2, COLOR_BLACK);
//     drawCircle(hole.x, hole.y, hole.size, COLOR_BLACK);
    
//     // Dibujar pelota blanca
//     drawCircle(ball.x, ball.y, ball.size, COLOR_WHITE);
    
//     // Dibujar paddles con sus flechas
//     int paddle1_center_x = paddle1.x + paddle1.width/2;
//     int paddle1_center_y = paddle1.y + paddle1.height/2;
//     drawCircle(paddle1_center_x, paddle1_center_y, paddle1.width/2, paddle1.color);
//     drawAimArrow(&paddle1);
    
//     if (gameMode == MODE_MULTIPLAYER) {
//         int paddle2_center_x = paddle2.x + paddle2.width/2;
//         int paddle2_center_y = paddle2.y + paddle2.height/2;
//         drawCircle(paddle2_center_x, paddle2_center_y, paddle2.width/2, paddle2.color);
//         drawAimArrow(&paddle2);
//     }
    
//     // Dibujar UI siempre al final
//     drawUI();
    
//     // Actualizar posiciones anteriores
//     last_ball_x = ball.x;
//     last_ball_y = ball.y;
//     last_paddle1_x = paddle1.x;
//     last_paddle1_y = paddle1.y;
//     last_angle1 = paddle1.aim_angle;
    
//     if (gameMode == MODE_MULTIPLAYER) {
//         last_paddle2_x = paddle2.x;
//         last_paddle2_y = paddle2.y;
//         last_angle2 = paddle2.aim_angle;
//     }
    
//     last_hole_x = hole.x;
//     last_hole_y = hole.y;
// }

// // UI sin fondo amarillo - solo recuadros flotantes
// void drawUI(void) {
//     // Calcular posiciones proporcionales a la pantalla
//     int left_box_width = 120;   // Ancho fijo para "NIVEL: X"
//     int right_box_width = 140;  // Ancho fijo para "GOLPES: XX"
    
//     // Caja del nivel (izquierda)
//     drawRect(10, 5, left_box_width, 25, COLOR_BLACK);
//     drawRect(12, 7, left_box_width - 4, 21, COLOR_WHITE);
    
//     // Texto centrado en la caja
//     int nivel_text_width = getTextWidth("NIVEL: ") + getTextWidth("9"); // Máximo 1 dígito
//     int nivel_x = 12 + (left_box_width - 4 - nivel_text_width) / 2;
//     drawSimpleText("NIVEL: ", nivel_x, 12);
//     drawNumber(currentLevel, nivel_x + getTextWidth("NIVEL: "), 12);
    
//     // Caja de golpes (derecha)
//     int right_box_x = SCREEN_WIDTH - right_box_width - 10;
//     drawRect(right_box_x, 5, right_box_width, 25, COLOR_BLACK);
//     drawRect(right_box_x + 2, 7, right_box_width - 4, 21, COLOR_WHITE);
    
//     // Texto centrado en la caja
//     int golpes_text_width = getTextWidth("GOLPES: ") + getTextWidth("999"); // Máximo 3 dígitos
//     int golpes_x = right_box_x + 2 + (right_box_width - 4 - golpes_text_width) / 2;
//     drawSimpleText("GOLPES: ", golpes_x, 12);
//     drawNumber(hits, golpes_x + getTextWidth("GOLPES: "), 12);
// }

// // Función para dibujar texto
// void drawSimpleText(const char* text, int x, int y) {
//     for (int i = 0; text[i] != '\0'; i++) {
//         const unsigned char* pattern = getCharPattern(text[i]);
//         drawCharPattern(pattern, x + (i * 9), y);   // Espaciado de 9 píxeles
//     }
// }

// void updateGame(void) {
//     static int last_paddle1_x = -1, last_paddle1_y = -1;
//     static int last_paddle2_x = -1, last_paddle2_y = -1;
//     static int collision_cooldown = 0;
//     static int was_far = 1;
    
//     if (collision_cooldown > 0) {
//         collision_cooldown--;
//     }
    
//     // Detectar colisiones con paddle 1
//     int paddle1_center_x = paddle1.x + paddle1.width/2;
//     int paddle1_center_y = paddle1.y + paddle1.height/2;
//     int dx1 = ball.x - paddle1_center_x;
//     int dy1 = ball.y - paddle1_center_y;
//     int distance1_sq = dx1*dx1 + dy1*dy1;
//     int collision_radius = (ball.size + paddle1.width/2 - 3);
    
//     // Detectar colisiones con paddle 2
//     int distance2_sq = 999999;
//     int dx2 = 0, dy2 = 0;
//     if (gameMode == MODE_MULTIPLAYER) {
//         int paddle2_center_x = paddle2.x + paddle2.width/2;
//         int paddle2_center_y = paddle2.y + paddle2.height/2;
//         dx2 = ball.x - paddle2_center_x;
//         dy2 = ball.y - paddle2_center_y;
//         distance2_sq = dx2*dx2 + dy2*dy2;
//     }
    
//     // Verificar si está lejos para resetear el cooldown
//     int far_distance = (collision_radius + 40) * (collision_radius + 40);
//     if (distance1_sq > far_distance && distance2_sq > far_distance) {
//         was_far = 1;
//     }
    
//     // Calcular velocidad de los paddles
//     int paddle1_speed = 0, paddle2_speed = 0;
//     if (last_paddle1_x != -1) {
//         int dx = paddle1.x - last_paddle1_x;
//         int dy = paddle1.y - last_paddle1_y;
//         paddle1_speed = isqrt(dx*dx + dy*dy);
//     }
//     if (gameMode == MODE_MULTIPLAYER && last_paddle2_x != -1) {
//         int dx = paddle2.x - last_paddle2_x;
//         int dy = paddle2.y - last_paddle2_y;
//         paddle2_speed = isqrt(dx*dx + dy*dy);
//     }
    
//     // Colisión con paddle 1
//     if (distance1_sq < (collision_radius * collision_radius) && 
//         collision_cooldown == 0 && was_far == 1 && paddle1_speed > 0) {
        
//         int dist = isqrt(distance1_sq);
//         if (dist > 1) {
//             int base_impact = 300; // Más impulso inicial
//             int speed_multiplier = paddle1_speed * 100;
//             int final_speed = base_impact + speed_multiplier;
            
//             if (final_speed < 300) final_speed = 300;
//             if (final_speed > 2000) final_speed = 2000; // Límite más alto
            
//             ball.vx = (dx1 * final_speed) / dist;
//             ball.vy = (dy1 * final_speed) / dist;
            
//             hits++;
//             playBeep(); // Sonido al golpear
            
//             // Separar la pelota del paddle
//             int separation = collision_radius + 25;
//             ball.x = paddle1_center_x + (dx1 * separation) / dist;
//             ball.y = paddle1_center_y + (dy1 * separation) / dist;
            
//             collision_cooldown = 20;
//             was_far = 0;
//         }
//     }
    
//     // Colisión con paddle 2
//     if (gameMode == MODE_MULTIPLAYER && 
//         distance2_sq < (collision_radius * collision_radius) && 
//         collision_cooldown == 0 && was_far == 1 && paddle2_speed > 0) {
        
//         int dist = isqrt(distance2_sq);
//         if (dist > 1) {
//             int base_impact = 300; // impulso inicial
//             int speed_multiplier = paddle2_speed * 100;
//             int final_speed = base_impact + speed_multiplier;
            
//             if (final_speed < 300) final_speed = 300;
//             if (final_speed > 2000) final_speed = 2000; // Límite
            
//             ball.vx = (dx2 * final_speed) / dist;
//             ball.vy = (dy2 * final_speed) / dist;
            
//             hits++;
//             playBeep();
            
//             int paddle2_center_x = paddle2.x + paddle2.width/2;
//             int paddle2_center_y = paddle2.y + paddle2.height/2;
//             int separation = collision_radius + 25;
//             ball.x = paddle2_center_x + (dx2 * separation) / dist;
//             ball.y = paddle2_center_y + (dy2 * separation) / dist;
            
//             collision_cooldown = 20;
//             was_far = 0;
//         }
//     }
    
//     // Actualizar posiciones anteriores
//     last_paddle1_x = paddle1.x;
//     last_paddle1_y = paddle1.y;
//     if (gameMode == MODE_MULTIPLAYER) {
//         last_paddle2_x = paddle2.x;
//         last_paddle2_y = paddle2.y;
//     }
    
//     // Física de la pelota mejorada - más fluida y con fricción optimizada
//     ball.x += ball.vx / 100; // Movimiento más fluido
//     ball.y += ball.vy / 100;
    
//     // Fricción optimizada para fluidez
//     ball.vx = (ball.vx * 996) / 1000; // Fricción balanceada
//     ball.vy = (ball.vy * 996) / 1000;
    
//     // Parar cuando la velocidad es baja
//     if (abs(ball.vx) < 6 && abs(ball.vy) < 6) { 
//         ball.vx = 0;
//         ball.vy = 0;
//     }
    
//     // Rebotes en las paredes
//     if (ball.x - ball.size <= 0) {
//         ball.x = ball.size;
//         ball.vx = -ball.vx * 87 / 100;
//         playSound(400, 60);
//     }
//     if (ball.x + ball.size >= SCREEN_WIDTH) {  
//         ball.x = SCREEN_WIDTH - ball.size;
//         ball.vx = -ball.vx * 87 / 100;
//         playSound(400, 60);
//     }
//     if (ball.y - ball.size <= 45) { 
//         ball.y = 45 + ball.size;
//         ball.vy = -ball.vy * 87 / 100;
//         playSound(400, 60);
//     }
//     if (ball.y + ball.size >= SCREEN_HEIGHT) {  
//         ball.y = SCREEN_HEIGHT - ball.size;
//         ball.vy = -ball.vy * 87 / 100;
//         playSound(400, 60);
//     }
    
//     // Verificar si la pelota entra en el hoyo
//     int dx = ball.x - hole.x;
//     int dy = ball.y - hole.y;
//     int distance_sq = dx*dx + dy*dy;
//     int hole_radius = hole.size - ball.size;
    
//     if (distance_sq < (hole_radius * hole_radius) && hole_radius > 0) {
//         if (currentLevel < maxLevels) {
//             currentLevel++;
//             gameState = GAME_LEVEL_COMPLETE;
//         } else {
//             showHoleMessage();
//             gameState = GAME_MENU;
//         }
//     }
// }

// void showLevelComplete(void) {
//     playWinSound();
    
//     fillScreen(COLOR_YELLOW);
    
//     int box_width = SCREEN_WIDTH * 3 / 5;
//     int box_height = 100;
//     int box_x = (SCREEN_WIDTH - box_width) / 2;
//     int box_y = (SCREEN_HEIGHT - box_height) / 2;
    
//     drawRect(box_x, box_y, box_width, box_height, COLOR_BLUE);
//     drawRect(box_x + 5, box_y + 5, box_width - 10, box_height - 10, COLOR_WHITE);
    
//     // Centrar "NIVEL COMPLETADO!"
//     const char* texto1 = "NIVEL COMPLETADO!";
//     int text1_width = getTextWidth(texto1);
//     int text1_x = box_x + (box_width - text1_width) / 2;
//     drawSimpleText(texto1, text1_x, box_y + 20);
    
//     // "GOLPES: " + número - CORREGIDO
//     const char* golpes_label = "GOLPES: ";
//     int label_width = getTextWidth(golpes_label);
//     int number_width = getTextWidth("999"); // Estimación para centrado
//     int total_width = label_width + number_width;
//     int text2_x = box_x + (box_width - total_width) / 2;
    
//     drawSimpleText(golpes_label, text2_x, box_y + 50);
//     drawNumber(hits, text2_x + label_width, box_y + 50);
    
//     // Pausa
//     for (volatile int i = 0; i < 50000000; i++);
    
//     resetBall();
//     placeHoleRandomly();
//     hits = 0;
//     gameState = GAME_PLAYING;
//     fillScreen(COLOR_GREEN);
// }

// void showHoleMessage(void) {
//     playWinSound();
    
//     fillScreen(COLOR_YELLOW);
    
//     int box_width = SCREEN_WIDTH * 3 / 5;
//     int box_height = 100;
//     int box_x = (SCREEN_WIDTH - box_width) / 2;
//     int box_y = (SCREEN_HEIGHT - box_height) / 2;
    
//     drawRect(box_x, box_y, box_width, box_height, COLOR_BLUE);
//     drawRect(box_x + 5, box_y + 5, box_width - 10, box_height - 10, COLOR_WHITE);
    
//     // Centrar "JUEGO COMPLETADO!"
//     const char* texto1 = "JUEGO COMPLETADO!";
//     int text1_width = getTextWidth(texto1);
//     int text1_x = box_x + (box_width - text1_width) / 2;
//     drawSimpleText(texto1, text1_x, box_y + 20);
    
//     // "GOLPES TOTALES: " + número - CORREGIDO
//     const char* golpes_label = "GOLPES TOTALES: ";
//     int label_width = getTextWidth(golpes_label);
//     int number_width = getTextWidth("999"); // Estimación para centrado
//     int total_width = label_width + number_width;
//     int text2_x = box_x + (box_width - total_width) / 2;
    
//     drawSimpleText(golpes_label, text2_x, box_y + 50);
//     drawNumber(hits, text2_x + label_width, box_y + 50);
    
//     // Pausa
//     for (volatile int i = 0; i < 60000000; i++);
    
//     fillScreen(COLOR_GREEN);
// }

// void resetBall(void) {
//     ball.x = SCREEN_WIDTH / 2;
//     ball.y = SCREEN_HEIGHT / 2;
//     ball.vx = 0;
//     ball.vy = 0;
// }

// void drawRect(int x, int y, int width, int height, uint32_t color) {
//     drawRectangle(color, x, y, x + width, y + height);
// }

// void drawCircle(int centerX, int centerY, int radius, uint32_t color) {
//     for (int y = -radius; y <= radius; y++) {
//         for (int x = -radius; x <= radius; x++) {
//             if (x*x + y*y <= radius*radius) {
//                 putPixel(color, centerX + x, centerY + y);
//             }
//         }
//     }
// }

// void drawNumber(int number, int x, int y) {
//     char numStr[16];
//     int i = 0;
    
//     if (number == 0) {
//         numStr[0] = '0';
//         numStr[1] = '\0';
//     } else {
//         // Manejar números negativos
//         int isNegative = 0;
//         if (number < 0) {
//             isNegative = 1;
//             number = -number;
//         }
        
//         while (number > 0) {
//             numStr[i++] = '0' + (number % 10);
//             number /= 10;
//         }
        
//         if (isNegative) {
//             numStr[i++] = '-';
//         }
        
//         numStr[i] = '\0';
        
//         // Invertir string
//         for (int j = 0; j < i/2; j++) {
//             char temp = numStr[j];
//             numStr[j] = numStr[i-1-j];
//             numStr[i-1-j] = temp;
//         }
//     }
    
//     // Usar drawSimpleText para renderizar
//     drawSimpleText(numStr, x, y);
// }

// void drawAimArrow(Paddle* paddle) {
//     int paddle_center_x = paddle->x + paddle->width/2;
//     int paddle_center_y = paddle->y + paddle->height/2;
    
//     // Usar las nuevas tablas para flechas más precisas
//     int angle_index = (paddle->aim_angle / 10) % 36;
//     int dir_x = cos_table_fine[angle_index];
//     int dir_y = sin_table_fine[angle_index];
    
//     // Flecha con longitud optimizada para visibilidad
//     int arrow_length = 35; // Longitud perfecta
//     int tip_x = paddle_center_x + (dir_x * arrow_length) / 100;
//     int tip_y = paddle_center_y + (dir_y * arrow_length) / 100;
    
//     // Línea principal más suave y visible
//     for (int t = 0; t < arrow_length; t += 1) {
//         int line_x = paddle_center_x + (dir_x * t) / 100;
//         int line_y = paddle_center_y + (dir_y * t) / 100;
        
//         // Línea más suave con mejor antialiasing
//         for (int dx = -1; dx <= 1; dx++) {
//             for (int dy = -1; dy <= 1; dy++) {
//                 putPixel(COLOR_RED, line_x + dx, line_y + dy);
//             }
//         }
//     }
    
//     // Punta de flecha bien proporcionada
//     int perp_x = -dir_y;
//     int perp_y = dir_x;
    
//     int head1_x = tip_x - (dir_x * 10) / 100 + (perp_x * 7) / 100; // Proporcionada
//     int head1_y = tip_y - (dir_y * 10) / 100 + (perp_y * 7) / 100;
//     int head2_x = tip_x - (dir_x * 10) / 100 - (perp_x * 7) / 100;
//     int head2_y = tip_y - (dir_y * 10) / 100 - (perp_y * 7) / 100;
    
//     drawLine(tip_x, tip_y, head1_x, head1_y, COLOR_RED);
//     drawLine(tip_x, tip_y, head2_x, head2_y, COLOR_RED);
    
//     // Círculo en la punta optimizado
//     drawCircle(tip_x, tip_y, 4, COLOR_RED); // Tamaño perfecto
// }

// void drawLine(int x1, int y1, int x2, int y2, uint32_t color) {
//     int dx = abs(x2 - x1);
//     int dy = abs(y2 - y1);
//     int sx = (x1 < x2) ? 1 : -1;
//     int sy = (y1 < y2) ? 1 : -1;
//     int err = dx - dy;
    
//     int x = x1, y = y1;
    
//     while (1) {
//         // Línea más gruesa
//         for (int i = -1; i <= 1; i++) {
//             for (int j = -1; j <= 1; j++) {
//                 putPixel(color, x + i, y + j);
//             }
//         }
        
//         if (x == x2 && y == y2) break;
        
//         int e2 = 2 * err;
//         if (e2 > -dy) {
//             err -= dy;
//             x += sx;
//         }
//         if (e2 < dx) {
//             err += dx;
//             y += sy;
//         }
//     }
// }

// const unsigned char* getCharPattern(char c) {
//     if (c >= 32 && c <= 126) {
//         return font_8x12[c - 32];   // ASCII 32 es espacio, índice 0
//     }
//     return font_8x12[0];    // Carácter por defecto (espacio)
// }

// int getTextWidth(const char* text) {
//     int len = strlen(text);
//     return len * 9 - 1;   // 8 píxeles + 1 de espaciado, menos 1 al final
// }