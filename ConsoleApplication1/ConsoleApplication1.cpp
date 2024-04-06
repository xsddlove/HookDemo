#include <iostream>

void toggleFeatures1() {
    char* send_page = new char[0x100];
    send_page[0] = 0x66;
    send_page[1] = 0x6F;
    send_page[2] = 0x01;
    send_page[3] = 0x00;


    //char* p1 = &send_page[4];
    //short* p2 = (short*)(p1);
    //*p2 = 1;


    std::cout << "填写发包数据: ";
    char* memory = new char[0x100];
    std::cin >> memory;
    
    

    size_t inputLength = std::strlen(memory);
    //std::cout << "输入了 " << inputLength << " 个字节。\n";

    *(short*)(&send_page[4]) = inputLength;


    std::cout << memory << std::endl;

    memmove((send_page + 6), memory, 0x94);

    delete[] memory;
}



void toggleFeatures() {
    std::cout << "填写发包数据: ";


}

int main1() {
    toggleFeatures1();
    std::cout << "Hello World!\n";
    return 0;
}

