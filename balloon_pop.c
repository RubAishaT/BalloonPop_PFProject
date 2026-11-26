
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>



#define ROWS        8          
#define COLS        10         
#define EMPTY       (-1)       
#define BOMB        5          
#define NUM_COLOURS 5          
#define MAX_LEVEL   5


#define WIN_W       800
#define WIN_H       580
#define CELL        52         
#define BOARD_X     20         
#define BOARD_Y     70         

#define PI          3.14159265


int board[ROWS][COLS];


int levelColours[MAX_LEVEL + 1] = { 0,    3,    3,    4,    4,    5 };
int levelTarget [MAX_LEVEL + 1] = { 0, 3400, 3700, 3900, 4050, 4250 };   
int levelMoves  [MAX_LEVEL + 1] = { 0,   20,   18,   18,   15,   15 };
int levelSeconds[MAX_LEVEL + 1] = { 0,   90,   85,   80,   75,   70 };


int   level;               
int   score;               
int   levelStartScore;   
int   moves;               
int   movesUsed;           
float timeLeft;           
int   combo;               
int   levelStatus;         
int   loseReason;          
int   bonus;               
int   stars;               
int   balloonsPopped;      

enum { STATUS_PLAYING, STATUS_WON, STATUS_LOST };
enum { LOSE_MOVES = 1, LOSE_TIME, LOSE_NO_MATCH };


int visited[ROWS][COLS];             
int groupRow[ROWS * COLS];             
int groupCol[ROWS * COLS];             
int groupSize;                         


int popRow[ROWS * COLS];
int popCol[ROWS * COLS];
int popColour[ROWS * COLS];
int popCount;
int madeBomb;                        
int clearedBoard;                      


#define NUM_SCORES 5
int highScores[NUM_SCORES];            
int newHighScore;                      


enum { SCREEN_MENU, SCREEN_HELP, SCREEN_SCORES, SCREEN_PLAYING, SCREEN_PAUSED,
       SCREEN_LEVEL_DONE, SCREEN_GAME_OVER, SCREEN_WIN };
int screen = SCREEN_MENU;             


int   mouseX = -100, mouseY = -100;    
int   soundOn = 1;                     
int   tick = 0;                        
int   endDelay = 0;                    
char  message[100] = "";               
int   messageLife = 0;                 


#define MAX_EFFECTS 100
int fxX[MAX_EFFECTS], fxY[MAX_EFFECTS], fxLife[MAX_EFFECTS], fxColour[MAX_EFFECTS];


char popupText[20];
int  popupX, popupY, popupLife = 0;


#define MAX_BUTTONS 8
enum { BTN_START = 1, BTN_HELP, BTN_SCORES, BTN_EXIT, BTN_BACK, BTN_PAUSE,
       BTN_RESUME, BTN_RESTART, BTN_MENU, BTN_SOUND, BTN_NEXT, BTN_AGAIN };

struct Button {
    int  id;                           
    int  x, y, w, h;                   
    char text[20];                     
    int  primary;                      
};
struct Button buttons[MAX_BUTTONS];
int buttonCount = 0;


COLORREF balloonColor[6] = {
    RGB(230,  57,  70),   
    RGB( 47, 125, 246),   
    RGB( 45, 184, 102),   
    RGB(255, 194,  26),   
    RGB(155,  89, 208),  
    RGB( 46,  47,  69)    
};
const char *colourName[6] = { "Red", "Blue", "Green", "Yellow", "Purple", "Bomb" };


HFONT fontSmall, fontNormal, fontMedium, fontLarge, fontTitle;





int CountPairs(void)
{
    int r, c, pairs = 0;
    for (r = 0; r < ROWS; r++) {
        for (c = 0; c < COLS; c++) {
            if (c + 1 < COLS && board[r][c] == board[r][c + 1]) pairs++;
            if (r + 1 < ROWS && board[r][c] == board[r + 1][c]) pairs++;
        }
    }
    return pairs;
}


void FillBoard(void)
{
    int r, c;
    do {
        for (r = 0; r < ROWS; r++)
            for (c = 0; c < COLS; c++)
                board[r][c] = rand() % levelColours[level];
    } while (CountPairs() < 20);
}


void StartLevel(int newLevel)
{
    level           = newLevel;
    levelStartScore = score;
    moves           = levelMoves[level];
    movesUsed       = 0;
    timeLeft        = (float)levelSeconds[level];
    combo           = 0;
    levelStatus     = STATUS_PLAYING;
    loseReason      = 0;
    bonus           = 0;
    stars           = 0;
    balloonsPopped  = 0;
    FillBoard();
}


void StartNewGame(void)
{
    score = 0;
    StartLevel(1);
}


int CountBalloons(void)
{
    int r, c, count = 0;
    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            if (board[r][c] != EMPTY) count++;
    return count;
}


void SpreadGroup(int r, int c, int colour)
{
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return;   
    if (visited[r][c]) return;                              
    if (board[r][c] != colour) return;                      

    visited[r][c] = 1;
    groupRow[groupSize] = r;
    groupCol[groupSize] = c;
    groupSize++;

    SpreadGroup(r - 1, c, colour);      
    SpreadGroup(r + 1, c, colour);      
    SpreadGroup(r, c - 1, colour);      
    SpreadGroup(r, c + 1, colour);      
}


int FindGroup(int r, int c)
{
    int i, j;
    for (i = 0; i < ROWS; i++)
        for (j = 0; j < COLS; j++)
            visited[i][j] = 0;
    groupSize = 0;

    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return 0;
    if (board[r][c] == EMPTY || board[r][c] == BOMB) return 0;

    SpreadGroup(r, c, board[r][c]);
    return groupSize;
}


int PopGroup(int clickedRow, int clickedCol)
{
    int i, n = 0;
    int size = groupSize;

    madeBomb = (size >= 6);           

    for (i = 0; i < size; i++) {
        int r = groupRow[i], c = groupCol[i];
        if (madeBomb && r == clickedRow && c == clickedCol)
            continue;                 
        popRow[n]    = r;
        popCol[n]    = c;
        popColour[n] = board[r][c];
        board[r][c]  = EMPTY;
        n++;
    }
    popCount = n;

    if (madeBomb) board[clickedRow][clickedCol] = BOMB;

    return size * size * 5;
}


int ExplodeBomb(int r, int c)
{
    int dr, dc, n = 0;

    madeBomb = 0;
    for (dr = -1; dr <= 1; dr++) {
        for (dc = -1; dc <= 1; dc++) {
            int rr = r + dr, cc = c + dc;
            if (rr < 0 || rr >= ROWS || cc < 0 || cc >= COLS) continue;
            if (board[rr][cc] == EMPTY) continue;
            popRow[n]    = rr;
            popCol[n]    = cc;
            popColour[n] = board[rr][cc];
            board[rr][cc] = EMPTY;
            n++;
        }
    }
    popCount = n;
    return n * 20;
}


