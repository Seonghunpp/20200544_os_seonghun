#include <iostream> // 입출력을 위한 헤더
#include <fstream> // 파일 입출력을 위한 헤더
#include <sstream> // 문자열 스트림을 위한 헤더
#include <vector> // 벡터를 사용하기 위한 헤더
#include <thread> // 스레드를 사용하기 위한 헤더
#include <chrono> // 시간을 다루기 위한 헤더
#include <mutex> // 뮤텍스를 사용하기 위한 헤더
#include <condition_variable> // 조건 변수를 사용하기 위한 헤더
#include <queue> // 큐를 사용하기 위한 헤더
#include <unordered_map> // 해시 맵을 사용하기 위한 헤더
#include <atomic> // 원자 변수를 사용하기 위한 헤더
#include <functional> // 함수 객체를 사용하기 위한 헤더
#include <locale> // 로케일을 사용하기 위한 헤더
#include <codecvt> // 문자 인코딩 변환을 위한 헤더

std::mutex mtx; // 뮤텍스 선언
std::condition_variable cv; // 조건 변수 선언
std::atomic<bool> done(false); // 원자적으로 동작하는 bool 변수 선언
std::queue<std::string> commandQueue; // 명령어 큐 선언
std::vector<std::thread> backgroundThreads; // 백그라운드 스레드 벡터 선언
std::unordered_map<std::string, std::function<void(std::vector<std::string>)>> commandMap; // 명령어와 함수를 매핑할 해시 맵 선언

int gcd(int a, int b) { // 최대공약수 계산 함수 정의
    while (b != 0) { // 유클리드 호제법으로 최대공약수 계산
        int t = b;
        b = a % t;
        a = t;
    }
    return a; // 최대공약수 반환
}

int prime_count(int n) { // 소수 개수 계산 함수 정의
    std::vector<bool> is_prime(n + 1, true); // 소수 여부를 저장할 벡터 선언
    int count = 0; // 소수 개수 초기화
    for (int i = 2; i <= n; ++i) { // 에라토스테네스의 체를 이용하여 소수 개수 계산
        if (is_prime[i]) {
            ++count;
            for (int j = i * 2; j <= n; j += i) {
                is_prime[j] = false;
            }
        }
    }
    return count; // 소수 개수 반환
}

int sum_mod(int n) { // 1부터 n까지의 합을 1000000으로 나눈 나머지 계산 함수 정의
    long long sum = 0; // 합을 저장할 변수 초기화
    for (int i = 1; i <= n; ++i) {
        sum = (sum + i) % 1000000; // 각 수를 더하고 나머지를 구하여 더함
    }
    return static_cast<int>(sum); // 계산된 나머지 반환
}

void echo(const std::vector<std::string>& args) { // 입력된 인자들을 출력하는 함수 정의
    for (const auto& arg : args) { // 인자들을 모두 출력
        std::cout << arg << " ";
    }
    std::cout << std::endl; // 개행 출력
}

void dummy(const std::vector<std::string>& args) { // 아무 동작도 하지 않는 함수 정의
    // 아무 동작도 하지 않음
}

void gcd_command(const std::vector<std::string>& args) { // 최대공약수 계산 명령어 함수 정의
    if (args.size() < 2) return; // 인자가 부족하면 함수 종료
    int a = std::stoi(args[0]); // 첫 번째 인자를 정수로 변환
    int b = std::stoi(args[1]); // 두 번째 인자를 정수로 변환
    std::cout << "GCD: " << gcd(a, b) << std::endl; // 최대공약수 출력
}

void prime(const std::vector<std::string>& args) { // 소수 개수 계산 명령어 함수 정의
    if (args.empty()) return; // 인자가 없으면 함수 종료
    int x = std::stoi(args[0]); // 인자를 정수로 변환
    std::cout << "Prime count: " << prime_count(x) << std::endl; // 소수 개수 출력
}

void sum(const std::vector<std::string>& args) { // 합을 계산하고 1000000으로 나눈 나머지 출력하는 함수 정의
    if (args.empty()) return; // 인자가 없으면 함수 종료
    int x = std::stoi(args[0]); // 인자를 정수로 변환
    std::cout << "Sum mod 1000000: " << sum_mod(x) << std::endl; // 합을 계산하고 나머지 출력
}

