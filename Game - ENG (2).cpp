/// The Code is not well written, I have made a lot of wrong programming decisions.
/// I pointed some with comments, but there are much more "quick fixes" to problems
/// which lead to spaghetti code.

/// Either way console game never will be actually really good because of limitations
/// (of course except things like text-based games)
#include <iostream>
#include <cstdlib>
#include <windows.h>
#include <time.h>
#include <fstream>
#include <conio.h>
using namespace std;

class Player
{
public:
    int x;
    int y;
    int lifes;
    int ammunition;
    int respawnTime;
    char movingDirection;   /// As in 'w' = UP; 's' = DOWN; 'a' = LEFT; 'd' = RIGHT
    bool lostLife;
    int checkPoint;
};
class PickUp
{
public:
    int x;
    int y;
    bool exists;
};

class CannonHorizontal        /// CannonHorizontal and CannonVertical should have been same object with "direction" variable
{
public:
    int x;
    int y;
    char shootingDirection;   /// As in 'w' = UP; 's' = DOWN; 'a' = LEFT; 'd' = RIGHT
    int speed;
    bool exists;
};
class CannonVertical          /// CannonHorizontal and CannonVertical should have been same object with "direction" variable
{
public:
    int x;
    int y;
    char shootingDirection;   /// As in 'w' = UP; 's' = DOWN; 'a' = LEFT; 'd' = RIGHT
    int speed;
    bool exists;
};

class RunnerHorizontal         /// Runner = name of the green enemy that bounces between walls
{                              /// Same mistake as cannons, but I haven't made the vertical version
public:
    int x;
    int y;
    int lifes;
    char movingDirection;      /// As in 'w' = UP; 's' = DOWN; 'a' = LEFT; 'd' = RIGHT
};

class Randomer                 /// Randomer = name of the violet enemy that moves randomly in certain area
{
public:
    int x;
    int y;
    int xMax;
    int xMin;
    int yMax;
    int yMin;
    int lifes;
    bool successfulMove;
    int speed;
};
class Chaser                   /// Chaser = name of the blue enemy that chaises you || Works very badly in the game :)
{
public:
    int x;
    int y;
    bool exists;
    int speed();
};

class Bullet
{
public:
    int x;
    int y;
    char movingDirection;       /// As in 'w' = UP; 's' = DOWN; 'a' = LEFT; 'd' = RIGHT
    bool exists;
    bool firstOccurrence;
};

Player player;
int framesDelay;
int bossLife;       /// Why the fuck is it a global variable?
bool bossPlaced=false;      /// Why the fuck is it a global variable?
bool chasersPlaced=false;       /// Why the fuck is it a global variable?
char c;     /// Why the fuck is it a global variable?
bool exitConsole=false;
bool inGame;
Chaser chaser[30];
PickUp pickUp[40];
Randomer randomer[50];
CannonHorizontal cannonHorizontal[50];
CannonVertical cannonVertical[50];
RunnerHorizontal runnerHorizontal[100];
Bullet bullet[400];
int nrBullet;       /// Bullet number is a shitty solution to a problem made by not using dynamic arrays or vectors
int frameCount; /// for game objects -> instead of adding and removing bullets from array they get a number
                    /// which represents its position in static array
void gotoxy(int x, int y)
{
  COORD coord;
  coord.X = x;
  coord.Y = y;
  SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
char getCursorChar()    /// Function which returns character on console's cursor position || Totally not copied from the Internet
{
    char c = '\0';
    CONSOLE_SCREEN_BUFFER_INFO con;
    HANDLE hcon = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hcon != INVALID_HANDLE_VALUE &&
        GetConsoleScreenBufferInfo(hcon,&con))
    {
        DWORD read = 0;
        if (!ReadConsoleOutputCharacterA(hcon,&c,1,
            con.dwCursorPosition,&read) || read != 1
            )
            c = '\0';
    }
    return c;
}


void loadLevelBase(string fileName)    /// Function which loads static level walls from text file
{
    string getContent;
    ifstream openfile(fileName.c_str());
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),15);
    if(openfile.is_open())
    {
        while(! openfile.eof())
        {
            getline(openfile, getContent);
            cout << getContent << endl;
        }
    }
}


char readChar(int x,int y)  /// Function which reads character at specific coordinates
{
    gotoxy(x,y);
    char ccccc = getCursorChar();
    return ccccc;
}

void createMovableBox(int x,int y)
{
    gotoxy(x,y);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),8);
    cout<<"@";
}
void createChaser(int id,int x,int y)
{
    chaser[id].x=x;
    chaser[id].y=y;
    chaser[id].exists=true;
    gotoxy(x,y);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),9);
    cout<<"&";
}

void createCannonHorizontal(int id, int x,int y, char shootingDirection,int speed)
{
    cannonHorizontal[id].x=x;
    cannonHorizontal[id].y=y;
    cannonHorizontal[id].shootingDirection=shootingDirection;
    cannonHorizontal[id].speed=speed;
    cannonHorizontal[id].exists=true;
    gotoxy(x,y);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),12);
    cout<<"-";
}
void createCannonVertical(int id, int x,int y, char shootingDirection,int speed)
{
    cannonVertical[id].x=x;
    cannonVertical[id].y=y;
    cannonVertical[id].shootingDirection=shootingDirection;
    cannonVertical[id].speed=speed;
    cannonVertical[id].exists=true;
    gotoxy(x,y);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),12);
    cout<<"|";
}

void createRandomer(int id,int x,int y, int lifes,int range,int speed)
{
    randomer[id].x=x;
    randomer[id].y=y;
    randomer[id].xMin=x-range;
    randomer[id].yMin=y-range;
    randomer[id].xMax=x+range;
    randomer[id].yMax=y+range;
    randomer[id].lifes=lifes;
    randomer[id].speed=speed;
    gotoxy(x,y);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),13);
    cout<<"%";
}

void createRunnerHorizontal(int id,int x,int y,char movingDirection,int lifes)
{
    runnerHorizontal[id].x=x;
    runnerHorizontal[id].y=y;
    runnerHorizontal[id].lifes=lifes;
    runnerHorizontal[id].movingDirection=movingDirection;
    gotoxy(x,y);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),10);
    cout<<"S";
}
void createPickUpLife(int id,int x,int y)
{
    pickUp[id].exists=true;
    pickUp[id].x=x;
    pickUp[id].y=y;
    gotoxy(x,y);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),11);
    cout<<"+";
}

void createPickUpAmunicja(int id,int x,int y)
{
    pickUp[id].exists=true;
    pickUp[id].x=x;
    pickUp[id].y=y;
    gotoxy(x,y);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),14);
    cout<<"A";
}

void createPickUpCheckPoint(int id,int x,int y)
{
    pickUp[id].exists=true;
    pickUp[id].x=x;
    pickUp[id].y=y;
    gotoxy(x,y);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),240);
    cout<<"C";
}


