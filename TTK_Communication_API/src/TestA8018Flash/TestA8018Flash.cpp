// TestA8018Flash.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>

#pragma comment(lib, "../../dependency/BulkAPI/lib/BulkDll.lib")

extern int A8015_Test();
extern bool A8018_Test();

int main()
{
	A8015_Test();
//	A8018_Test();

	system( "pause" );
	return 1;
}
