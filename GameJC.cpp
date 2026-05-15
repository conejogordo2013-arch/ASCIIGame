/* ============================
   GAME TERMUX PORT - PARTE 1/4
   ============================ */

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <cstdio>
#include <cmath>
using namespace std;

/* ---------------------------
   ANSI / Console
   --------------------------- */

inline void clearScreen() {
    printf("\033[2J");
    fflush(stdout);
}

inline void gotoxy(int x, int y) {
    printf("\033[%d;%dH", y, x);
    fflush(stdout);
}

inline void setColor(int c) {
    printf("\033[%dm", c);
    fflush(stdout);
}

inline void resetColor() {
    printf("\033[0m");
    fflush(stdout);
}

inline void hideCursor() {
    printf("\033[?25l");
}

inline void showCursor() {
    printf("\033[?25h");
}

inline void sleepMs(int ms) {
    usleep(ms * 1000);
}

/* ---------------------------
   INPUT (termios)
   --------------------------- */

int kbhit() {
    termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

int getch_block() {
    termios old, n;
    int ch;
    tcgetattr(0, &old);
    n = old;
    n.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &n);
    ch = getchar();
    tcsetattr(0, TCSANOW, &old);
    return ch;
}

int getch_nb() {
    if (!kbhit()) return -1;
    return getchar();
}

/* ---------------------------
   STRUCTS / OBJETOS DEL JUEGO
   --------------------------- */

struct Player {
    int x=1, y=1;
    int lifes=3;
    int ammo=10;
    int respawnTime=0;
    char dir='d';
    bool lostLife=false;
    int checkpoint=0;

    // Añadir estas variables para checkpoint real
    int checkpointX = 1;
    int checkpointY = 1;
};

struct PickUp {
    int x=0,y=0;
    char type=' ';
    bool exists=false;
};

struct Cannon {
    int x=0,y=0;
    char dir='a';
    int speed=30;
    bool exists=false;
    int counter=0;
};

struct Bullet {
    int x=0,y=0;
    char dir='d';
    bool exists=false;
    bool fromPlayer=false;
};

struct Runner {
    int x=0,y=0;
    char dir='d';
    int hp=2;
    bool exists=false;
};

struct Randomer {
    int x=0,y=0;
    int xmin=0,xmax=0,ymin=0,ymax=0;
    int hp=2;
    int speed=3;
    int counter=0;
    bool exists=false;
};

struct Chaser {
    int x=0,y=0;
    bool exists=false;
    int speed=1;
    int counter=0;
};

/* ---------------------------
   VARIABLES GLOBALES
   --------------------------- */

Player player;

vector<string> levelMap;
int mapW=0, mapH=0;

int framesDelay = 12;
bool inGame=false;
bool exitGame=false;
bool advancedPort=false;

int frameCount=0;
int bossLife=0;
bool bossPlaced=false;

vector<PickUp> pickups;
vector<Cannon> cannons;
vector<Bullet> bullets;
vector<Runner> runners;
vector<Randomer> randomers;
vector<Chaser> chasers;

inline bool insideMap(int x, int y) {
    return x>=1 && x<=mapW && y>=1 && y<=mapH;
}

char baseMapAt(int x, int y) {
    if (!insideMap(x,y)) return '#';
    return levelMap[y-1][x-1];
}

bool isBlockingStatic(int x,int y) {
    char b = baseMapAt(x,y);
    if (b==' ' || b=='.' ) return false;
    return true;
}

char dynamicAt(int x,int y) {
    for (auto &p: pickups) if (p.exists && p.x==x && p.y==y) return p.type;
    for (auto &r: runners) if (r.exists && r.x==x && r.y==y) return 'S';
    for (auto &r: randomers) if (r.exists && r.x==x && r.y==y) return '%';
    for (auto &c: chasers) if (c.exists && c.x==x && c.y==y) return '&';
    for (auto &b: bullets) if (b.exists && b.x==x && b.y==y) return 'o';
    if (player.x==x && player.y==y) return '>';
    return 0;
}

char readAt(int x,int y) {
    char d = dynamicAt(x,y);
    if (d) return d;
    return baseMapAt(x,y);
}
/* ============================
   GAME TERMUX PORT - PARTE 2/4
   ============================ */

/* ---------------------------
   CARGA DE NIVELES
   --------------------------- */

