#include <iostream>
#include <conio.h>
#include <thread>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>

using namespace std;

// Constants for the game board size
const int width = 70;
const int height = 20;

// Enumeration for direction control
enum snakesDirection { STOP = 0, LEFT, RIGHT, UP, DOWN };

// Define the Player class
class Player {
public:
    string name;
    int x, y;
    int score;
    int tailX[100], tailY[100];
    int tailLen;
    snakesDirection direction;
    bool isGameOver;

    // Constructor for initializing the player with a name
    Player(string playerName) : name(playerName), score(0), tailLen(0), direction(STOP), isGameOver(false) {
        x = width / 2;
        y = height / 2;
    }

    // Function to reset the player's state
    void reset() {
        score = 0;
        tailLen = 0;
        direction = STOP;
        isGameOver = false;
        x = width / 2;
        y = height / 2;
    }

    // Function to update the player's position based on the direction
    void updatePosition() {
        int prevX = tailX[0];
        int prevY = tailY[0];
        int prev2X, prev2Y;
        tailX[0] = x;
        tailY[0] = y;

        for (int i = 1; i < tailLen; i++) {
            prev2X = tailX[i];
            prev2Y = tailY[i];
            tailX[i] = prevX;
            tailY[i] = prevY;
            prevX = prev2X;
            prevY = prev2Y;
        }

        switch (direction) {
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

        if (x >= width || x < 0 || y >= height || y < 0) {
            isGameOver = true;
        }

        for (int i = 0; i < tailLen; i++) {
            if (tailX[i] == x && tailY[i] == y)
                isGameOver = true;
        }
    }

    // Function to increase the player's score and tail length
    void increaseScore() {
        score += 10;
        tailLen++;
    }
};

// Global variables for game state
int fruitCordX, fruitCordY;
int gameDuration = 180; // Game duration in seconds
vector<pair<int, int>> obstacles; // List of obstacle coordinates

// Function to set the difficulty level of the game
int SetDifficulty(int& obstacleCount, int& obstacleIncrement) {
    int difficulty;
    cout << "Select difficulty level (1: Easy, 2: Medium, 3: Hard): ";
    cin >> difficulty;

    switch (difficulty) {
    case 1:
        obstacleCount = 0;
        obstacleIncrement = 1;
        return 90; // Easy
    case 2:
        obstacleCount = 0;
        obstacleIncrement = 2;
        return 65;  // Medium
    case 3:
        obstacleCount = 0;
        obstacleIncrement = 3;
        return 35;  // Hard
    default:
        obstacleCount = 0;
        obstacleIncrement = 2;
        return 50;  // Default to Medium
    }
}

// Function to generate fruit coordinates randomly
void GenerateFruit() {
    fruitCordX = rand() % width;
    fruitCordY = rand() % height;
}

// Function to generate obstacles at random positions based on difficulty
void GenerateObstacles(int obstacleCount, const vector<Player>& players) {
    for (int i = 0; i < obstacleCount; i++) {
        int obsX, obsY;
        bool safeDistance;
        do {
            safeDistance = true;
            obsX = rand() % width;
            obsY = rand() % height;

            // Check if the obstacle is too close to any player or the fruit
            for (const auto& player : players) {
                if (abs(player.x - obsX) < 3 && abs(player.y - obsY) < 3) {
                    safeDistance = false;
                    break;
                }
                for (int j = 0; j < player.tailLen; j++) {
                    if (abs(player.tailX[j] - obsX) < 3 && abs(player.tailY[j] - obsY) < 3) {
                        safeDistance = false;
                        break;
                    }
                }
            }
            if (abs(fruitCordX - obsX) < 3 && abs(fruitCordY - obsY) < 3) {
                safeDistance = false;
            }
        } while (!safeDistance);

        obstacles.push_back(make_pair(obsX, obsY));
    }
}

// Function to render the game state onto the screen
void GameRender(const vector<Player>& players, int remainingTime) {
    string buffer;

    buffer += "+";
    for (int i = 0; i < width; i++)
        buffer += "-";
    buffer += "+\n";

    for (int i = 0; i < height; i++) {
        buffer += "|";
        for (int j = 0; j < width; j++) {
            bool drawn = false;
            for (const auto& player : players) {
                if (i == player.y && j == player.x) {
                    buffer += "O";
                    drawn = true;
                    break;
                }
                for (int k = 0; k < player.tailLen; k++) {
                    if (player.tailX[k] == j && player.tailY[k] == i) {
                        buffer += "o";
                        drawn = true;
                        break;
                    }
                }
            }
            if (!drawn) {
                if (i == fruitCordY && j == fruitCordX)
                    buffer += "#";
                else {
                    bool isObstacle = false;
                    for (const auto& obstacle : obstacles) {
                        if (obstacle.first == j && obstacle.second == i) {
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

    for (const auto& player : players) {
        buffer += player.name + "'s Score: " + to_string(player.score) + "\n";
    }
    buffer += "Remaining Time: " + to_string(remainingTime) + " seconds\n";
    buffer += "Press 'q' to Save & Exit\n";

    system("cls");
    cout << buffer;
}

// Function to handle user input for controlling the players
void UserInput(vector<Player>& players, bool& saveAndQuit) {
    if (_kbhit()) {
        switch (_getch()) {
        case 'a':
            if (players[0].direction != RIGHT) players[0].direction = LEFT;
            break;
        case 'd':
            if (players[0].direction != LEFT) players[0].direction = RIGHT;
            break;
        case 'w':
            if (players[0].direction != DOWN) players[0].direction = UP;
            break;
        case 's':
            if (players[0].direction != UP) players[0].direction = DOWN;
            break;
        case 'j':
            if (players[1].direction != RIGHT) players[1].direction = LEFT;
            break;
        case 'l':
            if (players[1].direction != LEFT) players[1].direction = RIGHT;
            break;
        case 'i':
            if (players[1].direction != DOWN) players[1].direction = UP;
            break;
        case 'k':
            if (players[1].direction != UP) players[1].direction = DOWN;
            break;
        case 'q':
            saveAndQuit = true;
            for (auto& player : players) {
                player.isGameOver = true;
            }
            break;
        }
    }
}

// Function to show the main menu
void ShowMenu() {
    cout << "1. Start New Game (Single Player)\n";
    cout << "2. Start New Game (Multiplayer)\n";
    cout << "3. Load Game\n";
    cout << "4. Exit\n";
    cout << "Enter your choice: ";
}

// Function to save the current game state to a file
void SaveGameState(const string& filename, const vector<Player>& players, int remainingTime, int dfc, int obstacleCount) {
    ofstream file(filename);
    if (file.is_open()) {
                file << players.size() << endl;
        for (const auto& player : players) {
            file << player.name << " " << player.x << " " << player.y << " " << player.score << " " << player.tailLen << " " << player.direction << endl;
            for (int i = 0; i < player.tailLen; i++) {
                file << player.tailX[i] << " " << player.tailY[i] << " ";
            }
            file << endl;
        }
        file << fruitCordX << " " << fruitCordY << endl;
        file << obstacles.size() << endl;
        for (const auto& obstacle : obstacles) {
            file << obstacle.first << " " << obstacle.second << " ";
        }
        file << endl;
        file << remainingTime << endl;
        file << dfc << endl;
        file << obstacleCount << endl;
        file.close();
    }
}

// Function to load a game state from a file
void LoadGameState(const string& filename, vector<Player>& players, int& remainingTime, int& dfc, int& obstacleCount) {
    ifstream file(filename);
    if (file.is_open()) {
        int numPlayers;
        file >> numPlayers;
        players.clear();
        for (int i = 0; i < numPlayers; i++) {
            string name;
            int x, y, score, tailLen, direction;
            file >> name >> x >> y >> score >> tailLen >> direction;
            Player player(name);
            player.x = x;
            player.y = y;
            player.score = score;
            player.tailLen = tailLen;
            player.direction = static_cast<snakesDirection>(direction);
            for (int j = 0; j < tailLen; j++) {
                file >> player.tailX[j] >> player.tailY[j];
            }
            players.push_back(player);
        }
        file >> fruitCordX >> fruitCordY;
        int numObstacles;
        file >> numObstacles;
        obstacles.clear();
        for (int i = 0; i < numObstacles; i++) {
            int obsX, obsY;
            file >> obsX >> obsY;
            obstacles.push_back(make_pair(obsX, obsY));
        }
        file >> remainingTime;
        file >> dfc;
        file >> obstacleCount;
        file.close();
    }
}

int main() {
    srand(unsigned(time(NULL)));
    int choice;
    string filename = "gamestate.txt";
    vector<Player> players;
    int obstacleCount; // Number of obstacles
    int obstacleIncrement; // Number of obstacles to add periodically
    int dfc = 50; // Default difficulty level

    ShowMenu();
    cin >> choice;

    switch (choice) {
    case 1: {
        string playerName;
        cout << "Enter your name: ";
        cin >> playerName;
        players.push_back(Player(playerName));
        players[0].reset();
        dfc = SetDifficulty(obstacleCount, obstacleIncrement); // Set difficulty and initial obstacle count
        break;
    }
    case 2: {
        string playerName1, playerName2;
        cout << "Enter Player 1 name: ";
        cin >> playerName1;
        cout << "Enter Player 2 name: ";
        cin >> playerName2;
        players.push_back(Player(playerName1));
        players.push_back(Player(playerName2));
        players[0].reset();
        players[1].reset();
        // Set initial positions for players to avoid collision
        players[1].x = players[0].x - 1;
        players[1].y = players[0].y;
        dfc = SetDifficulty(obstacleCount, obstacleIncrement); // Set difficulty and initial obstacle count
        break;
    }
    case 3: {
        LoadGameState(filename, players, gameDuration, dfc, obstacleCount);
        break;
    }
    case 4:
        return 0;
    default:
        cout << "Invalid choice.\n";
        return 0;
    }

    GenerateFruit(); // Generate initial fruit

    auto start_time = chrono::steady_clock::now();
    auto lastObstacleTime = start_time;
    bool saveAndQuit = false;

    while (true) {
        auto current_time = chrono::steady_clock::now();
        chrono::duration<double> elapsed_seconds = current_time - start_time;
        int remaining_time = gameDuration - static_cast<int>(elapsed_seconds.count());

        chrono::duration<double> obstacle_elapsed_seconds = current_time - lastObstacleTime;
        if (obstacle_elapsed_seconds.count() >= 30) {
            GenerateObstacles(obstacleIncrement, players); // Add new obstacles periodically
            lastObstacleTime = current_time;
        }

        bool anyGameOver = false;
        for (auto& player : players) {
            if (!player.isGameOver) {
                player.updatePosition();

                // Check if the player has eaten the fruit
                if (player.x == fruitCordX && player.y == fruitCordY) {
                    player.increaseScore();
                    GenerateFruit(); // Generate a new fruit
                }
                // Check if the player hits an obstacle
                for (const auto& obstacle : obstacles) {
                    if (player.x == obstacle.first && player.y == obstacle.second) {
                        player.isGameOver = true;
                        anyGameOver = true;
                    }
                }
            }
            if (player.isGameOver) {
                anyGameOver = true;
            }
        }

        // Check for collisions between snakes
        for (size_t i = 0; i < players.size(); i++) {
            for (size_t j = 0; j < players.size(); j++) {
                if (i != j) {
                    // Check if players[i] collides with players[j]
                    if (players[i].x == players[j].x && players[i].y == players[j].y) {
                        players[i].isGameOver = true;
                        players[j].isGameOver = true;
                        anyGameOver = true;
                    }
                    for (int k = 0; k < players[j].tailLen; k++) {
                        if (players[i].x == players[j].tailX[k] && players[i].y == players[j].tailY[k]) {
                            players[i].isGameOver = true;
                            anyGameOver = true;
                        }
                    }
                }
            }
        }

        GameRender(players, remaining_time);

        if (remaining_time <= 0 || anyGameOver) {
            cout << "Game over." << endl;
            break;
        }

        UserInput(players, saveAndQuit);

        if (saveAndQuit) {
            SaveGameState(filename, players, remaining_time, dfc, obstacleCount);
            break;
        }

        this_thread::sleep_for(chrono::milliseconds(dfc));
    }

    return 0;
}

