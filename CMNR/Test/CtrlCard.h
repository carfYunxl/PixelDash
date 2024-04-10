// CtrlCard.h: interface for the CCtrlCard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CTRLCARD_H__792BC8C3_F372_497B_B0A1_6F065F0C19DD__INCLUDED_)
#define AFX_CTRLCARD_H__792BC8C3_F372_497B_B0A1_6F065F0C19DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*********************** 运动控制模块 ********************

    为了简单、方便、快捷地开发出通用性好、可扩展性强、
    
    维护方便的应用系统，我们在控制卡函数库的基础上将
    
    所有库函数进行了分类封装。下面的示例使用一块运动
    
    控制卡

********************************************************/
#define  MULTIPLE  5//倍率
#define  MAXAXIS  4    //最大轴数

class CCtrlCard  
{
public:
	
	int Setup_Stop0Mode(int axis, int value, int logic);
	int Setup_Stop1Mode(int axis, int value, int logic);
	int Setup_Stop2Mode(int axis, int value, int logic);	
	
	int Actualcount_Mode(int axis,int value, int dir,int freq);
   	int AllowDec();
    int Axis_Pmove(int axis ,long value);
	int Axis_Cmove(int axis ,long value);
	
	
    int Setsoft_LimitMode1(int axis, int value);
    int Setsoft_LimitMode2(int axis, int value);
    int Setsoft_LimitMode3(int axis, int value);
	
	int Setup_LimitMode(int axis, int value, int logic);
	int Setup_PulseMode(int axis, int value);
	
	int Setup_Comp1(int axis, long value);
    int Setup_Comp2(int axis, long value);
	
	int Setup_Pos(int axis, long pos, int mode);
    int SetCircle_Mode(int axis, int value);
	
	int Write_Output(int number, int value);
	int Read_Input(int number);
	
	int Get_CurrentInf(int axis, long &LogPos, long &ActPos, long &Speed);
	
	int Get_Status(int axis, int &value, int mode);
	int Get_AllowInpStatus( int &value);
	
	int Set_DecPos(int axis, long value, long startv, long speed, long add);
    int Set_DecMode(int axis, int mode1, int mode2);
	
	int Get_ErrorInf(int axis, int &value);
	
	int StopRun(int axis, int mode);
	
	int Interp_Move2(int axis1,int axis2, long value1, long value2);
    int Interp_Move3(int axis1,int axis2,int axis3,long value1, long value2, long value3);
	
    int Setup_Range(int axis, long value);	
	
	int Interp_Arc(int axis1,int axis2, long x, long y, long i,long j);	
	int Interp_CcwArc(int axis1,int axis2, long x, long y, long i,long j);
	
	
	int End_Board();
    int ForbidDec();
	int Init_Board();	
    int Setup_Speed(int axis ,long startv ,long speed ,long  add ,long dec,long ratio,int mode);
	
	int Inpos_Mode(int axis, int value, int logic);
    int Setup_AlarmMode(int axis,int value,int logic);
	
	int Setup_InputFilter(int axis,int number,int value);
    int Setup_FilterTime(int axis,int value);
	