bool loadLevelFromFile(const string &filename) {
    ifstream f(filename);
    if (!f.is_open()) return false;

    levelMap.clear();
    string line;
    while (getline(f,line)) {
        if (!line.empty() && line.back()=='\r')
            line.pop_back();
        levelMap.push_back(line);
    }

    if (levelMap.empty()) return false;

    mapH = levelMap.size();
    mapW = 0;

    for (auto &l: levelMap)
        if ((int)l.size() > mapW)
            mapW = l.size();

    for (auto &l: levelMap)
        if ((int)l.size() < mapW)
            l += string(mapW - l.size(), ' ');

    pickups.clear();
    cannons.clear();
    bullets.clear();
    runners.clear();
    randomers.clear();
    chasers.clear();
    bossPlaced = false;
    bossLife = 0;

    for (int y=1;y<=mapH;y++){
        for (int x=1;x<=mapW;x++){
            char c = levelMap[y-1][x-1];

            if (c=='A' || c=='+' || c=='C') {
                PickUp p; p.x=x; p.y=y; p.type=c; p.exists=true;
                pickups.push_back(p);
                levelMap[y-1][x-1] = ' ';
            }
            else if (c=='-' || c=='|') {
                Cannon cc;
                cc.exists = true;
                cc.x=x; cc.y=y;
                cc.counter = 0;
                if (c=='-') cc.dir = 'd';
                else cc.dir = 's';
                cc.speed = 30;
                cannons.push_back(cc);
                levelMap[y-1][x-1] = ' ';
            }
            else if (c=='S') {
                Runner r;
                r.exists=true;
                r.x=x; r.y=y;
                r.hp=2;
                r.dir='d';
                runners.push_back(r);
                levelMap[y-1][x-1] = ' ';
            }
            else if (c=='%') {
                Randomer rr;
                rr.exists=true;
                rr.x=x; rr.y=y;
                rr.hp=2;
                rr.speed=3;
                rr.xmin = max(1, x-4);
                rr.xmax = min(mapW, x+4);
                rr.ymin = max(1, y-3);
                rr.ymax = min(mapH, y+3);
                randomers.push_back(rr);
                levelMap[y-1][x-1] = ' ';
            }
            else if (c=='&') {
                Chaser ch;
                ch.exists=true;
                ch.x=x; ch.y=y;
                ch.speed=1;
                chasers.push_back(ch);
                levelMap[y-1][x-1] = ' ';
            }
            else if (c=='>') {
                player.x=x; player.y=y;
                levelMap[y-1][x-1] = ' ';
            }
            else if (c=='X') {
                levelMap[y-1][x-1] = 'X';
            }
        }
    }

    return true;
}

/* ---------------------------
   RENDER
   --------------------------- */

void drawFullScreen() {
    clearScreen();
    hideCursor();

    for (int y=1;y<=mapH;y++) {
        gotoxy(1,y);
        for (int x=1;x<=mapW;x++) {
            char ch = baseMapAt(x,y);

            if(ch=='#') { setColor(37); putchar('#'); }
            else if(ch=='@') { setColor(90); putchar('@'); } // Pared empujable
            else if(ch=='-') { setColor(31); putchar('-'); }
            else if(ch=='|') { setColor(31); putchar('|'); }
            else if(ch=='X') { setColor(90); putchar('X'); }
            else putchar(ch);
        }
    }

    // Dibujar pickups, enemigos, balas...
    for (auto &p: pickups) if (p.exists) {
        gotoxy(p.x, p.y);
        if (p.type=='A') { setColor(33); putchar('A'); }
        else if (p.type=='+') { setColor(32); putchar('+'); }
        else if (p.type=='C') { setColor(37); putchar('C'); }
    }

    for (auto &c: cannons) if (c.exists) {
        gotoxy(c.x,c.y);
        setColor(35);
        putchar(c.dir=='a' || c.dir=='d' ? '-' : '|');
    }

    for (auto &r: runners) if (r.exists) {
        gotoxy(r.x,r.y);
        setColor(32);
        putchar('S');
    }

    for (auto &r: randomers) if (r.exists) {
        gotoxy(r.x,r.y);
        setColor(35);
        putchar('%');
    }

    for (auto &c: chasers) if (c.exists) {
        gotoxy(c.x,c.y);
        setColor(34);
        putchar('&');
    }

    for (auto &b: bullets) if (b.exists) {
        gotoxy(b.x,b.y);
        setColor(31);
        putchar('o');
    }

    // Dibujar jugador
  char playerChar = '>';  // por defecto derecha
if(player.dir=='w') playerChar = '^';
else if(player.dir=='s') playerChar = 'v';
else if(player.dir=='a') playerChar = '<';
else if(player.dir=='d') playerChar = '>';

    gotoxy(player.x, player.y);
    setColor(33); putchar(playerChar);

    resetColor();

    gotoxy(mapW+2,2);
    setColor(33); printf("Lives: %d", player.lifes);

    gotoxy(mapW+2,4);
    printf("Ammo: %d", player.ammo);

    gotoxy(mapW+2,6);
    printf("FPS: %dms", framesDelay);

    gotoxy(mapW+2,8);
    printf("Checkpoint: %d", player.checkpoint);

    if (advancedPort && bossPlaced) {
        gotoxy(mapW+2,10);
        printf("Boss HP: %d", bossLife);
    }

    resetColor();
    fflush(stdout);
}

