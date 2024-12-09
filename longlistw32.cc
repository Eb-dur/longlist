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
#include <limits>

template <typename T>
class LongList{
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using size_type = unsigned int;
    /*
    typename C::iterator 	an iterator type 	
        C::iterator is a LegacyForwardIterator, and its value type is T.
        C::iterator is convertible to C::const_iterator. 

    typename C::const_iterator 	a constant iterator type 	C::const_iterator is a LegacyForwardIterator, and its value type is T.
    typename C::difference_type 	a signed integer type 	C::difference_type is the same as the difference type of C::iterator and C::const_iterator. 
    */
    LongList() = default;
    LongList(const LongList&);
    LongList(const LongList&&);
    LongList<T>& operator=(const LongList&);
    LongList<T>& operator=(const LongList&&);
    ~LongList();
    void insert(T);
private:
    
    struct node
    {
        node* next{nullptr};
        T data{};
    };

    class iterator{
        // https://en.cppreference.com/w/cpp/named_req/ForwardIterator
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;


        iterator() = default;
        iterator(const iterator& other) = default;
        iterator& operator=(const iterator& other) = default;
        ~iterator() = default;

        bool operator==(const iterator& other) { return ptr == other->ptr;}
        bool operator!=(const iterator& other) {return !(*this == other);}
        reference operator*() { return ptr->data; }
        pointer operator->() { return &(ptr->data); }
        
        iterator& operator++() { ptr = ptr->next; return *this;}
        iterator operator++(int) {iterator old{*this}; ptr = ptr->next; return old;}


        //iterator(node* ptr) : ptr_(ptr) {} // really needed?


    private:
        node* ptr{nullptr};

    }

    class const_iterator{
        private:
            node* ptr{nullptr};
    }
    
    node* head{nullptr};
    size_type size{0};

    void PrintErrorMessage(DWORD errorCode) {
        LPVOID lpMsgBuf;
        DWORD bufLen = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL );
        if (bufLen) {
            std::cerr << "Error: " << (char*)lpMsgBuf << std::endl;
            LocalFree(lpMsgBuf);
        }
    }
};

template<typename T>
LongList<T>::LongList(const LongList& other) : size{other.size}{

}
template<typename T>
LongList<T>::LongList(const LongList&& other){

}
template<typename T>
LongList<T>& LongList<T>::operator=(const LongList& other){

}
template<typename T>
LongList<T>& LongList<T>::operator=(const LongList&& other){

}

template <typename T>
LongList<T>::~LongList(){
    node* current{head};
    while (current != nullptr)
    {
        node* next = current->next;
        VirtualFree(current, 0, MEM_RELEASE);
        current = next;
    }
}

template <typename T>
void LongList<T>::insert(T item){
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    
    uintptr_t address = reinterpret_cast<uintptr_t>(sysInfo.lpMinimumApplicationAddress);
    uintptr_t maxAddress = reinterpret_cast<uintptr_t>(sysInfo.lpMaximumApplicationAddress);

    std::vector<std::pair<uintptr_t, SIZE_T>> regions{};

    MEMORY_BASIC_INFORMATION mbi;
    
    uintptr_t best_start_address{std::numeric_limits<uintptr_t>::max()};

    SIZE_T size{0};

    while (address < maxAddress) {
        if (VirtualQuery(reinterpret_cast<void*>(address), &mbi, sizeof(mbi))){
            if (mbi.State == MEM_FREE && mbi.RegionSize >= sizeof(node) && mbi.RegionSize > size ) {
                best_start_address = address;
                size = mbi.RegionSize;
        }
        address += mbi.RegionSize;
        } else {
        address += sysInfo.dwPageSize;
        }
    }
    
    // Get a pointer at the middle of the region
    uintptr_t ideal_spot = best_start_address + (size/2);
    uintptr_t real_spot = ideal_spot - (ideal_spot % sysInfo.dwPageSize);
    void* put_here{reinterpret_cast<void*>(real_spot)};

    if (put_here){
        void* allocatedMemory = VirtualAlloc(put_here,sizeof(node), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (allocatedMemory == nullptr) {
                std::cerr << "Memory allocation failed at the specified address: " << put_here << std::endl;
                MEMORY_BASIC_INFORMATION mbi;
                PrintErrorMessage(GetLastError());

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
    LongList<int> list{};
    list.insert(1);
    list.insert(2);
    list.insert(3);
    list.insert(4);
}

#endif