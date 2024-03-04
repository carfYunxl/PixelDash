// HFST_SDK_Test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>

extern int HFST_ADB_SDK_Test();
int main()
{
    std::cout << "ADB SDK Test..." << std::endl;
    HFST_ADB_SDK_Test();
    std::cout << "ADB SDK Test Complete!" << std::endl;

    return 1;
}

