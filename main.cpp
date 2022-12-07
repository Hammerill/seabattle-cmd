#include <iostream>
#include <time.h>
#include <windows.h>
#include <conio.h>
using namespace std;
enum Color {Black = 0, Blue, Green, Cyan, Red, Magenta, Brown, LightGray, DarkGray, LightBlue, LightGreen, LightCyan, LightRed, LightMagenta, Yellow, White};

HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
void setc(Color text = White, Color background = Black)
{
	SetConsoleTextAttribute(hStdOut, (WORD)((background << 4) | text));
}

void setp(int x = 0, int y = 0)
{
	COORD myCoords = {x, y};
	SetConsoleCursorPosition(hStdOut, myCoords);
}

//Выводит на экран меню, даёт пользователю право выбрать определённый пункт и возвращает его номер
int menu(char* choises[], int kol, int x = 5, int y = 5)
{
    int choise = 0, oldchoise, button;
    bool key;

    for (size_t i = 0; i < kol; i++)
    {
        setp(x, y + i);
        if (i == 0)
        {
            setc(White, Red);
        }
        else
        {
            setc();
        }
        cout << choises[i] << '\n';
    }
    setc();
    setp(x, y + kol + 3);
    cout << "Используйте W и S для переключения по пунктам меню.\n";
    setp(x, y + kol + 4);
    cout << "Используйте Enter для выбора пункта.";
    do
    {
        oldchoise = choise;
        button = _getch();
        key = true;

        switch (button)
        {
        case 119:                       //W
            if (choise <= 0)
            {
                choise = kol - 1;
            }
            else
            {
                choise--;
            }
            break;

        case 115:                       //S
            if (choise >= kol - 1)
            {
                choise = 0;
            }
            else
            {
                choise++;
            }
            break;

        default:
            key = false;
            break;
        }

        if (key)
        {
            setc();
            setp(x, y + oldchoise);
            cout << choises[oldchoise];

            setc(White, Red);
            setp(x, y + choise);
            cout << choises[choise];
        }

    } while (button != 13);             //Enter

    return choise;
}

//Для описания одного кораблика необходимо записать:
struct Boat
{
    int x;              //его позицию X...
    int y;              //...и Y;
    int len;            //длину;
    bool isHorizontal;  //и стоит ли он горизонтально.
};

//Разум бота состоит из следующих полей:
struct Bot
{
    bool remember;  //помнит ли он, где в последний раз был замечен кораблик (сначала нет; потом, когда нашёл кораблик, да; затем, уничтожив его, ещё раз нет, и по кругу);
    int x;          //позиция X...
    int y;          //...и Y того блока, где в последний раз был замечен кораблик;
    int dir;        //каково, по его мнению, направление кораблика (0, если не знает; 1, если горизонтальное; 2, если вертикальное).
};

//Очищает структуру разума бота
void clearBot(Bot& bot)
{
    bot.remember = false;
    bot.x = 0;
    bot.y = 0;
    bot.dir = 0;
}

//Выводит на экран один блок поля
void drawTile(int tilenum, int x, int y, bool debug)
{
    setp(x, y);
    if (tilenum == 1 && debug)   //С кораблём (дебаг)
    {
        setc(Red, Red);
    }
    else if (tilenum == 2)       //Мимо
    {
        setc(DarkGray, Black);
    }
    else if (tilenum == 3)       //Повреждённый корабль
    {
        setc(Yellow, Red);
    }
    else if (tilenum == 4)       //Поверженный корабль
    {
        setc(DarkGray, DarkGray);
    }
    else                         //Пустая или есть корабль (без дебага)
    {
        setc(Black, Black);
    }
    cout << '*';
}

//Рисует простой блок
void drawTile(Color color, int x, int y)
{
    setc(color, color);
    setp(x, y);
    cout << '*';
}

//Выводит поле на экран
//Если включен дебаг, показывать нетронутые корабли
void printGround(int ground[10][10], int x = 1, int y = 1, bool debug = true)
{
    for (size_t yi = 0; yi < 10; yi++)
    {
        for (size_t xi = 0; xi < 10; xi++)
        {
            drawTile(ground[xi][yi], x + xi, y + yi, debug);
        }
    }
}

