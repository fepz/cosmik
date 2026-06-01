#include <ncurses.h>
#include <locale.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

bool game_over = false;

bool user_quit = false;

struct recursos {
    int combustible;
    int oxigeno;
    int deuterio;
    int kernelio;
    int semaforita;
    int mutexio;
    pthread_mutex_t mutex;
};

struct recursos minerales;

void* temporizador_oxigeno(void* arg) {
    while (!user_quit) {
        sleep(1);

        pthread_mutex_lock(&minerales.mutex);
        
        if (minerales.oxigeno > 0) {
            minerales.oxigeno--;
        }
        
        if (minerales.oxigeno == 0) {
            game_over = true;
            pthread_mutex_unlock(&minerales.mutex);
            break;
        }

        pthread_mutex_unlock(&minerales.mutex);
    }
    return NULL;
}

void game_over_screen()
{
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    // Limpiar pantalla para mostrar el mensaje de fin de juego limpio
    erase();
    box(stdscr, 0, 0);

    char* text = "GAME OVER";
    int center_x = (max_x - strlen(text)) / 2;
    int center_y = max_y / 2;

    // Activar el color rojo, imprimir centrado y desactivar el color
    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(center_y, center_x, "%s", text);
    attroff(COLOR_PAIR(1) | A_BOLD);

    refresh();

    // Volver a configurar getch() a modo bloqueante para esperar la pulsación definitiva de salida
    nodelay(stdscr, FALSE);
    getch();
}

int main() {
    minerales.combustible = 90;
    minerales.oxigeno = 5;
    minerales.deuterio = 155;
    minerales.kernelio = 78;
    minerales.semaforita = 30;
    minerales.mutexio = 143;

    pthread_mutex_init(&minerales.mutex, NULL);

    // Inicializar soporte local para caracteres especiales (UTF-8)
    setlocale(LC_ALL, "");

    // Inicializar la pantalla de ncurses
    initscr();
    cbreak();             // Deshabilitar el buffering de línea
    noecho();             // No mostrar las teclas presionadas
    curs_set(0);          // Ocultar el cursor para que quede más limpio

    // Inicializar colores
    if (has_colors()) {
        start_color();
        // Definir par de colores 1: Texto Rojo, Fondo Negro
        init_pair(1, COLOR_RED, COLOR_BLACK);
    }

    // Crear el hilo secundario para el oxígeno
    pthread_t hilo_oxigeno;
    if (pthread_create(&hilo_oxigeno, NULL, temporizador_oxigeno, NULL) != 0) {
        endwin();
        return 1;
    }

    // Configurar el modo de lectura no bloqueante para el bucle principal.
    // Esto permite actualizar la interfaz segundo a segundo sin detenerse a esperar que pulsen una tecla.
    nodelay(stdscr, TRUE);

    // Obtener las dimensiones actuales de la terminal
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    while (1) {
        pthread_mutex_lock(&minerales.mutex);

        if (game_over) {
            pthread_mutex_unlock(&minerales.mutex);
            break;
        }

        erase();

        // Dibujar el marco exterior que rodea toda la pantalla
        box(stdscr, 0, 0);
        refresh();

        // --- CALCULAR DIMENSIONES Y COORDENADAS ---
        // Reservamos espacio en la derecha (~28 columnas) y abajo (~4 filas)
        int main_box_width = max_x - 30;
        int main_box_height = max_y - 3;

        int right_col_x = max_x - 27;
        int msg_box_width = 24;
        int msg_box_height = main_box_height - 6;

        // --- VENTANA PRINCIPAL (Izquierda) ---
        // newwin(filas, columnas, y_inicial, x_inicial)
        WINDOW *main_win = newwin(main_box_height, main_box_width, 1, 1);
        box(main_win, 0, 0); // Borde de la ventana principal
        wrefresh(main_win);

        // --- SECCIÓN DE MENSAJES (Derecha - Arriba) ---
        mvprintw(1, right_col_x, "Mensajes");
        
        WINDOW *msg_win = newwin(msg_box_height, msg_box_width, 2, right_col_x);
        box(msg_win, 0, 0); // Borde de la ventana de mensajes
        wrefresh(msg_win);

        // --- SECCIÓN DE INVENTARIO (Derecha - Abajo) ---
        int stats_y = msg_box_height + 2;
        mvprintw(stats_y, right_col_x, "Deuterio:\t%3d", minerales.deuterio);
        mvprintw(stats_y + 1, right_col_x, "Mutexio:\t%3d", minerales.mutexio);
        mvprintw(stats_y + 2, right_col_x, "Semaforita:\t%3d", minerales.semaforita);
        mvprintw(stats_y + 3, right_col_x, "Kernelio:\t%3d", minerales.kernelio);

        // --- SECCIÓN DE RECURSOS INFERIORES (Izquierda - Abajo) ---
        mvprintw(max_y - 2, 2, "Combustible: %d    Oxígeno: %d", minerales.combustible, minerales.oxigeno);
        
        pthread_mutex_unlock(&minerales.mutex);

        // Refrescar stdscr para imprimir todos los textos enviados por mvprintw
        refresh();

        // Limpieza de memoria y restauración de la terminal
        delwin(main_win);
        delwin(msg_win);

        // verificar si el jugador desea terminar
        int ch = getch();
        if (ch == 'q' || ch == 'Q') {
            user_quit = true;
            break; 
        }

        usleep(100000); // pausa para no saturar la CPU 
    }

    pthread_join(hilo_oxigeno, NULL);

    if (!user_quit) {
        game_over_screen();
    }

    // Finalizar ncurses
    endwin();
    return 0;
}