void createBoss()   /// Should have read all the information from external file
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),8);
    gotoxy(69,19);
    cout<<"XXXXXXX";
    gotoxy(68,20);
    cout<<"XXXXXXXXX";
    gotoxy(67,21);
    cout<<"XXXXXXXXXXX";
    gotoxy(66,22);
    cout<<"XXXXXXXXXXXXX";
    gotoxy(66,23);
    cout<<"XXX XXXXX XXX";
    gotoxy(66,24);
    cout<<"XX   XXX   XX";
    gotoxy(66,25);
    cout<<"XXX XX XX XXX";
    gotoxy(66,26);
    cout<<"XXXXXX XXXXXX";
    gotoxy(67,27);
    cout<<"XXXXXXXXXXX";
    gotoxy(67,28);
    cout<<"X  XXXXX  X";
    gotoxy(67,29);
    cout<<"XX X X X XX";
    gotoxy(68,30);
    cout<<"X       X";
    gotoxy(68,31);
    cout<<"X       X";
    gotoxy(68,32);
    cout<<"XX X X XX";
    gotoxy(68,33);
    cout<<"XXXXXXXXX";
    gotoxy(69,34);
    cout<<"XXXXXXX";
    gotoxy(70,35);
    cout<<"XXXXX";
    gotoxy(70,36);
    cout<<"XXXXX";

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),15);
    gotoxy(85,14);
    cout<<"#";

    for(int i=0; i<=2;i++)
    {
        createCannonHorizontal(20+i,60,16+i,'d',30);
    }
    createCannonHorizontal(23,85,17,'a',30);
    createCannonHorizontal(24,85,18,'a',30);

    createCannonHorizontal(25,60,34,'d',30);
    createCannonHorizontal(26,68,34,'a',28);
    createCannonHorizontal(27,76,34,'d',28);
    createCannonHorizontal(28,85,34,'a',30);
    createCannonHorizontal(29,69,35,'a',20);
    createCannonHorizontal(30,75,35,'d',20);
    createCannonHorizontal(31,69,36,'a',20);
    createCannonHorizontal(32,75,36,'d',20);

    createRunnerHorizontal(43,60,15,'d',2);

    createRunnerHorizontal(44,60,19,'a',2);
    createRunnerHorizontal(45,60,20,'a',2);
    createRunnerHorizontal(46,85,19,'d',2);
    createRunnerHorizontal(47,85,20,'d',2);
    createRunnerHorizontal(48,60,27,'d',2);

    createRandomer(8,82,24,2,3,2);
    createRandomer(9,63,24,2,2,3);

    createRandomer(10,82,29,2,4,2);
    createRandomer(11,64,31,2,2,3);

    for (int i=0; i<=7;i++)
        {
            createPickUpAmunicja(12+i,60+i,36);
            createPickUpAmunicja(19+i,78+i,36);
        }
}

void createBossDefeated()
{
    for (int j=0;j<=4;j++)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),8);
        gotoxy(69,19);
        cout<<"XXXXXXX";
        gotoxy(68,20);
        cout<<"XXXXXXXXX";
        gotoxy(67,21);
        cout<<"XXXXXXXXXXX";
        gotoxy(66,22);
        cout<<"XXXXXXXXXXXXX";
        gotoxy(66,23);
        cout<<"XXX XXXXX XXX";
        gotoxy(66,24);
        cout<<"XX   XXX   XX";
        gotoxy(66,25);
        cout<<"XXX XX XX XXX";
        gotoxy(66,26);
        cout<<"XXXXXX XXXXXX";
        gotoxy(67,27);
        cout<<"XXXXXXXXXXX";
        gotoxy(67,28);
        cout<<"X  XXXXX  X";
        gotoxy(67,29);
        cout<<"XX X X X XX";
        gotoxy(68,30);
        cout<<"X       X";
        gotoxy(68,31);
        cout<<"X       X";
        gotoxy(68,32);
        cout<<"XX X X XX";
        gotoxy(68,33);
        cout<<"XXXXXXXXX";
        gotoxy(69,34);
        cout<<"XXXXXXX";
        gotoxy(70,35);
        cout<<"XXXXX";
        gotoxy(70,36);
        cout<<"XXXXX";
        Sleep(200);
        for(int i=0;i<=18;i++)
        {
            gotoxy(65,18+i);
            cout<<"               ";
        }
        Sleep(200);
    }
}

void createBossFirstTime()
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),8);
    gotoxy(69,19);
    cout<<"XXXXXXX";
    Sleep(50);
    gotoxy(68,20);
    cout<<"XXXXXXXXX";
    Sleep(50);
    gotoxy(67,21);
    cout<<"XXXXXXXXXXX";
    Sleep(50);
    gotoxy(66,22);
    cout<<"XXXXXXXXXXXXX";
    Sleep(50);
    gotoxy(66,23);
    cout<<"XXX XXXXX XXX";
    Sleep(50);
    gotoxy(66,24);
    cout<<"XX   XXX   XX";
    Sleep(50);
    gotoxy(66,25);
    cout<<"XXX XX XX XXX";
    Sleep(50);
    gotoxy(66,26);
    cout<<"XXXXXX XXXXXX";
    Sleep(50);
    gotoxy(67,27);
    cout<<"XXXXXXXXXXX";
    Sleep(50);
    gotoxy(67,28);
    cout<<"X  XXXXX  X";
    Sleep(50);
    gotoxy(67,29);
    cout<<"XX X X X XX";
    Sleep(50);
    gotoxy(68,30);
    cout<<"X       X";
    Sleep(50);
    gotoxy(68,31);
    cout<<"X       X";
    Sleep(50);
    gotoxy(68,32);
    cout<<"XX X X XX";
    Sleep(50);
    gotoxy(68,33);
    cout<<"XXXXXXXXX";
    Sleep(50);
    gotoxy(69,34);
    cout<<"XXXXXXX";
    Sleep(50);
    gotoxy(70,35);
    cout<<"XXXXX";
    Sleep(50);
    gotoxy(70,36);
    cout<<"XXXXX";
    Sleep(50);
    createBoss();
}