	int Setup_LockPosition(int axis,int regi,int logical);
	int Get_LockStatus(int axis,int &status);
	int Get_HomeStatus(int axis,int &status,int&err);
	int Get_HomeError(int axis,int&err);
	int Setup_VectorSpeed(int mode);
	int Setup_HomeMode(int axis,long speed,int logical0, int logical1, int logical2,int offset,int dir0, int dir1, int dir2,int offsetdir,int clear,long pulse);
	int Get_LockPosition(int axis,long &pos);
	int Get_LibVision(int &ver);
	int Manu_Pmove(int axis, long pulse);
	int Manu_Continue(int axis);
	int Manu_Disable(int axis);
	int Inp_Command2(int axis1,int axis2,long pulse1,long pulse2);
	int Inp_Command3(int axis1,int axis2,int axis3,long pulse1,long pulse2, long pulse3);
	int Inp_StepMove();
	int Inp_Signal2(int axis1,int axis2,long pulse1,long pulse2);
	int Inp_Signal3(int axis1,int axis2,int axis3,long pulse1,long pulse2, long pulse3);
	int Inp_Stop(int axis);
	int Move_Home(int axis);
	int Clear_HomeError(int axis);
	int Set_InMove1(int axis,int axis1,long pulse,long pulse1,int logical,int mode);
	int Set_InMove2(int axis, int axis1,int axis2 ,long pulse,long pulse1,long pulse2,int logical,int mode);
	int Set_InMove3(int axis,int axis1,int axis2,int axis3 ,long pulse,long pulse1,long pulse2, long pulse3,int logical,int mode);
	int Set_InStop1(int axis, int axis1 ,int logical, int mode);
	int Set_InStop2(int axis, int axis1,int axis2,int logical, int mode);
	int Set_InStop3(int axis,int logical, int mode);
	int Setup_Pmove1(int axis, int axis1, long pulse, long pulse1, int regi, int term);
	int Setup_Pmove2(int axis, int axis1,int axis2, long pulse, long pulse1,long pulse2, int regi, int term);
	int Setup_Pmove3(int axis, int axis1,int axis2, int axis3, long pulse, long pulse1,long pulse2,long pulse3, int regi, int term);
	int Setup_Stop1(int axis,int axis1,long pulse,int regi,int term,int mode);
	int Setup_Stop2(int axis,int axis1,int axis2,long pulse,int regi,int term,int mode);
	int Setup_Stop3(int axis,long pulse,int regi,int term,int mode);
	int Symmetry_RelativeMove(int axis, long pulse, long lspd ,long hspd, double tacc, long vacc, int mode);
	int Symmetry_AbsoluteMove(int axis, long pulse, long lspd ,long hspd, double tacc, long vacc, int mode);
	int Unsymmetry_RelativeMove( int axis, long pulse, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode);
	int Unsymmetry_AbsoluteMove(int axis, long pulse, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode);
	int Symmetry_RelativeLine2(int axis1, int axis2, long pulse1, long pulse2, long lspd ,long hspd, double tacc, long vacc, int mode);
	int Symmetry_AbsoluteLine2(int axis1, int axis2, long pulse1, long pulse2, long lspd ,long hspd, double tacc, long vacc, int mode);
	int Unsymmetry_RelativeLine2(int axis1, int axis2, long pulse1, long pulse2, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode);
	int Unsymmetry_AbsoluteLine2(int axis1, int axis2, long pulse1, long pulse2, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode);
	int Symmetry_RelativeLine3(int axis1, int axis2, int axis3, long pulse1, long pulse2, long pulse3, long lspd ,long hspd, double tacc, long vacc, int mode);
	int Symmetry_AbsoluteLine3(int axis1, int axis2, int axis3, long pulse1, long pulse2, long pulse3, long lspd ,long hspd, double tacc, long vacc, int mode);
	int Unsymmetry_RelativeLine3(int axis1, int axis2, int axis3, long pulse1, long pulse2, long pulse3, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode);
	int Unsymmetry_AbsoluteLine3(int axis1, int axis2, int axis3, long pulse1, long pulse2, long pulse3, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode);
	int Symmetry_Relativearc(int axis1, int axis2, long x, long y, long i, long j, int dir, long lspd ,long hspd, double tacc, long vacc, int mode);
	int Symmetry_AbsoluteArc(int axis1, int axis2, long x, long y, long i, long j, int dir, long lspd ,long hspd, double tacc, long vacc, int mode);
	int Unsymmetry_RelativeArc(int axis1, int axis2, long x, long y, long i, long j, int dir, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode);
	int Unsymmetry_AbsoluteArc(int axis1, int axis2, long x, long y, long i, long j, int dir, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode);
		
	void  OnParamSet();
    int OnButtonPmove(int axis, long pulse); //指定目标位置pulse(长度)移动。
    int OnButtonCmove(int axis, int direction); //连续运动。
	void OnButtonInp(int axis1, int axis2, long pulse1, long pulse2); //两轴直线插补运动
	int BackHome(int axis, CStatic& staticProMsg);
	CCtrlCard();
	int Result;  //返回值
	
public:
	int GetSpeed( int cardno, int axis );
};

#endif // !defined(AFX_CTRLCARD_H__792BC8C3_F372_497B_B0A1_6F065F0C19DD__INCLUDED_)
