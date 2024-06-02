#include <iostream>

using namespace std;

// 프로세스를 나타내는 구조체
struct Process {
    int id;
    Process* next;

    Process(int _id) : id(_id), next(nullptr) {}
};

// 스택 노드를 나타내는 클래스
class StackNode {
public:
    Process* processList; // 프로세스 리스트를 가리키는 포인터
    StackNode* next;      // 다음 스택 노드를 가리키는 포인터

    // 생성자: 초기화
    StackNode(Process* process) : processList(process), next(nullptr) {}
};

// 큐를 나타내는 클래스
class DynamicQueue {
public:
    StackNode* top;    // 스택의 맨 위(top) 노드를 가리키는 포인터
    StackNode* bottom; // 스택의 맨 아래(bottom) 노드를 가리키는 포인터

    // 생성자: 초기화
    DynamicQueue() : top(nullptr), bottom(nullptr) {
        // 초기 상태일 때 top과 bottom이 같은 노드를 가리키도록 설정
        StackNode* initialNode = new StackNode(nullptr);
        top = initialNode;
        bottom = initialNode;
    }

    // 백그라운드 프로세스를 큐에 추가하는 함수
    void enqueueBackground(Process* process) {
        if (bottom->processList == nullptr) {
            bottom->processList = process;
        }
        else {
            Process* currentProcess = bottom->processList;
            while (currentProcess->next != nullptr) {
                currentProcess = currentProcess->next;
            }
            currentProcess->next = process;
        }
    }

    // 포어그라운드 프로세스를 큐에 추가하는 함수
    void enqueueForeground(Process* process) {
        if (top->processList == nullptr) {
            top->processList = process;
        }
        else {
            Process* currentProcess = top->processList;
            while (currentProcess->next != nullptr) {
                currentProcess = currentProcess->next;
            }
            currentProcess->next = process;
        }
    }
};

int main() {
    // 초기화된 큐 생성
    DynamicQueue queue;

    // 백그라운드 프로세스 추가
    Process* backgroundProcess1 = new Process();
    backgroundProcess1->id = 1;
    queue.enqueueBackground(backgroundProcess1);

    Process* backgroundProcess2 = new Process();
    backgroundProcess2->id = 2;
    queue.enqueueBackground(backgroundProcess2);

    // 포어그라운드 프로세스 추가
    Process* foregroundProcess1 = new Process();
    foregroundProcess1->id = 3;
    queue.enqueueForeground(foregroundProcess1);

    Process* foregroundProcess2 = new Process();
    foregroundProcess2->id = 4;
    queue.enqueueForeground(foregroundProcess2);

    // 메모리 해제
    delete backgroundProcess1;
    delete backgroundProcess2;
    delete foregroundProcess1;
    delete foregroundProcess2;

    return 0;
}