/* ---------------------------
   BALAS
   --------------------------- */

void spawnBullet(int x, int y, char dir, bool fromPlayer=false) {
    Bullet b;
    b.x=x; b.y=y;
    b.dir=dir;
    b.exists=true;
    b.fromPlayer = fromPlayer;
    bullets.push_back(b);
}

void updateBullets() {
    for (auto &b: bullets) {
        if (!b.exists) continue;

        int nx=b.x, ny=b.y;

        if (b.dir=='w') ny--;
        else if (b.dir=='s') ny++;
        else if (b.dir=='a') nx--;
        else if (b.dir=='d') nx++;

        if (!insideMap(nx,ny) || isBlockingStatic(nx,ny) || baseMapAt(nx,ny)=='X') {

            if (baseMapAt(nx,ny)=='X' && b.fromPlayer && advancedPort && bossPlaced)
                bossLife--;

            b.exists=false;
            continue;
        }

        bool stopped=false;

        for (auto &r: runners)
            if (r.exists && r.x==nx && r.y==ny) {
                r.hp--;
                if (r.hp<=0) r.exists=false;
                b.exists=false;
                stopped=true;
                break;
            }

        if (stopped) continue;

        for (auto &r: randomers)
            if (r.exists && r.x==nx && r.y==ny) {
                r.hp--;
                if (r.hp<=0) r.exists=false;
                b.exists=false;
                stopped=true;
                break;
            }

        if (stopped) continue;

        for (auto &c: chasers)
            if (c.exists && c.x==nx && c.y==ny) {
                b.exists=false;
                stopped=true;
                break;
            }

        if (stopped) continue;

        if (nx==player.x && ny==player.y) {
            if (!b.fromPlayer) {
                player.lostLife = true;
                player.lifes--;
                player.respawnTime = 20;
            }
            b.exists=false;
            continue;
        }

        b.x=nx; b.y=ny;
    }

    if (bullets.size()>700) {
        vector<Bullet> nb;
        for (auto &b: bullets) if (b.exists) nb.push_back(b);
        bullets = nb;
    }
}
/* ============================
   GAME TERMUX PORT - PARTE 3/4
   ============================ */

/* ---------------------------
   ENEMIGOS ALEATORIOS (%)
   --------------------------- */

int randRange(int a,int b){ return a + (rand() % (b-a+1)); }

void updateRandomers() {
    for (auto &r: randomers) {
        if (!r.exists) continue;

        r.counter++;
        if (r.counter < r.speed) continue;
        r.counter = 0;

        int d = randRange(0,3);
        int nx=r.x, ny=r.y;

        if (d==0) ny--;
        if (d==1) ny++;
        if (d==2) nx--;
        if (d==3) nx++;

        if (nx<r.xmin || nx>r.xmax || ny<r.ymin || ny>r.ymax)
            continue;

        if (isBlockingStatic(nx,ny)) {
            if (nx==player.x && ny==player.y) {
                player.lostLife=true;
                player.lifes--;
                player.respawnTime=20;
            }
            continue;
        }

        r.x=nx; r.y=ny;

        if (r.x==player.x && r.y==player.y) {
            player.lostLife=true;
            player.lifes--;
            player.respawnTime=20;
        }
    }
}

/* ---------------------------
   ENEMIGOS RUNNER (S)
   --------------------------- */

void updateRunners() {
    for (auto &r: runners) {
        if (!r.exists) continue;

        int nx = r.x + (r.dir=='d' ? 1 : -1);

        if (isBlockingStatic(nx,r.y)) {
            r.dir = (r.dir=='d'? 'a':'d');
            continue;
        }

        if (nx==player.x && r.y==player.y) {
            player.lostLife=true;
            player.lifes--;
            player.respawnTime=20;
        }

        r.x = nx;
    }
}