void DropBalloons(void)
{
    int r, c, write;
    for (c = 0; c < COLS; c++) {
        write = ROWS - 1;                       
        for (r = ROWS - 1; r >= 0; r--) {
            if (board[r][c] != EMPTY) {
                int colour = board[r][c];
                board[r][c] = EMPTY;
                board[write][c] = colour;
                write--;
            }
        }
    }
}


void ShiftColumns(void)
{
    int r, c, target = 0;
    for (c = 0; c < COLS; c++) {
        if (board[ROWS - 1][c] != EMPTY) {     
            if (target != c) {
                for (r = 0; r < ROWS; r++) {
                    board[r][target] = board[r][c];
                    board[r][c] = EMPTY;
                }
            }
            target++;
        }
    }
}


int HasMove(void)
{
    int r, c;
    for (r = 0; r < ROWS; r++) {
        for (c = 0; c < COLS; c++) {
            if (board[r][c] == EMPTY) continue;
            if (board[r][c] == BOMB) return 1;
            if (c + 1 < COLS && board[r][c] == board[r][c + 1]) return 1;
            if (r + 1 < ROWS && board[r][c] == board[r + 1][c]) return 1;
        }
    }
    return 0;
}


int ComboPercentFor(int comboCount)
{
    int percent = 100 + 25 * (comboCount - 1);
    if (comboCount < 1) percent = 100;
    if (percent > 200)  percent = 200;
    return percent;
}

int ComboPercent(void)
{
    return ComboPercentFor(combo);
}


int ColourFactor(void)
{
    return levelColours[level] - 1;
}


int LevelScore(void)
{
    return score - levelStartScore;
}


void CheckLevelEnd(void)
{
    if (LevelScore() >= levelTarget[level]) {
        levelStatus = STATUS_WON;
        bonus = moves * 20 + (int)timeLeft * 2;
        score = score + bonus;

        
        if (moves * 100 / levelMoves[level] >= 40)      stars = 3;
        else if (moves * 100 / levelMoves[level] >= 20) stars = 2;
        else                                            stars = 1;
    }
    else if (moves <= 0) {
        levelStatus = STATUS_LOST;
        loseReason  = LOSE_MOVES;
    }
    else if (!HasMove()) {
        levelStatus = STATUS_LOST;
        loseReason  = LOSE_NO_MATCH;
    }
}


void CountDownTime(float seconds)
{
    if (levelStatus != STATUS_PLAYING) return;
    timeLeft = timeLeft - seconds;
    if (timeLeft <= 0) {
        timeLeft    = 0;
        levelStatus = STATUS_LOST;
        loseReason  = LOSE_TIME;
    }
}


int PlayerClick(int r, int c)
{
    int base, points;

    popCount = 0;
    madeBomb = 0;
    clearedBoard = 0;

    if (levelStatus != STATUS_PLAYING) return 0;
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return 0;
    if (board[r][c] == EMPTY) return 0;

    if (board[r][c] == BOMB) {
        base = ExplodeBomb(r, c);
    } else {
        if (FindGroup(r, c) < 2) {      
            combo = 0;                  
            return 0;
        }
        base = PopGroup(r, c);
    }

    base = base * ColourFactor() / 2;    
    combo++;
    points = base * ComboPercent() / 100;
    score = score + points;
    moves--;
    movesUsed++;
    balloonsPopped = balloonsPopped + popCount;

    DropBalloons();
    ShiftColumns();

    if (CountBalloons() == 0) {         
        score = score + 200;
        clearedBoard = 1;
        FillBoard();
    }

    CheckLevelEnd();
    return points;
}


void LoadHighScores(void)
{
    int i;
    FILE *file;

    for (i = 0; i < NUM_SCORES; i++) highScores[i] = 0;

    file = fopen("highscores.txt", "r");
    if (file == NULL) return;           

    for (i = 0; i < NUM_SCORES; i++) {
        if (fscanf(file, "%d", &highScores[i]) != 1) highScores[i] = 0;
    }
    fclose(file);
}


void SaveHighScores(void)
{
    int i;
    FILE *file = fopen("highscores.txt", "w");
    if (file == NULL) return;

    for (i = 0; i < NUM_SCORES; i++) fprintf(file, "%d\n", highScores[i]);
    fclose(file);
}


void AddHighScore(int newScore)
{
    int i, j, temp;

    newHighScore = 0;
    if (newScore <= highScores[NUM_SCORES - 1]) return;     

    highScores[NUM_SCORES - 1] = newScore;                 
    newHighScore = 1;

    for (i = 0; i < NUM_SCORES - 1; i++) {                  
        for (j = 0; j < NUM_SCORES - 1 - i; j++) {
            if (highScores[j] < highScores[j + 1]) {
                temp = highScores[j];
                highScores[j] = highScores[j + 1];
                highScores[j + 1] = temp;
            }
        }
    }
    SaveHighScores();
}



COLORREF Darker(COLORREF colour, int percent)
{
    return RGB(GetRValue(colour) * percent / 100,
               GetGValue(colour) * percent / 100,
               GetBValue(colour) * percent / 100);
}


void CreateFonts(void)
{
    fontSmall  = CreateFont(-15, 0, 0, 0, FW_BOLD,   0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                            CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, "Segoe UI");
    fontNormal = CreateFont(-18, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                            CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, "Segoe UI");
    fontMedium = CreateFont(-24, 0, 0, 0, FW_BOLD,   0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                            CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, "Segoe UI");
    fontLarge  = CreateFont(-38, 0, 0, 0, FW_BOLD,   0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                            CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, "Segoe UI");
    fontTitle  = CreateFont(-68, 0, 0, 0, FW_BOLD,   0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                            CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, "Segoe UI");
}


void TextLeft(HDC hdc, HFONT font, int x, int y, const char *text, COLORREF colour)
{
    SelectObject(hdc, font);
    SetTextColor(hdc, colour);
    TextOut(hdc, x, y, text, (int)strlen(text));
}


void TextCenter(HDC hdc, HFONT font, int x, int y, const char *text, COLORREF colour)
{
    SIZE size;
    SelectObject(hdc, font);
    GetTextExtentPoint32(hdc, text, (int)strlen(text), &size);
    TextLeft(hdc, font, x - size.cx / 2, y, text, colour);
}


