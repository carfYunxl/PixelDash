// pch.cpp: 与预编译标头对应的源文件

#include "pch.h"

// 当使用预编译的头时，需要使用此源文件，编译才能成功。
CCtrlCard	g_CtrlCard;

int g_cardCount = 0;

int g_cardno = 0;

long g_log, g_act, g_spd;//逻辑位置，实际位置，运行速度


int g_WorkStatus = 0;//工作状态
int g_ClearZero = 0;

int g_effectlogic_x = 0;
int g_effectlogic_y = 0;
int g_effectlogic_z = 0;
int g_effectlogic_w = 0;

int g_softlimitmode = 0;

long    g_editRio_A;
long	g_editRio_X;
long	g_editRio_Y;
long	g_editRio_Z;

void	DoEvent()
{
	static MSG msg;
	if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}

void Delay(long time)
{
	long tm = GetTickCount();
	while (true)
	{
		if ((long)GetTickCount() - tm >= time)
		{
			break;
		}
		::DoEvent();
	}
}