//Рисует границы поля
void printBorder(int x = 0, int y = 0)
{
    setc(White, White);
    setp(x, y);
    cout << '*';

    setc(Black, White);
    for (size_t i = 1; i < 11; i++)
    {
        setp(x, y + i);
        cout << i - 1;
    }

    setc(White, White);
    setp(x, y + 11);
    cout << '*';

    setc(White, White);
    for (size_t i = 0; i < 12; i++)
    {
        setp(x + 11, y + i);
        cout << '*';
    }

    setc(Black, White);
    setp(x + 1, y);
    cout << "АБВГДЕЁЖЗИ";

    setc(White, White);
    setp(x + 1, y + 11);
    cout << "**********";
}

//Очищает массив поля
void clearGround(int ground[10][10])
{
    for (size_t i = 0; i < 10; i++)
    {
        for (size_t j = 0; j < 10; j++)
        {
            ground[i][j] = 0;
        }
    }
}

//Очищает массив структур корабля
void clearBoats(Boat boats[10])
{
    for (size_t i = 0; i < 10; i++)
    {
        boats[i].x = 0;
        boats[i].y = 0;
        boats[i].len = 0;
        boats[i].isHorizontal = 0;
    }
}

//Копирует параметры одного кораблика в другой
void copyBoat(Boat source, Boat& destination)
{
    destination.x = source.x;
    destination.y = source.y;
    destination.len = source.len;
    destination.isHorizontal = source.isHorizontal;
}

//Добавляет один корабль на нетронутое поле
void addBoat(int ground[10][10], Boat boat, Boat boats_massive[10], int cur_id)
{
    for (size_t i = 0; i < boat.len; i++)
    {
        if (boat.isHorizontal && ground[boat.x + i][boat.y] == 0)
        {
            ground[boat.x + i][boat.y] = 1;
        }
        else if (ground[boat.x][boat.y + i] == 0)
        {
            ground[boat.x][boat.y + i] = 1;
        }
    }

    copyBoat(boat, boats_massive[cur_id]);
}

//Показывает один конкретный кораблик (только для конструктора)
void showBoat(Boat boat, Color color = Blue, int x = 1, int y = 1)
{
    for (size_t i = 0; i < boat.len; i++)
    {
        if (boat.isHorizontal)
        {
            drawTile(color, x + boat.x + i, y + boat.y);
        }
        else
        {
            drawTile(color, x + boat.x, y + boat.y + i);
        }
    }
}

//Показывает кораблик в режиме "хамелеон", т.е. исп. цвета поля
void showBoat(Boat boat, int ground[10][10], int x = 1, int y = 1)
{
    for (size_t i = 0; i < boat.len; i++)
    {
        if (boat.isHorizontal)
        {
            drawTile(ground[boat.x + i][boat.y], x + boat.x + i, y + boat.y, true);
        }
        else
        {
            drawTile(ground[boat.x][boat.y + i], x + boat.x, y + boat.y + i, true);
        }
    }
}

//Проверяет, влазит ли кораблик в поле
bool inBorders(Boat boat)
{
    if (boat.x < 0 || boat.y < 0)   //Если выходит своей основой за левый или верхний край
    {
        return false;
    }

    if (boat.isHorizontal)  //Если кораблик стоит горизонтально
    {
        if ((boat.x + boat.len > 10) || (boat.y > 9))   //Если выходит за правый или нижний край
        {
            return false;
        }
    }
    else    //Если стоит вертикально
    {
        if ((boat.y + boat.len > 10) || (boat.x > 9))   //Если выходит за нижний или правый край
        {
            return false;
        }
    }

    return true;
}