void TextRight(HDC hdc, HFONT font, int x, int y, const char *text, COLORREF colour)
{
    SIZE size;
    SelectObject(hdc, font);
    GetTextExtentPoint32(hdc, text, (int)strlen(text), &size);
    TextLeft(hdc, font, x - size.cx, y, text, colour);
}


void DrawEllipse(HDC hdc, int left, int top, int right, int bottom,
                 COLORREF fill, COLORREF line, int lineWidth)
{
    HBRUSH brush = CreateSolidBrush(fill);
    HPEN pen = (lineWidth > 0) ? CreatePen(PS_SOLID, lineWidth, line) : (HPEN)GetStockObject(NULL_PEN);
    HGDIOBJ oldBrush = SelectObject(hdc, brush);
    HGDIOBJ oldPen   = SelectObject(hdc, pen);

    Ellipse(hdc, left, top, right, bottom);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(brush);
    if (lineWidth > 0) DeleteObject(pen);
}

void DrawBox(HDC hdc, int x, int y, int w, int h, int corner,
             COLORREF fill, COLORREF line, int lineWidth)
{
    HBRUSH brush = CreateSolidBrush(fill);
    HPEN pen = (lineWidth > 0) ? CreatePen(PS_SOLID, lineWidth, line) : (HPEN)GetStockObject(NULL_PEN);
    HGDIOBJ oldBrush = SelectObject(hdc, brush);
    HGDIOBJ oldPen   = SelectObject(hdc, pen);

    RoundRect(hdc, x, y, x + w, y + h, corner * 2, corner * 2);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(brush);
    if (lineWidth > 0) DeleteObject(pen);
}


void DrawRing(HDC hdc, int cx, int cy, int radius, COLORREF colour, int lineWidth)
{
    HPEN pen = CreatePen(PS_SOLID, lineWidth, colour);
    HGDIOBJ oldPen   = SelectObject(hdc, pen);
    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));

    Ellipse(hdc, cx - radius, cy - radius, cx + radius, cy + radius);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}


void DrawPolygon(HDC hdc, POINT *points, int count, COLORREF fill)
{
    HBRUSH brush = CreateSolidBrush(fill);
    HGDIOBJ oldBrush = SelectObject(hdc, brush);
    HGDIOBJ oldPen   = SelectObject(hdc, GetStockObject(NULL_PEN));

    Polygon(hdc, points, count);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(brush);
}


void DrawLine(HDC hdc, int x1, int y1, int x2, int y2, COLORREF colour, int lineWidth)
{
    HPEN pen = CreatePen(PS_SOLID, lineWidth, colour);
    HGDIOBJ oldPen = SelectObject(hdc, pen);

    MoveToEx(hdc, x1, y1, NULL);
    LineTo(hdc, x2, y2);

    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}


void DrawStar(HDC hdc, int cx, int cy, int outer, int inner, int tips, COLORREF colour)
{
    POINT points[20];
    int i;
    for (i = 0; i < tips * 2; i++) {
        double angle  = -PI / 2 + i * PI / tips;        
        double radius = (i % 2 == 0) ? outer : inner;   
        points[i].x = cx + (int)(cos(angle) * radius);
        points[i].y = cy + (int)(sin(angle) * radius);
    }
    DrawPolygon(hdc, points, tips * 2, colour);
}


void DrawBalloon(HDC hdc, int cx, int cy, int w, int colour)
{
    COLORREF base = balloonColor[colour];
    int h = w * 6 / 5;                          
    int s = w / 5;                              
    POINT p[4];

    
    DrawLine(hdc, cx, cy + h / 2 + 2, cx + w / 12, cy + h / 2 + w / 3, RGB(120, 128, 150), 1);
    p[0].x = cx;          p[0].y = cy + h / 2 - 1;
    p[1].x = cx - w / 10; p[1].y = cy + h / 2 + w / 9;
    p[2].x = cx + w / 10; p[2].y = cy + h / 2 + w / 9;
    DrawPolygon(hdc, p, 3, Darker(base, 70));

    
    DrawEllipse(hdc, cx - w / 2, cy - h / 2, cx + w / 2, cy + h / 2, base, Darker(base, 65), 2);
    DrawEllipse(hdc, cx - w / 4 - 2, cy - h / 3, cx - w / 12, cy - h / 8, RGB(255, 255, 255), 0, 0);

    
    switch (colour) {
    case 0:  DrawStar(hdc, cx, cy + 2, s + 4, (s + 4) * 45 / 100, 5, RGB(255, 255, 255)); break;  
    case 1:  p[0].x = cx;     p[0].y = cy - s - 3;                                              
             p[1].x = cx + s; p[1].y = cy + 2;
             p[2].x = cx;     p[2].y = cy + s + 7;
             p[3].x = cx - s; p[3].y = cy + 2;
             DrawPolygon(hdc, p, 4, RGB(255, 255, 255));                                break;
    case 2:  DrawEllipse(hdc, cx - s, cy - s + 3, cx + s, cy + s + 3, RGB(255, 255, 255), 0, 0); break; 
    case 3:  p[0].x = cx;         p[0].y = cy - s;                                              
             p[1].x = cx + s + 2; p[1].y = cy + s + 3;
             p[2].x = cx - s - 2; p[2].y = cy + s + 3;
             DrawPolygon(hdc, p, 3, RGB(255, 255, 255));                                break;
    case 4:  DrawBox(hdc, cx - s, cy - s + 3, 2 * s, 2 * s, 1, RGB(255, 255, 255), 0, 0);       break; 
    case BOMB: DrawStar(hdc, cx, cy + 2, s + 6, (s + 6) / 2, 8, RGB(255, 205, 50));      break;  
    }
}


void DrawSky(HDC hdc)
{
    int y;
    for (y = 0; y < WIN_H; y += 10) {                   
        RECT band;
        HBRUSH brush = CreateSolidBrush(RGB(95 + 115 * y / WIN_H, 175 + 65 * y / WIN_H, 255));
        band.left = 0;  band.right = WIN_W;  band.top = y;  band.bottom = y + 10;
        FillRect(hdc, &band, brush);
        DeleteObject(brush);
    }
    DrawEllipse(hdc, 60, 70, 180, 120, RGB(255, 255, 255), 0, 0);     
    DrawEllipse(hdc, 110, 50, 220, 105, RGB(255, 255, 255), 0, 0);
    DrawEllipse(hdc, 560, 30, 700, 85, RGB(255, 255, 255), 0, 0);     
    DrawEllipse(hdc, 620, 10, 740, 70, RGB(255, 255, 255), 0, 0);
    DrawEllipse(hdc, -150, 520, 450, 800, RGB(130, 210, 140), 0, 0);  
    DrawEllipse(hdc, 300, 540, 950, 820, RGB(100, 190, 125), 0, 0);
}


