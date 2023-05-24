#include <iostream>
// #include <windows.h>

int main() {
    int* ptr = nullptr;
    std::cout << "Size of a memory address: " << sizeof(ptr) * 8 << " bits" << std::endl;

    std::cout << '\a';

    char d=(char)(7);
    printf("%c\n",d);

    std::cout << '\7';

    // Beep(523,500); // 523 hertz (C5) for 500 milliseconds     
    // cin.get(); // wait
    
    return 0;
}
