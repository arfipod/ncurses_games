#include <ncurses.h>
#include <algorithm>

int main() {
    // --- ncurses init ---
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);           // hide cursor

    // Grid size in cells
    const int GRID_W = 32;
    const int GRID_H = 16;

    // "Square" size in terminal characters
    // 2 chars wide x 1 char tall looks closer to square on most terminals
    const int CELL_W = 2;
    const int CELL_H = 1;

    // Window size needed to fit the grid
    const int win_w = GRID_W * CELL_W;
    const int win_h = GRID_H * CELL_H;

    // Center the window in the terminal
    int term_h, term_w;
    getmaxyx(stdscr, term_h, term_w);

    int start_y = std::max(0, (term_h - win_h) / 2);
    int start_x = std::max(0, (term_w - win_w) / 2);

    // If terminal is too small, show message and exit
    if (term_h < win_h + 2 || term_w < win_w + 2) {
        printw("Terminal too small.\n");
        printw("Need at least %dx%d, you have %dx%d.\n", win_w + 2, win_h + 2, term_w, term_h);
        printw("Resize the terminal and run again.\n");
        refresh();
        getch();
        endwin();
        return 1;
    }

    // Create the grid window (+2 to draw a border)
    WINDOW* grid = newwin(win_h + 2, win_w + 2, start_y, start_x);
    box(grid, 0, 0);

    // Draw 32x16 "squares" made of '#'
    // Each cell is drawn as "##"
    for (int y = 0; y < GRID_H; ++y) {
        for (int x = 0; x < GRID_W; ++x) {
            int py = 1 + y * CELL_H;
            int px = 1 + x * CELL_W;

            // Fill the cell area
            mvwaddch(grid, py,     px,     '#');
            mvwaddch(grid, py,     px + 1, '#');
        }
    }

    // Instruction line under the window
    mvprintw(start_y + win_h + 2, start_x, "32x16 grid. Press any key to exit...");
    refresh();
    wrefresh(grid);

    getch();

    delwin(grid);
    endwin();
    return 0;
}