void loadLevelObjects(int levelNumber)    /// Should have read all the information from external file
{
    if (levelNumber==1)
    {
        createMovableBox(53,31);
        createMovableBox(56,33);
        createMovableBox(57,34);
        for (int i=0; i<=3; i++)
        {
            createMovableBox(52,15+i*2);
        }
        createMovableBox(53,15);
        for (int i=0; i<=2; i++)
        {
            createMovableBox(53,16+i*2);
        }
        for (int i=0; i<=3; i++)
        {
            createMovableBox(54,15+i*2);
        }
        for (int i=0; i<=1; i++)
        {
            createMovableBox(55,16+i*2);
        }
        for (int i=0; i<=2; i++)
        {
            createMovableBox(56,15+i*2);
        }
        createMovableBox(13,25);

        createMovableBox(1,13);
        createMovableBox(2,12);
        createMovableBox(2,11);
        createMovableBox(2,10);
        createMovableBox(3,11);
        createMovableBox(4,12);
        createMovableBox(4,11);
        createMovableBox(5,10);
        createMovableBox(1,9);
        createMovableBox(3,8);

        createMovableBox(4,9);
        createMovableBox(4,13);
        createMovableBox(6,12);
        createMovableBox(7,11);
        createMovableBox(8,10);
        createMovableBox(9,9);
        createMovableBox(10,8);
        createMovableBox(11,9);
        createMovableBox(11,12);
        createMovableBox(6,13);

        createMovableBox(35,12);
        createMovableBox(35,13);
        createMovableBox(36,12);
        createMovableBox(34,13);

        createMovableBox(34,7);
        createMovableBox(34,8);
        createMovableBox(34,6);

        createMovableBox(35,7);
        createMovableBox(35,8);
        createMovableBox(36,8);

        createMovableBox(35,10);

        for (int i=0; i<=4; i++)
        {
            createMovableBox(36+i,9);
            createMovableBox(36+i,11);
        }
        createMovableBox(52,3);
        createMovableBox(42,13);


//---------------------------------------------------------------
        createCannonHorizontal(1,11,28,'d',35);

        createCannonHorizontal(2,54,29,'a',20);
        createCannonHorizontal(3,54,33,'a',15);

        createCannonHorizontal(4,58,23,'a',3);
        createCannonHorizontal(5,58,24,'a',3);
        createCannonHorizontal(6,58,25,'a',3);

        for (int i=0; i<=5; i++)
        {
            createCannonHorizontal(7+i,17,1+i,'d',40-i*3);
        }
        createCannonHorizontal(13,33,6,'a',30);

        for (int i=0; i<=4; i++)
        {
            createCannonHorizontal(14+i,85,2+i,'a',50-i*5);
        }
//---------------------------------------------------------------
        createCannonVertical(1,5,25,'s',40);

        createCannonVertical(2,26,29,'s',30);
        createCannonVertical(3,27,29,'s',40);
        createCannonVertical(4,28,29,'s',35);
        createCannonVertical(5,29,29,'s',50);

        createCannonVertical(6,31,29,'s',25);
        createCannonVertical(7,32,29,'s',30);
        createCannonVertical(8,33,29,'s',35);
        createCannonVertical(9,34,29,'s',40);
        createCannonVertical(10,35,29,'s',45);
        createCannonVertical(11,36,29,'s',50);

        createCannonVertical(12,41,36,'w',20);
        createCannonVertical(13,43,36,'w',20);
        createCannonVertical(14,45,36,'w',20);
        createCannonVertical(15,47,36,'w',20);

        createCannonVertical(16,47,15,'s',1);

        createCannonVertical(17,13,26,'w',2);

        createCannonVertical(18,5,15,'s',40);

        for (int i=0; i<=14; i++)
        {
            createCannonVertical(19+i,18+i,8,'s',2);
        }

        for (int i=0; i<=5; i++)
        {
            createCannonVertical(34+i,78+i,1,'s',25+i*3);
        }

//---------------------------------------------------------------

        createRunnerHorizontal(1,7,33,'d',2);
        createRunnerHorizontal(2,13,31,'a',2);

        createRunnerHorizontal(3,38,33,'d',2);
        createRunnerHorizontal(4,40,32,'d',2);
        createRunnerHorizontal(5,42,31,'a',2);
        createRunnerHorizontal(6,44,30,'d',2);
        createRunnerHorizontal(7,46,29,'d',2);

        createRunnerHorizontal(8,48,28,'d',2);
        createRunnerHorizontal(9,54,30,'a',2);
        createRunnerHorizontal(10,54,34,'a',2);

        createRunnerHorizontal(11,7,17,'d',2);
        createRunnerHorizontal(12,22,18,'a',2);
        createRunnerHorizontal(13,7,19,'d',2);
        createRunnerHorizontal(14,22,20,'a',2);
        createRunnerHorizontal(15,7,21,'d',2);
        createRunnerHorizontal(16,22,22,'a',2);
        createRunnerHorizontal(17,7,23,'d',2);
        createRunnerHorizontal(18,22,24,'a',2);

        createRunnerHorizontal(19,1,2,'a',2);
        createRunnerHorizontal(20,2,3,'d',2);
        createRunnerHorizontal(23,8,2,'a',2);
        createRunnerHorizontal(24,7,3,'a',2);
        createRunnerHorizontal(27,9,4,'a',2);
        createRunnerHorizontal(28,9,3,'a',2);
        createRunnerHorizontal(29,15,4,'a',2);
        createRunnerHorizontal(30,15,5,'a',2);

        createRunnerHorizontal(31,36,10,'d',2);

        createRunnerHorizontal(32,37,7,'d',2);
        createRunnerHorizontal(33,37,8,'d',2);
        createRunnerHorizontal(34,37,12,'d',2);
        createRunnerHorizontal(35,37,13,'d',2);
        createRunnerHorizontal(36,36,6,'d',2);

        createRunnerHorizontal(37,59,11,'d',2);
        createRunnerHorizontal(38,64,12,'d',2);
        createRunnerHorizontal(39,62,13,'d',2);
        createRunnerHorizontal(40,75,11,'d',2);
        createRunnerHorizontal(42,77,13,'d',2);
//---------------------------------------------------------------
        if (player.checkPoint>=4)
        {
            createChaser(1,42,9);
            createChaser(2,45,13);
            createChaser(3,42,4);
            createChaser(4,51,7);
            createChaser(5,55,7);
            createChaser(6,59,7);
            createChaser(7,61,7);
        }
        if (player.checkPoint>=5)
        {
            createBoss();
        }
//---------------------------------------------------------------
        createRandomer(1,18,33,2,3,3);
        createRandomer(2,30,21,2,6,3);

        createRandomer(3,41,18,2,3,2);
        createRandomer(4,30,21,2,6,2);
        createRandomer(5,30,21,2,6,3);
        createRandomer(6,30,21,2,6,3);
//---------------------------------------------------------------
        if (pickUp[1].exists==true)
        {
            createPickUpAmunicja(1,15,36);
        }
        if (pickUp[2].exists==true)
        {
            createPickUpAmunicja(2,8,1);
        }
        if (pickUp[3].exists==true)
        {
            createPickUpLife(3,40,35);
        }
        if (pickUp[4].exists==true)
        {
            createPickUpLife(4,25,15);
        }
        if (pickUp[5].exists==true)
        {
            createPickUpLife(5,22,6);
        }
        if (pickUp[6].exists==true)
        {
            createPickUpCheckPoint(6,56,22);
        }
        if (pickUp[7].exists==true)
        {
            createPickUpCheckPoint(7,4,23);
        }
        if (pickUp[8].exists==true)
        {
            createPickUpCheckPoint(8,33,11);
        }
        if (pickUp[9].exists==true)
        {
            createPickUpCheckPoint(9,42,12);
        }
        if (pickUp[10].exists==true)
        {
            createPickUpLife(10,58,12);
        }
        if (pickUp[11].exists==true)
        {
            createPickUpCheckPoint(11,85,15);
        }
    }
}