void executeCommand(const std::string& command, bool isBackground) { // 명령어를 실행하는 함수 정의
    std::istringstream iss(command); // 문자열 스트림 생성
    std::vector<std::string> args; // 인자를 저장할 벡터 생성
    std::string arg; // 인자를 임시로 저장할 문자열
    while (iss >> arg) { // 공백으로 구분된 인자들을 읽어 벡터에 저장
        args.push_back(arg);
    }

    if (args.empty()) return; // 인자가 없으면 함수 종료

    std::string cmd = args[0]; // 명령어 추출
    args.erase(args.begin()); // 명령어를 제외한 나머지 인자들 삭제

    int n = 1; // 실행 횟수 초기화

    int duration = 0; // 실행 시간 초기화
    int period = 0; // 반복 주기 초기화

    auto it = args.begin(); // 인자를 반복자로 순회
    while (it != args.end()) { // 모든 인자에 대해 처리
        if (*it == "-n" && (it + 1) != args.end()) { // '-n' 옵션 처리
            n = std::stoi(*(it + 1)); // 실행 횟수 설정
            it = args.erase(it, it + 2); // '-n' 옵션 삭제
        }
        else if (*it == "-d" && (it + 1) != args.end()) { // '-d' 옵션 처리
            duration = std::stoi(*(it + 1)); // 실행 시간 설정
            it = args.erase(it, it + 2); // '-d' 옵션 삭제
        }
        else if (*it == "-p" && (it + 1) != args.end()) { // '-p' 옵션 처리
            period = std::stoi(*(it + 1)); // 반복 주기 설정
            it = args.erase(it, it + 2); // '-p' 옵션 삭제
        }
        else { // 다른 옵션이면 다음 인자로 이동
            ++it;
        }
    }

    auto executeFunc = [cmd, args]() { // 실행할 함수 객체 정의
        if (commandMap.find(cmd) != commandMap.end()) { // 명령어가 맵에 존재하는지 확인
            commandMap[cmd](args); // 명령어에 해당하는 함수 호출
        }
        else { // 알 수 없는 명령어일 경우
            std::cout << "Unknown command: " << cmd << std::endl; // 알림 출력
        }
    };

    std::vector<std::thread> threads; // 스레드를 저장할 벡터 생성
    for (int i = 0; i < n; ++i) { // 실행 횟수만큼 반복
        if (period > 0) { // 반복 주기가 있는 경우
            threads.push_back(std::thread([executeFunc, period, duration]() { // 주기적 실행을 위한 람다 함수
                auto start = std::chrono::steady_clock::now(); // 시작 시간 기록
            while (duration == 0 || std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count() < duration) { // 실행 시간이 duration 이내인 동안 반복
                executeFunc(); // 함수 실행
                std::this_thread::sleep_for(std::chrono::seconds(period)); // 주기만큼 대기
            }
                }));
        }
        else if (duration > 0) { // 실행 시간이 있는 경우
            threads.push_back(std::thread([executeFunc, duration]() { // 시간 제한을 위한 람다 함수
                auto start = std::chrono::steady_clock::now(); // 시작 시간 기록
            while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count() < duration) { // 실행 시간이 duration 이내인 동안 반복
                executeFunc(); // 함수 실행
            }
                }));
        }
        else { // 실행 횟수만큼 단순 반복하는 경우
            threads.push_back(std::thread(executeFunc)); // 함수 실행 스레드 생성
        }
    }

    for (auto& thread : threads) { // 생성한 스레드들에 대해
        if (thread.joinable()) { // 스레드가 조인 가능한지 확인
            thread.join(); // 스레드 조인
        }
    }
}

void shell() { // 셸 스레드의 역할을 수행하는 함수 정의
    std::wifstream file("commands.txt", std::ios::in | std::ios::binary); // 유니코드 파일을 읽기 위한 wifstream 객체 생성
    file.imbue(std::locale(file.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>)); // 파일 로케일 설정
    std::wstring line; // 파일의 한 줄을 저장할 변수 선언
    while (std::getline(file, line)) { // 파일에서 한 줄씩 읽어옴
        std::this_thread::sleep_for(std::chrono::seconds(5)); // 5초 대기
        { // 뮤텍스를 사용하여 스레드 간 동기화
            std::lock_guard<std::mutex> lock(mtx); // 뮤텍스 락을 사용하여 임계 영역 설정
            std::wcout << L"prompt> " << line << std::endl; // 프롬프트 출력
            std::cout << std::endl; // 개행
            commandQueue.push(std::string(line.begin(), line.end())); // 명령어 큐에 한 줄 추가
            commandQueue.push(""); // 한 줄 띄우기 위해 빈 문자열 추가
            cv.notify_one(); // 대기 중인 스레드에 알림
        }
    }
    done = true; // 종료 플래그 설정
    cv.notify_all(); // 모든 스레드에 알림
}