//Проверяет, можно ли установить кораблик на этом поле
bool canPlaceHere(int ground[10][10], Boat boat, int scanFor = 1)
{
    if (!inBorders(boat))   //Если выходит за границы поля
    {
        return false;
    }

    if (boat.isHorizontal)  //Если кораблик стоит горизонтально
    {
        for (int i = -1; i < boat.len + 1; i++) //Проверка линии сверху кораблика на предмет свободности
        {
            if ((boat.x + i >= 0) && (boat.y - 1 >= 0) && (boat.x + i <= 9))    //Если не выходит за границы
            {
                if (ground[boat.x + i][boat.y - 1] == scanFor)    //Если здесь занято
                {
                    return false;
                }
            }
        }
        for (int i = -1; i < boat.len + 1; i++) //Проверка линии снизу кораблика на предмет свободности
        {
            if ((boat.x + i >= 0) && (boat.y + 1 <= 9) && (boat.x + i <= 9))    //Если не выходит за границы
            {
                if (ground[boat.x + i][boat.y + 1] == scanFor)    //Если здесь занято
                {
                    return false;
                }
            }
        }

        for (size_t i = 0; i < boat.len; i++)   //Проверка линии самого кораблика на предмет свободности
        {
            if (ground[boat.x + i][boat.y] == scanFor)    //Если здесь занято
            {
                return false;
            }
        }

        if (boat.x >= 1) //Если слева не граница поля
        {
            if (ground[boat.x - 1][boat.y] == scanFor)    //Если здесь занято
            {
                return false;
            }
        }
        if (boat.x + boat.len <= 10) //Если справа не граница поля
        {
            if (ground[boat.x + boat.len][boat.y] == scanFor) //Если здесь занято
            {
                return false;
            }
        }
    }
    else    //Если стоит вертикально
    {
        for (int i = -1; i < boat.len + 1; i++) //Проверка линии слева кораблика на предмет свободности
        {
            if ((boat.y + i >= 0) && (boat.x - 1 >= 0) && (boat.y + i <= 9))    //Если не выходит за границы
            {
                if (ground[boat.x - 1][boat.y + i] == scanFor)    //Если здесь занято
                {
                    return false;
                }
            }
        }
        for (int i = -1; i < boat.len + 1; i++) //Проверка линии справа кораблика на предмет свободности
        {
            if ((boat.y + i >= 0) && (boat.x + 1 <= 9) && (boat.y + i <= 9))    //Если не выходит за границы
            {
                if (ground[boat.x + 1][boat.y + i] == scanFor)    //Если здесь занято
                {
                    return false;
                }
            }
        }

        for (size_t i = 0; i < boat.len; i++)   //Проверка линии самого кораблика на предмет свободности
        {
            if (ground[boat.x][boat.y + i] == scanFor)    //Если здесь занято
            {
                return false;
            }
        }

        if (boat.y >= 1) //Если сверху не граница поля
        {
            if (ground[boat.x][boat.y - 1] == scanFor)    //Если здесь занято
            {
                return false;
            }
        }
        if (boat.y + boat.len <= 10) //Если снизу не граница поля
        {
            if (ground[boat.x][boat.y + boat.len] == scanFor) //Если здесь занято
            {
                return false;
            }
        }
    }

    return true;
}

//Смотрит, где на поле есть свободное место под кораблик (не рандом)
int* detectFreeSpace(int ground[10][10], Boat boat)
{
    int* ret = new int[3];

    boat.isHorizontal = true;
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            boat.x = j;
            boat.y = i;
            if (canPlaceHere(ground, boat))
            {
                ret[0] = boat.x;
                ret[1] = boat.y;
                ret[2] = boat.isHorizontal;
                return ret;
            }
        }
    }

    boat.isHorizontal = false;
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            boat.x = j;
            boat.y = i;
            if (canPlaceHere(ground, boat))
            {
                ret[0] = boat.x;
                ret[1] = boat.y;
                ret[2] = boat.isHorizontal;
                return ret;
            }
        }
    }

    return 0;
}

//Конструктор поля
void groundConstructor(int ground[10][10], Boat boats[10])
{
    Boat cur_boat, old_boat;
    bool key;
    int cur_id = 0, button;
    int* params = new int[3];

    clearGround(ground);
    clearBoats(boats);

    setc();
    system("cls");
    printBorder();

    setc();
    setp(0, 13);
    cout << "Используйте WASD для передвижения кораблика.\nQ для его поворота.\nEnter для его размещения.";

    do
    {
        if (cur_id == 0)
        {
            cur_boat.len = 4;
        }
        else if ((cur_id == 1) || (cur_id == 2))
        {
            cur_boat.len = 3;
        }
        else if ((cur_id >= 3) && (cur_id <= 5))
        {
            cur_boat.len = 2;
        }
        else
        {
            cur_boat.len = 1;
        }

        params = detectFreeSpace(ground, cur_boat);
        cur_boat.x = params[0];
        cur_boat.y = params[1];
        cur_boat.isHorizontal = params[2];

        copyBoat(cur_boat, old_boat);

        printGround(ground);

        do
        {
            key = true;

            showBoat(old_boat, ground);
            showBoat(cur_boat);
            copyBoat(cur_boat, old_boat);

            button = _getch();

            setc(Black);
            setp(0, 17);
            cout << "Вы не можете установить кораблик здесь!";

            switch (button)
            {
            case 119:   //W
                cur_boat.y--;
                if (!inBorders(cur_boat))
                {
                    copyBoat(old_boat, cur_boat);
                }
                break;
            case 97:    //A
                cur_boat.x--;
                if (!inBorders(cur_boat))
                {
                    copyBoat(old_boat, cur_boat);
                }
                break;
            case 115:   //S
                cur_boat.y++;
                if (!inBorders(cur_boat))
                {
                    copyBoat(old_boat, cur_boat);
                }
                break;
            case 100:   //D
                cur_boat.x++;
                if (!inBorders(cur_boat))
                {
                    copyBoat(old_boat, cur_boat);
                }
                break;
            case 113:   //Q
                cur_boat.isHorizontal = !cur_boat.isHorizontal;
                if (!inBorders(cur_boat))
                {
                    copyBoat(old_boat, cur_boat);
                }
                break;
            case 13:    //Enter
                if (canPlaceHere(ground, cur_boat))
                {
                    key = false;
                }
                else
                {
                    setc(White);
                    setp(0, 17);
                    cout << "Вы не можете установить кораблик здесь!";
                }
            }
        } while (key);

        addBoat(ground, cur_boat, boats, cur_id);
        cur_id++;

    } while (cur_id < 10);

    delete[] params;
}