int collisionRandomer(int id,char movingDirection)
{
    if(movingDirection=='w')
    {
        if ((readChar(randomer[id].x,randomer[id].y-1)==' ')||(readChar(randomer[id].x,randomer[id].y-1)=='%'))
        {
            return 0;
        }else if ((readChar(randomer[id].x,randomer[id].y-1)=='#') || (readChar(randomer[id].x,randomer[id].y-1)=='@')|| (readChar(randomer[id].x,randomer[id].y-1)=='-')|| (readChar(randomer[id].x,randomer[id].y-1)=='|')|| (readChar(randomer[id].x,randomer[id].y-1)=='A')|| (readChar(randomer[id].x,randomer[id].y-1)=='+')|| (readChar(randomer[id].x,randomer[id].y-1)=='S')|| (readChar(randomer[id].x,randomer[id].y-1)=='%')|| (readChar(randomer[id].x,randomer[id].y-1)=='X'))
        {
            return 2;
        }else return 1;
    }
    if(movingDirection=='s')
    {
        if ((readChar(randomer[id].x,randomer[id].y+1)==' ')||(readChar(randomer[id].x,randomer[id].y+1)=='%'))
        {
            return 0;
        }else if ((readChar(randomer[id].x,randomer[id].y+1)=='#') || (readChar(randomer[id].x,randomer[id].y+1)=='@')|| (readChar(randomer[id].x,randomer[id].y+1)=='-')|| (readChar(randomer[id].x,randomer[id].y+1)=='|')|| (readChar(randomer[id].x,randomer[id].y+1)=='A')|| (readChar(randomer[id].x,randomer[id].y+1)=='+')|| (readChar(randomer[id].x,randomer[id].y+1)=='S')|| (readChar(randomer[id].x,randomer[id].y+1)=='%')|| (readChar(randomer[id].x,randomer[id].y+1)=='X'))
        {
            return 2;
        }else return 1;
    }
    if(movingDirection=='a')
    {
        if ((readChar(randomer[id].x-1,randomer[id].y)==' ')||(readChar(randomer[id].x-1,randomer[id].y)=='%'))
        {
            return 0;
        }else if ((readChar(randomer[id].x-1,randomer[id].y)=='#') || (readChar(randomer[id].x-1,randomer[id].y)=='@')|| (readChar(randomer[id].x-1,randomer[id].y)=='-')|| (readChar(randomer[id].x-1,randomer[id].y)=='|')|| (readChar(randomer[id].x-1,randomer[id].y)=='A')|| (readChar(randomer[id].x-1,randomer[id].y)=='+')|| (readChar(randomer[id].x-1,randomer[id].y)=='S')|| (readChar(randomer[id].x-1,randomer[id].y)=='%')|| (readChar(randomer[id].x-1,randomer[id].y)=='X'))
        {
            return 2;
        }else return 1;
    }
    if (movingDirection=='d')
    {
        if ((readChar(randomer[id].x+1,randomer[id].y)==' ')||(readChar(randomer[id].x+1,randomer[id].y)=='%'))
        {
            return 0;
        }else if ((readChar(randomer[id].x+1,randomer[id].y)=='#') || (readChar(randomer[id].x+1,randomer[id].y)=='@')|| (readChar(randomer[id].x+1,randomer[id].y)=='-')|| (readChar(randomer[id].x+1,randomer[id].y)=='|')|| (readChar(randomer[id].x+1,randomer[id].y)=='A')|| (readChar(randomer[id].x+1,randomer[id].y)=='+')|| (readChar(randomer[id].x+1,randomer[id].y)=='S')|| (readChar(randomer[id].x+1,randomer[id].y)=='%')|| (readChar(randomer[id].x+1,randomer[id].y)=='X'))
        {
            return 2;
        }else return 1;
    }
}
void update()
{
    if ((player.checkPoint==4) && (chasersPlaced==false))
    {
        createChaser(1,42,9);
        createChaser(2,45,13);
        createChaser(3,42,4);
        createChaser(4,51,7);
        createChaser(5,55,7);
        createChaser(6,59,7);
        createChaser(7,61,7);
        chasersPlaced=true;
    }
    if ((player.checkPoint==5)&& (bossPlaced==false))
    {
        createBossFirstTime();
        bossPlaced=true;
    }
    if ((c=='w')|| (c=='s') || (c=='a') || (c=='d'))
    {
        if (c=='w')
        {
            player.movingDirection='w';
            gotoxy(player.x,player.y);
            cout<<" ";
            if ((readChar(player.x,player.y-1)!= '#') && (readChar(player.x,player.y-1)!= '-') && (readChar(player.x,player.y-1)!= '|')&& (readChar(player.x,player.y-1)!= 'X'))
            {
                if (readChar(player.x,player.y-1)=='@')
                {
                    if ((readChar(player.x,player.y-2)==' ') || (readChar(player.x,player.y-2)=='o'))
                    {
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),8);
                        gotoxy(player.x,player.y-2);
                        cout<<"@";
                        player.y-=1;
                    }
                }else
                {
                    player.y-=1;
                }
            }
        }

        if (c=='s')
        {
            player.movingDirection='s';
            gotoxy(player.x,player.y);
            cout<<" ";
            if ((readChar(player.x,player.y+1)!= '#') && (readChar(player.x,player.y+1)!= '-') && (readChar(player.x,player.y+1)!= '|')&& (readChar(player.x,player.y+1)!= 'X'))
            {
                if (readChar(player.x,player.y+1)=='@')
                {
                    if ((readChar(player.x,player.y+2)==' ') || (readChar(player.x,player.y+2)=='o'))
                    {
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),8);
                        gotoxy(player.x,player.y+2);
                        cout<<"@";
                        player.y+=1;
                    }
                }else
                {
                    player.y+=1;
                }
            }
        }

        if (c=='a')
        {
            player.movingDirection='a';
            gotoxy(player.x,player.y);
            cout<<" ";
            if ((readChar(player.x-1,player.y)!= '#') && (readChar(player.x-1,player.y)!= '-') && (readChar(player.x-1,player.y)!= '|')&& (readChar(player.x-1,player.y)!= 'X'))
            {
                if (readChar(player.x-1,player.y)=='@')
                {
                    if ((readChar(player.x-2,player.y)==' ') || (readChar(player.x-2,player.y)=='o'))
                    {
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),8);
                        gotoxy(player.x-2,player.y);
                        cout<<"@";
                        player.x-=1;
                    }
                }else
                {
                    player.x-=1;
                }
            }
        }

        if (c=='d')
        {
            player.movingDirection='d';
            gotoxy(player.x,player.y);
            cout<<" ";
            if ((readChar(player.x+1,player.y)!= '#') && (readChar(player.x+1,player.y)!= '-') && (readChar(player.x+1,player.y)!= '|')&& (readChar(player.x+1,player.y)!= 'X'))
            {
                if (readChar(player.x+1,player.y)=='@')
            {
                    if ((readChar(player.x+2,player.y)==' ') || (readChar(player.x+2,player.y)=='o'))
                    {
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),8);
                        gotoxy(player.x+2,player.y);
                        cout<<"@";
                        player.x+=1;
                    }
                }else
                {
                    player.x+=1;
                }
            }
        }
        if (readChar(player.x,player.y)== 'A')
        {
            player.ammunition+=5;
            for(int i=0; i<=20; i++)
            {
                if ((player.x==pickUp[i].x) && (player.y==pickUp[i].y))
                {
                    pickUp[i].exists=false;
                }
            }
        }
        if (readChar(player.x,player.y)== '+')
        {
            player.lifes+=1;
            for(int i=0; i<=20; i++)
            {
                if ((player.x==pickUp[i].x) && (player.y==pickUp[i].y))
                {
                    pickUp[i].exists=false;
                }
            }
        }
        if (readChar(player.x,player.y)== 'C')
        {
            player.checkPoint+=1;
            for(int i=0; i<=20; i++)
            {
                if ((player.x==pickUp[i].x) && (player.y==pickUp[i].y))
                {
                    pickUp[i].exists=false;
                }
            }
        }
        if (readChar(player.x,player.y)== 'S')
        {
            player.lifes-=1;
            player.lostLife=true;
        }
        if (readChar(player.x,player.y)== '%')
        {
            player.lifes-=1;
            player.lostLife=true;
        }
        if (readChar(player.x,player.y)== 'o')
        {
            player.lifes-=1;
            player.lostLife=true;
        }
        if (readChar(player.x,player.y)== '&')
        {
            player.lifes-=1;
            player.lostLife=true;
        }

        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),11);
        gotoxy(player.x,player.y);
        if (player.movingDirection=='w')
        {
            cout<<char(24);
        }
        if (player.movingDirection=='s')
        {
            cout<<char(25);
        }
        if (player.movingDirection=='a')
        {
            cout<<char(27);
        }
        if (player.movingDirection=='d')
        {
            cout<<char(26);
        }
    }
    int r;
    int emergencyLoopBreak;     /// In case Randomer gets stuck || Shitty solution, should just see surroundings first
    for (int i=0; i<=50; i++)
    {
        if (randomer[i].lifes>0)
        {
            if ((frameCount % randomer[i].speed)==0)
            {
                gotoxy(randomer[i].x,randomer[i].y);
                cout<<" ";
                randomer[i].successfulMove=false;
                while (randomer[i].successfulMove==false)
                {
                    emergencyLoopBreak+=1;
                    r=rand()%4+1;
                    if ((r==1) && (randomer[i].y>randomer[i].yMin) && (collisionRandomer(i,'w')==0))
                    {
                        randomer[i].y-=1;
                        randomer[i].successfulMove=true;
                    } else if ((r==1) && (randomer[i].y>randomer[i].yMin) && (collisionRandomer(i,'w')==1))
                    {
                        player.lifes-=1;
                        player.lostLife=true;
                    }
                    if ((r==2) && (randomer[i].y<randomer[i].yMax) && (collisionRandomer(i,'s')==0))
                    {
                        randomer[i].y+=1;
                        randomer[i].successfulMove=true;
                    } else if ((r==2) && (randomer[i].y<randomer[i].yMax) && (collisionRandomer(i,'s')==1))
                    {
                        player.lifes-=1;
                        player.lostLife=true;
                    }
                    if ((r==3) && (randomer[i].x>randomer[i].xMin) && (collisionRandomer(i,'a')==0))
                    {
                        randomer[i].x-=1;
                        randomer[i].successfulMove=true;
                    } else if ((r==3) && (randomer[i].x>randomer[i].xMin) && (collisionRandomer(i,'a')==1))
                    {
                        player.lifes-=1;
                        player.lostLife=true;
                    }
                    if ((r==4) && (randomer[i].x<randomer[i].xMax) && (collisionRandomer(i,'d')==0))
                    {
                        randomer[i].x+=1;
                        randomer[i].successfulMove=true;
                    } else if ((r==4) && (randomer[i].x<randomer[i].xMax) && (collisionRandomer(i,'d')==1))
                    {
                        player.lifes-=1;
                        player.lostLife=true;
                    }
                    if (emergencyLoopBreak>=50)
                    {
                        randomer[i].successfulMove=true;
                    }
                }
                gotoxy(randomer[i].x,randomer[i].y);
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),13);
                if (randomer[i].lifes<=1)
                {
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),5);
                }
                cout<<"%";
            }
        }
    }

    for (int i=0; i<=30; i++)
    {
        if (chaser[i].exists==true)
        {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),9);
            if (abs(chaser[i].x-player.x)>=(abs(chaser[i].y-player.y)))
            {
                if(chaser[i].x>=player.x)
                {
                    if(readChar(chaser[i].x-1,chaser[i].y)==' ')
                    {
                        gotoxy(chaser[i].x,chaser[i].y);
                        cout<<" ";
                        chaser[i].x-=1;
                        gotoxy(chaser[i].x,chaser[i].y);
                        cout<<"&";
                    }else if ((readChar(chaser[i].x-1,chaser[i].y)!='#') && (readChar(chaser[i].x-1,chaser[i].y)!='@') &&(readChar(chaser[i].x-1,chaser[i].y)!='A') && (readChar(chaser[i].x-1,chaser[i].y)!='+') && (readChar(chaser[i].x-1,chaser[i].y)!='&') && (readChar(chaser[i].x-1,chaser[i].y)!='S')&&(readChar(chaser[i].x-1,chaser[i].y)!='o') && (readChar(chaser[i].x-1,chaser[i].y)!='%'))
                    {
                        player.lostLife=true;
                        player.lifes-=1;
                    }
                }else if(chaser[i].x<player.x)
                {
                    if(readChar(chaser[i].x+1,chaser[i].y)==' ')
                    {
                        gotoxy(chaser[i].x,chaser[i].y);
                        cout<<" ";
                        chaser[i].x+=1;
                        gotoxy(chaser[i].x,chaser[i].y);
                        cout<<"&";
                    }else if ((readChar(chaser[i].x+1,chaser[i].y)!='#') && (readChar(chaser[i].x+1,chaser[i].y)!='@') &&(readChar(chaser[i].x+1,chaser[i].y)!='A') && (readChar(chaser[i].x+1,chaser[i].y)!='+') && (readChar(chaser[i].x+1,chaser[i].y)!='&') && (readChar(chaser[i].x+1,chaser[i].y)!='S')&&(readChar(chaser[i].x+1,chaser[i].y)!='o') && (readChar(chaser[i].x+1,chaser[i].y)!='%'))
                    {
                        player.lostLife=true;
                        player.lifes-=1;
                    }
                }
            }else if (abs(chaser[i].x-player.x)<(abs(chaser[i].y-player.y)))
            {
                if(chaser[i].y>=player.y)
                {
                    if(readChar(chaser[i].x,chaser[i].y-1)==' ')
                    {
                        gotoxy(chaser[i].x,chaser[i].y);
                        cout<<" ";
                        chaser[i].y-=1;
                        gotoxy(chaser[i].x,chaser[i].y);
                        cout<<"&";
                    }else if ((readChar(chaser[i].x,chaser[i].y-1)!='#') && (readChar(chaser[i].x,chaser[i].y-1)!='@') &&(readChar(chaser[i].x,chaser[i].y-1)!='A') && (readChar(chaser[i].x,chaser[i].y-1)!='+') && (readChar(chaser[i].x,chaser[i].y-1)!='&') && (readChar(chaser[i].x,chaser[i].y-1)!='S')&&(readChar(chaser[i].x,chaser[i].y-1)!='o') && (readChar(chaser[i].x,chaser[i].y-1)!='%'))
                    {
                        player.lostLife=true;
                        player.lifes-=1;
                    }
                }else if(chaser[i].y<player.y)
                {
                    if(readChar(chaser[i].x,chaser[i].y+1)==' ')
                    {
                        gotoxy(chaser[i].x,chaser[i].y);
                        cout<<" ";
                        chaser[i].y+=1;
                        gotoxy(chaser[i].x,chaser[i].y);
                        cout<<"&";
                    }else if ((readChar(chaser[i].x,chaser[i].y+1)!='#') && (readChar(chaser[i].x,chaser[i].y+1)!='@') &&(readChar(chaser[i].x,chaser[i].y+1)!='A') && (readChar(chaser[i].x,chaser[i].y+1)!='+') && (readChar(chaser[i].x,chaser[i].y+1)!='&') && (readChar(chaser[i].x,chaser[i].y+1)!='S')&&(readChar(chaser[i].x,chaser[i].y+1)!='o') && (readChar(chaser[i].x,chaser[i].y+1)!='%'))
                    {
                        player.lostLife=true;
                        player.lifes-=1;
                    }
                }
            }
        }
    }

    for (int i=0;i<=100;i++)
    {
        if (runnerHorizontal[i].lifes>0)
        {
            gotoxy(runnerHorizontal[i].x,runnerHorizontal[i].y);
            cout<<" ";
            if (runnerHorizontal[i].movingDirection=='d')
            {
                if (readChar(runnerHorizontal[i].x+1,runnerHorizontal[i].y)==' ')
                {
                    runnerHorizontal[i].x+=1;
                }else if (readChar(runnerHorizontal[i].x+1,runnerHorizontal[i].y)=='#')
                {
                    runnerHorizontal[i].movingDirection='a';
                }else if (readChar(runnerHorizontal[i].x+1,runnerHorizontal[i].y)=='X')
                {
                    runnerHorizontal[i].movingDirection='a';
                }else if (readChar(runnerHorizontal[i].x+1,runnerHorizontal[i].y)=='@')
                {
                    runnerHorizontal[i].movingDirection='a';
                }else if (readChar(runnerHorizontal[i].x+1,runnerHorizontal[i].y)=='+')
                {
                    runnerHorizontal[i].movingDirection='a';
                }else if (readChar(runnerHorizontal[i].x+1,runnerHorizontal[i].y)=='A')
                {
                    runnerHorizontal[i].movingDirection='a';
                }else if (readChar(runnerHorizontal[i].x+1,runnerHorizontal[i].y)=='S')
                {
                    runnerHorizontal[i].movingDirection='a';
                }else if (readChar(runnerHorizontal[i].x+1,runnerHorizontal[i].y)=='o')
                {
                    //NIC
                }else
                {
                    player.lifes-=1;
                    player.lostLife=true;
                }
            }else if (runnerHorizontal[i].movingDirection=='a')
            {
                if (readChar(runnerHorizontal[i].x-1,runnerHorizontal[i].y)==' ')
                {
                    runnerHorizontal[i].x-=1;
                }else if (readChar(runnerHorizontal[i].x-1,runnerHorizontal[i].y)=='#')
                {
                    runnerHorizontal[i].movingDirection='d';
                }else if (readChar(runnerHorizontal[i].x-1,runnerHorizontal[i].y)=='X')
                {
                    runnerHorizontal[i].movingDirection='d';
                }else if (readChar(runnerHorizontal[i].x-1,runnerHorizontal[i].y)=='@')
                {
                    runnerHorizontal[i].movingDirection='d';
                }else if (readChar(runnerHorizontal[i].x-1,runnerHorizontal[i].y)=='+')
                {
                    runnerHorizontal[i].movingDirection='d';
                }else if (readChar(runnerHorizontal[i].x-1,runnerHorizontal[i].y)=='A')
                {
                    runnerHorizontal[i].movingDirection='d';
                }else if (readChar(runnerHorizontal[i].x-1,runnerHorizontal[i].y)=='S')
                {
                    runnerHorizontal[i].movingDirection='d';
                }else if (readChar(runnerHorizontal[i].x-1,runnerHorizontal[i].y)=='o')
                {
                    //NIC
                }else
                {
                    player.lifes-=1;
                    player.lostLife=true;
                }
            }
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),10);
            if (runnerHorizontal[i].lifes==1)
            {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),2);
            }
            gotoxy(runnerHorizontal[i].x,runnerHorizontal[i].y);
            cout<<"S";
        }
    }

    for (int i=0; i<=50; i++)
    {
        if (cannonHorizontal[i].exists==true)
        {
            if ((frameCount % cannonHorizontal[i].speed)==0)
            {
                if (cannonHorizontal[i].shootingDirection=='a')
                {
                    nrBullet+=1;
                    bullet[nrBullet].exists=true;
                    bullet[nrBullet].x=cannonHorizontal[i].x;
                    bullet[nrBullet].y=cannonHorizontal[i].y;
                    bullet[nrBullet].movingDirection=cannonHorizontal[i].shootingDirection;
                    bullet[nrBullet].firstOccurrence=true;
                }
                if (cannonHorizontal[i].shootingDirection=='d')
                {
                    nrBullet+=1;
                    bullet[nrBullet].exists=true;
                    bullet[nrBullet].x=cannonHorizontal[i].x;
                    bullet[nrBullet].y=cannonHorizontal[i].y;
                    bullet[nrBullet].movingDirection=cannonHorizontal[i].shootingDirection;
                    bullet[nrBullet].firstOccurrence=true;
                }
            }
        }
    }
    for (int i=0; i<=50; i++)
    {
        if (cannonVertical[i].exists==true)
        {
            if ((frameCount % cannonVertical[i].speed)==0)
            {
                if (cannonVertical[i].shootingDirection=='w')
                {
                    nrBullet+=1;
                    bullet[nrBullet].exists=true;
                    bullet[nrBullet].x=cannonVertical[i].x;
                    bullet[nrBullet].y=cannonVertical[i].y;
                    bullet[nrBullet].movingDirection=cannonVertical[i].shootingDirection;
                    bullet[nrBullet].firstOccurrence=true;
                }
                if (cannonVertical[i].shootingDirection=='s')
                {
                    nrBullet+=1;
                    bullet[nrBullet].exists=true;
                    bullet[nrBullet].x=cannonVertical[i].x;
                    bullet[nrBullet].y=cannonVertical[i].y;
                    bullet[nrBullet].movingDirection=cannonVertical[i].shootingDirection;
                    bullet[nrBullet].firstOccurrence=true;
                }
            }
        }
    }

    if (c=='m')
    {
        if (player.ammunition!=0)
        {
            nrBullet+=1;
            bullet[nrBullet].exists=false;
            bullet[nrBullet].firstOccurrence=true;
            player.ammunition-=1;
            if (player.movingDirection=='w')
            {
                bullet[nrBullet].x=player.x;
                bullet[nrBullet].y=player.y;
                bullet[nrBullet].movingDirection='w';
                if ((readChar(player.x,player.y-1)==' ') || (readChar(player.x,player.y-1)=='S') || (readChar(player.x,player.y-1)=='%')|| (readChar(player.x,player.y-1)=='X'))
                {
                    bullet[nrBullet].exists=true;
                }
            }
            if (player.movingDirection=='s')
            {
                bullet[nrBullet].x=player.x;
                bullet[nrBullet].y=player.y;
                bullet[nrBullet].movingDirection='s';
                if ((readChar(player.x,player.y+1)==' ') || (readChar(player.x,player.y+1)=='S') || (readChar(player.x,player.y+1)=='%')|| (readChar(player.x,player.y+1)=='X'))
                {
                    bullet[nrBullet].exists=true;
                }
            }
            if (player.movingDirection=='a')
            {
                bullet[nrBullet].x=player.x;
                bullet[nrBullet].y=player.y;
                bullet[nrBullet].movingDirection='a';
                if ((readChar(player.x-1,player.y)==' ') || (readChar(player.x-1,player.y)=='S') || (readChar(player.x-1,player.y)=='%')|| (readChar(player.x-1,player.y)=='X'))
                {
                    bullet[nrBullet].exists=true;
                }
            }
            if (player.movingDirection=='d')
            {
                bullet[nrBullet].x=player.x;
                bullet[nrBullet].y=player.y;
                bullet[nrBullet].movingDirection='d';
                if ((readChar(player.x+1,player.y)==' ') || (readChar(player.x+1,player.y)=='S') || (readChar(player.x+1,player.y)=='%')|| (readChar(player.x+1,player.y)=='X'))
                {
                    bullet[nrBullet].exists=true;
                }
            }
        }

    }

    for (int i=0; i<=400; i++)
    {
        if (bullet[i].exists==true)
        {
            bullet[i].exists=false;
            if (readChar(bullet[i].x,bullet[i].y)!='@')
            {
                if ((bullet[i].firstOccurrence==false) && (readChar(bullet[i].x,bullet[i].y)!='@'))
                {
                    cout<<" ";
                }
                if (bullet[i].movingDirection=='w')
                {
                    bullet[i].y-=1;
                    if ((readChar(bullet[i].x,bullet[i].y)==' ') || (readChar(bullet[i].x,bullet[i].y)=='o'))
                    {
                        bullet[i].exists=true;
                    }
                }
                if (bullet[i].movingDirection=='s')
                {
                    bullet[i].y+=1;
                    if ((readChar(bullet[i].x,bullet[i].y)==' ') || (readChar(bullet[i].x,bullet[i].y)=='o'))
                    {
                        bullet[i].exists=true;
                    }
                }
                if (bullet[i].movingDirection=='a')
                {
                    bullet[i].x-=1;
                    if ((readChar(bullet[i].x,bullet[i].y)==' ') || (readChar(bullet[i].x,bullet[i].y)=='o'))
                    {
                        bullet[i].exists=true;
                    }
                }
                if (bullet[i].movingDirection=='d')
                {
                    bullet[i].x+=1;
                    if ((readChar(bullet[i].x,bullet[i].y)==' ') || (readChar(bullet[i].x,bullet[i].y)=='o'))
                    {
                        bullet[i].exists=true;
                    }
                }
                if (bullet[i].exists==false)
                {
                    if (readChar(bullet[i].x,bullet[i].y)=='S')
                    {
                        for (int j=0; j<=100; j++)
                        {
                            if((bullet[i].x==runnerHorizontal[j].x) && (bullet[i].y==runnerHorizontal[j].y))
                            {
                                runnerHorizontal[j].lifes-=1;
                                gotoxy(runnerHorizontal[j].x,runnerHorizontal[j].y);
                                cout<<" ";
                            }
                        }
                    }else if (readChar(bullet[i].x,bullet[i].y)=='%')
                    {
                        for (int j=0; j<=50; j++)
                        {
                            if((bullet[i].x==randomer[j].x) && (bullet[i].y==randomer[j].y))
                            {
                                randomer[j].lifes-=1;
                                gotoxy(randomer[j].x,randomer[j].y);
                                cout<<" ";
                            }
                        }
                    }else if (readChar(bullet[i].x,bullet[i].y)=='X')
                    {
                        bossLife-=1;
                    }else if ((readChar(bullet[i].x,bullet[i].y)!='@') && (readChar(bullet[i].x,bullet[i].y)!='#') && (readChar(bullet[i].x,bullet[i].y)!='o') && (readChar(bullet[i].x,bullet[i].y)!='-') && (readChar(bullet[i].x,bullet[i].y)!='|')&& (readChar(bullet[i].x,bullet[i].y)!='A') && (readChar(bullet[i].x,bullet[i].y)!='+')&& (readChar(bullet[i].x,bullet[i].y)!='C'))
                    {
                        player.lifes-=1;
                        player.lostLife=true;
                    }
                }
                bullet[i].firstOccurrence=false;


            }
        }
    }
    for(int i=0; i<=400; i++)
    {
        if (bullet[i].exists==true)
        {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),12);
            gotoxy(bullet[i].x,bullet[i].y);
            cout<<"o";
        }
    }
    gotoxy(90,3);
    cout<<"                ";
    gotoxy(90,3);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),4);
    cout<<"Lives: "<<player.lifes;

    gotoxy(90,5);
    cout<<"                ";
    gotoxy(90,5);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),14);
    cout<<"Ammo: "<<player.ammunition;

    gotoxy(90,7);
    cout<<"                           ";
    if (player.respawnTime>0)
    {
        gotoxy(90,7);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),15);
        cout<<"Respawn time: "<<player.respawnTime;
    }


    gotoxy(11,40);
    cout<<"                ";
    gotoxy(11,40);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),11);
    cout<<"XY: ( "<<player.x<<", "<<player.y<<" )";

    if (bossPlaced==true)
    {
        gotoxy(70,31);
        cout<<"      ";
        gotoxy(70,31);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),11);
        cout<<bossLife<<" HP";
    }


}