void DrawCard(HDC hdc, int x, int y, int w, int h)
{
    DrawBox(hdc, x + 4, y + 6, w, h, 18, RGB(150, 175, 210), 0, 0);          /* shadow */
    DrawBox(hdc, x, y, w, h, 18, RGB(250, 252, 255), RGB(255, 255, 255), 2);
}


void DrawBar(HDC hdc, int x, int y, int w, int h, int percent, COLORREF colour)
{
    if (percent < 0)   percent = 0;
    if (percent > 100) percent = 100;
    DrawBox(hdc, x, y, w, h, h / 2, RGB(210, 222, 242), 0, 0);
    if (percent > 0)
        DrawBox(hdc, x, y, h + (w - h) * percent / 100, h, h / 2, colour, 0, 0);
}

/* "Balloon Pop" with every letter in another balloon colour. */
void DrawRainbowTitle(HDC hdc, HFONT font, const char *text, int centerX, int y)
{
    SIZE size;
    int i, colour = 0, x;

    SelectObject(hdc, font);
    GetTextExtentPoint32(hdc, text, (int)strlen(text), &size);
    x = centerX - size.cx / 2;

    for (i = 0; text[i] != '\0'; i++) {
        char letter[2];
        letter[0] = text[i];
        letter[1] = '\0';
        GetTextExtentPoint32(hdc, letter, 1, &size);
        if (text[i] != ' ') {
            TextLeft(hdc, font, x + 3, y + 3, letter, RGB(40, 64, 128));       
            TextLeft(hdc, font, x, y, letter, balloonColor[colour % 5]);
            colour++;
        }
        x = x + size.cx;
    }
}




void ShowMessage(const char *text)
{
    strcpy(message, text);
    messageLife = 50;                   
}



enum { SND_CLICK, SND_POP, SND_BIG, SND_BOMB, SND_WRONG, SND_WIN, SND_LOSE, NUM_SOUNDS };

#define SOUND_RATE   22050                     
#define MAX_SAMPLES  12000                      
#define HEADER_SIZE  44                        

unsigned char soundData[NUM_SOUNDS][HEADER_SIZE + MAX_SAMPLES * 2];


void PutNumber(unsigned char *where, unsigned int value, int bytes)
{
    int i;
    for (i = 0; i < bytes; i++) {
        where[i] = (unsigned char)(value & 255);
        value = value >> 8;
    }
}


void MakeSound(int id, float startHz, float endHz, float seconds, float noise, float volume)
{
    unsigned char *data = soundData[id];
    int count = (int)(seconds * SOUND_RATE);
    float angle = 0;
    int i;

    if (count > MAX_SAMPLES) count = MAX_SAMPLES;

    for (i = 0; i < count; i++) {
        float t = (float)i / count;                        
        float hz = startHz + (endHz - startHz) * t;
        float fade = (1 - t) * (1 - t);                     
        float hiss = ((float)rand() / RAND_MAX) * 2 - 1;
        float wave = (1 - noise) * sinf(angle) + noise * hiss;
        int sample = (int)(wave * fade * volume * 32000);

        PutNumber(data + HEADER_SIZE + i * 2, (unsigned int)(sample & 0xFFFF), 2);
        angle = angle + 2 * 3.14159265f * hz / SOUND_RATE;
    }

    
    memcpy(data, "RIFF", 4);
    PutNumber(data + 4, HEADER_SIZE - 8 + count * 2, 4);
    memcpy(data + 8, "WAVEfmt ", 8);
    PutNumber(data + 16, 16, 4);                            
    PutNumber(data + 20, 1, 2);                             
    PutNumber(data + 22, 1, 2);                             
    PutNumber(data + 24, SOUND_RATE, 4);
    PutNumber(data + 28, SOUND_RATE * 2, 4);                
    PutNumber(data + 32, 2, 2);                             
    PutNumber(data + 34, 16, 2);                            
    memcpy(data + 36, "data", 4);
    PutNumber(data + 40, count * 2, 4);
}


void MakeAllSounds(void)
{
    MakeSound(SND_CLICK, 700, 600, 0.04f, 0.0f, 0.35f);     
    MakeSound(SND_POP,   900, 250, 0.09f, 0.3f, 0.60f);     
    MakeSound(SND_BIG,   700, 180, 0.16f, 0.3f, 0.70f);     
    MakeSound(SND_BOMB,  220,  50, 0.40f, 0.7f, 0.90f);     
    MakeSound(SND_WRONG, 160, 120, 0.12f, 0.1f, 0.50f);     
    MakeSound(SND_WIN,   500, 1100, 0.40f, 0.0f, 0.50f);    
    MakeSound(SND_LOSE,  400,  90, 0.45f, 0.0f, 0.50f);     
}


void PlaySoundEffect(int id)
{
    if (soundOn) PlaySound((LPCSTR)soundData[id], NULL, SND_MEMORY | SND_ASYNC);
}



void AddButton(int id, int x, int y, int w, int h, const char *text, int primary)
{
    struct Button *b = &buttons[buttonCount];
    b->id = id;  b->x = x;  b->y = y;  b->w = w;  b->h = h;
    b->primary = primary;
    strcpy(b->text, text);
    buttonCount++;
}


void SetupButtons(void)
{
    buttonCount = 0;

    switch (screen) {
    case SCREEN_MENU:
        AddButton(BTN_START,  300, 210, 200, 46, "Start Game",   1);
        AddButton(BTN_HELP,   300, 270, 200, 46, "Instructions", 0);
        AddButton(BTN_SCORES, 300, 330, 200, 46, "High Scores",  0);
        AddButton(BTN_EXIT,   300, 390, 200, 46, "Exit",         0);
        break;
    case SCREEN_HELP:
        AddButton(BTN_BACK, 300, 492, 200, 40, "Back", 1);
        break;
    case SCREEN_SCORES:
        AddButton(BTN_BACK, 300, 470, 200, 40, "Back", 1);
        break;
    case SCREEN_PLAYING:
        AddButton(BTN_PAUSE,   585, 392, 90, 38, "Pause",   0);
        AddButton(BTN_RESTART, 685, 392, 90, 38, "Restart", 0);
        AddButton(BTN_MENU,    585, 438, 90, 38, "Menu",    0);
        AddButton(BTN_SOUND,   685, 438, 90, 38, soundOn ? "Sound ON" : "Sound OFF", 0);
        break;
    case SCREEN_PAUSED:
        AddButton(BTN_RESUME,  180, 190, 200, 44, "Resume",        1);
        AddButton(BTN_RESTART, 180, 250, 200, 44, "Restart level", 0);
        AddButton(BTN_MENU,    180, 310, 200, 44, "Main menu",     0);
        break;
    case SCREEN_LEVEL_DONE:
        AddButton(BTN_NEXT, 300, 400, 200, 46, "Next Level", 1);
        AddButton(BTN_MENU, 300, 458, 200, 40, "Main menu",  0);
        break;
    case SCREEN_GAME_OVER:
    case SCREEN_WIN:
        AddButton(BTN_AGAIN, 300, 400, 200, 46, "Play Again", 1);
        AddButton(BTN_MENU,  300, 458, 200, 40, "Main menu",  0);
        break;
    }
}


