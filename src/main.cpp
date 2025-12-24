#include <ncurses.h>
#include <algorithm>
#include <cstdlib>
#include <string>

// =====================================================
// Constants
// =====================================================
const int GRID_WIDTH = 32;
const int GRID_HEIGHT = 16;

// Each cell = 1x1 character
const int CELL_W = 1;
const int CELL_H = 1;

// Game speed
const int TICK_MS = 120;

// =====================================================
// Types
// =====================================================
enum class ObjectType
{
    EMPTY,
    SNAKE,
    FOOD
};

char objectTypeToChar(ObjectType t)
{
    switch (t)
    {
    case ObjectType::EMPTY:
        return ' ';
    case ObjectType::SNAKE:
        return '#';
    case ObjectType::FOOD:
        return 'O';
    default:
        return '?';
    }
}

enum class GameState
{
    START,
    PLAYING,
    END
};

enum class Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

// =====================================================
// Game board (board[y][x])
// =====================================================
ObjectType game_board[GRID_HEIGHT][GRID_WIDTH];

// =====================================================
// ncurses objects / geometry (globals for simplicity)
// =====================================================
WINDOW *grid = nullptr;

int term_h = 0, term_w = 0;
int win_h = 0, win_w = 0;
int start_y = 0, start_x = 0;

// =====================================================
// Prototypes
// =====================================================
void initializeBoard();
void shutdownBoard();

void computeLayout();
bool terminalIsBigEnough();

void fillGameBoard(ObjectType objectType);
void drawGameBoard();
void drawCell(int x, int y, ObjectType t);

void drawCenteredTextOnGrid(const std::string &line1,
                            const std::string &line2 = "");

void drawStartScreen();
void drawEndScreen(int points);

void resetGame();
void handleInputPlaying(int ch);
bool updateGameTick(); // returns false when player loses

// =====================================================
// Game variables (replace/expand later for real Snake)
// =====================================================
GameState g_state = GameState::START;

int g_points = 0;
int g_head_x = 0;
int g_head_y = 0;
Direction g_dir = Direction::RIGHT;

// =====================================================
// Implementation
// =====================================================

void initializeBoard()
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    clear();
    refresh();

    computeLayout();

    if (!terminalIsBigEnough())
    {
        clear();
        printw("Terminal too small.\n");
        printw("TERM=%s  term=%dx%d  need=%dx%d\n",
               getenv("TERM"), term_w, term_h, win_w, win_h);
        printw("Resize terminal and run again.\n");
        refresh();
        getch();
        endwin();
        std::exit(1);
    }

    grid = newwin(win_h, win_w, start_y, start_x);
    if (!grid)
    {
        endwin();
        std::exit(1);
    }
}

void shutdownBoard()
{
    if (grid)
    {
        delwin(grid);
        grid = nullptr;
    }
    endwin();
}

void computeLayout()
{
    getmaxyx(stdscr, term_h, term_w);

    const int inner_w = GRID_WIDTH * CELL_W;
    const int inner_h = GRID_HEIGHT * CELL_H;

    win_w = inner_w + 2; // border
    win_h = inner_h + 2; // border

    start_y = std::max(0, (term_h - win_h) / 2);
    start_x = std::max(0, (term_w - win_w) / 2);
}

bool terminalIsBigEnough()
{
    return (term_h >= win_h) && (term_w >= win_w);
}

void fillGameBoard(ObjectType objectType)
{
    for (int y = 0; y < GRID_HEIGHT; ++y)
        for (int x = 0; x < GRID_WIDTH; ++x)
            game_board[y][x] = objectType;
}

void drawCell(int x, int y, ObjectType t)
{
    const char c = objectTypeToChar(t);
    const int py = 1 + y;
    const int px = 1 + x;
    mvwaddch(grid, py, px, c);
}

void drawGameBoard()
{
    werase(grid);

    // ASCII border
    wborder(grid, '|', '|', '-', '-', '+', '+', '+', '+');

    for (int y = 0; y < GRID_HEIGHT; ++y)
        for (int x = 0; x < GRID_WIDTH; ++x)
            drawCell(x, y, game_board[y][x]);

    // Optional: show points (top-left inside the box)
    mvwprintw(grid, 0, 2, " Points: %d ", g_points);

    // Update order: stdscr first (if used), then grid last
    refresh();
    wrefresh(grid);
}