void clearAllObjects()
{
    for (int i=0; i<=100; i++)
    {
        runnerHorizontal[i].lifes=0;
    }
    for (int i=0; i<=50; i++)
    {
        randomer[i].lifes=0;
        cannonVertical[i].exists=false;
        cannonHorizontal[i].exists=false;
    }
}

void poruszeniePlayer()
{
    c='0';
    if (_kbhit()==1)
    {
        c=getch();
    }
}

void drawLegend()
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),9);
    gotoxy(87,9);
    cout<<"--------------------------";
    gotoxy(91,10);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),15);
    cout<<"     LEGEND: ";

    gotoxy(89,12);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),11);
    cout<<char(24)<<" -> YOU";

    gotoxy(89,14);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),15);
    cout<<"# -> Unmovable object";

    gotoxy(89,16);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),8);
    cout<<"@ -> Movable object";

    gotoxy(89,18);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),4);
    cout<<"- , | -> Cannons";
    gotoxy(95,19);
    cout<<"(Indestructible)";

    gotoxy(89,21);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),10);
    cout<<"S -> Vertically-moving";
    gotoxy(94,22);
    cout<<"enemy [Have 2HP]";

    gotoxy(89,24);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),13);
    cout<<"% -> Randomly-moving";
    gotoxy(94,25);
    cout<<"enemy [Have 2HP]";
    gotoxy(91,26);
    cout<<"(move in certain area)";

    gotoxy(89,28);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),9);
    cout<<"& -> Follows YOU";
    gotoxy(94,29);
    cout<<"(longer axis dist.)";

    gotoxy(89,31);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),14);
    cout<<"A -> Ammo (+5)";

    gotoxy(89,33);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),11);
    cout<<"+ -> life (+1)";

    gotoxy(89,35);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),240);
    cout<<"C";
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),15);
    cout<<" -> CheckPoint";

    gotoxy(87,37);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),9);
    cout<<"--------------------------";

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),9);
    for (int i=0; i<=5; i++)
    {
        gotoxy(67,38+i);
        cout<<"|";
    }

    gotoxy(69,39);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),15);
    cout<<"ESC -> Quit to menu";

    gotoxy(69,41);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),15);
    cout<<"R -> Restart from CheckPoint (loses life)";

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),9);
    for (int i=0; i<=5; i++)
    {
        gotoxy(38,38+i);
        cout<<"|";
    }

    gotoxy(40,39);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),11);
    cout<<"W/S/A/D -> Move character";

    gotoxy(40,41);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),12);
    cout<<"M -> Shoot";
}