void ChangeScreen(int newScreen)
{
    screen = newScreen;
    SetupButtons();
}


void DrawButtons(HDC hdc)
{
    int i;
    for (i = 0; i < buttonCount; i++) {
        struct Button *b = &buttons[i];
        int hover = (mouseX >= b->x && mouseX < b->x + b->w && mouseY >= b->y && mouseY < b->y + b->h);
        COLORREF fill, line, textColour;

        if (b->primary) {
            fill = hover ? RGB(95, 160, 255) : RGB(60, 125, 235);
            line = RGB(30, 80, 190);
            textColour = RGB(255, 255, 255);
        } else {
            fill = hover ? RGB(255, 255, 255) : RGB(232, 240, 252);
            line = RGB(150, 175, 215);
            textColour = RGB(33, 48, 87);
        }
        DrawBox(hdc, b->x + 2, b->y + 3, b->w, b->h, b->h / 2, RGB(150, 175, 210), 0, 0);   /* shadow */
        DrawBox(hdc, b->x, b->y, b->w, b->h, b->h / 2, fill, line, 2);

        if (b->h >= 40)                                   
            TextCenter(hdc, fontMedium, b->x + b->w / 2, b->y + (b->h - 32) / 2, b->text, textColour);
        else                                             
            TextCenter(hdc, fontSmall, b->x + b->w / 2, b->y + (b->h - 20) / 2, b->text, textColour);
    }
}



void DrawMenu(HDC hdc)
{
    char text[60];
    int i;
    
    int decoX[6]      = {  95, 700, 150, 660,  90, 710 };
    int decoY[6]      = { 230, 210, 430, 420, 520, 520 };
    int decoColour[6] = {   0,   1,   2,   3,   4,   0 };

    DrawSky(hdc);
    for (i = 0; i < 6; i++)
        DrawBalloon(hdc, decoX[i], decoY[i] + (int)(sin((tick + i * 7) / 8.0) * 8), 70, decoColour[i]);

    DrawRainbowTitle(hdc, fontTitle, "Balloon Pop", WIN_W / 2, 45);
    TextCenter(hdc, fontMedium, WIN_W / 2, 135, "Connect & Match", RGB(255, 255, 255));

    sprintf(text, "High score: %d", highScores[0]);
    TextCenter(hdc, fontNormal, WIN_W / 2, 170, text, RGB(33, 48, 87));

    DrawButtons(hdc);
    TextCenter(hdc, fontSmall, WIN_W / 2, 548, "Programming Fundamentals Project  -  written in C", RGB(20, 70, 50));
}



void DrawHelp(HDC hdc)
{
    int i, y = 88;
    const char *lines[] = {
        "GOAL:  reach the target score before you run out of moves or time.",
        "Click a group of 2 or more TOUCHING balloons of the same colour",
        "(up, down, left or right) to pop them. One balloon alone cannot be popped.",
        "Group score = size x size x 5 (more on harder levels). Big groups win!",
        "COMBO:  pop balloons one after another for extra points. A wrong click",
        "(a single balloon) breaks the combo, but it never costs points or moves.",
        "BOMB:  pop 6 or more at once and you get a black bomb. Click the bomb",
        "to blast all balloons in the 3x3 area around it.",
        "Balloons fall down, and empty columns close up from the right.",
        "Finish with moves and time left for a bonus and up to 3 stars.",
        "Keys:   P = pause     R = restart level     M = sound on/off"
    };

    DrawSky(hdc);
    DrawCard(hdc, 50, 25, 700, 520);
    TextCenter(hdc, fontLarge, WIN_W / 2, 35, "How to Play", RGB(33, 48, 87));

    for (i = 0; i < 11; i++) {
        TextLeft(hdc, fontNormal, 80, y, lines[i], RGB(33, 48, 87));
        y = y + 28;
    }

    TextCenter(hdc, fontSmall, WIN_W / 2, 390, "Every colour has its own symbol:", RGB(104, 120, 152));
    for (i = 0; i < 6; i++) {
        DrawBalloon(hdc, 115 + i * 115, 424, 36, i);
        TextCenter(hdc, fontSmall, 115 + i * 115, 462, colourName[i], RGB(104, 120, 152));
    }
    DrawButtons(hdc);
}



void DrawScores(HDC hdc)
{
    int i;
    char text[40];

    DrawSky(hdc);
    DrawCard(hdc, 200, 25, 400, 500);
    TextCenter(hdc, fontLarge, WIN_W / 2, 40, "High Scores", RGB(33, 48, 87));

    for (i = 0; i < NUM_SCORES; i++) {
        int y = 110 + i * 70;
        DrawBox(hdc, 235, y, 330, 56, 14, RGB(232, 240, 252), RGB(190, 208, 238), 2);
        DrawEllipse(hdc, 250, y + 12, 282, y + 44, i == 0 ? RGB(255, 190, 30) : RGB(150, 170, 205), 0, 0);

        sprintf(text, "%d", i + 1);
        TextCenter(hdc, fontNormal, 266, y + 14, text, RGB(255, 255, 255));

        if (highScores[i] > 0) sprintf(text, "%d", highScores[i]);
        else                   strcpy(text, "- - -");
        TextLeft(hdc, fontMedium, 305, y + 12, text, RGB(33, 48, 87));
    }
    DrawButtons(hdc);
}


