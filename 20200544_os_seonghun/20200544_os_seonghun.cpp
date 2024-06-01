#include <iostream>
#include <vector>
#include <stdexcept>>



// 프로세스를 나타내는 클래스
class Process {
public:
    int id;
    bool is_background;

    Process(int id, bool is_background) : id(id), is_background(is_background) {}
};

class StackNode {
public:
    std::vector<Process> process_list;
    std::shared_ptr<StackNode> next;

    StackNode() : next(nullptr) {}
};


// 스택 클래스 정의
class Stack {
private:
    std::shared_ptr<StackNode> top; // 스택의 top을 가리키는 포인터
    std::shared_ptr<StackNode> bottom; // 스택의 bottom을 가리키는 포인터
    std::shared_ptr<StackNode> P; // 다음에 promote()할 스택 노드를 가리키는 포인터
public:
    Stack() {
        auto initial_node = std::make_shared<StackNode>();
        top = bottom = initial_node; // 초기에는 top과 bottom이 동일한 노드를 가리킴
    }

    void enqueue(const Process& process) {
        if (process.is_background) {
            // 백그라운드 프로세스인 경우 bottom 리스트 끝에 삽입
            bottom->process_list.push_back(process);
        }
        else {
            // 포어그라운드 프로세스인 경우 top 리스트 끝에 삽입
            top->process_list.push_back(process);
        }

        // 프로모션 로직 및 스플릿 앤 머지 로직은 생략
    }

    Process* dequeue() {
        if (top->process_list.empty()) {
            return nullptr; // top 리스트가 비어 있으면 nullptr 반환
        }

        // top 리스트의 첫 번째 프로세스를 반환
        Process* process = &top->process_list.front();
        top->process_list.erase(top->process_list.begin());

        // 리스트가 비어 있으면 스택 노드를 제거
        if (top->process_list.empty() && top != bottom) {
            top = top->next;
        }

        return process;
    }
    
    void promote() {
        // 다음에 promote()할 스택 노드가 리스트의 헤드 노드를 가리키도록 함
        P = P->next;

        // P가 가리키는 리스트를 상위 리스트의 꼬리에 붙임
        if (!P->process_list.empty()) {
            top->next = P;
            top = P;

            // 리스트가 비어 있으면 해당 스택 노드를 제거
            if (P->next == nullptr) {
                P = top = bottom;
            } else {
                P = P->next;
            }
        }
    }

    void print() const {
        std::shared_ptr<StackNode> current = bottom;
        while (current != nullptr) {
            for (const auto& process : current->process_list) {
                std::cout << "Process ID: " << process.id << ", Background: " << process.is_background << std::endl;
            }
            current = current->next;
        }
    }
};

int main() {
    Stack stack;

    // 초기 프로세스 생성
    Process p1(0, false); // ID가 0이고 포어그라운드 프로세스인 p1 생성
    Process p2(1, true);  // ID가 1이고 백그라운드 프로세스인 p2 생성
    Process p3(2, false); // ID가 2이고 포어그라운드 프로세스인 p3 생성

    // 프로세스 큐에 삽입
    stack.enqueue(p1);
    stack.enqueue(p2);
    stack.enqueue(p3);

    // 프로세스 상태 출력
    std::cout << "Initial queue state:" << std::endl;
    stack.print();

    // 디스패치하여 프로세스를 큐에서 제거
    Process* dispatched_process = stack.dequeue();
    if (dispatched_process != nullptr) {
        std::cout << "Dispatched Process ID: " << dispatched_process->id << std::endl;
    }

    // 프로세스 상태 출력
    std::cout << "Queue state after dequeue:" << std::endl;
    stack.print();

    return 0;
}