void levelRestart()
{
    c='0';
    frameCount=0;
    system("CLS");
    if(player.checkPoint==0)
    {
        player.x=2;
        player.y=35;
    }
    if (player.checkPoint==1)
    {
        player.x=56;
        player.y=22;
    }
    if (player.checkPoint==2)
    {
        player.x=4;
        player.y=23;
    }
    if (player.checkPoint==3)
    {
        player.x=33;
        player.y=11;
    }
    if (player.checkPoint==4)
    {
        player.x=42;
        player.y=12;
    }
    if (player.checkPoint==5)
    {
        player.x=85;
        player.y=15;
    }
    clearAllObjects();
    loadLevelBase("level_1.txt");
    loadLevelObjects(1);
    drawLegend();
    gotoxy(player.x,player.y);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),11);
    cout<<char(24);
}

void speed()
{
    system("CLS");
    char c=' ';
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),15);
    gotoxy(48,16);
    cout<<"CURRENT SPEED: ";
    gotoxy(41,21);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),12);
    cout<<"<- A(-)";
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),15);
    cout<<" MOVING SPEED ";
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),10);
    cout<<"(+)D ->";
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),15);
    gotoxy(41,24);
    cout<<"   (Press ENTER to go back)   ";
    while (c!=char(13))
    {
        gotoxy(54,18);
        cout<<"                    ";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),15);
        gotoxy(54,18);
        cout<<framesDelay;
        gotoxy(0,0);
        c=getch();
        if (c=='d')
        {
            framesDelay+=1;
        }
        if (c=='a')
        {
            if (framesDelay>=2)
            {
                framesDelay-=1;
            }
        }
    }
}

