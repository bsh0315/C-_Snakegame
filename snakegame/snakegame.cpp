#include <iostream>     // 표준 입출력 스트림을 위한 헤더 파일
#include <conio.h>      // _kbhit() 및 _getch()로 키보드 입력을 감지하기 위한 헤더 파일
#include <windows.h>    // 콘솔을 지우기 위해 system("cls")를 사용하기 위한 헤더 파일
#include <string>       // 문자열 처리를 위한 헤더 파일
#include <chrono>       // 시간 처리를 위한 헤더 파일
#include <thread>       // this_thread::sleep_for로 게임 속도를 제어하기 위한 헤더 파일
#include <vector>       // 장애물을 관리하기 위한 벡터 헤더 파일
#include <ctime>        // 시간 처리를 위한 헤더 파일

using namespace std;

// 게임 크기 및 지속 시간을 정의
const int width = 80;
const int height = 20;
const int gameDuration = 180; // 게임 지속 시간(초)

// 게임 상태 변수 선언
int x, y; // 뱀 머리 좌표
int fruitCordX, fruitCordY; // 과일 좌표
int playerScore; // 플레이어 점수
int snakeTailX[100], snakeTailY[100]; // 뱀 꼬리 좌표
int snakeTailLen; // 뱀 꼬리 길이

// 뱀의 방향 정의
enum snakesDirection { STOP = 0, LEFT, RIGHT, UP, DOWN };
snakesDirection sDir; // 현재 뱀의 방향
bool isGameOver; // 게임 종료 여부

vector<pair<int, int>> obstacles; // 장애물 좌표를 저장할 벡터

// 게임 초기화 함수
void GameInit()
{
    isGameOver = false; // 게임 종료 플래그 초기화
    sDir = STOP; // 초기 방향을 STOP으로 설정
    x = width / 2; // 뱀 머리 위치를 중앙으로 설정
    y = height / 2;
    fruitCordX = rand() % width; // 과일 위치를 랜덤으로 설정
    fruitCordY = rand() % height;
    playerScore = 0; // 플레이어 점수 초기화
    snakeTailLen = 0; // 뱀 꼬리 길이 초기화
}