void DrawPanel(HDC hdc)
{
    char text[40];
    int x = 585;
    COLORREF dark = RGB(33, 48, 87), grey = RGB(104, 120, 152);

    DrawCard(hdc, 570, 60, 220, 436);

    sprintf(text, "LEVEL %d / %d", level, MAX_LEVEL);
    TextLeft(hdc, fontMedium, x, 70, text, dark);

    TextLeft(hdc, fontSmall, x, 108, "SCORE", grey);
    sprintf(text, "%d", score);
    TextLeft(hdc, fontLarge, x, 122, text, dark);

    TextLeft(hdc, fontSmall, x, 176, "TARGET", grey);
    sprintf(text, "%d / %d", LevelScore(), levelTarget[level]);
    TextRight(hdc, fontSmall, 775, 176, text, dark);
    DrawBar(hdc, x, 198, 190, 14, LevelScore() * 100 / levelTarget[level],
            LevelScore() >= levelTarget[level] ? RGB(255, 190, 30) : RGB(60, 190, 110));

    TextLeft(hdc, fontSmall, x, 232, "MOVES", grey);
    sprintf(text, "%d", moves);
    TextLeft(hdc, fontMedium, x, 250, text, moves <= 3 ? RGB(220, 58, 74) : dark);

    TextLeft(hdc, fontSmall, 690, 232, "TIME", grey);
    sprintf(text, "%d:%02d", (int)ceil(timeLeft) / 60, (int)ceil(timeLeft) % 60);
    TextLeft(hdc, fontMedium, 690, 250, text, timeLeft <= 10 ? RGB(220, 58, 74) : dark);

    TextLeft(hdc, fontSmall, x, 300, "COMBO", grey);
    sprintf(text, "x%.2f", ComboPercent() / 100.0);
    TextLeft(hdc, fontMedium, x, 318, text, combo > 1 ? RGB(230, 110, 20) : dark);

    TextLeft(hdc, fontSmall, 690, 300, "BEST", grey);
    sprintf(text, "%d", score > highScores[0] ? score : highScores[0]);
    TextLeft(hdc, fontMedium, 690, 318, text, dark);

    if (screen == SCREEN_PLAYING) DrawButtons(hdc);
}


void DrawBoard(HDC hdc)
{
    int r, c, i, k;
    int hoverRow = (mouseY - BOARD_Y) / CELL;
    int hoverCol = (mouseX - BOARD_X) / CELL;
    int mouseOnBoard = (mouseX >= BOARD_X && mouseY >= BOARD_Y && hoverRow < ROWS && hoverCol < COLS);

    DrawCard(hdc, BOARD_X - 10, BOARD_Y - 10, COLS * CELL + 20, ROWS * CELL + 20);

    
    if (mouseOnBoard && levelStatus == STATUS_PLAYING && board[hoverRow][hoverCol] != EMPTY) {
        if (board[hoverRow][hoverCol] == BOMB) {
            for (r = hoverRow - 1; r <= hoverRow + 1; r++)
                for (c = hoverCol - 1; c <= hoverCol + 1; c++)
                    if (r >= 0 && r < ROWS && c >= 0 && c < COLS)
                        DrawBox(hdc, BOARD_X + c * CELL + 2, BOARD_Y + r * CELL + 2, CELL - 4, CELL - 4,
                                10, RGB(255, 222, 218), RGB(235, 90, 80), 2);
        } else if (FindGroup(hoverRow, hoverCol) >= 2) {
            for (i = 0; i < groupSize; i++)
                DrawBox(hdc, BOARD_X + groupCol[i] * CELL + 2, BOARD_Y + groupRow[i] * CELL + 2,
                        CELL - 4, CELL - 4, 10, RGB(255, 243, 200), RGB(240, 170, 40), 2);
        }
    }

   
    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            if (board[r][c] != EMPTY)
                DrawBalloon(hdc, BOARD_X + c * CELL + CELL / 2, BOARD_Y + r * CELL + CELL / 2 - 4,
                            38, board[r][c]);

    
    for (i = 0; i < MAX_EFFECTS; i++) {
        if (fxLife[i] > 0) {
            int radius = 8 + (7 - fxLife[i]) * 5;
            DrawRing(hdc, fxX[i], fxY[i], radius, balloonColor[fxColour[i]], 3);
            for (k = 0; k < 6; k++) {
                int dx = (int)(cos(k * PI / 3) * radius * 1.4);
                int dy = (int)(sin(k * PI / 3) * radius * 1.4);
                DrawEllipse(hdc, fxX[i] + dx - 3, fxY[i] + dy - 3, fxX[i] + dx + 3, fxY[i] + dy + 3,
                            balloonColor[fxColour[i]], 0, 0);
            }
        }
    }

    
    if (popupLife > 0) {
        TextCenter(hdc, fontLarge, popupX + 2, popupY + 2, popupText, RGB(90, 50, 0));
        TextCenter(hdc, fontLarge, popupX, popupY, popupText, RGB(255, 200, 30));
    }
}


void DrawMessageBar(HDC hdc)
{
    char text[100];

    DrawBox(hdc, BOARD_X - 10, 512, COLS * CELL + 20, 38, 19, RGB(250, 252, 255), RGB(255, 255, 255), 2);

    if (messageLife > 0) {
        TextCenter(hdc, fontNormal, BOARD_X + COLS * CELL / 2, 520, message, RGB(33, 48, 87));
        return;
    }

    
    strcpy(text, "Click a group of 2 or more touching balloons of the same colour.");
    {
        int r = (mouseY - BOARD_Y) / CELL, c = (mouseX - BOARD_X) / CELL;
        if (screen == SCREEN_PLAYING && mouseX >= BOARD_X && mouseY >= BOARD_Y && r < ROWS && c < COLS) {
            if (board[r][c] == BOMB)
                strcpy(text, "BOMB!  Click it to blast the 3x3 area around it.");
            else if (FindGroup(r, c) >= 2)
                sprintf(text, "%d balloons  =  %d points", groupSize, groupSize * groupSize * 5 * ColourFactor() / 2 * ComboPercentFor(combo + 1) / 100);
        }
    }
    TextCenter(hdc, fontNormal, BOARD_X + COLS * CELL / 2, 520, text, RGB(33, 48, 87));
}

void DrawPlaying(HDC hdc)
{
    DrawSky(hdc);
    DrawRainbowTitle(hdc, fontMedium, "Balloon Pop", 110, 16);
    DrawBoard(hdc);
    DrawPanel(hdc);
    DrawMessageBar(hdc);
}



void DrawPaused(HDC hdc)
{
    DrawSky(hdc);
    DrawRainbowTitle(hdc, fontMedium, "Balloon Pop", 110, 16);
    DrawCard(hdc, BOARD_X - 10, BOARD_Y - 10, COLS * CELL + 20, ROWS * CELL + 20);   /* the balloons are hidden */
    TextCenter(hdc, fontLarge, 280, 100, "PAUSED", RGB(33, 48, 87));
    TextCenter(hdc, fontNormal, 280, 150, "The clock is stopped.", RGB(104, 120, 152));
    DrawPanel(hdc);
    DrawButtons(hdc);
    DrawBox(hdc, BOARD_X - 10, 512, COLS * CELL + 20, 38, 19, RGB(250, 252, 255), RGB(255, 255, 255), 2);
}


