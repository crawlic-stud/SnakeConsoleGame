#include <Windows.h>
#include <iostream>
#include <conio.h>
#include <ctime>
#include <string>


enum COLOUR {
    BLACK = 0x0000,
    DARK_BLUE = 0x0010,
    DARK_GREEN = 0x0020,
    DARK_CYAN = 0x0030,
    DARK_RED = 0x0040,
    DARK_MAGENTA = 0x0050,
    DARK_YELLOW = 0x0060,
    GREY = 0x0070,
    DARK_GREY = 0x0080,
    BLUE = 0x0090,
    GREEN = 0x00A0,
    CYAN = 0x00B0,
    RED = 0x00C0,
    MAGENTA = 0x00D0,
    YELLOW = 0x00E0,
    WHITE = 0x00F0,
};


// point
struct tuple {
public:
    int x; int y;

    tuple(int x_=0, int y_=0) : x(x_), y(y_) {}

    void set(int x_, int y_) {
        x = x_;
        y = y_;
    }

    // sum of tuples
    tuple operator+(const tuple& obj) {
        tuple temp;
        temp.x = x + obj.x;
        temp.y = y + obj.y;
        return temp;
    }

    // print tuple
    friend std::ostream& operator<<(std::ostream& os, const tuple& obj) {
        os << '(' << obj.x << ", " << obj.y << ')';
        return os;
    }
};


class Snake {
public:
    // snake velocity
    tuple vel;

    // head position
    tuple head;

    // array of tail segments
    int tailSize = 2;
    tuple* tail = new tuple[tailSize];

    // apple position
    tuple apple;

    bool gameOver = false;

    Snake() {
        vel.x = 1; 
        vel.y = 0;
        head.x = 5;
        head.y = 5;

        for (int i = 0; i < tailSize - 1; i++) {
            tuple tailPiece(5 - (i + 1), 5);
            tail[i] = tailPiece;
        }
        tail[tailSize - 1] = head;

        spawnApple();

    }

    void move(short borderX, short borderY) {
        // std::cout << head << '\n';
        head = head + vel;

        if (head.x + vel.x < -1) head.x = borderX - 1;
        if (head.x + vel.x > borderX) head.x = 0;

        if (head.y + vel.y < -1) head.y = borderY - 1;
        if (head.y + vel.y > borderY) head.y = 0;
    }

    bool keyHold(char key) {
        return GetKeyState(key) & 0x8000;
    }

    // Controlling input
    void control() {
        if (keyHold('W') && (vel.y != 1)) vel.set(0, -1);
        if (keyHold('A') && (vel.x != 1)) vel.set(-1, 0);
        if (keyHold('S') && (vel.y != -1)) vel.set(0, 1);
        if (keyHold('D') && (vel.x != -1)) vel.set(1, 0);
        //if (keyHold('Q')) tailSize++;
    }

    void appendTail() {
        tuple* newTail = new tuple[tailSize];
        newTail[tailSize - 1] = head; // new head pos

        if (appleHit()) {
            tailSize++;
            newTail = new tuple[tailSize];
            newTail[tailSize - 1] = head; // new head pos

            for (int i = 0; i < tailSize - 1; i++) {
                newTail[i] = tail[i];
            }
        }
        else {
            for (int i = 1; i < tailSize; i++) {
                newTail[i - 1] = tail[i];
            }
        }
        tail = newTail;
    }

    // checks whether point is in tail array
    bool inSnake(tuple point) {
        for (int i = 0; i < tailSize; i++) {
            if ((point.x == tail[i].x) && (point.y == tail[i].y)) return true;
        }
        return false;
    }

    void spawnApple(short borderX=10, short borderY=10) {
        srand(GetTickCount64());
        while (true) {
            apple.set(rand() % borderX, rand() % borderY);
            if (!inSnake(apple)) break;
        }
    }

    bool appleHit() {
        if ((head.x == apple.x) && (head.y == apple.y)) return true;
        return false;
    }

    bool tailHit() {
        for (int i = 0; i < tailSize - 1; i++) {
            if ((head.x == tail[i].x) && (head.y == tail[i].y)) return true;
        }
        return false;
    }