// 장애물을 랜덤하게 생성하는 함수
void GenerateObstacles()
{
    int obstaclesToAdd = 3; // 30초마다 추가할 장애물 수
    while (obstaclesToAdd > 0) {
        int newObstacleX = rand() % width;
        int newObstacleY = rand() % height;

        // 장애물이 뱀의 머리나 꼬리 위치에 생성되지 않도록 확인
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

// 게임 상태를 렌더링하는 함수
void GameRender(string playerName, int remainingTime)
{
    string buffer; // 게임 화면을 만들 버퍼 생성

    // 상단 경계 생성
    buffer += "+";
    for (int i = 0; i < width; i++)
        buffer += "-";
    buffer += "+\n";

    // 게임 영역 생성
    for (int i = 0; i < height; i++) {
        buffer += "|";
        for (int j = 0; j < width; j++) {
            if (i == y && j == x)
                buffer += "O"; // 뱀 머리 그리기
            else if (i == fruitCordY && j == fruitCordX)
                buffer += "#"; // 과일 그리기
            else {
                bool prTail = false;
                for (int k = 0; k < snakeTailLen; k++) {
                    if (snakeTailX[k] == j && snakeTailY[k] == i) {
                        buffer += "o"; // 뱀 꼬리 그리기
                        prTail = true;
                        break; // 중복 검사를 피하기 위해 break
                    }
                }
                if (!prTail) {
                    bool isObstacle = false;
                    for (int k = 0; k < obstacles.size(); k++) {
                        if (obstacles[k].first == j && obstacles[k].second == i) {
                            buffer += "!"; // 장애물 그리기
                            isObstacle = true;
                            break;
                        }
                    }
                    if (!isObstacle)
                        buffer += " "; // 빈 공간 그리기
                }
            }
        }
        buffer += "|\n"; // 줄 끝 경계
    }

    // 하단 경계 생성
    buffer += "+";
    for (int i = 0; i < width; i++)
        buffer += "-";
    buffer += "+\n";

    // 점수 및 남은 시간 표시
    buffer += playerName + "'s Score: " + to_string(playerScore) + "\n";
    buffer += "Remaining Time: " + to_string(remainingTime) + " seconds\n";

    system("cls"); // 콘솔 지우기
    cout << buffer; // 콘솔에 버퍼 출력
}

// 게임 상태를 업데이트하는 함수
void UpdateGame()
{
    int prevX = snakeTailX[0]; // 이전 머리 위치 저장
    int prevY = snakeTailY[0];
    int prev2X, prev2Y;
    snakeTailX[0] = x; // 머리 위치를 꼬리 앞에 이동
    snakeTailY[0] = y;

    // 꼬리 이동
    for (int i = 1; i < snakeTailLen; i++) {
        prev2X = snakeTailX[i];
        prev2Y = snakeTailY[i];
        snakeTailX[i] = prevX;
        snakeTailY[i] = prevY;
        prevX = prev2X;
        prevY = prev2Y;
    }

    // 방향에 따라 머리 위치 업데이트
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
    }

    // 벽과의 충돌 검사
    if (x >= width || x < 0 || y >= height || y < 0)
        isGameOver = true;

    // 꼬리와의 충돌 검사
    for (int i = 0; i < snakeTailLen; i++) {
        if (snakeTailX[i] == x && snakeTailY[i] == y)
            isGameOver = true;
    }

    // 장애물과의 충돌 검사
    for (int i = 0; i < obstacles.size(); i++) {
        if (obstacles[i].first == x && obstacles[i].second == y)
            isGameOver = true;
    }

    // 뱀이 과일을 먹었는지 검사
    if (x == fruitCordX && y == fruitCordY) {
        playerScore += 10; // 점수 증가
        fruitCordX = rand() % width; // 과일 위치 재설정
        fruitCordY = rand() % height;
        snakeTailLen++; // 꼬리 길이 증가
    }
}

// 게임 난이도를 설정하는 함수
int SetDifficulty()
{
    int dfc, choice;
    cout << "\nSET DIFFICULTY\n1: Easy\n2: Medium\n3: Hard "
        "\nNOTE: if not chosen or pressed any other "
        "key, the difficulty will be automatically set "
        "to medium\nChoose difficulty level: ";
    cin >> choice;
    switch (choice) {
    case 1:
        dfc = 150; // Easy: 150 밀리초
        break;
    case 2:
        dfc = 100; // Medium: 100 밀리초
        break;
    case 3:
        dfc = 50; // Hard: 50 밀리초
        break;
    default:
        dfc = 100; // 기본값: 중간 난이도
    }
    return dfc;
}

// 사용자 입력을 처리하는 함수
void UserInput()
{
    if (_kbhit()) { // 키가 눌렸는지 확인
        switch (_getch()) {
        case 'a':
            if (sDir != RIGHT) sDir = LEFT; // 오른쪽으로 이동 중이 아니면 왼쪽으로 이동
            break;
        case 'd':
            if (sDir != LEFT) sDir = RIGHT; // 왼쪽으로 이동 중이 아니면 오른쪽으로 이동
            break;
        case 'w':
            if (sDir != DOWN) sDir = UP; // 아래로 이동 중이 아니면 위로 이동
            break;
        case 's':
            if (sDir != UP) sDir = DOWN; // 위로 이동 중이 아니면 아래로 이동
            break;
        case 'x':
            isGameOver = true; // 'x'를 누르면 게임 종료
            break;
        }
    }
}

int main()
{
    srand(unsigned(time(NULL))); // 난수 초기화
    string playerName;
    cout << "Enter your name: ";
    cin >> playerName; // 플레이어 이름 입력
    int dfc = SetDifficulty(); // 난이도 설정

    GameInit(); // 게임 초기화
    auto start_time = chrono::steady_clock::now(); // 게임 시작 시간 기록
    auto lastObstacleTime = start_time; // 마지막 장애물 생성 시간 기록

    while (!isGameOver) {
        auto current_time = chrono::steady_clock::now();
        chrono::duration<double> elapsed_seconds = current_time - start_time;
        int remaining_time = gameDuration - static_cast<int>(elapsed_seconds.count());

        // 30초마다 장애물 3개 추가
        chrono::duration<double> obstacle_elapsed_seconds = current_time - lastObstacleTime;
        if (obstacle_elapsed_seconds.count() >= 30) {
            GenerateObstacles();
            lastObstacleTime = current_time;
        }

        GameRender(playerName, remaining_time); // 남은 시간을 포함하여 게임 화면 렌더링

        if (remaining_time <= 0) {
            cout << "Time's up! Game over." << endl;
            isGameOver = true; // 시간이 다 되면 게임 종료
            break;
        }

        UserInput(); // 사용자 입력 처리
        UpdateGame(); // 게임 상태 업데이트

        auto end = chrono::steady_clock::now(); // 프레임 끝 시간 가져오기
        chrono::duration<double> frame_duration = end - current_time;
        this_thread::sleep_for(chrono::milliseconds(dfc) - frame_duration); // 게임 속도 제어
    }

    cout << "Final Score: " << playerScore << endl; // 최종 점수 출력
    return 0; // 프로그램 종료
}

