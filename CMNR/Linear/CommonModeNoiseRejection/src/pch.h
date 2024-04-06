#ifndef PCH_H
#define PCH_H

// 添加要在此处预编译的标头
#include "../framework.h"
#include "CtrlCard/src/CtrlCard.h"
#include "CtrlCard/src/HY5610V.h"

#include "TekVisa/src/TekVisaApi.h"
#include "inifile.h"
#include <math.h>
#include <random>
#include "TekVisa/src/TekVisaApi.h"
#include <vector>
#include <deque>
#include <mutex>
#include <regex> //匹配字符
#include <string>
#include "afxdialogex.h"
#include "../resource.h"
#include <memory>


#include "HFST_DevToolbox/include/Utility/HFST_FileTool.h"
#include "HFST_DevToolbox/include/Utility/HFST_Utility_RedirectExeInputOutput.h"

#define AXIS_MOTION_TYPE    0 //对称加减速
#define AXIS_X              1
#define AXIS_Y              2
#define AXIS_Z              3
#define POSITION_DIRECTION  0
#define NEGATIVE_DIRECTION  1

constexpr int A = 1;				//< the distance between start pos and center pos
constexpr int B = 1;				//< 控制点的稀疏程度 
constexpr double PI = 3.1415926;	//< PI
constexpr double ACCX = 0.001;		//< 线性机台的精度X（一个脉冲移动的距离）
constexpr double ACCY = 0.001012;	//< 线性机台的精度Y（一个脉冲移动的距离）
constexpr double ACCZ = 0.001;		//< 线性机台的精度Z（一个脉冲移动的距离）

constexpr size_t    ZOOM        = 2;
constexpr size_t    SPEED_RATIO = 2000;
constexpr int       Dist_Z      = 10000;    //< 10000 pulse/s

extern long g_log, g_act, g_spd;
extern CCtrlCard	g_CtrlCard;
extern int g_cardCount;
extern int g_WorkStatus;
extern int g_ClearZero;
extern int g_cardno;
extern int g_effectlogic_x;
extern int g_effectlogic_y;
extern int g_effectlogic_z;
extern int g_effectlogic_w;

extern long g_editRio_A;
extern long g_editRio_X;
extern long g_editRio_Y;
extern long g_editRio_Z;

extern int g_softlimitmode;

void Delay(long time);

void DoEvent();

#endif //PCH_H