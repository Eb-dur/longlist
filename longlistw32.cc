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
#include <algorithm>

template <typename T>
class LongList{
public: 
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using size_type = unsigned int;
    using difference_type = std::ptrdiff_t;

private:
    struct node
    {
        node* next{nullptr};
        T data{};
    };

    
    node* head{nullptr};
    size_type size_{0};

    T remove_handler(node* remove_next);

public:
  
    LongList() = default;
    LongList(const LongList&);
    LongList(const LongList&&);
    LongList& operator=(const LongList&);
    LongList& operator=(const LongList&&);
    ~LongList();
    void insert(T);
    T remove(size_type index);
    bool operator==(const LongList& other) const { return std::equal(this->cbegin(), this->cend(), other->cbegin(), other->cend()); }
    bool operator!=(const LongList& other) const { return !(*this==other); }
    size_type size() const { return this->size_; }
    size_type max_size() const { return 10000; } 
    void swap(LongList& rhs){ std::swap(this->head, rhs->head); std::swap(this->size_, rhs->size_); }

    template <typename ITERATOR_T>
    class base_iterator{
        // https://en.cppreference.com/w/cpp/named_req/ForwardIterator
    public:
        using pointer = ITERATOR_T*;
        using reference = ITERATOR_T&;
        using value_type = ITERATOR_T;

        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;


        base_iterator() = default;
        base_iterator(const base_iterator& other) = default;
        base_iterator& operator=(const base_iterator& other) = default;
        ~base_iterator() = default;


        bool operator==(const base_iterator& other) const { return ptr == other.ptr; }
        bool operator!=(const base_iterator& other) const { return !(*this == other); }
        reference operator*() const { return ptr->data; }
        pointer operator->() const { return &(ptr->data); }
        
        base_iterator& operator++() { ptr = ptr->next; return *this;}
        base_iterator operator++(int) { iterator old{*this}; ptr = ptr->next; return old;}
    
    private:
        friend class LongList;
        base_iterator(node* ptr) : ptr{ptr} {}
        node* ptr{nullptr};
    };

    using iterator = base_iterator<value_type>;
    using const_iterator = base_iterator<const value_type>;

    iterator begin(){ iterator it{head}; return it; }
    iterator end(){ iterator it{nullptr}; return it; }
    const_iterator cbegin(){ const_iterator it{head}; return it; }
    const_iterator cend(){ const_iterator it{nullptr}; return it; }
    
    T remove(iterator& pos) { }

private:
    

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
T LongList<T>::remove_handler(node* remove_next){
    node* to_remove = remove_next->next;
    remove_next->next = to_remove->next;
    T data = to_remove->data;
    VirtualFree(to_remove, 0, MEM_RELEASE);
    return data;
}

// TODO: IMPLEMENT DELETE FUNCTIONS



template<typename T>
void swap(const LongList<T>& lhs, const LongList<T>& rhs){ lhs.swap(rhs); }

template<typename T>
LongList<T>::LongList(const LongList& other) : size_{other.size_}{

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