/* ---------------------------
   ENEMIGOS PERSEGUIDORES (&)
   --------------------------- */

void updateChasers() {
    for (auto &c: chasers) {
        if (!c.exists) continue;

        c.counter++;
        if (c.counter < c.speed) continue;
        c.counter = 0;

        int dx = player.x - c.x;
        int dy = player.y - c.y;

        int nx=c.x, ny=c.y;

        if (abs(dx) >= abs(dy))
            nx += (dx>0?1:-1);
        else
            ny += (dy>0?1:-1);

        if (!isBlockingStatic(nx,ny)) {
            if (nx==player.x && ny==player.y) {
                player.lostLife=true;
                player.lifes--;
                player.respawnTime=20;
            }
            c.x=nx; c.y=ny;
        } else {
            if (abs(dx)>=abs(dy)) {
                int ty = c.y + (dy>0?1:-1);
                if (!isBlockingStatic(c.x,ty)) {
                    if (c.x==player.x && ty==player.y) {
                        player.lostLife=true;
                        player.lifes--;
                        player.respawnTime=20;
                    }
                    c.y=ty;
                }
            } else {
                int tx = c.x + (dx>0?1:-1);
                if (!isBlockingStatic(tx,c.y)) {
                    if (tx==player.x && c.y==player.y) {
                        player.lostLife=true;
                        player.lifes--;
                        player.respawnTime=20;
                    }
                    c.x=tx;
                }
            }
        }
    }
}

/* ---------------------------
   CAÑONES
   --------------------------- */

void updateCannons() {
    for (auto &c: cannons) {
        if (!c.exists) continue;

        c.counter++;

        if (c.counter >= c.speed) {
            c.counter = 0;

            int bx=c.x, by=c.y;

            if (c.dir=='w') by--;
            else if (c.dir=='s') by++;
            else if (c.dir=='a') bx--;
            else if (c.dir=='d') bx++;

            if (!isBlockingStatic(bx,by))
                spawnBullet(bx,by,c.dir,false);
        }
    }
}

/* ---------------------------
   RECOLECCIÓN
   --------------------------- */

void collectPickups() {
    for (auto &p: pickups)
        if (p.exists && p.x==player.x && p.y==player.y) {

            if (p.type=='A') player.ammo += 5;
            else if (p.type=='+') player.lifes += 1;            p.exists = false;
            
          if (p.type == 'C') {
    // Solo se guarda la posición del objeto 'C', no la del jugador en cualquier momento
    player.checkpointX = p.x; 
    player.checkpointY = p.y;
    player.checkpoint += 1;
    p.exists = false; // El checkpoint desaparece al tocarlo
}
        }
}

/* ---------------------------
   INPUT DEL JUGADOR
   --------------------------- */

void handlePlayerInput(int c) {
    if (c == -1) return;

    if (c == 27) { // ESC
        inGame = false;
        return;
    }

    if (c == 'w' || c == 's' || c == 'a' || c == 'd') {
        player.dir = c;
        int nx = player.x;
        int ny = player.y;

        if (c == 'w') ny--;
        else if (c == 's') ny++;
        else if (c == 'a') nx--;
        else if (c == 'd') nx++;

        char d = dynamicAt(nx, ny);

// Detecta @ en el mapa estático
if (levelMap[ny-1][nx-1] == '@') {

    // Calcular posición detrás
    int px = nx, py = ny;
    if (c == 'w') py--;
    else if (c == 's') py++;
    else if (c == 'a') px--;
    else if (c == 'd') px++;

    // Si DETRÁS está libre → empuja
    if (!isBlockingStatic(px, py) && dynamicAt(px, py) == 0) {
        // mover la @
        levelMap[ny-1][nx-1] = ' ';
        levelMap[py-1][px-1] = '@';

        // mover jugador
        player.x = nx;
        player.y = ny;
    }

    // Si NO se puede empujar → bloquea
    return;
}
        // Mover jugador normalmente
        else if (!isBlockingStatic(nx, ny) && d != 'S' && d != '%' && d != '&') {
            player.x = nx;
            player.y = ny;
        }
        // Colisión con enemigos
        else if (d == 'S' || d == '%' || d == '&') {
            player.lostLife = true;
            player.lifes--;
            player.respawnTime = 20;
        }
    }

    // Disparo
    else if (c == 'm') {
        if (player.ammo > 0) {
            int bx = player.x;
            int by = player.y;

            if (player.dir == 'w') by--;
            else if (player.dir == 's') by++;
            else if (player.dir == 'a') bx--;
            else if (player.dir == 'd') bx++;

            if (!isBlockingStatic(bx, by)) {
                spawnBullet(bx, by, player.dir, true);
                player.ammo--;
            }
        }
    }

    // Respawn / checkpoint
    else if (c == 'r') {
       player.lifes -= 1;
       player.lostLife = true; 
       player.respawnTime = 20;

        if (player.checkpoint == 0) {
            player.x = 2;
            player.y = mapH - 2;
        } else {
            player.x = max(2, mapW / 2);
            player.y = max(2, mapH / 2);
        }
    }
}
/* ---------------------------
   JEFE (BOSS)
   --------------------------- */