void DrawStars(HDC hdc, int centerX, int y, int earned)
{
    int i;
    for (i = 0; i < 3; i++)
        DrawStar(hdc, centerX + (i - 1) * 80, y, 36, 15, 5,
                 i < earned ? RGB(255, 190, 30) : RGB(200, 206, 218));
}


void StatLine(HDC hdc, int y, const char *label, const char *value, COLORREF colour)
{
    TextLeft(hdc, fontNormal, 230, y, label, RGB(104, 120, 152));
    TextRight(hdc, fontNormal, 570, y, value, colour);
}

void DrawLevelDone(HDC hdc)
{
    char text[40];

    DrawSky(hdc);
    DrawCard(hdc, 150, 30, 500, 490);
    sprintf(text, "LEVEL %d COMPLETE!", level);
    TextCenter(hdc, fontLarge, WIN_W / 2, 50, text, RGB(33, 48, 87));
    DrawStars(hdc, WIN_W / 2, 150, stars);

    sprintf(text, "%d / %d", movesUsed, levelMoves[level]);
    StatLine(hdc, 215, "Moves used", text, RGB(33, 48, 87));
    sprintf(text, "%d", balloonsPopped);
    StatLine(hdc, 245, "Balloons popped", text, RGB(33, 48, 87));
    sprintf(text, "+%d", bonus);
    StatLine(hdc, 275, "Bonus (moves left + time left)", text, RGB(230, 140, 10));

    TextLeft(hdc, fontMedium, 230, 325, "Total score", RGB(33, 48, 87));
    sprintf(text, "%d", score);
    TextRight(hdc, fontLarge, 570, 315, text, RGB(33, 48, 87));

    DrawButtons(hdc);
}

void DrawGameOver(HDC hdc)
{
    char text[60];
    const char *reason = "Better luck next time!";
    int i;

    if (loseReason == LOSE_MOVES)    reason = "You ran out of moves.";
    if (loseReason == LOSE_TIME)     reason = "Time is up!";
    if (loseReason == LOSE_NO_MATCH) reason = "No more matches are possible.";

    DrawSky(hdc);
    DrawCard(hdc, 150, 30, 500, 490);
    TextCenter(hdc, fontTitle, WIN_W / 2, 45, "GAME OVER", RGB(220, 58, 74));
    TextCenter(hdc, fontNormal, WIN_W / 2, 135, reason, RGB(104, 120, 152));

    for (i = 0; i < 5; i++)                                  
        DrawBalloon(hdc, 240 + i * 80, 215, 40, i);

    sprintf(text, "Level reached: %d", level);
    TextCenter(hdc, fontNormal, WIN_W / 2, 265, text, RGB(33, 48, 87));
    sprintf(text, "Final score: %d", score);
    TextCenter(hdc, fontLarge, WIN_W / 2, 295, text, RGB(33, 48, 87));

    if (newHighScore)
        TextCenter(hdc, fontMedium, WIN_W / 2, 350, "NEW HIGH SCORE!", RGB(240, 150, 10));
    DrawButtons(hdc);
}

void DrawWin(HDC hdc)
{
    char text[60];
    int i;

    DrawSky(hdc);
    DrawCard(hdc, 150, 30, 500, 490);
    TextCenter(hdc, fontTitle, WIN_W / 2, 45, "YOU WIN!", RGB(40, 150, 80));
    TextCenter(hdc, fontNormal, WIN_W / 2, 135, "You cleared all 5 levels - fantastic!", RGB(104, 120, 152));

    for (i = 0; i < 5; i++)
        DrawBalloon(hdc, 240 + i * 80, 215 + (int)(sin((tick + i * 4) / 6.0) * 6), 44, i);

    sprintf(text, "Final score: %d", score);
    TextCenter(hdc, fontLarge, WIN_W / 2, 285, text, RGB(33, 48, 87));

    if (newHighScore)
        TextCenter(hdc, fontMedium, WIN_W / 2, 345, "NEW HIGH SCORE!", RGB(240, 150, 10));
    DrawButtons(hdc);
}



void DrawScreen(HDC hdc)
{
    SetBkMode(hdc, TRANSPARENT);

    switch (screen) {
    case SCREEN_MENU:       DrawMenu(hdc);      break;
    case SCREEN_HELP:       DrawHelp(hdc);      break;
    case SCREEN_SCORES:     DrawScores(hdc);    break;
    case SCREEN_PLAYING:    DrawPlaying(hdc);   break;
    case SCREEN_PAUSED:     DrawPaused(hdc);    break;
    case SCREEN_LEVEL_DONE: DrawLevelDone(hdc); break;
    case SCREEN_GAME_OVER:  DrawGameOver(hdc);  break;
    case SCREEN_WIN:        DrawWin(hdc);       break;
    }
}



void BeginLevel(void)
{
    int i;
    for (i = 0; i < MAX_EFFECTS; i++) fxLife[i] = 0;
    popupLife = 0;
    endDelay  = 0;

    sprintf(message, "Level %d: score %d points in %d moves!", level, levelTarget[level], moves);
    messageLife = 60;
    ChangeScreen(SCREEN_PLAYING);
}

void BeginNewGame(void)
{
    StartNewGame();
    BeginLevel();
}


void SpawnEffects(void)
{
    int i, j;
    for (i = 0; i < popCount; i++) {
        for (j = 0; j < MAX_EFFECTS; j++) {
            if (fxLife[j] == 0) {                       
                fxX[j] = BOARD_X + popCol[i] * CELL + CELL / 2;
                fxY[j] = BOARD_Y + popRow[i] * CELL + CELL / 2 - 4;
                fxLife[j] = 7;
                fxColour[j] = popColour[i];
                break;
            }
        }
    }
}


void HandleBoardClick(int x, int y)
{
    int r = (y - BOARD_Y) / CELL;
    int c = (x - BOARD_X) / CELL;
    int points, wasBomb;

    if (x < BOARD_X || y < BOARD_Y || r >= ROWS || c >= COLS) return;   
    if (levelStatus != STATUS_PLAYING || board[r][c] == EMPTY) return;

    wasBomb = (board[r][c] == BOMB);                   
    points = PlayerClick(r, c);

    if (points == 0) {                                  
        PlaySoundEffect(SND_WRONG);
        ShowMessage("No match! Pick 2 or more touching balloons of one colour.");
        return;
    }

    SpawnEffects();
    sprintf(popupText, "+%d", points);
    popupX = x;  popupY = y - 20;  popupLife = 16;

    if (wasBomb)            PlaySoundEffect(SND_BOMB);
    else if (popCount > 6)  PlaySoundEffect(SND_BIG);
    else                    PlaySoundEffect(SND_POP);

    if (clearedBoard)    ShowMessage("Board cleared!  +200 bonus - new balloons arrive!");
    else if (madeBomb)   ShowMessage("You earned a BOMB! Click the black balloon.");
    else if (combo >= 3) {
        sprintf(message, "Combo x%d!  Keep popping for more points.", combo);
        messageLife = 40;
    }
}


