#include <iostream>
#include <conio.h>
#include <windows.h>
#include <string>
#include <chrono>
#include <thread>
#include <vector>
#include <ctime>
#include <fstream>

using namespace std;

const int width = 80;
const int height = 20;
const int gameDuration = 180;

int x, y;
int fruitCordX, fruitCordY;
int playerScore;
int snakeTailX[100], snakeTailY[100];
int snakeTailLen;
string playerName;
int dfc;

enum snakesDirection { STOP = 0, LEFT, RIGHT, UP, DOWN };
snakesDirection sDir;
bool isGameOver;

vector<pair<int, int>> obstacles;
int obstaclesPerInterval;

void GameInit()
{
    isGameOver = false;
    sDir = STOP;
    x = width / 2;
    y = height / 2;
    fruitCordX = rand() % width;
    fruitCordY = rand() % height;
    playerScore = 0;
    snakeTailLen = 0;
    obstacles.clear();
}

void GenerateObstacles()
{
    int obstaclesToAdd = obstaclesPerInterval;
    while (obstaclesToAdd > 0) {
        int newObstacleX = rand() % width;
        int newObstacleY = rand() % height;
        bool conflict = false;
        if (newObstacleX == x && newObstacleY == y) {
            conflict = true;
        }
        for (int i = 0; i < snakeTailLen; i++) {
            if (newObstacleX == snakeTailX[i] && newObstacleY == snakeTailY[i]) {
                conflict = true;
                break;
            }
        }
        if (!conflict) {
            obstacles.push_back(make_pair(newObstacleX, newObstacleY));
            obstaclesToAdd--;
        }
    }
}

void GameRender(string playerName, int remainingTime)
{
    string buffer;
    buffer += "+";
    for (int i = 0; i < width; i++)
        buffer += "-";
    buffer += "+\n";

    for (int i = 0; i < height; i++) {
        buffer += "|";
        for (int j = 0; j < width; j++) {
            if (i == y && j == x)
                buffer += "O";
            else if (i == fruitCordY && j == fruitCordX)
                buffer += "#";
            else {
                bool prTail = false;
                for (int k = 0; k < snakeTailLen; k++) {
                    if (snakeTailX[k] == j && snakeTailY[k] == i) {
                        buffer += "o";
                        prTail = true;
                        break;
                    }
                }
                if (!prTail) {
                    bool isObstacle = false;
                    for (int k = 0; k < obstacles.size(); k++) {
                        if (obstacles[k].first == j && obstacles[k].second == i) {
                            buffer += "!";
                            isObstacle = true;
                            break;
                        }
                    }
                    if (!isObstacle)
                        buffer += " ";
                }
            }
        }
        buffer += "|\n";
    }

    buffer += "+";
    for (int i = 0; i < width; i++)
        buffer += "-";
    buffer += "+\n";

    buffer += playerName + "'s Score: " + to_string(playerScore) + "\n";
    buffer += "Remaining Time: " + to_string(remainingTime) + " seconds\n";
    buffer += "Press 'q' to Save & Exit\n";

    system("cls");
    cout << buffer;
}

void UpdateGame()
{
    int prevX = snakeTailX[0];
    int prevY = snakeTailY[0];
    int prev2X, prev2Y;
    snakeTailX[0] = x;
    snakeTailY[0] = y;

    for (int i = 1; i < snakeTailLen; i++) {
        prev2X = snakeTailX[i];
        prev2Y = snakeTailY[i];
        snakeTailX[i] = prevX;
        snakeTailY[i] = prevY;
        prevX = prev2X;
        prevY = prev2Y;
    }

    switch (sDir) {
    case LEFT:
        x--;
        break;
    case RIGHT:
        x++;
        break;
    case UP:
        y--;
        break;
    case DOWN:
        y++;
        break;
    default:
        break;
    }

    if (x >= width) x = 0; else if (x < 0) x = width - 1;
    if (y >= height) y = 0; else if (y < 0) y = height - 1;

    for (int i = 0; i < snakeTailLen; i++) {
        if (snakeTailX[i] == x && snakeTailY[i] == y)
            isGameOver = true;
    }

    for (int i = 0; i < obstacles.size(); i++) {
        if (obstacles[i].first == x && obstacles[i].second == y)
            isGameOver = true;
    }

    if (x == fruitCordX && y == fruitCordY) {
        playerScore += 10;
        fruitCordX = rand() % width;
        fruitCordY = rand() % height;
        snakeTailLen++;
    }
}