void maybeSpawnBoss() {
    if (!advancedPort) return;

    if (player.checkpoint >= 5 && !bossPlaced) {
        bossPlaced = true;
        bossLife = 60;

        int cx = min(mapW-5, max(10,mapW*3/4));
        int cy = min(mapH-6, max(6,mapH/2));

        for (int yy=cy;yy<cy+5;yy++)
            for (int xx=cx;xx<cx+7;xx++)
                if (insideMap(xx,yy))
                    levelMap[yy-1][xx-1] = 'X';

        for (int i=0;i<4;i++) {
            Cannon c;
            c.exists=true;
            c.x=cx+i*2;
            c.y=cy-1;
            c.dir='s';
            c.speed=20;
            cannons.push_back(c);
        }
    }
}
/* ============================
   GAME TERMUX PORT - PARTE 4/4
   ============================ */

/* ---------------------------
   GAME LOOP
   --------------------------- */

void gameLoop() {
    inGame = true;
    frameCount = 0;

    if (player.x < 1 || player.y < 1) {
        player.x = 2;
        player.y = max(2, mapH-2);
    }

    hideCursor();
    while (inGame && !exitGame) {
        int c = -1;
        if (kbhit()) c = getchar();
        handlePlayerInput(c);

        if (frameCount % 1 == 0) updateCannons();
        if (frameCount % 2 == 0) updateRandomers();
        if (frameCount % 2 == 0) updateRunners();
        if (frameCount % 1 == 0) updateChasers();

        updateBullets();
        collectPickups();
        maybeSpawnBoss();

        drawFullScreen();

        if (player.lostLife) {
    for (int i=0;i<3;i++){
        gotoxy(player.x, player.y);
        setColor(31);
        putchar('>');
        fflush(stdout);
        sleepMs(120);
        gotoxy(player.x, player.y);
        putchar(' ');
        fflush(stdout);
        sleepMs(120);
    }
    player.lostLife = false;

    // REAPARECER EN CHECKPOINT
    player.checkpointX = player.x;
    player.checkpointY = player.y;
}

        if (player.lifes <= 0) {
            inGame = false;
            gotoxy(max(1, mapW/2 - 6), max(1, mapH/2));
            setColor(33);
            printf("GAME OVER");
            resetColor();
            gotoxy(1, mapH + 2);
            printf("Press any key to return to menu...");
            fflush(stdout);
            getch_block();
            break;
        }

        if (bossPlaced && bossLife <= 0) {
            inGame = false;
            gotoxy(max(1, mapW/2 - 6), max(1, mapH/2));
            setColor(32);
            printf("YOU WON!");
            resetColor();
            gotoxy(1, mapH + 2);
            printf("Press any key to return to menu...");
            fflush(stdout);
            getch_block();
            break;
        }

        frameCount++;
        sleepMs(framesDelay);
    }
    showCursor();
}

/* ---------------------------
   MENÚ Y OPCIONES
   --------------------------- */

string promptFilename(const string &hint="levels/level1.txt") {
    showCursor();
    gotoxy(1, mapH + 2);
    printf("Enter level filename (or ENTER for %s): ", hint.c_str());
    fflush(stdout);
    string s;
    getline(cin, s);
    if (s.empty()) s = hint;
    hideCursor();
    return s;
}

void adjustSpeedMenu() {
    bool back=false;
    while (!back) {
        clearScreen();
        gotoxy(10,5);
        setColor(33);
        printf("Adjust speed (frame delay ms)");
        resetColor();
        gotoxy(10,7);
        printf("Current frames delay: %d ms", framesDelay);
        gotoxy(10,9);
        printf("Use '+' to increase delay (slower), '-' to decrease (faster).");
        gotoxy(10,11);
        printf("Press ENTER to go back.");
        fflush(stdout);
        int c = getch_block();
        if (c=='+') framesDelay += 2;
        else if (c=='-' && framesDelay>1) framesDelay -= 2;
        else if (c==10 || c==13) back=true;
    }
}