//Автоматическое построение поля ботом
void autoGroundConstructor(int ground[10][10], Boat boats[10])
{
    Boat cur_boat;

    clearGround(ground);
    clearBoats(boats);

    for (size_t i = 0; i < 10; i++)
    {
        if (i == 0)
        {
            cur_boat.len = 4;
        }
        else if ((i == 1) || (i == 2))
        {
            cur_boat.len = 3;
        }
        else if ((i >= 3) && (i <= 5))
        {
            cur_boat.len = 2;
        }
        else
        {
            cur_boat.len = 1;
        }

        do
        {
            cur_boat.x = rand() % 10;
            cur_boat.y = rand() % 10;
            cur_boat.isHorizontal = rand() % 2;
            Sleep(200);
        } while (!canPlaceHere(ground, cur_boat));

        addBoat(ground, cur_boat, boats, i);

        Sleep(200);
        cout << '.';
    }
}

//Даёт пользователю управление и возвращает координаты той точки, куда он нацелился
int* aim(int ground[10][10], int x, int y, int defx = 0, int defy = 0)
{
    int button, xtable, ytable, oldx = 0, oldy = 0;
    bool key;
    int* coords = new int[2];
    coords[0] = defx;
    coords[1] = defy;

    if (y >= 2) { ytable = y - 2; }
    else { ytable = y + 11; }

    if (x >= 1) { xtable = x - 1; }
    else { xtable = x; }

    setc();
    setp(xtable, ytable + 16);
    cout << "Используйте WASD для перемещения курсора.";
    setp(xtable, ytable + 17);
    cout << "Используйте Enter для выстрела.";

    do
    {
        key = true;

        drawTile(ground[oldx][oldy], x + oldx, y + oldy, false);
        drawTile(Blue, x + coords[0], y + coords[1]);

        oldx = coords[0];
        oldy = coords[1];

        button = _getch();

        setc(Black);
        setp(xtable, ytable);
        cout << "Вы уже пытались сюда выстрелить!";

        switch (button)
        {
        case 119:   //W
            if (coords[1] > 0)
            {
                coords[1]--;
            }
            else
            {
                coords[1] = 0;
            }
            break;

        case 97:    //A
            if (coords[0] > 0)
            {
                coords[0]--;
            }
            else
            {
                coords[0] = 0;
            }
            break;

        case 115:   //S
            if (coords[1] < 9)
            {
                coords[1]++;
            }
            else
            {
                coords[1] = 9;
            }
            break;

        case 100:   //D
            if (coords[0] < 9)
            {
                coords[0]++;
            }
            else
            {
                coords[0] = 9;
            }
            break;

        case 13:    //Enter
            if (ground[coords[0]][coords[1]] >= 2)
            {
                setc();
                setp(xtable, ytable);
                cout << "Вы уже пытались сюда выстрелить!";
            }
            else
            {
                key = false;
            }
            break;
        }

    } while(key);

    setc(Black);
    setp(xtable, ytable + 16);
    cout << "Используйте WASD для перемещения курсора.";
    setp(xtable, ytable + 17);
    cout << "Используйте Enter для выстрела.";

    return coords;
}

//Удаляет один корабль из массива кораблей, помечает его визуально поверженным на поле
void fragged(int ground[10][10], Boat boats[10], int element, int& boat_count)
{
    for (size_t i = 0; i < boats[element].len; i++)
    {
        if (boats[element].isHorizontal)
        {
            ground[boats[element].x + i][boats[element].y] = 4;
        }
        else
        {
            ground[boats[element].x][boats[element].y + i] = 4;
        }
    }
    boat_count--;
}