void scheduler() { // 스케줄러 스레드의 역할을 수행하는 함수 정의
    while (!done || !commandQueue.empty()) { // 종료 플래그가 설정되거나 명령어 큐가 비어있지 않은 동안 반복
        std::unique_lock<std::mutex> lock(mtx); // 뮤텍스 락을 사용하여 임계 영역 설정
        cv.wait(lock, [] { return !commandQueue.empty() || done; }); // 명령어 큐가 비어있지 않거나 종료 플래그가 설정될 때까지 대기

        if (!commandQueue.empty()) { // 명령어 큐가 비어있지 않은 경우
            std::string command = commandQueue.front(); // 명령어 큐에서 명령어를 가져옴
            commandQueue.pop(); // 명령어 큐에서 명령어 제거
            lock.unlock(); // 뮤텍스 락 해제

            std::vector < std::string > tokens;
            size_t start = 0; // 토큰의 시작 위치 초기화
            size_t end = command.find_first_of(";&"); // 명령어에서 ';' 또는 '&'의 위치 찾기

            while (end != std::string::npos) { // ';' 또는 '&'가 존재하는 동안 반복
                tokens.push_back(command.substr(start, end - start)); // 토큰 추가
                tokens.push_back(command.substr(end, 1)); // 구분자 추가
                start = end + 1; // 시작 위치 업데이트
                end = command.find_first_of(";&", start); // 다음 구분자의 위치 찾기
            }
            tokens.push_back(command.substr(start)); // 나머지 부분 추가

            std::vector<std::thread> fgThreads; // 포그라운드 스레드 벡터 생성
            for (size_t i = 0; i < tokens.size(); ++i) { // 토큰들에 대해 반복
                if (tokens[i] == ";") { // 세미콜론인 경우
                    continue; // 다음 토큰으로 이동
                }
                else if (tokens[i] == "&") { // 앰퍼샌드인 경우
                    if (i + 1 < tokens.size()) { // 다음 토큰이 존재하는 경우
                        backgroundThreads.push_back(std::thread(executeCommand, tokens[++i], true)); // 백그라운드 스레드 생성
                    }
                }
                else { // 일반 명령어인 경우
                    fgThreads.push_back(std::thread(executeCommand, tokens[i], false)); // 포그라운드 스레드 생성
                }

                if (i + 1 < tokens.size() && tokens[i + 1] == ";") { // 다음 토큰이 세미콜론인 경우
                    for (auto& t : fgThreads) { // 포그라운드 스레드들에 대해 반복
                        if (t.joinable()) { // 스레드가 조인 가능한 경우
                            t.join(); // 스레드 조인
                        }
                    }
                    fgThreads.clear(); // 포그라운드 스레드 벡터 초기화
                }
            }

            for (auto& t : fgThreads) { // 남아있는 포그라운드 스레드에 대해
                if (t.joinable()) { // 스레드가 조인 가능한 경우
                    t.join(); // 스레드 조인
                }
            }
        }
    }
}

int main() { // 프로그램의 시작점
    commandMap["echo"] = echo; // 'echo' 명령어를 해당 함수와 연결
    commandMap["dummy"] = dummy; // 'dummy' 명령어를 해당 함수와 연결
    commandMap["gcd"] = gcd_command; // 'gcd' 명령어를 해당 함수와 연결
    commandMap["prime"] = prime; // 'prime' 명령어를 해당 함수와 연결
    commandMap["sum"] = sum; // 'sum' 명령어를 해당 함수와 연결
    std::thread shellThread(shell); // 셸 스레드 생성
    std::thread schedulerThread(scheduler); // 스케줄러 스레드 생성

    shellThread.join(); // 셸 스레드 조인
    schedulerThread.join(); // 스케줄러 스레드 조인
    done = true; // 종료 플래그 설정

    for (auto& bgThread : backgroundThreads) { // 백그라운드 스레드들에 대해
        if (bgThread.joinable()) { // 스레드가 조인 가능한 경우
            bgThread.join(); // 스레드 조인
        }
    }

    return 0; // 프로그램 종료
}