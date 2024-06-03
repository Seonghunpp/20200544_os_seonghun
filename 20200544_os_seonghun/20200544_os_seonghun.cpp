#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include <atomic>
#include <functional>
#include <locale>
#include <codecvt>

// 전역 변수 및 동기화 객체
std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> done(false);
std::queue<std::string> commandQueue;
std::vector<std::thread> backgroundThreads;
std::unordered_map<std::string, std::function<void(std::vector<std::string>)>> commandMap;

// 유틸리티 함수
int gcd(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % t;
        a = t;
    }
    return a;
}

int prime_count(int n) {
    std::vector<bool> is_prime(n + 1, true);
    int count = 0;
    for (int i = 2; i <= n; ++i) {
        if (is_prime[i]) {
            ++count;
            for (int j = i * 2; j <= n; j += i) {
                is_prime[j] = false;
            }
        }
    }
    return count;
}

int sum_mod(int n) {
    long long sum = 0;
    for (int i = 1; i <= n; ++i) {
        sum = (sum + i) % 1000000;
    }
    return static_cast<int>(sum);
}

// 프로세스 함수
void echo(const std::vector<std::string>& args) {
    for (const auto& arg : args) {
        std::cout << arg << " ";
    }
    std::cout << std::endl;
}

void dummy(const std::vector<std::string>& args) {
    // Do nothing
}

void gcd_command(const std::vector<std::string>& args) {
    if (args.size() < 2) return;
    int a = std::stoi(args[0]);
    int b = std::stoi(args[1]);
    std::cout << "GCD: " << gcd(a, b) << std::endl;
}

void prime(const std::vector<std::string>& args) {
    if (args.empty()) return;
    int x = std::stoi(args[0]);
    std::cout << "Prime count: " << prime_count(x) << std::endl;
}

void sum(const std::vector<std::string>& args) {
    if (args.empty()) return;
    int x = std::stoi(args[0]);
    std::cout << "Sum mod 1000000: " << sum_mod(x) << std::endl;
}

void executeCommand(const std::string& command, bool isBackground);

void periodicTask(const std::string& command, int period, int duration) {
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count() < duration) {
        executeCommand(command, true);
        std::this_thread::sleep_for(std::chrono::seconds(period));
    }
}
void executeCommand(const std::string& command, bool isBackground) {
    std::istringstream iss(command);
    std::vector<std::string> args;
    std::string arg;
    while (iss >> arg) {
        args.push_back(arg);
    }

    if (args.empty()) return;

    std::string cmd = args[0];
    args.erase(args.begin());

    // 옵션 파싱
    int n = 1; // 기본 실행 횟수
    int duration = 300; // 기본 실행 시간 (5분)
    int period = 0; // 반복 주기

    auto it = args.begin();
    while (it != args.end()) {
        if (*it == "-n" && (it + 1) != args.end()) {
            n = std::stoi(*(it + 1));
            it = args.erase(it, it + 2);
        }
        else if (*it == "-d" && (it + 1) != args.end()) {
            duration = std::stoi(*(it + 1));
            it = args.erase(it, it + 2);
        }
        else if (*it == "-p" && (it + 1) != args.end()) {
            period = std::stoi(*(it + 1));
            it = args.erase(it, it + 2);
        }
        else {
            ++it;
        }
    }

    // 명령어를 실행할 함수와 인자들을 설정
    auto executeFunc = [cmd, args, duration, period, n]() {
        if (commandMap.find(cmd) != commandMap.end()) {
            for (int i = 0; i < n; ++i) {
                if (period > 0) {
                    periodicTask(cmd, period, duration);
                }
                else {
                    commandMap[cmd](args);
                }
            }
        }
        else {
            std::cout << "Unknown command: " << cmd << std::endl;
        }
    };

    // 명령어를 실행할 스레드들 생성
    std::vector<std::thread> threads;
    for (int i = 0; i < n; ++i) {
        threads.push_back(std::thread(executeFunc));
    }

    // 생성된 스레드들 조인
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}



void shell() {
    std::wifstream file("commands.txt", std::ios::in | std::ios::binary);
    file.imbue(std::locale(file.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));
    std::wstring line;
    while (std::getline(file, line)) {
        std::this_thread::sleep_for(std::chrono::seconds(5)); // 5초마다 1줄씩 실행
        {
            std::lock_guard<std::mutex> lock(mtx);
            std::wcout << L"prompt> " << line << std::endl;
            std::cout << std::endl;
            commandQueue.push(std::string(line.begin(), line.end()));
            commandQueue.push(""); // 한 줄씩 띄우기 위해 빈 문자열 추가
        }
        cv.notify_one();
    }
}

int main() {
    // 명령어 맵 설정
    commandMap["echo"] = echo;
    commandMap["dummy"] = dummy;
    commandMap["gcd"] = gcd_command;
    commandMap["prime"] = prime;
    commandMap["sum"] = sum;

    // 셸 시작
    std::thread shellThread(shell);

    // 셸 쓰레드 조인
    if (shellThread.joinable()) {
        shellThread.join();
    }

    return 0;
}