void FinishLevel(void)
{
    if (levelStatus == STATUS_WON) {
        PlaySoundEffect(SND_WIN);
        if (level == MAX_LEVEL) {
            AddHighScore(score);
            ChangeScreen(SCREEN_WIN);
        } else {
            ChangeScreen(SCREEN_LEVEL_DONE);
        }
    } else {
        PlaySoundEffect(SND_LOSE);
        AddHighScore(score);
        ChangeScreen(SCREEN_GAME_OVER);
    }
}


void HandleButton(int id)
{
    switch (id) {
    case BTN_START:
    case BTN_AGAIN:   BeginNewGame();               break;
    case BTN_HELP:    ChangeScreen(SCREEN_HELP);    break;
    case BTN_SCORES:  ChangeScreen(SCREEN_SCORES);  break;
    case BTN_EXIT:    PostQuitMessage(0);           break;
    case BTN_BACK:
    case BTN_MENU:    ChangeScreen(SCREEN_MENU);    break;
    case BTN_PAUSE:   ChangeScreen(SCREEN_PAUSED);  break;
    case BTN_RESUME:  ChangeScreen(SCREEN_PLAYING); break;
    case BTN_RESTART: score = levelStartScore;      
                      StartLevel(level);
                      BeginLevel();                 break;
    case BTN_SOUND:   soundOn = !soundOn;
                      SetupButtons();               break;
    case BTN_NEXT:    StartLevel(level + 1);
                      BeginLevel();                 break;
    }
}


void HandleClick(int x, int y)
{
    int i;

    for (i = 0; i < buttonCount; i++) {                 
        struct Button *b = &buttons[i];
        if (x >= b->x && x < b->x + b->w && y >= b->y && y < b->y + b->h) {
            PlaySoundEffect(SND_CLICK);
            HandleButton(b->id);
            return;
        }
    }
    if (screen == SCREEN_PLAYING) HandleBoardClick(x, y);
}


void HandleKey(int key)
{
    if (key == 'M') { soundOn = !soundOn; SetupButtons(); }

    if (screen == SCREEN_PLAYING) {
        if (key == 'P' || key == 27) HandleButton(BTN_PAUSE);       
        if (key == 'R')              HandleButton(BTN_RESTART);
    }
    else if (screen == SCREEN_PAUSED) {
        if (key == 'P' || key == 27) HandleButton(BTN_RESUME);
        if (key == 'R')              HandleButton(BTN_RESTART);
    }
    else if (screen == SCREEN_MENU && key == 13) HandleButton(BTN_START);         
    else if (screen == SCREEN_LEVEL_DONE && key == 13) HandleButton(BTN_NEXT);
    else if ((screen == SCREEN_GAME_OVER || screen == SCREEN_WIN) && key == 13) HandleButton(BTN_AGAIN);
    else if ((screen == SCREEN_HELP || screen == SCREEN_SCORES) && key == 27) HandleButton(BTN_BACK);
}


void UpdateGame(void)
{
    int i;

    tick++;

    
    for (i = 0; i < MAX_EFFECTS; i++)
        if (fxLife[i] > 0) fxLife[i]--;
    if (popupLife > 0) { popupLife--; popupY = popupY - 2; }
    if (messageLife > 0) messageLife--;

    if (screen != SCREEN_PLAYING) return;

    CountDownTime(0.05f);                              

    if (levelStatus != STATUS_PLAYING) {                
        endDelay++;
        if (endDelay >= 15) FinishLevel();
    }
}



LRESULT CALLBACK WindowProc(HWND hwnd, UINT message_id, WPARAM wParam, LPARAM lParam)
{
    switch (message_id) {

    case WM_CREATE:                                     
        srand((unsigned)time(NULL));
        CreateFonts();
        MakeAllSounds();
        LoadHighScores();
        ChangeScreen(SCREEN_MENU);
        SetTimer(hwnd, 1, 50, NULL);                    
        return 0;

    case WM_TIMER:                                      
        UpdateGame();
        InvalidateRect(hwnd, NULL, FALSE);              
        return 0;

    case WM_MOUSEMOVE:
        mouseX = (short)LOWORD(lParam);
        mouseY = (short)HIWORD(lParam);
        return 0;

    case WM_LBUTTONDOWN:
        HandleClick((short)LOWORD(lParam), (short)HIWORD(lParam));
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;

    case WM_KEYDOWN:
        HandleKey((int)wParam);
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;

    case WM_KILLFOCUS:                                 
        if (screen == SCREEN_PLAYING) ChangeScreen(SCREEN_PAUSED);
        return 0;

    case WM_ERASEBKGND:
        return 1;                                      

    case WM_PAINT: {                                   
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        
        HDC memory = CreateCompatibleDC(hdc);
        HBITMAP picture = CreateCompatibleBitmap(hdc, WIN_W, WIN_H);
        HGDIOBJ oldPicture = SelectObject(memory, picture);

        DrawScreen(memory);
        BitBlt(hdc, 0, 0, WIN_W, WIN_H, memory, 0, 0, SRCCOPY);

        SelectObject(memory, oldPicture);
        DeleteObject(picture);
        DeleteDC(memory);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:                                
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, message_id, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR commandLine, int showMode)
{
    WNDCLASS windowClass;
    HWND window;
    MSG msg;
    RECT size = { 0, 0, WIN_W, WIN_H };

    (void)previous;                 
    (void)commandLine;
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;  

    
    memset(&windowClass, 0, sizeof(windowClass));
    windowClass.lpfnWndProc   = WindowProc;
    windowClass.hInstance     = instance;
    windowClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = "BalloonPopWindow";
    RegisterClass(&windowClass);

  
    AdjustWindowRect(&size, style, FALSE);
    window = CreateWindow("BalloonPopWindow", "Balloon Pop: Connect & Match", style,
                          CW_USEDEFAULT, CW_USEDEFAULT, size.right - size.left, size.bottom - size.top,
                          NULL, NULL, instance, NULL);
    ShowWindow(window, showMode);

    
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
