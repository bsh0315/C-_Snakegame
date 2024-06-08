#include <iostream>     // 표준 입출력 스트림을 위한 헤더 파일    
#include <conio.h>      // _kbhit() 및 _getch()로 키보드 입력을 감지하기 위한 헤더 파일
#include <thread>       // 콘솔을 지우기 위해 system("cls")를 사용하기 위한 헤더 파일
#include <vector>       // 문자열 처리를 위한 헤더 파일
#include <chrono>       // 시간 처리를 위한 헤더 파일
#include <cstdlib>      // this_thread::sleep_for로 게임 속도를 제어하기 위한 헤더 파일
#include <ctime>        // 장애물을 관리하기 위한 벡터 헤더 파일
#include <fstream>      // 시간 처리를 위한 헤더 파일
#include <string>       // 파일 입출력을 위한 헤더 파일

using namespace std;

// 게임 크기 정의
const int width = 80;
const int height = 20;

// 방향 컨트롤 열거(2인 플레이시 대비)
enum snakesDirection { STOP = 0, LEFT, RIGHT, UP, DOWN };

// Player 클래스 정의
class Player {
public:
    string name;                             // 플레이어 이름
    int x, y;                                // 뱀 머리 좌표
    int score;                               // 플레이어 점수
    int tailX[100], tailY[100];              // 뱀 꼬리 좌표
    int tailLen;                             // 뱀 꼬리 길이
    snakesDirection direction;               // 뱀 방향
    bool isGameOver;                         // 게임오버 여부

    // Player 생성자
    Player(string playerName) : name(playerName), score(0), tailLen(0), direction(STOP), isGameOver(false) {
        x = width / 2;
        y = height / 2;
    }

    // Player 상태 초기화
    void reset() {
        score = 0;
        tailLen = 0;
        direction = STOP;
        isGameOver = false;
        x = width / 2;
        y = height / 2;
    }

    // Player의 자리 업데이트하는 함수
    void updatePosition() {
        int prevX = tailX[0];   // 이전 머리 위치 저장
        int prevY = tailY[0];
        int prev2X, prev2Y;
        tailX[0] = x;   // 머리 위치를 꼬리 앞에 이동
        tailY[0] = y;

        // 꼬리 이동
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

        // 벽과의 충돌 검사
        if (x >= width || x < 0 || y >= height || y < 0) {
            isGameOver = true;
        }

        // 꼬리와의 충돌 검사
        for (int i = 0; i < tailLen; i++) {
            if (tailX[i] == x && tailY[i] == y)
                isGameOver = true;
        }
    }

    // 점수 증가 함수
    void increaseScore() {
        score += 10;
        tailLen++;
    }
};

// 과일 좌표
int fruitCordX, fruitCordY;
int gameDuration = 180; // 게임 지속 시간
vector<pair<int, int>> obstacles; // 장애물 좌표

// 게임 난이도를 설정하는 함수
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
        return 65;  // Default(Medium)
    }
}

// 과일을 랜덤하게 생성하는 함수
void GenerateFruit() {
    fruitCordX = rand() % width;
    fruitCordY = rand() % height;
}

// 난이도에 따른 장애물 생성하는 함수
void GenerateObstacles(int obstacleCount, const vector<Player>& players) {
    for (int i = 0; i < obstacleCount; i++) {
        int obsX, obsY;
        bool safeDistance; // 장애물이 플레이어에 너무 가깝지 않은지 확인하는 변수
        do {
            safeDistance = true; 
            obsX = rand() % width;
            obsY = rand() % height;

            // 과일이 플레이어에 너무 가깝지는 않은지 체크
            for (const auto& player : players) {
                if (abs(player.x - obsX) < 3 && abs(player.y - obsY) < 3) { 
                    // 플레이어와 장애물 사이의 거리가 3보다 작으면 안전거리가 아님
                    safeDistance = false; 
                    break;
                }
                for (int j = 0; j < player.tailLen; j++) {
                    if (abs(player.tailX[j] - obsX) < 3 && abs(player.tailY[j] - obsY) < 3) {
                        // 꼬리와 장애물 사이의 거리가 3보다 작으면 안전거리가 아님
                        safeDistance = false;
                        break;
                    }
                }
            }
            if (abs(fruitCordX - obsX) < 3 && abs(fruitCordY - obsY) < 3) { 
                // 과일과 장애물 사이의 거리가 3보다 작으면 안전거리가 아님
                safeDistance = false;
            }
        } while (!safeDistance);

        obstacles.push_back(make_pair(obsX, obsY));
    }
}