int SetDifficulty()
{
    int choice;
    cout << "\nSET DIFFICULTY\n1: Easy\n2: Medium\n3: Hard "
        "\nNOTE: if not chosen or pressed any other "
        "key, the difficulty will be automatically set "
        "to medium\nChoose difficulty level: ";
    cin >> choice;
    switch (choice) {
    case 1:
        dfc = 150;
        obstaclesPerInterval = 1;
        break;
    case 2:
        dfc = 100;
        obstaclesPerInterval = 2;
        break;
    case 3:
        dfc = 50;
        obstaclesPerInterval = 3;
        break;
    default:
        dfc = 100;
        obstaclesPerInterval = 2;
    }
    return dfc;
}

void UserInput(bool& saveAndQuit)
{
    if (_kbhit()) {
        switch (_getch()) {
        case 'a':
            if (sDir != RIGHT) sDir = LEFT;
            break;
        case 'd':
            if (sDir != LEFT) sDir = RIGHT;
            break;
        case 'w':
            if (sDir != DOWN) sDir = UP;
            break;
        case 's':
            if (sDir != UP) sDir = DOWN;
            break;
        case 'x':
            isGameOver = true;
            break;
        case 'q':
            saveAndQuit = true;
            isGameOver = true;
            break;
        }
    }
}

void SaveGameState(const string& filename)
{
    ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << playerName << "\n";
        outFile << dfc << "\n";
        outFile << x << " " << y << " " << fruitCordX << " " << fruitCordY << " "
            << playerScore << " " << snakeTailLen << " " << static_cast<int>(sDir) << " " << obstacles.size() << "\n";
        for (int i = 0; i < snakeTailLen; i++) {
            outFile << snakeTailX[i] << " " << snakeTailY[i] << " ";
        }
        outFile << "\n";
        for (const auto& obstacle : obstacles) {
            outFile << obstacle.first << " " << obstacle.second << " ";
        }
        outFile.close();
        cout << "Game state saved to " << filename << endl;
    }
    else {
        cout << "Unable to open file for saving.\n";
    }
}

void LoadGameState(const string& filename)
{
    ifstream inFile(filename);
    if (inFile.is_open()) {
        int dir, obstacleCount;
        inFile >> playerName;
        inFile >> dfc;
        inFile >> x >> y >> fruitCordX >> fruitCordY >> playerScore >> snakeTailLen >> dir >> obstacleCount;
        sDir = static_cast<snakesDirection>(dir);
        for (int i = 0; i < snakeTailLen; i++) {
            inFile >> snakeTailX[i] >> snakeTailY[i];
        }
        obstacles.clear();
        for (int i = 0; i < obstacleCount; i++) {
            int obsX, obsY;
            inFile >> obsX >> obsY;
            obstacles.push_back(make_pair(obsX, obsY));
        }
        inFile.close();
        cout << "Game state loaded from " << filename << endl;
    }
    else {
        cout << "Unable to open file for loading.\n";
    }
}

void ShowMenu()
{
    cout << "1. Start New Game\n";
    cout << "2. Load Game\n";
    cout << "3. Exit\n";
    cout << "Enter your choice: ";
}

int main()
{
    srand(unsigned(time(NULL)));
    int choice;
    string filename = "gamestate.txt";

    ShowMenu();
    cin >> choice;

    switch (choice) {
    case 1:
        cout << "Enter your name: ";
        cin >> playerName;
        GameInit();
        dfc = SetDifficulty();
        break;
    case 2:
        LoadGameState(filename);
        break;
    case 3:
        return 0;
    default:
        cout << "Invalid choice.\n";
        return 0;
    }

    auto start_time = chrono::steady_clock::now();
    auto lastObstacleTime = start_time;
    bool saveAndQuit = false;

    while (!isGameOver) {
        auto current_time = chrono::steady_clock::now();
        chrono::duration<double> elapsed_seconds = current_time - start_time;
        int remaining_time = gameDuration - static_cast<int>(elapsed_seconds.count());

        chrono::duration<double> obstacle_elapsed_seconds = current_time - lastObstacleTime;
        if (obstacle_elapsed_seconds.count() >= 30) {
            GenerateObstacles();
            lastObstacleTime = current_time;
        }

        GameRender(playerName, remaining_time);

        if (remaining_time <= 0) {
            cout << "Time's up! Game over." << endl;
            isGameOver = true;
            break;
        }

        UserInput(saveAndQuit);
        UpdateGame();

        auto end = chrono::steady_clock::now();
        chrono::duration<double> frame_duration = end - current_time;
        this_thread::sleep_for(chrono::milliseconds(dfc) - frame_duration);
    }

    if (saveAndQuit) {
        SaveGameState(filename);
    }

    cout << "Final Score: " << playerScore << endl;
    return 0;
}