    void update(short borderX, short borderY) {

        move(borderX, borderY);
        appendTail();

        if (appleHit()) spawnApple(borderX, borderY);

        if (tailHit()) {
            gameOver = true;
        }
    }
};


class Game {
    short screenWidth;
    short screenHeight;
    short pixelSize = 20;

    CHAR_INFO* screen;

    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    SMALL_RECT writeRect;

    Snake snake;

public:
    Game(short width, short height) : screenWidth(width), screenHeight(height) {
        screen = new CHAR_INFO[width * height];
        setup();
    }

protected:
    CHAR_INFO createPixel(WORD color) {
        CHAR_INFO char_;
        char_.Char.AsciiChar = ' ';
        char_.Attributes = color;
        return char_;
    }

    void drawPixel(WORD color, int x, int y) {
        screen[x + (screenWidth * y)] = createPixel(color);
    }

    void fill(WORD color) {
        for (int row = 0; row < screenHeight; row++)
        {
            for (int column = 0; column < screenWidth; column++)
            {
                drawPixel(color, column, row);
            }
        }
    }

    CHAR_INFO createChar(char ch, WORD bgColor, WORD textColor) {
        CHAR_INFO char_;
        char_.Char.AsciiChar = ch;
        char_.Attributes = bgColor;
        char_.Attributes = textColor;
        return char_;
    }

    void drawText(char text[], int x, int y, WORD bgColor, WORD textColor) {
        for (int i = 0; i < sizeof(text); i++) {
            screen[x + (screenWidth * y) + i] = createChar(text[i], bgColor, textColor);
        }
    }

    void setup() {
        // Setting up font
        CONSOLE_FONT_INFOEX cfi;
        cfi.cbSize = sizeof(cfi);
        cfi.nFont = 0;
        cfi.dwFontSize.X = pixelSize;                  // Width of each character in the font
        cfi.dwFontSize.Y = pixelSize;                  // Height
        cfi.FontFamily = FF_DONTCARE;
        cfi.FontWeight = FW_NORMAL;

        SetCurrentConsoleFontEx(consoleHandle, FALSE, &cfi);

        // Setting up buffer size
        SetConsoleScreenBufferSize(consoleHandle, { screenWidth, screenHeight });

        // Setting up buffer rect
        writeRect.Top = 0;
        writeRect.Left = 0;
        writeRect.Bottom = screenHeight - 1;
        writeRect.Right = screenWidth - 1;

        // Makes console window size of game's screen
        SetConsoleWindowInfo(consoleHandle, TRUE, &writeRect);

    }

    void draw() {
        // draw screen background
        fill(COLOUR::BLACK);

        // draw apple
        drawPixel(COLOUR::RED, snake.apple.x, snake.apple.y);

        // draw snake 
        for (int i = 0; i < snake.tailSize; i++) {
            drawPixel(COLOUR::DARK_GREEN, snake.tail[i].x, snake.tail[i].y);
        }
        drawPixel(COLOUR::GREEN, snake.head.x, snake.head.y);

        // draw score
        //std::string scoreText = std::to_string(snake.tailSize);
        //char score[] = "25";
        //drawText(score, 0, 0, COLOUR::BLACK, FOREGROUND_GREEN);
    }

    void update() {

        snake.update(screenWidth, screenHeight);
        draw();

        WriteConsoleOutputA(
            consoleHandle,                      // screen buffer to write to
            screen,                             // buffer to copy from
            { screenWidth, screenHeight },      // col-row size of buffer
            { 0, 0 },                           // top left src cell in buffer
            &writeRect);                        // buffer rect

    }

public:
    void run() {
        int startTime = GetTickCount64();
        float frameInterval = 100; // in milliseconds
        while (!snake.gameOver) {
            snake.control(); 
            if (GetTickCount64() - startTime >= frameInterval) {
                update();
                startTime = GetTickCount64();
            }
        }
        std::cout << "GAME OVER\n" << "SCORE: " << snake.tailSize << '\n';
    }
};


int main() {
    Game game(30, 30);
    game.run();
    //char mem[] = "";
    //std::cout << sizeof(mem) << '\n'; // sizeof(char);
    //std::cout << sizeof(char);
}