// 게임 상태를 렌더링하는 함수
void GameRender(const vector<Player>& players, int remainingTime) {
    string buffer;      // 게임 화면을 만들 버퍼 생성

    // 상단 경계 생성
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
                    buffer += "O";  // 뱀 머리 그리기
                    drawn = true;
                    break;
                }
                for (int k = 0; k < player.tailLen; k++) {
                    if (player.tailX[k] == j && player.tailY[k] == i) {
                        buffer += "o";  // 뱀 꼬리 그리기
                        drawn = true;
                        break;
                    }
                }
            }
            if (!drawn) {
                if (i == fruitCordY && j == fruitCordX)
                    buffer += "#";  // 과일 그리기
                else {
                    bool isObstacle = false;
                    for (const auto& obstacle : obstacles) {
                        if (obstacle.first == j && obstacle.second == i) {
                            buffer += "!";  // 장애물 그리기
                            isObstacle = true;
                            break;
                        }
                    }
                    if (!isObstacle)
                        buffer += " ";  // 빈 공간 그리기
                }
            }
        }
        buffer += "|\n";    // 줄 끝 경계
    }

    // 하단 경계 생성
    buffer += "+";
    for (int i = 0; i < width; i++)
        buffer += "-";
    buffer += "+\n";

    for (const auto& player : players) {
        buffer += player.name + "'s Score: " + to_string(player.score) + "\n";
    }
    // 점수 및 남은 시간 표시
    buffer += "Remaining Time: " + to_string(remainingTime) + " seconds\n";
    buffer += "Press 'q' to Save & Exit\n";

    system("cls");  // 콘솔 지우기
    cout << buffer; // 콘솔에 버퍼 출력
}

// 방향키 및 'q' 처리하는 함수
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

// 메뉴 출력 함수
void ShowMenu() {
    cout << "1. Start New Game (Single Player)\n";
    cout << "2. Start New Game (Multiplayer)\n";
    cout << "3. Load Game\n";
    cout << "4. Exit\n";
    cout << "Enter your choice: ";
}

// 현재 상태를 파일에 저장하는 함수
void SaveGameState(const string& filename, const vector<Player>& players, int remainingTime, int dfc, int obstacleCount, int obstacleIncrement) {
    // 파일을 쓰기 모드로 열기
    ofstream file(filename);
    if (file.is_open()) { // 파일이 열렸는지 확인
        file << players.size() << endl;
        for (const auto& player : players) {
            file << player.name << " " << player.x << " " << player.y << " " << player.score << " " << player.tailLen << " " << player.direction << endl;
            for (int i = 0; i < player.tailLen; i++) {
                file << player.tailX[i] << " " << player.tailY[i] << " ";
            }
            file << endl;
            // 플레이어의 정보를 파일에 저장
        }
        file << fruitCordX << " " << fruitCordY << endl; // 과일의 좌표를 파일에 저장
        file << obstacles.size() << endl; // 장애물 수를 파일에 저장
        for (const auto& obstacle : obstacles) { // 장애물 좌표를 파일에 저장
            file << obstacle.first << " " << obstacle.second << " ";
        }
        file << endl;
        file << remainingTime << endl;
        file << dfc << endl;
        file << obstacleCount << endl;
        file << obstacleIncrement << endl; // Save obstacleIncrement
        file.close();
    }
}