//Проверяет поле на наличие поверженных кораблей. Если находит поверженный кораблик, вызывает для него функцию fragged
void checkFrags(int ground[10][10], Boat boats[10], int& boat_count)
{
    bool key;

    for (size_t i = 0; i < 10; i++)
    {
        key = true;

        for (size_t j = 0; j < boats[i].len; j++)
        {
            if (boats[i].isHorizontal)
            {
                if (ground[boats[i].x + j][boats[i].y] != 3)
                {
                    key = false;
                }
            }
            else
            {
                if (ground[boats[i].x][boats[i].y + j] != 3)
                {
                    key = false;
                }
            }
        }

        if (key)
        {
            fragged(ground, boats, i, boat_count);
        }
    }
}

//Если в параметры функции внесли разум бота, то при нахождении поверженного корабля очистить его
void checkFrags(int ground[10][10], Boat boats[10], int& boat_count, Bot& bot)
{
    bool key;

    for (size_t i = 0; i < 10; i++)
    {
        key = true;

        for (size_t j = 0; j < boats[i].len; j++)
        {
            if (boats[i].isHorizontal)
            {
                if (ground[boats[i].x + j][boats[i].y] != 3)
                {
                    key = false;
                }
            }
            else
            {
                if (ground[boats[i].x][boats[i].y + j] != 3)
                {
                    key = false;
                }
            }
        }

        if (key)
        {
            fragged(ground, boats, i, boat_count);
            clearBot(bot);  //Очистка разума бота
        }
    }
}

//Просто функция для принуждённого сжатия кода (исп. в botAim)
void forwardingx(int ground[10][10], int& x, int y, int updx, int& stats)
{
    Boat check;
    check.isHorizontal = true;
    check.len = 1;
    do
    {
        x += updx;
        if ((x >= 0) && (x <= 9))
        {
            check.x = x;
            check.y = y;
            if ((ground[x][y] < 2) && (canPlaceHere(ground, check, 4)))
            {
                stats = 2;
            }
            else if ((ground[x][y] != 3) || (!canPlaceHere(ground, check, 4)))
            {
                stats = 1;
            }
        }
        else
        {
            stats = 1;
        }
    } while (stats == 0);
}

void forwardingy(int ground[10][10], int x, int& y, int updy, int& stats)
{
    Boat check;
    check.isHorizontal = true;
    check.len = 1;
    do
    {
        y += updy;
        if ((y >= 0) && (y <= 9))
        {
            check.x = x;
            check.y = y;
            if ((ground[x][y] < 2) && (canPlaceHere(ground, check, 4)))
            {
                stats = 2;
            }
            else if ((ground[x][y] != 3) || (!canPlaceHere(ground, check, 4)))
            {
                stats = 1;
            }
        }
        else
        {
            stats = 1;
        }
    } while (stats == 0);
}

