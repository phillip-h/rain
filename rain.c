#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <getopt.h> 

#include <ncurses.h>

#define SPLASH "#"
#define PUDDLE "-"

const int EXIT_OK = 0;
const int EXIT_DONE = 1;
const int EXIT_BAD_ARGS = -1;
const int EXIT_BAD_ALLOC = -3;

const char *vn = "0.1";

/* this MUST start with a ' ' character !! */
const char *rain_chars = " .':";

int colors = true;
int color = 4;

int dir = -1;
int rain_chance = 15;
int tick = 60;

int rain_r = 0;
int rain_c = 0;

const char* check_settings(void);
void help(void);
int parse_args(int argc, char **argv);
void version(void);

void start_nc(void);
void end_nc(void);

char pick_rain_char(void);

void add_rain(char **rain);
void draw_rain(char **rain);
void scale_rain(char ***rain);
void update_rain(char **rain);

int main(int argc, char **argv) {
    int arg_rc = parse_args(argc, argv);
    if (arg_rc != EXIT_OK) return arg_rc;

    const char *ok = check_settings();
    if (ok) {
        printf("Error--%s\n", ok);
        return EXIT_BAD_ARGS;
    }

    start_nc();

    srand(time(NULL));
    if (dir < 0) dir = rand() % 3;
    char **rain = NULL;

    while (true) {
        scale_rain(&rain);
        update_rain(rain); 
        draw_rain(rain);

        refresh();
        if (getch() == 'q') break;
        erase();
    }

    end_nc();
    return EXIT_OK;
}

void add_rain(char **rain) {
    for (int c = 0; c < COLS; ++c)
        rain[0][c] = pick_rain_char();

    if (dir == 0) return;

    for (int r = 0; r < LINES; ++r)
        rain[r][dir == 1 ? 0 : COLS - 1] = pick_rain_char();
}

void draw_rain(char **rain) {
    for (int r = 0; r < rain_r - 1; ++r)
       mvprintw(r, 0, rain[r]);  

    /* draw the last row as a puddle */
    for (int c = 0; c < rain_c; ++c) {
        const char *s = rain[LINES - 1][c] == rain_chars[0] ? PUDDLE : SPLASH;
        mvprintw(LINES - 1, c, s);
    }
}

char pick_rain_char(void) {
    if (rand() % rain_chance != 0) return rain_chars[0];

    char c = rain_chars[rand() % strlen(rain_chars)];
    if (c == rain_chars[0])
        c = dir == 0 ? '|' : dir == 1 ? '\\' : '/';

    return c;
}

void scale_rain(char ***rain_ptr) {
    char **rain = *rain_ptr;

    if (LINES > rain_r) {
        rain = realloc(rain, LINES * sizeof(char*));
        for (int r = rain_r; r < LINES; ++r) rain[r] = NULL;
        rain_r = LINES;
    }

    for (int r = 0; r < rain_r; ++r) {
        if (rain[r] == NULL || COLS > rain_c) {
            rain[r] = realloc(rain[r], (COLS + 1));
            memset(rain[r] + rain_c, ' ', COLS - rain_c);
            rain[r][rain_c + 1] = '\0';
        }
    }

    rain_c = COLS;
    *rain_ptr = rain;
}

void update_rain(char **rain) {
    const int off_dest = dir == 1 ? 1 : 0;
    const int off_src  = dir == 2 ? 1 : 0;
    const int len      = dir == 0 ? COLS : COLS - 1;

    /*
     * shift each row down and to the side,
     * if applicable
     */
    for (int r = rain_r - 1; r > 0; --r)
        memcpy(rain[r] + off_dest, rain[r - 1] + off_src, len);

    /* clear the top row and add more rain */
    memset(rain[0], ' ', COLS);
    add_rain(rain);
}

const char* check_settings(void) {
    if (tick <= 0) {
        return "Tick rate must be positive.";
    }

    if (rain_chance <= 0) {
        return "Rain rate must be positive.";
    }

    /* 
     * -1 is the default value, and will result in in
     * a random direction being chosen, so allow it
     */
    if (dir < -1 || dir > 2) {
        return "Rain direction must be 1, 2, or 3.";
    }

    return NULL;
}

void start_nc(void) {
    initscr();
    raw();
    use_default_colors();
    if (colors && has_colors()) {
        start_color();
        init_pair(1, color, -1);
        attron(COLOR_PAIR(1));
    }
    curs_set(0);
    noecho();
    timeout(tick);
}

void end_nc(void) {
    endwin();
    curs_set(1);
}

int parse_args(int argc, char **argv) {
    int c = '\0';
    while ((c = getopt(argc, argv, "d:r:hvmbt:c:")) != -1) {
        switch (c) {
            case 'h':
                help();
                return EXIT_DONE;
            case 'v':
                version();
                return EXIT_DONE;
            case 't':
                tick = strtol(optarg, NULL, 10);
                break;
            case 'c':
               color = strtol(optarg, NULL, 10);
               break;
            case 'd':
               dir = strtol(optarg, NULL, 10);
               break;
            case 'm':
               colors = false;
               break;
            case 'r':
               rain_chance = strtol(optarg, NULL, 10);
               break;
            case '?':
            case ':':
               return EXIT_BAD_ARGS;
            default:
                printf("unknown argument '%c'\n", c);
                return EXIT_BAD_ARGS;
        }
    }

    return EXIT_OK;
}

void help(void) {
    printf("usage: rain [args]\n");
    printf("\n");
    printf("args:\n");
    printf("-c COLOR   \tset the rain color (defaults to 04--blue)\n");
    printf("-d [1,2,3] \tset the rain direction (defaults to random)\n");
    printf("-h         \tprint program usage\n");
    printf("-m         \tset monochrome mode (overrides -c)\n");
    printf("-t RATE    \tset the update rate to RATE milliseconds\n");
    printf("-r RATE    \tset the rain rate (defaults to 15)\n");
    printf("-v         \tprint program version\n");
}

void version(void) {
    printf("rain v%s\n", vn);
    printf("(c) Phillip Heikoop 2017\n");
}