// 파일의 상태를 로드하는 함수
void LoadGameState(const string& filename, vector<Player>& players, int& remainingTime, int& dfc, int& obstacleCount, int& obstacleIncrement) {
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
            players.push_back(player); // 플레이어 추가
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
        file >> obstacleIncrement; // Load obstacleIncrement
        file.close();
    }
}

int main() {
    srand(unsigned(time(NULL)));
    int choice;
    string filename = "gamestate.txt";
    vector<Player> players;
    int obstacleCount; // 장애물 수
    int obstacleIncrement; // 장애물 증가 수
    int dfc = 50; // 난이도

    ShowMenu();
    cin >> choice;

    switch (choice) {
    case 1: {
        string playerName;
        cout << "Enter your name: ";
        cin >> playerName;
        players.push_back(Player(playerName));
        players[0].reset(); // 플레이어 초기화
        dfc = SetDifficulty(obstacleCount, obstacleIncrement); // 난이도와 장애물 설정
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
        players[1].x = players[0].x - 1;
        players[1].y = players[0].y;
        // 각 플레이어의 시작 위치가 겹치지 않도록 설정
        dfc = SetDifficulty(obstacleCount, obstacleIncrement); // 난이도와 장애물 수 설정
        break;
    }
    case 3: {
        LoadGameState(filename, players, gameDuration, dfc, obstacleCount, obstacleIncrement);
        // 기존 게임 상태 로드
        break;
    }
    case 4:
        return 0;
    default:
        cout << "Invalid choice.\n";
        return 0;
    }

    GenerateFruit(); // 처음 과일 위치 설정

    auto start_time = chrono::steady_clock::now(); // 게임 시작 시간 설정
    auto lastObstacleTime = start_time; // 마지막 장애물 생성 시간 설정
    bool saveAndQuit = false; // 저장하고 종료하는 플래그 설정

    while (true) {
        auto current_time = chrono::steady_clock::now(); // 현재 시간 설정
        chrono::duration<double> elapsed_seconds = current_time - start_time; // 경과 시간 계산
        int remaining_time = gameDuration - static_cast<int>(elapsed_seconds.count()); // 남은 시간 계산

        chrono::duration<double> obstacle_elapsed_seconds = current_time - lastObstacleTime; // 장애물 생성 시간 계산
        if (obstacle_elapsed_seconds.count() >= 30) {
            GenerateObstacles(obstacleIncrement, players); // 장애물 생성
            lastObstacleTime = current_time; // 마지막 장애물 생성 시간 업데이트
        }

        bool anyGameOver = false;
        for (auto& player : players) {
            if (!player.isGameOver) {
                player.updatePosition();

                // 과일 먹었는지 확인
                if (player.x == fruitCordX && player.y == fruitCordY) {
                    player.increaseScore();
                    GenerateFruit(); // 새로운 과일 생성
                }
                // 장애물에 닿았는지 확인
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

        // snake 사이의 충돌 확인
        // 충돌이 존재하면 게임오버 처리
        for (size_t i = 0; i < players.size(); i++) {
            for (size_t j = 0; j < players.size(); j++) {
                if (i != j) {
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

        GameRender(players, remaining_time); // 게임 상태 렌더링

        if (remaining_time <= 0 || anyGameOver) { 
            // 게임 종료 조건, remaining_time이 0이거나 누군가 게임오버되면 게임 종료
            cout << "Game over." << endl;
            break;
        }

        UserInput(players, saveAndQuit); // 사용자 입력 처리

        if (saveAndQuit) { // 'q'를 눌러 저장하고 종료
            SaveGameState(filename, players, remaining_time, dfc, obstacleCount, obstacleIncrement);
            break;
        }

        this_thread::sleep_for(chrono::milliseconds(dfc)); // 게임 속도 제어
    }

    return 0;
}