//Бот на основе своей памяти (см. выше) делает вывод, куда надо стрелять, а функция возвращает эти координаты
int* botAim(int ground[10][10], Bot& bot)
{
    int* coords = new int[2];
    int temp_dir, stats;    //Для stats: 0 - надо продолжать, 1 - нет смысла продолжать, на другую сторону, 2 - надо стрелять
    bool key;
    Boat check; //Для проверки ореола (функция canPlaceHere)

    Sleep(500);

    if (bot.remember)
    {
        coords[0] = bot.x;
        coords[1] = bot.y;

        if (bot.dir == 0)
        {
            key = true;
            do
            {
                temp_dir = rand() % 4;
                switch (temp_dir)
                {
                case 0:     //UP = Y-
                    if (bot.y > 0)  //Если сверху не граница поля
                    {
                        if (ground[bot.x][bot.y - 1] < 2)   //Если здесь неизвестность
                        {
                            key = false;
                        }
                    }
                    break;

                case 1:     //RIGHT = X+
                    if (bot.x < 9)  //Если справа не граница поля
                    {
                        if (ground[bot.x + 1][bot.y] < 2)   //Если здесь неизвестность
                        {
                            key = false;
                        }
                    }
                    break;

                case 2:     //DOWN = Y+
                    if (bot.y < 9)  //Если снизу не граница поля
                    {
                        if (ground[bot.x][bot.y + 1] < 2)   //Если здесь неизвестность
                        {
                            key = false;
                        }
                    }
                    break;

                default:    //LEFT = X-
                    if (bot.x > 0)  //Если слева не граница поля
                    {
                        if (ground[bot.x - 1][bot.y] < 2)   //Если здесь неизвестность
                        {
                            key = false;
                        }
                    }
                    break;
                }
            } while (key);

            switch (temp_dir)
            {
            case 0:
                if (ground[coords[0]][--coords[1]] == 1)
                {
                    bot.dir = 2;
                }
                break;

            case 1:
                if (ground[++coords[0]][coords[1]] == 1)
                {
                    bot.dir = 1;
                }
                break;

            case 2:
                if (ground[coords[0]][++coords[1]] == 1)
                {
                    bot.dir = 2;
                }
                break;

            default:
                if (ground[--coords[0]][coords[1]] == 1)
                {
                    bot.dir = 1;
                }
                break;
            }
            return coords;
        }

        else if (bot.dir == 1)
        {
            temp_dir = rand() % 2; //Бот выбирает направление, которое он будет сначала добивать (для разнообразия)
            stats = 0;
            if (temp_dir) //Начинает налево
            {
                forwardingx(ground, coords[0], coords[1], -1, stats);

                if (stats == 2)
                {
                    return coords;
                }
                else
                {
                    stats = 0;
                    forwardingx(ground, coords[0], coords[1], 1, stats);

                    if (stats == 2)
                    {
                        return coords;
                    }
                }
            }
            else    //Направо
            {
                forwardingx(ground, coords[0], coords[1], 1, stats);

                if (stats == 2)
                {
                    return coords;
                }
                else
                {
                    stats = 0;
                    forwardingx(ground, coords[0], coords[1], -1, stats);

                    if (stats == 2)
                    {
                        return coords;
                    }
                }
            }
        }

        else
        {
            temp_dir = rand() % 2; //Бот выбирает направление, которое он будет сначала добивать (для разнообразия)
            stats = 0;
            if (temp_dir) //Начинает наверх
            {
                forwardingy(ground, coords[0], coords[1], -1, stats);

                if (stats == 2)
                {
                    return coords;
                }
                else
                {
                    stats = 0;
                    forwardingy(ground, coords[0], coords[1], 1, stats);

                    if (stats == 2)
                    {
                        return coords;
                    }
                }
            }
            else    //Вниз
            {
                forwardingy(ground, coords[0], coords[1], 1, stats);

                if (stats == 2)
                {
                    return coords;
                }
                else
                {
                    stats = 0;
                    forwardingy(ground, coords[0], coords[1], -1, stats);

                    if (stats == 2)
                    {
                        return coords;
                    }
                }
            }
        }
    }
    else
    {
        check.isHorizontal = true;
        check.len = 1;
        do
        {
            coords[0] = rand() % 10;
            coords[1] = rand() % 10;
            check.x = coords[0];
            check.y = coords[1];
        } while ((ground[coords[0]][coords[1]] >= 2) || (!canPlaceHere(ground, check, 4)));
    }

    if (ground[coords[0]][coords[1]] == 1)
    {
        bot.remember = true;
        bot.x = coords[0];
        bot.y = coords[1];
    }

    return coords;
}

