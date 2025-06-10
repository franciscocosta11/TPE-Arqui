#ifndef GOLF_H
#define GOLF_H

#include <stdint.h>

// CONSTANTES DEL JUEGO

extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;
#define BALL_SIZE 8
#define PADDLE_WIDTH 15
#define PADDLE_HEIGHT 60
#define MAX_SPEED 8

// COLORES DEL JUEGO
#define COLOR_GREEN   0x00AA00
#define COLOR_WHITE   0xFFFFFF
#define COLOR_GRAY    0x808080
#define COLOR_BLACK   0x000000
#define COLOR_RED     0xFF0000
#define COLOR_YELLOW  0xFFFF00
#define COLOR_BLUE    0x0000FF
#define COLOR_CYAN    0x00FFFF
#define COLOR_MAGENTA 0xFF00FF

// ESTADOS DEL JUEGO
#define GAME_MENU 0
#define GAME_PLAYING 1
#define GAME_LEVEL_COMPLETE 2
#define GAME_OVER 3

// MODOS DE JUEGO
#define MODE_SINGLE 1
#define MODE_MULTIPLAYER 2

// ESTRUCTURAS DEL JUEGO

/**
 * @brief Estructura que representa la pelota del golf
 */
typedef struct {
    int x, y;      // Posición en píxeles
    int vx, vy;    // Velocidad en x e y
    int size;      // Tamaño de la pelota
} Ball;

/**
 * @brief Estructura que representa el palo de golf
 */
typedef struct {
    int x, y;           // Posición en píxeles
    int width, height;  // Dimensiones del palo
    uint32_t color;     // Color del palo
    int aim_angle;      // Ángulo de apuntado en grados
} Paddle;

/**
 * @brief Estructura que representa el hoyo 
 */
typedef struct {
    int x, y;      // Posición en píxeles
    int size;      // Tamaño del hoyo
} Hole;

// FUNCIONES PRINCIPALES DEL JUEGO

/**
 * @brief Inicia y ejecuta el juego de golf completo
 */
void startGolfGame(void);

/**
 * @brief Muestra el menú principal del juego
 */
void showMenu(void); // falta corregir todavia 

/**
 * @brief Inicializa todas las variables y estructuras del juego
 */
void initGame(void);

/**
 * @brief Actualiza la lógica del juego (física, colisiones, etc.)
 */
void updateGame(void);

/**
 * @brief Renderiza todos los elementos del juego en pantalla
 */
void drawGame(void);

/**
 * @brief Maneja la entrada del usuario (teclado)
 */
void handleInput(void);

// FUNCIONES DE UTILIDAD Y MATEMÁTICAS


/**
 * @brief Genera un número aleatorio entre min y max
 * @param min Valor mínimo del rango
 * @param max Valor máximo del rango
 */
int simpleRandom(int min, int max);

/**
 * @brief Obtiene el tamaño del hoyo según el nivel de dificultad
 * @param level Nivel actual del juego
 */
int getHoleSize(int level);

// FUNCIONES DE RENDERIZADO

/**
 * @brief Dibuja un rectángulo lleno en pantalla
 * @param x Coordenada X de la esquina superior izquierda
 * @param y Coordenada Y de la esquina superior izquierda
 * @param width Ancho del rectángulo
 * @param height Alto del rectángulo
 * @param color Color del rectángulo en formato RGB hexadecimal
 */
void drawRect(int x, int y, int width, int height, uint32_t color);

/**
 * @brief Dibuja un círculo lleno en pantalla
 * @param centerX Coordenada X del centro
 * @param centerY Coordenada Y del centro
 * @param radius Radio del círculo
 * @param color Color del círculo en formato RGB hexadecimal
 */
void drawCircle(int centerX, int centerY, int radius, uint32_t color);

/**
 * @brief Dibuja una línea entre dos puntos
 * @param x1 Coordenada X del punto inicial
 * @param y1 Coordenada Y del punto inicial
 * @param x2 Coordenada X del punto final
 * @param y2 Coordenada Y del punto final
 * @param color Color de la línea en formato RGB hexadecimal
 */
void drawLine(int x1, int y1, int x2, int y2, uint32_t color);

/**
 * @brief Dibuja un número en pantalla usando patrones de píxeles
 * @param number Número a dibujar
 * @param x Coordenada X donde dibujar
 * @param y Coordenada Y donde dibujar
 */
void drawNumber(int number, int x, int y);

/**
 * @brief Dibuja la flecha de apuntado del palo especificado
 * @param paddle Puntero al palo para el cual dibujar la flecha
 */
void drawAimArrow(Paddle* paddle);

/**
 * @brief Dibuja la interfaz de usuario (UI) del juego
 */
void drawUI(void);

// FUNCIONES DE LÓGICA DEL JUEGO

/**
 * @brief Coloca el hoyo en una posición aleatoria válida
 */
void placeHoleRandomly(void);

/**
 * @brief Muestra el mensaje de victoria cuando se completa el juego
 */
void showHoleMessage(void);

/**
 * @brief Muestra el mensaje de nivel completado
 */
void showLevelComplete(void);

/**
 * @brief Reinicia la pelota a su posición inicial
 */
void resetBall(void);

// Agregar estas declaraciones:
/**
 * @brief Actualiza el estado de las teclas presionadas
 */
void updateKeyStates(void);

/**
 * @brief Verifica si una tecla específica está presionada
 * @param key Código de la tecla a verificar
 * @return 1 si está presionada, 0 si no
 */
int isKeyPressed(unsigned char key);

#endif