void mainMenu(const string &initialFile) {
    string currentLevel = initialFile;
    bool inMenu=true;
    int choice = 0;
    vector<string> items = { "START", "CARGAR NIVEL (.txt)", "OPCIONES (SPEED)", "HACER EL PORT COMPLETO", "SALIR" };

    while (inMenu && !exitGame) {
        clearScreen();
        gotoxy(10,3);
        setColor(33);
        printf("=== CONSOLE GAME (Termux) ===");
        resetColor();
        gotoxy(10,5);
        printf("Level file: %s", currentLevel.c_str());

        for (int i=0;i<(int)items.size();++i) {
            gotoxy(10,8+i*2);
            if (i==choice) {
                setColor(32);
                printf("-> %s", items[i].c_str());
                resetColor();
            } else {
                printf("   %s", items[i].c_str());
            }
        }
        fflush(stdout);

        int c = getch_block();
        if (c=='w') { if (choice>0) choice--; }
        else if (c=='s') { if (choice<(int)items.size()-1) choice++; }
        else if (c==10 || c==13) {
            string sel = items[choice];
            if (sel=="START") {
                if (!loadLevelFromFile(currentLevel)) {
                    gotoxy(10, 18);
                    setColor(31);
                    printf("Cannot open level: %s", currentLevel.c_str());
                    resetColor();
                    gotoxy(10,20);
                    printf("Press any key...");
                    fflush(stdout);
                    getch_block();
                } else {
                    player.lifes = 3; player.ammo = 15; player.respawnTime = 0; player.lostLife = false;
                    if (player.x<1 || player.y<1) { player.x=2; player.y=max(2,mapH-2); }
                    if (advancedPort) {
                        for (int i=0;i<5;i++) {
                            Runner r; r.exists=true; r.hp=2;
                            r.x = min(mapW, 3 + i*4); r.y = min(mapH-2, 4 + (i%3)*3); r.dir = (i%2? 'a':'d');
                            runners.push_back(r);
                        }
                        for (int i=0;i<4;i++) {
                            Randomer rr; rr.exists=true; rr.hp=2; rr.speed = 2 + (i%3);
                            rr.x = min(mapW-3, 6 + i*6); rr.y = min(mapH-3, 6 + (i%2)*8);
                            rr.xmin = max(1, rr.x-3); rr.xmax = min(mapW, rr.x+3);
                            rr.ymin = max(1, rr.y-2); rr.ymax = min(mapH, rr.y+2);
                            randomers.push_back(rr);
                        }
                        for (int i=0;i<3;i++) {
                            Chaser ch; ch.exists=true; ch.x = max(2, mapW-5 - i*4); ch.y = 4 + i*5;
                            chasers.push_back(ch);
                        }
                    }
                    hideCursor();
                    gameLoop();
                }
            } else if (sel=="CARGAR NIVEL (.txt)") {
                showCursor();
                gotoxy(10,18);
                printf("File: ");
                fflush(stdout);
                string fn; getline(cin, fn);
                if (!fn.empty()) currentLevel = fn;
                hideCursor();
            } else if (sel=="OPCIONES (SPEED)") {
                adjustSpeedMenu();
            } else if (sel=="HACER EL PORT COMPLETO") {
                advancedPort = !advancedPort;
                gotoxy(10,18);
                setColor(33);
                printf("Advanced port: %s", advancedPort? "ENABLED":"DISABLED");
                resetColor();
                gotoxy(10,20);
                printf("Press any key...");
                fflush(stdout);
                getch_block();
            } else if (sel=="SALIR") {
                exitGame = true;
                inMenu = false;
            }
        } else if (c==27) {
            exitGame = true;
            inMenu = false;
        }
    }
}

/* ---------------------------
   MAIN
   --------------------------- */

int main(int argc, char** argv) {
    srand((unsigned)time(nullptr));

    string defaultLevel = "level1.txt";
    if (argc > 1) defaultLevel = string(argv[1]);

    if (argc > 1) {
        if (!loadLevelFromFile(defaultLevel)) {
            printf("Cannot open level: %s\n", defaultLevel.c_str());
            printf("Starting menu. Press ENTER to continue...\n");
            getch_block();
        } else {
            hideCursor();
            gameLoop();
            showCursor();
            return 0;
        }
    }

    mainMenu(defaultLevel);

    resetColor();
    showCursor();
    clearScreen();
    return 0;
}