//Основная функция игры с ботом
//Не путайте ПОВРЕЖДЁННЫЙ и ПОВЕРЖЕННЫЙ
void gameBot()
{
    int ground_p1[10][10], ground_p2[10][10];   //0 - пустая ячейка, в которую не попадали ни разу, 1 - ячейка с кораблем, в который ни разу не попадали, 2 - пустая ячейка, в которую попадали, 3 - ячейка с кораблем, в который попадали, 4 - ячейка с поверженным кораблём (только для визуала)
    int boats_count_p1 = 10;
    int boats_count_p2 = 10;
    Boat boats_p1[10];
    Boat boats_p2[10];

    bool cancel;
    char* yes_no[] = {"Да", "Нет"};

    int* aim_coords_p1 = new int[2];
    aim_coords_p1[0] = 0;
    aim_coords_p1[1] = 0;
    int* aim_coords_p2 = new int[2];
    aim_coords_p2[0] = 0;
    aim_coords_p2[1] = 0;

    Bot bot;
    clearBot(bot);

    do
    {
        groundConstructor(ground_p1, boats_p1);

        setc();
        system("cls");

        printBorder();
        printGround(ground_p1);

        setc();
        setp(15);
        cout << "Начать игру с этим полем?";
        cancel = (bool)menu(yes_no, 2, 15, 2);
    } while (cancel);

    setc();
    system("cls");

    setp(3, 3);
    cout << "Ок, подождите пока бот построит своё поле";

    autoGroundConstructor(ground_p2, boats_p2);

    system("cls");

    printBorder(3, 3);
    printGround(ground_p1, 4, 4, false);
    setc();
    setp(3, 15);
    cout << "ПОЛЕ ИГРОКА №1";
    setp(3, 16);
    cout << "(Вы)";

    printBorder(25, 3);
    printGround(ground_p2, 26, 4, false);
    setc();
    setp(25, 15);
    cout << "ПОЛЕ ИГРОКА №2";
    setp(25, 16);
    cout << "(Бот)";

    do  //Игра идёт до тех пор, пока не уничтожатся все корабли игрока или бота
    {
        do  //Игрок ходит до тех пор, пока не промажет или не уничтожит все корабли бота
        {
            aim_coords_p1 = aim(ground_p2, 26, 4, aim_coords_p1[0], aim_coords_p1[1]);  //Игрок прицеливается
            if (ground_p2[aim_coords_p1[0]][aim_coords_p1[1]] == 0) //Если здесь ничего не оказалось
            {
                ground_p2[aim_coords_p1[0]][aim_coords_p1[1]] = 2;      //Показать, что промахнулся
            }
            else                                                    //Иначе
            {
                ground_p2[aim_coords_p1[0]][aim_coords_p1[1]] = 3;      //Показать, что попал
            }

            checkFrags(ground_p2, boats_p2, boats_count_p2);    //Проверка кораблей

            printGround(ground_p2, 26, 4, false);   //Обновить поле

        } while ((ground_p2[aim_coords_p1[0]][aim_coords_p1[1]] != 2) && (boats_count_p2 > 0));

        if (boats_count_p2 > 0)
        {
            do  //Бот ходит до тех пор, пока не промажет или не уничтожит все корабли игрока
            {
                aim_coords_p2 = botAim(ground_p1, bot);   //Бот прицеливается
                if (ground_p1[aim_coords_p2[0]][aim_coords_p2[1]] == 0) //Если здесь ничего не оказалось
                {
                    ground_p1[aim_coords_p2[0]][aim_coords_p2[1]] = 2;      //Показать, что промахнулся
                }
                else                                                    //Иначе
                {
                    ground_p1[aim_coords_p2[0]][aim_coords_p2[1]] = 3;      //Показать, что попал
                }

                checkFrags(ground_p1, boats_p1, boats_count_p1, bot);    //Проверка кораблей

                printGround(ground_p1, 4, 4, false);    //Обновить поле

            } while ((ground_p1[aim_coords_p2[0]][aim_coords_p2[1]] != 2) && (boats_count_p1 > 0));
        }
    } while ((boats_count_p1 > 0) && (boats_count_p2 > 0));

    setc();
    system("cls");

    printBorder(3, 3);
    printGround(ground_p1, 4, 4);
    setc();
    setp(3, 15);
    cout << "ПОЛЕ ИГРОКА №1";

    printBorder(25, 3);
    printGround(ground_p2, 26, 4);
    setc();
    setp(25, 15);
    cout << "ПОЛЕ ИГРОКА №2";

    if (boats_count_p1 < 1)
    {
        setp(3, 16);
        cout << "Проиграл...";
        setp(25, 16);
        cout << "Выиграл!";
    }
    else
    {
        setp(3, 16);
        cout << "Выиграл!";
        setp(25, 16);
        cout << "Проиграл...";
    }

    delete[] yes_no;
    delete[] aim_coords_p1;
    delete[] aim_coords_p2;
}

