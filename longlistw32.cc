#ifndef CONATINER_FILE
#define CONTAINER_FILE

/*
#include <heapapi.h>
#include <memoryapi.h>
#include <errhandlingapi.h>
*/

#include <Windows.h>

#include <vector>
#include <algorithm>
#include <iostream>

template <typename T>
class LongLinkedList{
public:
    LongLinkedList() = default;
    LongLinkedList(const LongLinkedList&);
    ~LongLinkedList();
    void insert(T);
private:
    
    struct node
    {
        node* next{nullptr};
        T data{};
    };
    
    node* head{nullptr};


};

template <typename T>
LongLinkedList<T>::LongLinkedList(const LongLinkedList& other){

}

template <typename T>
LongLinkedList<T>::~LongLinkedList(){

}

template <typename T>
void LongLinkedList<T>::insert(T item){
    HANDLE defaultHeap = GetProcessHeap();
    std::vector<std::pair<void*,SIZE_T>> regions{};
    PROCESS_HEAP_ENTRY entry;
    entry.lpData = NULL;
    while (HeapWalk(defaultHeap, &entry))
    {   
        if (entry.wFlags & PROCESS_HEAP_ENTRY_BUSY)
            regions.push_back(std::make_pair(entry.lpData, entry.cbData));
    }

    if (GetLastError() != ERROR_NO_MORE_ITEMS) {
        std::cerr << "HeapWalk failed." << std::endl;
        return;
    }

    std::sort(regions.begin(), regions.end(), [] (const auto& l, const auto& r) { return l.first < r.first; } );


    void* put_here{nullptr};
    uintptr_t longest_distance{0};

    for (auto it = regions.cbegin(); it != regions.cend(); it = std::next(it)){
        auto next = std::next(it);
        std::pair<void*, SIZE_T> first = *it;

        if (next != regions.cend()){
            
            std::pair<void*, SIZE_T> second = *next;
            uintptr_t distance = reinterpret_cast<uintptr_t>(second.first) - (reinterpret_cast<uintptr_t>(first.first) + first.second);
            if ( distance > longest_distance && distance > sizeof(node)){
                longest_distance = distance;
                put_here = second.first - distance/2;
            }
            
        }
    }

    if (put_here){
        void* allocatedMemory = VirtualAlloc(put_here,sizeof(node), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (allocatedMemory == nullptr) {
                std::cerr << "Memory allocation failed at the specified address." << std::endl;
        } else {
                std::cout << "Memory allocated at address: " << allocatedMemory << std::endl;
                node* newNode = new (allocatedMemory) node{nullptr, item};
                newNode->next = head;
                head = newNode;
        }
    } else {
        std::cerr << "No suitable position found for memory allocation." << std::endl;
        node* newNode = new node{nullptr, item};
        newNode->next = head;
        head = newNode;
    }
    


}

int main(){
    LongLinkedList<int> list{};
    list.insert(1);
    list.insert(2);
    list.insert(3);
    list.insert(4);
}

#endif