// Centered text inside the grid window (inside border)
void drawCenteredTextOnGrid(const std::string &line1, const std::string &line2)
{
    // inner area size
    const int inner_w = GRID_WIDTH * CELL_W;
    const int inner_h = GRID_HEIGHT * CELL_H;

    const int center_y = 1 + inner_h / 2;
    const int center_x = 1 + inner_w / 2;

    auto drawLine = [&](int y, const std::string &s)
    {
        int x = center_x - (int)s.size() / 2;
        if (x < 1)
            x = 1;
        mvwprintw(grid, y, x, "%s", s.c_str());
    };

    // draw over board (assumes board already drawn/cleared)
    drawLine(center_y - (line2.empty() ? 0 : 1), line1);
    if (!line2.empty())
        drawLine(center_y + 1, line2);
}

void drawStartScreen()
{
    // Make a clean board background
    fillGameBoard(ObjectType::EMPTY);
    drawGameBoard();

    drawCenteredTextOnGrid("S N A K E", "Press any key to start");
    wrefresh(grid);

    // Block until key press in START
    nodelay(stdscr, FALSE);
    getch();
    nodelay(stdscr, TRUE);
}

void drawEndScreen(int points)
{
    // Show final frame
    drawGameBoard();

    drawCenteredTextOnGrid("GAME OVER",
                           "Points: " + std::to_string(points) + "  (press any key)");
    wrefresh(grid);

    // Block until key press in END
    nodelay(stdscr, FALSE);
    getch();
    nodelay(stdscr, TRUE);
}

void resetGame()
{
    g_points = 0;
    g_dir = Direction::RIGHT;

    // Start roughly in the center
    g_head_x = GRID_WIDTH / 2;
    g_head_y = GRID_HEIGHT / 2;

    fillGameBoard(ObjectType::EMPTY);
    game_board[g_head_y][g_head_x] = ObjectType::SNAKE;
}

void handleInputPlaying(int ch)
{
    // Arrow keys (because keypad(stdscr, TRUE))
    // Prevent immediate reversal if you later add body segments
    switch (ch)
    {
    case KEY_UP:
        if (g_dir != Direction::DOWN)
            g_dir = Direction::UP;
        break;
    case KEY_DOWN:
        if (g_dir != Direction::UP)
            g_dir = Direction::DOWN;
        break;
    case KEY_LEFT:
        if (g_dir != Direction::RIGHT)
            g_dir = Direction::LEFT;
        break;
    case KEY_RIGHT:
        if (g_dir != Direction::LEFT)
            g_dir = Direction::RIGHT;
        break;
    case 'q':
        g_state = GameState::END;
        break; // quick exit to end screen
    default:
        break;
    }
}

// Returns false when the player loses
bool updateGameTick()
{
    // Move head 1 cell per tick
    int nx = g_head_x;
    int ny = g_head_y;

    switch (g_dir)
    {
    case Direction::UP:
        ny--;
        break;
    case Direction::DOWN:
        ny++;
        break;
    case Direction::LEFT:
        nx--;
        break;
    case Direction::RIGHT:
        nx++;
        break;
    }

    // Lose if hit walls (for now)
    if (nx < 0 || nx >= GRID_WIDTH || ny < 0 || ny >= GRID_HEIGHT)
        return false;

    // Lose if hit snake (for now this is just head, later your body)
    if (game_board[ny][nx] == ObjectType::SNAKE)
        return false;

    // Update board (temporary: single-segment snake)
    game_board[g_head_y][g_head_x] = ObjectType::EMPTY;
    g_head_x = nx;
    g_head_y = ny;
    game_board[g_head_y][g_head_x] = ObjectType::SNAKE;

    // Points just for demo (you’ll replace with FOOD logic)
    g_points++;

    return true;
}

int main()
{
    initializeBoard();

    // Non-blocking by default during PLAYING
    nodelay(stdscr, TRUE);
    timeout(TICK_MS); // getch() waits up to TICK_MS and returns ERR if no input

    bool running = true;
    while (running)
    {
        switch (g_state)
        {
        case GameState::START:
            drawStartScreen();
            resetGame();
            g_state = GameState::PLAYING;
            break;

        case GameState::PLAYING:
        {
            // Read input (non-blocking with timeout)
            int ch = getch();
            if (ch != ERR)
                handleInputPlaying(ch);

            // Update 1 tick; if false -> lose
            if (!updateGameTick())
                g_state = GameState::END;

            drawGameBoard();
            break;
        }

        case GameState::END:
            drawEndScreen(g_points);
            g_state = GameState::START; // restart on keypress
            break;
        }
    }

    shutdownBoard();
    return 0;
}