void trudnosc()
{

}

void options()
{
    char c='0';
    int chosen=2;
    while (c!=' ')
    {
    system("CLS");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),11);
    gotoxy(55,14);
    cout<<char(24);
    gotoxy(55,15);
    cout<<"W";

    gotoxy(55,28);
    cout<<char(25);
    gotoxy(55,27);
    cout<<"S";

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),15);
    gotoxy(49,19);
    cout<<" DIFFICULTY ";
    gotoxy(49,21);
    cout<<"MOVING SPEED";
    gotoxy(49,23);
    cout<<"    BACK    ";
    while (c!=char(13))
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),10);
        gotoxy(46,19+chosen);
        cout<<"->";
        gotoxy(63,19+chosen);
        cout<<"<-";
        gotoxy(0,0);
        c=getch();
        gotoxy(46,19+chosen);
        cout<<"  ";
        gotoxy(63,19+chosen);
        cout<<"  ";
        if (c=='s')
        {
            if (chosen<4)
            {
                chosen+=2;
            }
        }
        if (c=='w')
        {
            if (chosen>0)
            {
                chosen-=2;
            }
        }
    }
    if (chosen==0)
    {
        trudnosc();
        c='0';
    }
    if (chosen==2)
    {
        speed();
        c='0';
    }
    if (chosen==4)
    {
        c=' ';
    }
    system("CLS");
    chosen=2;
    }

}