//Основной цикл игры PvP
void gameLocal()
{
    int ground_p1[10][10], ground_p2[10][10];
    int boats_count_p1 = 10;
    int boats_count_p2 = 10;
    Boat boats_p1[10];
    Boat boats_p2[10];

    bool cancel;
    char* yes_no[] = {"Да", "Нет"};

    int* aim_coords_p1 = new int[2];
    aim_coords_p1[0] = 0;
    aim_coords_p1[1] = 0;
    int* aim_coords_p2 = new int[2];
    aim_coords_p2[0] = 0;
    aim_coords_p2[1] = 0;

    do
    {
        setc();
        system("cls");
        setp();
        cout << "Пусть первый игрок подойдёт к экрану и нажмёт любую клавишу, \nа второй не будет подсматривать.";
        system("pause>>NULL");

        groundConstructor(ground_p1, boats_p1);

        setc();
        system("cls");

        printBorder();
        printGround(ground_p1);

        setc();
        setp(15);
        cout << "Хотите играть с этим полем?";
        cancel = (bool)menu(yes_no, 2, 15, 2);
    } while (cancel);

    do
    {
        setc();
        system("cls");
        setp();
        cout << "Пусть второй игрок подойдёт к экрану и нажмёт любую клавишу, \nа первый не будет подсматривать.";
        system("pause>>NULL");

        groundConstructor(ground_p2, boats_p2);

        setc();
        system("cls");

        printBorder();
        printGround(ground_p2);

        setc();
        setp(15);
        cout << "Хотите играть с этим полем?";
        cancel = (bool)menu(yes_no, 2, 15, 2);
    } while (cancel);

    setc();
    system("cls");

    printBorder(3, 3);
    printGround(ground_p1, 4, 4, false);
    setc();
    setp(3, 15);
    cout << "ПОЛЕ ИГРОКА №1";

    printBorder(25, 3);
    printGround(ground_p2, 26, 4, false);
    setc();
    setp(25, 15);
    cout << "ПОЛЕ ИГРОКА №2";

    do  //Игра идёт до тех пор, пока не уничтожатся все корабли одного из игроков
    {
        do  //Первый игрок ходит до тех пор, пока не промажет или не уничтожит все корабли второго
        {
            aim_coords_p1 = aim(ground_p2, 26, 4, aim_coords_p1[0], aim_coords_p1[1]);  //Первый игрок прицеливается
            if (ground_p2[aim_coords_p1[0]][aim_coords_p1[1]] == 0) //Если здесь ничего не оказалось
            {
                ground_p2[aim_coords_p1[0]][aim_coords_p1[1]] = 2;      //Показать, что промахнулся
            }
            else                                                    //Иначе
            {
                ground_p2[aim_coords_p1[0]][aim_coords_p1[1]] = 3;      //Показать, что попал
            }

            checkFrags(ground_p2, boats_p2, boats_count_p2);    //Проверка кораблей

            printGround(ground_p2, 26, 4, false);   //Обновить поле

        } while ((ground_p2[aim_coords_p1[0]][aim_coords_p1[1]] != 2) && (boats_count_p2 > 0));

        if (boats_count_p2 > 0)
        {
            do  //Второй игрок ходит до тех пор, пока не промажет или не уничтожит все корабли первого
            {
                aim_coords_p2 = aim(ground_p1, 4, 4, aim_coords_p2[0], aim_coords_p2[1]);   //Второй игрок прицеливается
                if (ground_p1[aim_coords_p2[0]][aim_coords_p2[1]] == 0) //Если здесь ничего не оказалось
                {
                    ground_p1[aim_coords_p2[0]][aim_coords_p2[1]] = 2;      //Показать, что промахнулся
                }
                else                                                    //Иначе
                {
                    ground_p1[aim_coords_p2[0]][aim_coords_p2[1]] = 3;      //Показать, что попал
                }

                checkFrags(ground_p1, boats_p1, boats_count_p1);    //Проверка кораблей

                printGround(ground_p1, 4, 4, false);    //Обновить поле

            } while ((ground_p1[aim_coords_p2[0]][aim_coords_p2[1]] != 2) && (boats_count_p1 > 0));
        }
    } while ((boats_count_p1 > 0) && (boats_count_p2 > 0));

    setc();
    system("cls");

    printBorder(3, 3);
    printGround(ground_p1, 4, 4);
    setc();
    setp(3, 15);
    cout << "ПОЛЕ ИГРОКА №1";

    printBorder(25, 3);
    printGround(ground_p2, 26, 4);
    setc();
    setp(25, 15);
    cout << "ПОЛЕ ИГРОКА №2";

    if (boats_count_p1 < 1)
    {
        setp(3, 16);
        cout << "Проиграл...";
        setp(25, 16);
        cout << "Выиграл!";
    }
    else
    {
        setp(3, 16);
        cout << "Выиграл!";
        setp(25, 16);
        cout << "Проиграл...";
    }

    delete[] yes_no;
    delete[] aim_coords_p1;
    delete[] aim_coords_p2;
}

int main()
{
    setlocale(LC_ALL, "rus");
    srand(time(NULL));
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    char* menu_choises[] = {"Игра с ботом", "Игра с человеком (локально)", "Выйти"};
    const unsigned int menu_len = 3;
    int choised;

    setc();
    system("cls");
    choised = menu(menu_choises, menu_len);
    setc();
    system("cls");

    switch (choised)
    {
    case 0:
        gameBot();
        break;

    case 1:
        gameLocal();
        break;

    default:
        setp();
        cout << "Bye.\n";
        break;
    }

    delete[] menu_choises;

    system("pause>>NULL");
}
