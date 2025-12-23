#include <iostream>
#include <ncurses.h>

int main() {
    // Initialize ncurses
    initscr(); // Start ncurses mode
    cbreak(); // Disable line buffering
    noecho(); // Don't echo user input

    // Print a welcome message
    printw("Welcome to the ncurses text-based UI!\n");
    printw("Press any key to exit...\n");

    // Refresh the screen to show the message
    refresh();

    // Wait for user input
    getch();

    // End ncurses mode
    endwin();

    return 0;
}