void startGame()
{
    srand(time(NULL));
    frameCount=0;
    player.x=2;
    player.y=35;
    player.ammunition=15;
    player.lifes=3;
    player.checkPoint=0;
    bossLife=50;
    bossPlaced=false;
    for (int i=0; i<=20;i++)
    {
        pickUp[i].exists=true;
    }
    clearAllObjects();
    loadLevelBase("level_1.txt");
    loadLevelObjects(1);
    drawLegend();
    gotoxy(player.x,player.y);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),11);
    cout<<char(24);
}

void menu()
{
    char c='0';
    int chosen=2;
    exitConsole=false;
    system("CLS");
    while ((inGame==false) && (exitConsole==false))
    {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),11);
    gotoxy(55,14);
    cout<<char(24);
    gotoxy(55,15);
    cout<<"W";

    gotoxy(55,28);
    cout<<char(25);
    gotoxy(55,27);
    cout<<"S";

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),15);
    gotoxy(52,19);
    cout<<" START ";
    gotoxy(52,21);
    cout<<"OPTIONS";
    gotoxy(52,23);
    cout<<" QUIT  ";
    while (c!=char(13))
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),10);
        gotoxy(49,19+chosen);
        cout<<"->";
        gotoxy(60,19+chosen);
        cout<<"<-";
        gotoxy(0,0);
        c=getch();
        gotoxy(49,19+chosen);
        cout<<"  ";
        gotoxy(60,19+chosen);
        cout<<"  ";
        if (c=='s')
        {
            if (chosen<4)
            {
                chosen+=2;
            }
        }
        if (c=='w')
        {
            if (chosen>0)
            {
                chosen-=2;
            }
        }
    }
    if (chosen==0)
    {
        inGame=true;
    }
    if (chosen==2)
    {
        options();
    }
    if (chosen==4)
    {
        inGame=false;
        exitConsole=true;
    }
    chosen=2;
    c=' ';
    system("CLS");
    }
}


int main()
{
    framesDelay=10;
    menu();
    while (exitConsole==false)
    {
        startGame();
        while(inGame==true)
        {
            player.lostLife=false;
            update();
            poruszeniePlayer();
            if (player.respawnTime>=1)
            {
                c='0';
                player.respawnTime-=1;
            }
            if (c==char(27))
            {
                inGame=false;
            }
            if ((player.lostLife==true) || (c=='r'))
            {
                for (int i=0; i<=4; i++)
                {
                    gotoxy(player.x,player.y);
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),3);
                    cout<<char(24);
                    Sleep(150);
                    gotoxy(player.x,player.y);
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),11);
                    cout<<char(24);
                    Sleep(150);
                }
                if (c=='r')
                    {
                        player.lifes-=1;
                    }
                player.respawnTime=20;
                levelRestart();
            }
            if (player.lifes<=0)
            {
                inGame=false;
            }
            gotoxy(0,0);
            Sleep(framesDelay);
            frameCount+=1;
            if (frameCount>2000000)
            {
                frameCount=0;
            }
            if (nrBullet>350)
            {
                nrBullet=0;
            }
            if(bossLife<=0)
            {
                inGame=false;
            }
        }
        if ((exitConsole==false)&&(bossLife>0))
        {
            system("CLS");
            gotoxy(52,20);
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),6);
            cout<<"GAME OVER";
            gotoxy(44,23);
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),15);
            cout<<"Press ESC to go back to menu";
            while (c!=char(27))
            {
                c=getch();
            }
            menu();
        }
        if (bossLife<=0)
        {
            createBossDefeated();
            system("CLS");
            gotoxy(52,20);
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),6);
            cout<<"YOU WON!";
            gotoxy(43,23);
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),15);
            cout<<"Press ESC to go back to menu";
            while (c!=char(27))
            {
                c=getch();
            }
            menu();
        }
    }
    return 0;
}
