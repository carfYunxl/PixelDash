// CtrlCard.cpp: implementation of the CCtrlCard class.
//
////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "CtrlCard.h"
#include "HY5610V.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

////////////////////////////////////////////////////////////////////
// Construction/Destruction
////////////////////////////////////////////////////////////////////

int cardno=0;

CCtrlCard::CCtrlCard()
	:Result(-1)
{

}
/***********************初始化函数**************************
    该函数中包含了控制卡初始化常用的库函数，这是调用    
    其他函数的基础，所以必须在示例程序中最先调用
    返回值<=0表示初始化失败，返回值>0表示初始化成功
***********************************************************/
int CCtrlCard::Init_Board()
{
	Result = HY5610V_initial() ;         //卡初始化函数    
	
	if (Result <= 0) return Result;	
    
    for (int i = 1; i<=MAXAXIS; i++)
	{
		for(int j=0;j<Result;j++)
		{
		
			set_range (j, i, 8000000 / 5);    //设定范围,设置初始倍率为5
			
		//		set_limit_mode (0, i, 0,  0); //设定限位模式，设正负限位有效，低电平有效
			
			set_command_pos (j, i, 0);        //清逻辑计数器
			
			set_actual_pos (j, i, 0);         //清实位计数器
			
			set_startv (j, i, 100);           //设定起始速度
			
			set_speed (j, i, 100);            //设定驱动速度

			set_acc (j, i, 0);              //设定加速度

			set_actualcount_mode(j,i,0,0,0);  //设置实际位置计数器的工作模式

		//		set_pulse_mode(0,i,1,1,0);        //设置输出脉冲的工作方式
		}
    }    
	inp_dec_enable(cardno);

    return Result;	
}


/**********************释放控制卡函数***********************

    该函数中包含了释放控制卡的库函数，这是此函数应在

	程序结束的时候调用   
***********************************************************/

int CCtrlCard::End_Board ()
{
	Result = HY5610V_end();
	
	return Result;
}


/********************设置stop0信号方式**********************

   该函数用于设定stop0信号的模式

   参数： axis－轴号
          value   0－无效  1－有效
		  logic   0－低电平停止  1－高电平停止
   默认模式为：无效

   返回值=0正确，返回值=1错误
  *********************************************************/
int CCtrlCard::Setup_Stop0Mode(int axis, int value, int logic)
{
	Result = set_stop0_mode(cardno, axis, value ,logic);

	return Result;
}


/********************设置stop1信号方式**********************

   该函数用于设定stop1信号的模式

   参数： axis－轴号
          value   0－无效  1－有效
		  logic   0－低电平停止  1－高电平停止
   默认模式为：无效

   返回值=0正确，返回值=1错误
  *********************************************************/
int CCtrlCard::Setup_Stop1Mode(int axis, int value, int logic)
{
	Result = set_stop1_mode(cardno, axis, value, logic);

	return Result;

}

/********************设置stop2信号方式**********************

   该函数用于设定stop2信号的模式

   参数： axis－轴号
          value   0－无效  1－有效
		  logic   0－低电平停止  1－高电平停止
   默认模式为：无效

   返回值=0正确，返回值=1错误
***********************************************************/
int CCtrlCard::Setup_Stop2Mode(int axis, int value, int logic)
{
	Result = set_stop2_mode(cardno, axis, value, logic);

	return Result;

}

/*********************设置实际位置计数器********************
cardno	   卡号
axis	   轴号（1-4）
value	   输入脉冲方式
0：        A/B脉冲输入		1：上/下（PPIN/PMIN）脉冲输入
dir		  计数方向
0：	      A超前B或PPIN脉冲输入向上计数
		  B超前A或PMIN脉冲输入向下计数
1：       B超前A或PMIN脉冲输入向上计数
		  A超前B或PPIN脉冲输入向下计数
freq	  A/B脉冲输入时的倍频，上/下脉冲输入时无效
0：4倍频     1：2倍频        2：不倍频
返回值	     0：正确	   	 1：错误
初始化时状态为：A/B相脉冲输入，方向为0，4倍频
***********************************************************/
int CCtrlCard::Actualcount_Mode(int axis,int value, int dir,int freq)
{
	Result = set_actualcount_mode(cardno, axis, value,dir,freq);
	
	return Result;
}

/********************设置脉冲输出方式***********************
      
	该函数用于设置脉冲的工作方式

	参数：axis-轴号， value-脉冲方式 0－脉冲＋脉冲方式 1－脉冲＋方向方式

    返回值=0正确，返回值=1错误

    默认脉冲方式为脉冲＋方向方式

    本程序采用默认的正逻辑脉冲和方向输出信号正逻辑

***********************************************************/
int CCtrlCard::Setup_PulseMode(int axis, int value)
{
	Result = set_pulse_mode(cardno, axis, value, 0, 0);
	
	return Result;	

}

/*********************设置限位信号方式**********************

   该函数用于设定正/负方向限位输入nLMT信号的模式

   参数： axis－轴号

          value   0－有效时立即停止  1－有效时减速停止
		  
		  logic    0－低电平有效  1－高电平有效

   默认模式为：正限位有效、负限位有效、低电平有效

   返回值=0正确，返回值=1错误
***********************************************************/
int CCtrlCard::Setup_LimitMode(int axis, int value, int logic)
{
	Result = set_limit_mode(cardno, axis, value,  logic);

	return Result;

}

/******************设置COMP+寄存器做软件限制****************
cardno	   卡号

axis		轴号（1-4）

value		0：无效				1：有效

返回值	    0：正确				1：错误

初始化时状态为：无效
注意：软件限位总是减速停止，那么计数值就可能回超出设定值，
因此在设定范围时应考虑到这一点。
***********************************************************/
int CCtrlCard::Setsoft_LimitMode1(int axis, int value)
{
    Result = set_softlimit_mode1(cardno, axis, value);

	return Result;

}

/******************设置COMP-寄存器做软件限制****************
cardno	   卡号

axis		轴号（1-4）

value		0：无效				1：有效

返回值	    0：正确				1：错误

初始化时状态为：无效

注意：软件限位总是减速停止，那么计数值就可能回超出设定值，
因此在设定范围时应考虑到这一点。
***********************************************************/
int CCtrlCard::Setsoft_LimitMode2(int axis, int value)
{
    Result = set_softlimit_mode2(cardno, axis, value);

	return Result;

}

/****************设置COMP+/-寄存器做软件限制****************
cardno	   卡号

axis		轴号（1-4）

value		0：逻辑位置计数器		1：实际位置计数器

返回值	    0：正确				1：错误

默认模式为 : 逻辑位置计数器
此函数是设定软件限位的比较对象。
***********************************************************/
int CCtrlCard::Setsoft_LimitMode3(int axis, int value)
{
    Result = set_softlimit_mode3(cardno, axis, value);

	return Result;

}

/****************伺服到位信号nINPOS的设定*******************
cardno	   卡号

axis		轴号（1-4）

value		0：无效				1：有效

logic		0：低电平有效		1：高电平有效

返回值	    0：正确				1：错误
默认模式为 : 无效，低电平有效
***********************************************************/
int CCtrlCard::Inpos_Mode(int axis, int value, int logic)
{
	Result = set_inpos_mode(cardno, axis,value,logic);
	
	return Result;
}

/****************伺服报警信号nALARM的设定*******************
cardno	   卡号

axis		轴号（1-4）

value		0：无效					1：有效

logic		0：低电平有效			1：高电平有效

返回值	    0：正确					1：错误
默认模式为 : 无效，低电平有效
***********************************************************/
int CCtrlCard:: Setup_AlarmMode(int axis,int value,int logic)
{
	Result = set_alarm_mode(cardno, axis,value,logic);
	
	return Result;
}


/************************设置速度模块***********************
    依据参数的值，判断是匀速还是加减速
    设置范围，是决定倍率的参数  	
	设置轴的初始速度、驱动速度和加速度
    参数：axis   -轴号
	      startv -初始速度
		  speed  -驱动速度
          add    -加速度
          dec    -减速度
		  ratio  -倍率
          mode   -模式    
    返回值=0正确，返回值=1错误
***********************************************************/
int CCtrlCard::Setup_Speed(int axis, long startv, long speed, long add ,long dec,long ratio,int mode)
{
	//匀速运动
	if (startv - speed >= 0) 
	{
		Result = set_range(cardno, axis, 8000000/ratio);
		
		set_startv(cardno, axis, startv/ratio);
		
		set_speed (cardno, axis, startv/ratio);
	}
	else//加/减速运动
	{
		if (mode == 0)//当选择直线加减速时进行相应的处理
		{
            set_dec1_mode(cardno,axis,0);//设为对称方式
			
			set_dec2_mode(cardno,axis,0);//设置为自动减速

            set_ad_mode(cardno,axis,0);//设置为直线加减速方式
			
			Result = set_range(cardno, axis, 8000000/ratio);
			
			set_startv(cardno, axis, startv/ratio);
			
			set_speed (cardno, axis, speed/ratio);
			
			set_acc (cardno, axis, add/125/ratio);
					
			
		}
		else if(mode==1)//当选择直线加减速时进行相应的处理
		{
			
			set_dec1_mode(cardno,axis,1);//设为非对称方式
			
			set_dec2_mode(cardno,axis,0);//设置为自动减速

		    set_ad_mode(cardno,axis,0);////设置为直线加减速方式	

			Result = set_range(cardno, axis, 8000000/ratio);
			
			set_startv(cardno, axis, startv/ratio);
			
			set_speed (cardno, axis, speed/ratio);
			
			set_acc (cardno, axis, add/125/ratio);
			
			set_dec (cardno, axis, dec/125/ratio);
						
		
		}
		else if(mode==2)//当选择S曲线加减速时进行相应的处理
		{
			float time;//时间的定义

			float addvar;//加速度变化率

			long k;//要算出来的结果

            time = (float)(speed-startv)/(add/2);//整个加速度时间

			addvar=add/(time/2);//加速度变化率

			k=(long)(62500000/addvar)*ratio;

            set_dec2_mode(cardno,axis,0);//设置为自动减速

            set_ad_mode(cardno,axis,1);////设置为S曲线加减速方式	

			Result = set_range(cardno, axis, 8000000/ratio);
			
			set_startv(cardno, axis, startv/ratio);
			
			set_speed (cardno, axis, speed/ratio);
			
			set_acc (cardno, axis, add/125/ratio);
			
			set_acac (cardno, axis,k );					
			
		}		
		
	}
	
	return Result;
	
}

/************************单轴驱动函数***********************

    该函数用于驱动单个运动轴运动

    参数： axis-轴号，value-输出脉冲数
    
    返回值=0正确，返回值=1错误

***********************************************************/
int CCtrlCard::Axis_Pmove(int axis, long value)
{
	Result = pmove(cardno, axis, value);

	TRACE ("%ld",cardno);
	
	return Result;

}


/************************单轴函数连续驱动***********************

    该函数用于驱动单个运动轴运动

    参数： axis-轴号，value-脉冲方向
    
    返回值=0正确，返回值=1错误

***********************************************************/
int CCtrlCard::Axis_Cmove(int axis, long value)
{
	Result = continue_move(cardno, axis, value);
	
	return Result;

}


/**********************两轴插补函数*********************

     该函数用于驱动XY或ZA两轴进行插补运动
	 no ->   1: X-Y       2:Z-A
    
    返回值=0正确，返回值=1错误

***********************************************************/
int CCtrlCard::Interp_Move2(int axis1,int axis2, long value1, long value2)
{
	Result = inp_move2(cardno, axis1,axis2, value1, value2);

	return Result;

}


/*********************三轴插补函数**********************

    该函数用于驱动XYZ三轴进行插补运动
    
    返回值=0正确，返回值=1错误

***********************************************************/
int CCtrlCard::Interp_Move3(int axis1,int axis2,int axis3,long value1, long value2, long value3)
{
	Result = inp_move3(cardno,axis1,axis2,axis3, value1, value2, value3);

	return Result;
}


/**********************顺时针CW圆弧插补函数*****************

 axis1,axis2	   参与插补的轴号    1：X   2:Y   3：Z  4:A

 x,y		圆弧插补的终点位置（相对于起点）
				
 i,j		圆弧插补的圆心点位置（相对于起点）

            该函数用于驱动XY或ZW轴进行圆弧插补运动
    
             返回值=0正确，返回值=1错误

***********************************************************/
int CCtrlCard::Interp_Arc(int axis1,int axis2, long x, long y, long i,long j)
{
	Result = inp_cw_arc(cardno,axis1,axis2,x,y,i,j);

	return Result;
}


/**********************逆时针CCW圆弧插补函数****************
axis1,axis2    	参与插补的轴号  1：X    2：Y    3：Z   4：W

x,y		圆弧插补的终点位置（相对于起点）
				
i,j		圆弧插补的圆心点位置（相对于起点）

    该函数用于任意轴进行圆弧插补运动
    
    返回值=0正确，返回值=1错误

***********************************************************/
int CCtrlCard::Interp_CcwArc(int axis1,int axis2, long x, long y, long i,long j)
{
	Result = inp_ccw_arc(cardno,axis1,axis2,x,y,i,j);

	return Result;
}


/*****************计数器的可变环功能的设定******************
      可变环功能是可以把这个环形计数器的最大数值任选设定，
	  如果定位不是直线而是旋转运动的话，用此功能控制位置	  
    很方便.
    返回值	0：正确					1：错误
***********************************************************/
int CCtrlCard::SetCircle_Mode(int axis, int value)
{
	Result = set_circle_mode(cardno,  axis, value);
	
	return Result;
}


/********************输入信号滤波功能设置*******************

  	value		0：滤波无效			1：滤波有效

	默认模式为 : 无效

***********************************************************/
int CCtrlCard::Setup_InputFilter(int axis,int number,int value)
{	
	Result = set_input_filter(cardno, axis, number, value);

	return Result;
}


/*******************输入信号滤波时间常数设置****************
axis		轴号（1-4）

value      可以除去最大噪音幅度      输入延迟
***********************************************************/
int CCtrlCard::Setup_FilterTime(int axis,int value)
{
	
	Result = set_filter_time(cardno, axis, value);
	
	return Result;
}

/****************************位置锁存设置函数**********************
功能:设置到位信号功能,锁定所有轴的逻辑位置和实际位置
参数:
	axis—参照轴
	regi—计数器模式  |0:逻辑位置
					  |1:实际位置 
	logical—电平信号 |0:由高到低 
				      |1:由低到高
返回值         0：正确          1：错误
	说明:使用指定轴axis的IN信号作为触发信号						  
*******************************************************************/
int CCtrlCard::Setup_LockPosition(int axis,int regi,int logical)
{
	Result = set_lock_position(cardno, axis, regi , logical);
	
	return Result;
}

/*****************停止轴驱动********************************

    该函数用于立即或减速停止轴的驱动

	参数：axis-轴号、mode-减速方式(0－立即停止, 1－减速停止)
    
    返回值=0正确，返回值=1错误

***********************************************************/
int CCtrlCard::StopRun(int axis, int mode)
{
	if(mode == 0)       //立即停止
    {
        Result = sudden_stop(cardno, axis);
	}   
    else                 //减速停止
    {
        Result = dec_stop(cardno, axis);
	}   
    return Result;
	
}


/*****************获取轴的驱动状态**************************

    该函数用于获取单轴的驱动状态或插补驱动状态

    参数：axis -轴号，value-状态指针(0-驱动结束，非0－正在驱动)
	  
		  mode(0-获取单轴驱动状态，1－获取插补驱动状态)
    
    返回值=0正确，返回值=1错误

***********************************************************/
int CCtrlCard::Get_Status(int axis, int &value, int mode)
{
	if (mode==0)          //获取单轴驱动状态

		Result=get_status(cardno,axis,&value);

	else                  //获取插补驱动状态

		Result=get_inp_status(cardno,&value);

	return Result;

}


/*************************获取同步操作的状态***********************
功能:获取同步操作的状态
参数:
	cardno       卡号
	axis         轴号
	status      0|未执行同步操作
			    1|执行过同步操作
返回值         0：正确          1：错误
	说明:利用该函数可以捕捉位置锁存是否执行		
******************************************************************/
int CCtrlCard::Get_LockStatus(int axis,int &status)
{
      Result=get_lock_status(cardno,  axis, &status);

	  return Result;
}


/**************************原点驱动状态函数************************
功能:获取回原点是否结束,以及判断是否有错误
参数：
	cardno      卡号
	axis        轴号
    status—驱动状态,0|驱动结束
	                 1|正在驱动
	err—错误标志,   0|正确
	                 1|错误  
返回值         0：正确          1：错误
******************************************************************/
int CCtrlCard::Get_HomeStatus(int axis,int &status,int &err)
{
      Result=get_home_status(cardno,  axis, &status,&err);

	  return Result;
}


/************************获取回原点出错信息************************
功能:获取回原点出错原因    
参数:	        
      err—错误标志 
				0  |正确
	            非0|错误  
				D0:comp+限位
				D1:comp-限位
				D2:LMT+限位
				D3:LMT-限位
				D4:伺服报警
				D5:紧急停止
				D6:Z相信号超前到达
返回值         0：正确          1：错误
******************************************************************/
int CCtrlCard::Get_HomeError(int axis,int &err)
{
      Result=get_home_error(cardno,  axis, &err);

	  return Result;
}


/*****************************允许减速函数**************************

      该函数用于驱动过程中允许减速

      返回值＝0正确 返回值＝1错误

*******************************************************************/
int CCtrlCard::AllowDec()
{
      Result=inp_dec_enable(cardno);

	  return Result;
}


/**************************禁止减速函数******************************

      该函数用于驱动过程中禁止减速

      返回值＝0正确 返回值＝1错误

********************************************************************/
int CCtrlCard::ForbidDec( )
{
	Result=inp_dec_disable(cardno);

	return Result;

}


/**************************获取轴的错误停止信息**********************
       该函数用于获取轴停止的信息
     
	   value:错误状态的指针  0：无错误  1：为两字节长度的值

	   返回值＝0正确 返回值＝1错误

*********************************************************************/
int CCtrlCard::Get_ErrorInf(int axis, int &value)
{
	Result=get_stopdata(cardno,axis,&value);

	return Result;
}


/***************************获取连续插补状态*************************
       该函数用于获取连续插补允许写入状态

       value：插补状态的指针  0：不允许写入  1：允许写入
     
	   返回值＝0正确 返回值＝1错误

********************************************************************/
int CCtrlCard::Get_AllowInpStatus( int &value)
{
	Result=get_inp_status2(cardno,&value);

	return Result;
}


/******************************设定减速方式***************************
       该函数用于设定对称或非对称和自动或手动减速

	   返回值＝0正确 返回值＝1错误
*********************************************************************/
int CCtrlCard::Set_DecMode(int axis, int mode1, int mode2)
{   
	int result1,result2;

	result1=set_dec1_mode(cardno,axis,mode1);

	result2=set_dec2_mode(cardno,axis,mode2);

	Result=result1 && result2;

	return Result;
}


/******************************设定减速点*****************************
       该函数用于设定手动减速过程中的减速点

	   返回值＝0正确 返回值＝1错误
*********************************************************************/
int CCtrlCard::Set_DecPos(int axis, long value, long startv, long speed, long add)
{
	float addtime;

	long  DecPulse;   //减速时花费的脉冲

	addtime=float(speed-startv)/add;

	DecPulse=long((startv+speed)*addtime)/2;

	Result=set_dec_pos(cardno,axis,value-DecPulse);

    return Result;
}

/****************************读取输入点*******************************

     该函数用于读取单个输入点

     参数：number-输入点(0 ~ 31)

     返回值：0 － 低电平，1 － 高电平，-1 － 错误

*********************************************************************/
int CCtrlCard::Read_Input(int number)
{
	Result = read_bit(cardno, number);
    
	return Result;
}


/***************************输出单点函数******************************

    该函数用于输出单点信号

    参数： number-输出点(0 ~ 31),value 0-低电平、1－高电平

    返回值=0正确，返回值=1错误
**********************************************************************/

int CCtrlCard::Write_Output(int number, int value)
{
	Result = write_bit(cardno, number, value);

	return Result;
}

/*************************设置位置计数器*******************************

     该函数用于设置逻辑位置和实际位置

     参数：axis-轴号,pos-设置的位置值

      mode 0－设置逻辑位置,非0－设置实际位置

     返回值=0正确，返回值=1错误
**********************************************************************/
	 
int CCtrlCard::Setup_Pos(int axis, long pos, int mode)
{
	if(mode==0)
	{
		Result = set_command_pos(cardno, axis, pos);
	}
	else
	{
		Result = set_actual_pos(cardno, axis, pos);
	}
	
	return Result;
	
}


/*************************设置COMP+寄存器******************************
      cardno	 卡号
	  axis		 轴号
      value	     范围值（-2147483648~+2147483647）
	 返回值	     0：正确				1：错误
**********************************************************************/
int CCtrlCard::Setup_Comp1(int axis, long value)
{
	Result= set_comp1(cardno, axis, value);

    return Result;
}


/**************************设置COMP-寄存器*****************************

      cardno	 卡号
	  axis		 轴号
      value	     范围值（-2147483648~+2147483647）
	 返回值	     0：正确				1：错误

**********************************************************************/
int CCtrlCard::Setup_Comp2(int axis, long value)
{
	Result= set_comp2(cardno, axis, value);
	
    return Result;
}


/***************************固定线速度模式设置************************
功能:固定线速度模式设置
参数:
	cardno 卡号   
	mode—0|不采用固定线速度
		  1|采用固定线速度
返回值         0：正确          1：错误
	说明:线速度是指矢量速度,固定线速度可以保证插补时合成速度固定
*******************************************************************/
int CCtrlCard::Setup_VectorSpeed(int mode)
{
	Result= set_vector_speed(cardno, mode);
	
    return Result;
}


/******************************原点模式设置**************************
功能：设置指定轴回原点的模式
参数:
	   logical0—stop0|0:低电平停止
					  |1:高电平停止
					  |-1:无效
	   logical1—stop1|0:低电平停止
					  |1:高电平停止
					  |-1:无效	
	   logical2—stop2|0:低电平停止
					  |1:高电平停止
					  |-1:无效	   
       0ffset— 0|不偏移原点
	            1|偏移原点
	   dir0—表示方向|0:正向
	                 |1:负向 
	   dir1—表示方向|0:正向
	                 |1:负向 
	   dir2—表示方向|0:正向
	                 |1:负向 	   
	   offsetdir—表示方向|0:正向
	                      |1:负向 				 
	   speed—低速搜寻速度，要求低于高速初始速度
	   clear—是否清除计数器|0:清除为零
	                        |1:不清除 
返回值         0：正确          1：错误
   说明:
		(1)回原点分为四大步:
		   |第一步:快速接近stop0(logical0近原点设置);
		   |第二步:慢速接近stop1(logical1原点设置);
		   |第三步:慢速接近stop2(logical2编码器Z相);
		   |第四步:偏移距离(用于工作原点);
        (2)上述四步可以选择是否执行,通过logical0、logical1、logical2和offset来选择
		(3)可以使用一个接近开关充当几个信号使用
******************************************************************/
int CCtrlCard::Setup_HomeMode(int axis,long speed,int logical0, int logical1, int logical2,int offset,int dir0, int dir1, int dir2,int offsetdir,int clear,long pulse)
{
	Result= set_home_mode(cardno, axis,speed,logical0,logical1,logical2,offset,dir0,dir1,dir2,offsetdir,clear,pulse);
	
    return Result;
}

/************************获取运动信息***********************
该函数用于反馈轴当前的逻辑位置，实际位置和运行速度
参数：axis-轴号,LogPos-逻辑位置,ActPos-实际位置,Speed-运行速度   
返回值=0正确，返回值=1错误
***********************************************************/
int CCtrlCard::Get_CurrentInf(int axis, long &LogPos, long &ActPos, long &Speed)
{
	Result = get_command_pos(cardno, axis, &LogPos);
    
    get_actual_pos (cardno, axis, &ActPos);
    
    get_speed (cardno, axis, &Speed);

	return Result;
}

/**************************获取锁定的位置**************************
功能:获取锁定的位置
参数:
	cardno      卡号
	axis         轴号
	pos         锁存的位置
返回值         0：正确          1：错误
******************************************************************/
int CCtrlCard::Get_LockPosition(int axis,long &pos)
{
	Result= get_lock_position(cardno, axis, &pos);
	
    return Result;
}


/************************库函数版本号函数**************************
*功能:获取库函数版本
*参数:
      ver-版本号(前两位为主版本号,后两位为次版本号)
返回值         0：正确          1：错误
******************************************************************/
int CCtrlCard::Get_LibVision(int &ver)
{
	Result= get_lib_vision( &ver);
	
    return Result;
}


/************************外部信号定量驱动函数**********************
功能:外部信号定量驱动函数
参数:
	cardno      卡号
	axis        轴号
	pulse       脉冲
返回值         0：正确          1：错误
	说明:(1)发出定量脉冲，但驱动没有立即进行，需要等到外部信号电平发生变化
		 (2)可以使用普通按钮,也可以接手轮
******************************************************************/
int CCtrlCard::Manu_Pmove(int axis, long pulse)
{
	Result= manual_pmove(cardno, axis, pulse);

    return Result;
}

/************************外部信号连续驱动函数**********************
功能:外部信号连续驱动函数
参数:
	cardno     卡号
	axis       轴号
返回值         0：正确          1：错误
	说明:(1)发出定量脉冲，但驱动没有立即进行，需要等到外部信号电平发生变化
		 (2)可以使用普通按钮,也可以接手轮
******************************************************************/
int CCtrlCard::Manu_Continue(int axis)
{
	Result= manual_continue(cardno, axis);

    return Result;
}

/***********************关闭外部信号驱动使能***********************
功能:关闭外部信号驱动使能
参数：
	cardno      卡号
	axis        轴号
返回值         0：正确          1：错误
******************************************************************/
int CCtrlCard::Manu_Disable(int axis)
{
	Result= manual_disable(cardno, axis);

    return Result;
}

/*********************两轴命令步进插补设置函数*********************	
功能：设置两轴命令式步进插补的数据
参数:
	和两轴插补函数相同
返回值         0：正确          1：错误
	功能说明:发出步进插补的数据，但驱动没有进行,等待发出命令驱动函数
******************************************************************/
int CCtrlCard::Inp_Command2(int axis1,int axis2,long pulse1,long pulse2)
{
	Result= inp_step_command2(cardno, axis1,axis2,pulse1,pulse2);

    return Result;
}

/*********************三轴命令步进插补设置函数*********************
功能：设置三轴命令式步进插补的数据
参数:
	和三轴插补函数相同
返回值         0：正确          1：错误
	功能说明:发出步进插补的数据，但驱动没有进行,等待发出命令驱动函数
******************************************************************/
int CCtrlCard::Inp_Command3(int axis1,int axis2,int axis3,long pulse1,long pulse2, long pulse3)
{
	Result= inp_step_command3(cardno, axis1,axis2,axis3,pulse1,pulse2,pulse3);

    return Result;
}


/***********************步进插补命令驱动函数***********************
功能:单步执行命令步进插补
参数:
	cardno      卡号
返回值         0：正确          1：错误
	功能说明:根据设置函数设定的参数,以命令的方式驱动步进运动
******************************************************************/
int CCtrlCard::Inp_StepMove()
{
	Result= inp_step_move(cardno);

    return Result;
}

/*********************两轴信号步进插补设置函数**********************
功能：设置两轴信号式步进插补的数据
参数:
	和两轴插补函数相同
返回值         0：正确          1：错误
	功能说明:发出步进插补的数据，但驱动没有进行,等待外部信号电平下降到低电平
*******************************************************************/
int CCtrlCard::Inp_Signal2(int axis1,int axis2,long pulse1,long pulse2)
{
	Result= inp_step_signal2(cardno, axis1,axis2,pulse1,pulse2);

    return Result;
}


/*********************三轴信号步进插补设置函数*********************
功能:设置三轴步进插补的数据
参数:
	和三轴插补函数相同
返回值         0：正确          1：错误
	功能说明:发出步进插补的数据，但驱动没有进行,等待外部信号电平下降到低电平
******************************************************************/
int CCtrlCard::Inp_Signal3(int axis1,int axis2,int axis3,long pulse1,long pulse2, long pulse3)
{
	Result= inp_step_signal3(cardno,  axis1,axis2,axis3,pulse1,pulse2,pulse3);

    return Result;
}


/*************************步进插补停止函数*************************	
功能:停止步进插补的执行过程
参数:
	cardno      卡号
	axis        轴号
返回值         0：正确          1：错误
	
	注意:处于步进插补的轴必须使用步进插补停止命令后,才能进行其它驱动
******************************************************************/
int CCtrlCard::Inp_Stop(int axis)
{
	Result= inp_step_stop(cardno, axis);

    return Result;
}


/****************************原点驱动函数**************************
功能:按照设定的方式,实现回原点的操作。
参数:
	cardno      卡号
	axis        轴号
返回值         0：正确          1：错误
******************************************************************/
int CCtrlCard::Move_Home(int axis)
{
	Result= home(cardno, axis);

    return Result;
}


/***************************清除原点出错信息***********************
功能:清除原点出错信息
参数:
	cardno      卡号
	axis        轴号
返回值         0：正确          1：错误
******************************************************************/
int CCtrlCard::Clear_HomeError(int axis)
{
	Result= clear_home_error(cardno, axis);

    return Result;
}


/**************************单轴随动设置函数************************
功能:IN同步运动设置函数
参数:
		axis—主动轴
		axis1—从动轴
		pulse—主动轴脉冲
		pulse1—从动轴脉冲
		logical—电平信号 |0:由高到低 
					      |1:由低到高					  
		mode—主动轴是否产生运动 0|主动轴不产生动作
								 1|主动轴动作
返回值         0：正确          1：错误
	说明:使用主动轴axis的IN信号作为触发信号						  
*******************************************************************/
int CCtrlCard::Set_InMove1(int axis,int axis1,long pulse,long pulse1,int logical,int mode)
{
	Result= set_in_move1(cardno, axis,axis1,pulse,pulse1,logical,mode);

    return Result;
}


/**************************两轴随动设置函数*************************
功能:设置IN同步动作
参数:
		axis—主动轴
		axis1—从动轴
		axis2—从动轴
		pulse1—脉冲
		pulse2—脉冲
		logical—电平信号 |0:由高到低 
					      |1:由低到高					  
		mode—主动轴是否产生运动 0|主动轴不产生动作
								 1|主动轴动作
返回值         0：正确          1：错误
	说明:使用主动轴的IN信号作为触发信号						  
*******************************************************************/
int CCtrlCard::Set_InMove2(int axis, int axis1,int axis2 ,long pulse,long pulse1,long pulse2,int logical,int mode)
{
	Result= set_in_move2(cardno, axis,axis1,axis2,pulse,pulse1,pulse2,logical,mode);

    return Result;
}


/*************************三轴随动设置函数*************************	
功能:设置IN同步动作
参数:
		axis—主动轴
		axis1—从动轴1
		axis2—从动轴2
		axis3—从动轴3
		pulse—主动轴的脉冲
		pulse1—从动轴1的脉冲
		pulse2—从动轴2的脉冲
		pulse3—从动轴3的脉冲
		logical—电平信号 |0:由高到低 
					      |1:由低到高	
		mode—主动轴是否产生运动 0|主动轴不产生动作
								 1|主动轴动作
返回值         0：正确          1：错误
	说明:使用主动轴的IN信号作为触发信号						  
********************************************************************/
int CCtrlCard::Set_InMove3(int axis,int axis1,int axis2,int axis3 ,long pulse,long pulse1,long pulse2, long pulse3,int logical,int mode)
{
	Result= set_in_move3(cardno, axis,axis1,axis2,axis3,pulse,pulse1,pulse2,pulse3,logical, mode);

    return Result;
}

/**************************单轴随停设置函数*************************
功能:设置IN同步动作
参数:
		axis—主动轴
		axis1—从动轴1
		logical—电平信号 |0:由高到低 
					      |1:由低到高
        mode—主轴停止 |0:停止
		              1|:不停
返回值         0：正确          1：错误
	说明:
		检测到信号变化,从动轴停止，主动轴驱动状态可以设置
*******************************************************************/
int CCtrlCard::Set_InStop1(int axis, int axis1 ,int logical, int mode)
{
	Result= set_in_stop1(cardno, axis,axis1,logical,mode);

    return Result;
}


/*************************两轴随停设置函数*************************
功能:设置IN同步动作
参数:
		axis—主动轴
		axis1—从动轴1
		axis1—从动轴2
		logical—电平信号 |0:由高到低 
					      |1:由低到高
        mode—主轴停止 |0:停止
		              1|:不停
返回值         0：正确          1：错误
	说明:
		检测到信号变化,从动轴停止，主动轴驱动状态可以设置
******************************************************************/
int CCtrlCard::Set_InStop2(int axis, int axis1,int axis2,int logical, int mode)
{
	Result= set_in_stop2(cardno, axis,axis1,axis2,logical,mode);

    return Result;
}

/*************************三轴随停设置函数************************
功能:设置IN同步动作
参数:
		axis—主动轴		
		logical—电平信号 |0:由高到低 
					      |1:由低到高
        mode—主轴停止 |0:停止
		              1|:不停
返回值         0：正确          1：错误
说明:
		检测到信号变化,从动轴停止，主动轴驱动状态可以设置

*******************************************************************/
int CCtrlCard::Set_InStop3(int axis,int logical, int mode)
{
	Result= set_in_stop3(cardno, axis,logical,mode);

    return Result;
}

/********************到达目标位置单轴驱动设置*********************
功能:到达目标位置单轴驱动设置
参数:
		axis—主动轴
		axis2—从动轴
		pulse1—主动轴的目标位置
		pulse2—从动轴的驱动脉冲
		regi—0|comp+  选择比较寄存器
			  1|comp-
        term—0|>=  选择比较寄存器 
			  1|<   
返回值         0：正确          1：错误
******************************************************************/
int CCtrlCard::Setup_Pmove1(int axis, int axis1, long pulse, long pulse1, int regi, int term)
{
	Result= set_comp_pmove1(cardno, axis,axis1,pulse,pulse1,regi,term);

    return Result;
}

/********************到达目标位置两轴驱动设置**********************
功能:到达目标位置两轴驱动设置
参数:
		axis—主动轴
		axis1—从动轴1
		axis2—从动轴2
		pulse—主动轴的目标位置
		pulse1—从动轴1的驱动脉冲
		pulse2—从动轴2的驱动脉冲
		regi—0|comp+  选择比较寄存器
			  1|comp-
        term—0|>=  选择比较寄存器 
			  1|<    
返回值         0：正确          1：错误
*****************************************************************/
int CCtrlCard::Setup_Pmove2(int axis, int axis1,int axis2, long pulse, long pulse1,long pulse2, int regi, int term)
{
	Result= set_comp_pmove2(cardno, axis,axis1,axis2,pulse,pulse1,pulse2,regi,term);

    return Result;
}


/*********************到达目标位置三轴驱动设置*********************
功能:到达目标位置三轴驱动设置
参数:
		axis—主动轴
		axis1—从动轴1
		axis2—从动轴2
		axis2—从动轴3
		pulse—主动轴的目标位置
		pulse1—从动轴1的驱动脉冲
		pulse2—从动轴2的驱动脉冲
		pulse3—从动轴3的驱动脉冲
		regi—0|comp+  选择比较寄存器
			  1|comp-
        term—0|>=  选择比较寄存器 
			  1|<       
返回值         0：正确          1：错误
*******************************************************************/
int CCtrlCard::Setup_Pmove3(int axis, int axis1,int axis2, int axis3, long pulse, long pulse1,long pulse2,long pulse3, int regi, int term)
{
	Result= set_comp_pmove3(cardno, axis,axis1,axis2,axis3,pulse,pulse1,pulse2,pulse3,regi,term);

    return Result;
}

/**********************到达目标位置停止驱动设置*********************
功能:到达目标位置停止驱动设置
参数:
		axis—主动轴
		axis1—从动轴1		
		pulse—主动轴的目标位置		
		regi—0|comp+  选择比较寄存器
			  1|comp-
        term—0|>=  选择比较寄存器 
			  1|<
        mode—|0:主轴停止
			  |1:主轴不停
返回值         0：正确          1：错误
*******************************************************************/
int CCtrlCard::Setup_Stop1(int axis,int axis1,long pulse,int regi,int term,int mode)
{
	Result= set_comp_stop1(cardno, axis,axis1,pulse,regi,term,mode);

    return Result;
}


/*********************到达目标位置停止驱动设置**********************
功能:到达目标位置停止驱动设置
参数:
		axis—主动轴
		axis1—从动轴1	
		axis2—从动轴2
		pulse—主动轴的目标位置		
		regi—0|comp+  选择比较寄存器
			  1|comp-
        term—0|>=  选择比较寄存器 
			  1|<
        mode—|0:主轴停止
			  |1:主轴不停	
返回值         0：正确          1：错误
******************************************************************/
int CCtrlCard::Setup_Stop2(int axis,int axis1,int axis2,long pulse,int regi,int term,int mode)
{
	Result= set_comp_stop2(cardno, axis,axis1,axis2,pulse,regi,term,mode);

    return Result;
}


/***********************到达目标位置停止驱动设置*******************
功能:到达目标位置停止驱动设置
参数:
		axis—主动轴		
		pulse—主动轴的目标位置		
		regi—0|comp+  选择比较寄存器
			  1|comp-
        term—0|>=  选择比较寄存器 
			  1|<
        mode—|0:主轴停止
			  |1:主轴不停	
返回值         0：正确          1：错误
*******************************************************************/
int CCtrlCard::Setup_Stop3(int axis,long pulse,int regi,int term,int mode)
{
	Result= set_comp_stop3(cardno, axis,pulse,regi,term,mode);

    return Result;
}

//------------------------复合驱动类--------------------------
//说明:以下函数是为了方便客户的使用而增加的函数
//------------------------------------------------------------

/*****************************单轴对称相对运动*********************
*功能:参照当前位置,以对称加减速进行定量移动
*参数:
      cardno-卡号
	  axis---轴号
	  pulse--脉冲
	  lspd---低速
	  hspd---高速
      tacc---加速时间(单位:秒)
	  vacc---加速度变化率
	  mode---模式(梯形(0)还是S曲线(1))
返回值         0：正确          1：错误
*******************************************************************/
int CCtrlCard::Symmetry_RelativeMove(int axis, long pulse, long lspd ,long hspd, double tacc, long vacc, int mode)
{
	Result= symmetry_relative_move(cardno,axis, pulse, lspd ,hspd, tacc, vacc, mode);

    return Result;
}

/***************************单轴对称绝对移动************************
*功能:参照零点位置,以对称加减速进行定量移动
*参数:
      cardno-卡号
	  axis---轴号
	  pulse--脉冲
	  lspd---低速
	  hspd---高速
      tacc---加速时间(单位:秒)
	  vacc---加速度变化率
	  mode---模式(梯形(0)还是S曲线(1))
返回值         0：正确          1：错误
********************************************************************/
int CCtrlCard::Symmetry_AbsoluteMove(int axis, long pulse, long lspd ,long hspd, double tacc, long vacc, int mode)
{
	Result= symmetry_absolute_move(cardno,axis, pulse, lspd ,hspd, tacc, vacc, mode);

    return Result;
}


/**************************单轴非对称相对运动************************
*功能:参照当前位置,以非对称加减速进行定量移动
*参数:
      cardno-卡号
	  axis---轴号
	  pulse--脉冲
	  lspd---低速
	  hspd---高速
      tacc---加速时间(单位:秒)
	  tdec---减速时间(单位:秒)
	  vacc---加/减速度变化率
	  mode---模式(梯形(0)还是S曲线(1))
返回值         0：正确          1：错误
*********************************************************************/
int CCtrlCard::Unsymmetry_RelativeMove( int axis, long pulse, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode)
{
	Result= unsymmetry_relative_move(cardno,  axis, pulse, lspd ,hspd,tacc, tdec, vacc, mode);

    return Result;
}


/*************************单轴非对称绝对运动*************************
*功能:参照零点位置,以非对称加减速进行定量移动
*参数:
      cardno-卡号
	  axis---轴号
	  pulse--脉冲
	  lspd---低速
	  hspd---高速
      tacc---加速时间(单位:秒)
	  tdec---减速时间(单位:秒)
	  vacc---加速度变化率
	  mode---模式(梯形(0)还是S曲线(1))
返回值         0：正确          1：错误
********************************************************************/
int CCtrlCard::Unsymmetry_AbsoluteMove(int axis, long pulse, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode)
{
	Result= unsymmetry_absolute_move(cardno, axis, pulse, lspd ,hspd, tacc, tdec, vacc, mode);

    return Result;
}

/**********************两轴对称直线插补相对移动********************
*功能:参照当前位置,以对称加减速进行直线插补
*参数:
      cardno-卡号
	  axis1---轴号1
	  axis2---轴号2	
	  pulse1--脉冲1
	  pulse2--脉冲2
	  lspd---低速
	  hspd---高速
      tacc---加速时间(单位:秒)
	  vacc---加速度变化率
	  mode---模式(梯形(0)还是S曲线(1))
返回值         0：正确          1：错误
******************************************************************/
int CCtrlCard::Symmetry_RelativeLine2(int axis1, int axis2, long pulse1, long pulse2, long lspd ,long hspd, double tacc, long vacc, int mode)
{
	Result= symmetry_relative_line2(cardno, axis1, axis2, pulse1,pulse2, lspd ,hspd,tacc, vacc,mode);

    return Result;
}

/********************两轴对称直线插补绝对移动**********************
*功能:参照零点位置,以对称加减速进行直线插补
*参数:
      cardno-卡号
	  axis1---轴号1
	  axis2---轴号2	
	  pulse1--脉冲1
	  pulse2--脉冲2
	  lspd---低速
	  hspd---高速
      tacc---加速时间(单位:秒)
	  vacc---加速度变化率
	  mode---模式(梯形(0)还是S曲线(1))
返回值         0：正确          1：错误
******************************************************************/
int CCtrlCard::Symmetry_AbsoluteLine2(int axis1, int axis2, long pulse1, long pulse2, long lspd ,long hspd, double tacc, long vacc, int mode)
{
	Result= symmetry_absolute_line2(cardno, axis1, axis2, pulse1, pulse2, lspd ,hspd, tacc, vacc,mode);

    return Result;
}

/********************两轴非对称直线插补相对移动********************
*功能:参照当前位置,以非对称加减速进行直线插补
*参数:
      cardno-卡号
	  axis1---轴号1
	  axis2---轴号2	
	  pulse1--脉冲1
	  pulse2--脉冲2
	  lspd---低速
	  hspd---高速
      tacc---加速时间(单位:秒)
	  tdec---减速时间(单位:秒)
	  vacc---加/减速度变化率
	  mode---模式(梯形(0)还是S曲线(1))
返回值         0：正确          1：错误
******************************************************************/
int CCtrlCard::Unsymmetry_RelativeLine2(int axis1, int axis2, long pulse1, long pulse2, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode)
{
	Result= unsymmetry_relative_line2(cardno, axis1,axis2, pulse1, pulse2, lspd , hspd, tacc,  tdec,  vacc,  mode);

    return Result;
}


/*******************两轴非对称直线插补绝对移动*********************
*功能:参照零点位置,以非对称加减速进行直线插补
*参数:
      cardno-卡号
	  axis1---轴号1
	  axis2---轴号2	
	  pulse1--脉冲1
	  pulse2--脉冲2
	  lspd---低速
	  hspd---高速
      tacc---加速时间(单位:秒)
	  tdec---减速时间(单位:秒)
	  vacc---加速度变化率
	  mode---模式(梯形(0)还是S曲线(1))
返回值         0：正确          1：错误
******************************************************************/
int CCtrlCard::Unsymmetry_AbsoluteLine2(int axis1, int axis2, long pulse1, long pulse2, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode)
{
	Result= unsymmetry_absolute_line2(cardno, axis1, axis2, pulse1, pulse2, lspd ,hspd, tacc, tdec, vacc, mode);

    return Result;
}


/**********************三轴对称直线插补相对运动********************
*功能:参照当前位置,以对称加减速进行直线插补
*参数:
      cardno-卡号
	  axis1---轴号1
	  axis2---轴号2	
	  axis3---轴号3	
	  pulse1--脉冲1
	  pulse2--脉冲2
	  pulse3--脉冲3
	  lspd---低速
	  hspd---高速
      tacc---加速时间(单位:秒)
	  vacc---加速度变化率
	  mode---模式(梯形(0)还是S曲线(1))
返回值         0：正确          1：错误
******************************************************************/
int CCtrlCard::Symmetry_RelativeLine3(int axis1, int axis2, int axis3, long pulse1, long pulse2, long pulse3, long lspd ,long hspd, double tacc, long vacc, int mode)
{
	Result= symmetry_relative_line3(cardno,axis1, axis2, axis3, pulse1, pulse2, pulse3, lspd ,hspd, tacc, vacc, mode);

    return Result;
}


/*********************三轴对称直线插补绝对运动*********************
功能:参照零点位置,以对称加减速进行直线插补
参数:
      cardno-卡号
	  axis1---轴号1
	  axis2---轴号2	
	  axis3---轴号3
	  pulse1--脉冲1
	  pulse2--脉冲2
	  pulse3--脉冲3
	  lspd---低速
	  hspd---高速
      tacc---加速时间(单位:秒)
	  vacc---加速度变化率
	  mode---模式(梯形(0)还是S曲线(1))
返回值         0：正确          1：错误
******************************************************************/
int CCtrlCard::Symmetry_AbsoluteLine3(int axis1, int axis2, int axis3, long pulse1, long pulse2, long pulse3, long lspd ,long hspd, double tacc, long vacc, int mode)
{
	Result= symmetry_absolute_line3(cardno, axis1, axis2,axis3,pulse1, pulse2,  pulse3,  lspd , hspd,  tacc,  vacc,  mode);

    return Result;
}


/*********************三轴非对称直线插补相对运动*******************
功能:参照当前位置,以非对称加减速进行直线插补
参数:
      cardno-卡号
	  axis1---轴号1
	  axis2---轴号2	
	  axis3---轴号3	
	  pulse1--脉冲1
	  pulse2--脉冲2
	  pulse3--脉冲3
	  lspd---低速
	  hspd---高速
      tacc---加速时间(单位:秒)
	  tdec---减速时间(单位:秒)
	  vacc---加速度变化率
	  mode---模式(梯形(0)还是S曲线(1))
返回值         0：正确          1：错误
******************************************************************/
int CCtrlCard::Unsymmetry_RelativeLine3(int axis1, int axis2, int axis3, long pulse1, long pulse2, long pulse3, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode)
{
	Result= unsymmetry_relative_line3(cardno, axis1, axis2, axis3, pulse1, pulse2, pulse3, lspd ,hspd, tacc, tdec, vacc, mode);

    return Result;
}


/**********************三轴非对称直线插补绝对运动******************
*功能:参照零点位置,以非对称加减速进行直线插补
*参数:
      cardno-卡号
	  axis1---轴号1
	  axis2---轴号2	
	  axis3---轴号3
	  pulse1--脉冲1
	  pulse2--脉冲2
	  pulse3--脉冲3
	  lspd---低速
	  hspd---高速
      tacc---加速时间(单位:秒)
	  tdec---减速时间(单位:秒)
	  vacc---加速度变化率
	  mode---模式(梯形(0)还是S曲线(1))
******************************************************************/
int CCtrlCard::Unsymmetry_AbsoluteLine3(int axis1, int axis2, int axis3, long pulse1, long pulse2, long pulse3, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode)
{
	Result= unsymmetry_absolute_line3(cardno, axis1,  axis2,  axis3,  pulse1,  pulse2,  pulse3,  lspd , hspd,  tacc,  tdec,  vacc,  mode);

    return Result;
}

/********************两轴对称圆弧插补相对移动**********************
功能:参照当前位置,以对称加减速进行圆弧插补
参数:
      cardno-卡号
	  axis1---轴号1
	  axis2---轴号2	
	  x、y----圆弧终点坐标(参照当前点,即圆弧起点)
	  i、j----圆心坐标(参照当前点,即圆弧起点)
	  dir-----运动方向(0-顺时针,1-逆时针)
	  lspd---低速
	  hspd---高速
      tacc---加速时间(单位:秒)
	  vacc---加速度变化率
	  mode---模式(梯形(0)还是S曲线(1))
返回值         0：正确          1：错误
******************************************************************/
int CCtrlCard::Symmetry_Relativearc(int axis1, int axis2, long x, long y, long i, long j, int dir, long lspd ,long hspd, double tacc, long vacc, int mode)
{
	Result= symmetry_relative_arc(cardno, axis1,axis2,x,y,i,j, dir,  lspd , hspd,  tacc,  vacc,  mode);

    return Result;
}


/**********************两轴对称圆弧插补绝对移动********************
功能:参照零点位置,以对称加减速进行圆弧插补
参数:
      cardno-卡号
	  axis1---轴号1
	  axis2---轴号2	
	  x、y----圆弧终点坐标
	  i、j----圆心坐标
	  dir-----运动方向(0-顺时针,1-逆时针)
	  lspd---低速
	  hspd---高速
      tacc---加速时间(单位:秒)
	  vacc---加速度变化率
	  mode---模式(梯形(0)还是S曲线(1))
返回值         0：正确          1：错误
******************************************************************/
int CCtrlCard::Symmetry_AbsoluteArc(int axis1, int axis2, long x, long y, long i, long j, int dir, long lspd ,long hspd, double tacc, long vacc, int mode)
{
	Result= symmetry_absolute_arc(cardno, axis1,axis2,x,y,i,j,dir,lspd,hspd,tacc, vacc, mode);

    return Result;
}


/**********************两轴非对称圆弧插补相对移动******************
功能:参照当前位置,以非对称加减速进行圆弧插补
参数:
      cardno-卡号
	  axis1---轴号1
	  axis2---轴号2	
	  x、y----圆弧终点坐标(参照当前点,即圆弧起点)
	  i、j----圆心坐标(参照当前点,即圆弧起点)
	  dir-----运动方向(0-顺时针,1-逆时针)
	  lspd---低速
	  hspd---高速
      tacc---加速时间(单位:秒)
	  tdec---减速时间(单位:秒)
	  vacc---加速度变化率
	  mode---模式(梯形(0)还是S曲线(1))
返回值         0：正确          1：错误
******************************************************************/
int CCtrlCard::Unsymmetry_RelativeArc(int axis1, int axis2, long x, long y, long i, long j, int dir, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode)
{
	Result= unsymmetry_relative_arc(cardno, axis1,axis2,x,y,i,j,dir,lspd,hspd,tacc, tdec, vacc, mode);

    return Result;
}


/*********************两轴非对称圆弧插补绝对移动*******************
功能:参照零点位置,以非对称加减速进行圆弧插补
参数:
      cardno-卡号
	  axis1---轴号1
	  axis2---轴号2	
	  x、y----圆弧终点坐标
	  i、j----圆心坐标
	  dir-----运动方向(0-顺时针,1-逆时针)
	  lspd---低速
	  hspd---高速
      tacc---加速时间(单位:秒)
	  tdec---减速时间(单位:秒)
	  vacc---加速度变化率
	  mode---模式(梯形(0)还是S曲线(1))
返回值         0：正确          1：错误
******************************************************************/
int CCtrlCard::Unsymmetry_AbsoluteArc(int axis1, int axis2, long x, long y, long i, long j, int dir, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode)
{
	Result= unsymmetry_absolute_arc(cardno, axis1,axis2,x,y,i,j,dir,lspd,hspd,tacc, tdec, vacc, mode);

    return Result;
}


//Linear Machine Operation
void CCtrlCard::OnParamSet()
{
    // TODO: Add your control notification handler code here
    int  pulsemode[] = { 0, 0, 0 };    //脉冲输出方式  0表示是脉冲+脉冲，这样负脉冲就是反方向。    

    int  limitmode[] = { 0,0,0 };     //限位模式 0－有效时立即停止  1－有效时减速停止

    int inposmode[] = { 0,0,0 };      //到位信号模式 0：无效				1：有效

    int alarmmode[] = { 0,0,0 };      //伺服报警信号的模式 0：无效					1：有效

    int  stop0mode[] = { 1,1,1 };     //stop0模式 0－无效  1－有效

    int  stop1mode[] = { 0,0,0 };     //stop1模式

    int  stop2mode[] = { 0,0,0 };     //stop2模式

    int  effectlogic[] = { 0,0,0 };   //电平模式 0－低电平有效  1－高电平有效

    int  circlemode[] = { 0,0,0 };    //可变环型模式设定 0：无效					1：有效

    long Rio[] = { 5,5,5 };

    for (int i = 1; i < MAXAXIS; i++)
    {
        //脉冲输出方式设置
        Setup_PulseMode(i, pulsemode[i - 1]);

        //限位模式设置
        Setup_LimitMode(i, limitmode[i - 1], effectlogic[i - 1]);

        //stop0信号模式设置
        //Setup_Stop0Mode(i, stop0mode[i - 1], effectlogic[i - 1]);

        //stop1信号模式设置
        Setup_Stop1Mode(i, stop1mode[i - 1], effectlogic[i - 1]);

        //stop2信号模式设置
        Setup_Stop2Mode(i, stop2mode[i - 1], effectlogic[i - 1]);

        //comp+寄存器做软件的设定 0：无效				1：有效
        Setsoft_LimitMode1(i, 0);

        //comp-寄存器做软件的设定 0：无效				1：有效
        Setsoft_LimitMode2(i, 0);

        //comp+/-寄存器比较对象的设定 0：逻辑位置计数器		1：实际位置计数器
        Setsoft_LimitMode3(i, 0);

        //comp+寄存器的数值设定
        Setup_Comp1(i, 1000 / 2);

        //comp-寄存器的数值设定
        Setup_Comp2(i, 1000 / 2);

        //伺服到位信号设定 
        g_CtrlCard.Inpos_Mode(i, inposmode[i - 1], effectlogic[i - 1]);

        //伺服报警信号设定
        Setup_AlarmMode(i, alarmmode[i - 1], effectlogic[i - 1]);

        //计数器的可变环功能的设定
        SetCircle_Mode(i, circlemode[i - 1]);

        //输入信号滤波功能设置 0：滤波无效			1：滤波有效
        Setup_InputFilter(i, 1, 0);//问题出在LMT+,LMT-,STOP0,STOP1硬件处理定义，在这里1代表LMT+,LMT-,STOP0,STOP1

        //输入信号滤波时间常数设置
        Setup_FilterTime(i, 1);//当参数值设为 1: 可以除去最大噪音幅度  输入信号延迟

        set_range(g_cardno, i, 8000000 / Rio[i - 1]);
    }
}

int CCtrlCard::OnButtonPmove(int axis, long pulse) //指定目标位置pulse(长度)移动。
{
    int status = -1;//驱动状态初始化为无效状态
	int ret = 0;
    long speed = 0;

    g_WorkStatus = 1;//工作状态初始化为有效状态

	ret = Setup_Speed(axis, 25000, 50000, 25000, 25000, 5, AXIS_MOTION_TYPE);
    if (ret > 0)
    {
        return -1;
    }
    /*****************************************************/
    //                                                  //
    //Pmove 根据所发的脉冲数来定方向                    //
    //pulse 输出的脉冲数  范围(-268435455~+268435455)   //
    //		>0：正方向移动 < 0：负方向移动              //
    /*****************************************************/
	ret = Axis_Pmove(axis, pulse);
    if (ret > 0)
    {
        return -1;
    }

    while (true)
    {
        DoEvent();

        Get_Status(axis, status, AXIS_MOTION_TYPE);
        get_speed(g_cardno, axis, &speed);

        if ((status == 0) || g_WorkStatus == 0)//当停止按钮按下，或者驱动状态为0时，各按钮恢复有效
        {
            if (speed == 0)
            {
                break;
            }
        }
    }

	return 0;
}

int CCtrlCard::OnButtonCmove(int axis, int direction) //按指定速度连续运动
{
    int status = -1;//驱动状态初始化为无效状态
    long speed = 0;

    g_WorkStatus = 1;//工作状态初始化为有效状态
    //	startv - 初始速度 1000
    //	speed - 驱动速度 2000
    //	add - 加速度 625
    //	dec - 减速度 625
    //	ratio - 倍率 5
    //	mode - 模式  0 //对称加减速
	status = Setup_Speed(axis, 20000, 80000, 12500, 12500, 5, AXIS_MOTION_TYPE);
	if (status > 0)
	{
		return -1;
	}
    /**************************************************/
    //                                                //
    //连续运动是根据运动的方向来的，所以要判断，它跟  //
    //Pmove不同的是它是根据所发的脉冲数来定方向       //
    //                                                //
    /**************************************************/
	status = Axis_Cmove(axis, direction); //第二个参数是0说明是正方向, 1表示反方向
    if (status > 0)
    {
        return -2;
    }
    //不需要去调用，因为会一直运行，直到停止。 Down启动运动，up停止运动。
    //while (true)
    //{
    //	DoEvent();

    //	g_CtrlCard.Get_Status(axis, status, AXIS_MOTION_TYPE);
    //	get_speed(g_cardno, axis, &speed);
    //	status = 0;//驱动状态初始化为无效状态
    //	speed = 0;
    //	if ((status == 0) || g_WorkStatus == 0)//当停止按钮按下，或者驱动状态为0时，各按钮恢复有效
    //	{
    //		if (speed == 0)
    //		{
    //			break;
    //		}
    //	}
    //}
    TRACE(_T("over\n"));
	return 0;
}

void CCtrlCard::OnButtonInp(int axis1, int axis2, long pulse1, long pulse2)//直线插补 
{
    int status1 = -1, status2 = -1;//驱动状态初始化为无效状态
    long speed1 = 0, speed2 = 0;

    g_WorkStatus = 1;//工作状态初始化为有效状态


    //***********两轴插补**********//	
	//XY两轴插补
	Setup_Speed(axis1, 2000, 10000, 12500, 12500, 5, AXIS_MOTION_TYPE);
    Interp_Move2(axis1, axis2, pulse1, pulse2);

    //*************各轴驱动状态检测********************//
    while (true)
    {
        DoEvent();

        g_CtrlCard.Get_Status(1, status1, AXIS_MOTION_TYPE);
        g_CtrlCard.Get_Status(2, status2, AXIS_MOTION_TYPE);

        get_speed(g_cardno, 1, &speed1);
        get_speed(g_cardno, 1, &speed2);

        if ((status1 == 0 && status2 == 0) || g_WorkStatus == 0)//当停止按钮按下，或者驱动状态为0时，各按钮恢复有效
        {
            if (speed1 == 0 && speed2 == 0)
            {
                break;
            }

        }
    }

}

int  CCtrlCard::BackHome(int axis, CStatic& staticProMsg)
{
	int     retn = -1;
	int		HomeDir[] = { 0,0,0 };              //回零方向 0:负方向 1:正方向
	int		Stop0[] = { 0,0,0 };                //stop0 有效电平设置；0：低电平停止	1：高电平停止
	int		Limit[] = { 0,0,0 };                //limit 信号 有效电平设置；0：低电平停止	1：高电平停止
	int		Stop1[] = { -1,-1,-1 };             //stop1 有效电平设置；0：低电平停止	1：高电平停止 -1:不启用编码器Z相信号
	long	BackRange[] = { 2000,2000,2000 };   //碰到stop0信号后，反向退出stop0的距离 >1
	long	ZRange[] = { 80,80,80 }; //{ 400/5,400/5,400/5}; //编码器Z相范围(电机旋转一圈所需脉冲数，不启用编码器Z相时也需设置成大于1的值) >1
	long	Offset[] = { 50,50,50 };            //原点偏移量(回原点成功后是否需要偏移一段位移)；==0不偏移，>0正方向偏移，<0负方向偏移
	//原点(STOP0)搜寻起始速度不能大于原点搜寻速度; 设定的值需要除以倍率。
	long	StartSpeed[] = { 2000,2000,2000 }; //{ 100/5,100/5,100/5 };//原点(STOP0)搜寻起始速度
	long	SearchSpeed[] = { 20000,20000,20000 }; //{ 500/5,500/5,500/5 };//原点搜寻速度
	long	HomeSpeed[] = { 2000,2000,2000 }; //{ 200/5,200/5,200/5 };//低速接近原点速度
	long	Acc[] = { 20,20,20 }; // { 12500 / 5 / 125, 12500 / 5 / 125, 12500 / 5 / 125 };//回原点过程中的加速度
	long	ZSpeed[] = { 40,40,40 }; //{ 200/5,200/5,200/5 };//编码器Z相(STOP1)搜寻速度
	bool    FailFlag = true;;
	bool    SuccessFlag = false;
	char    Axis[] = { 'X', 'Y', 'Z' };
	CString msg, statusRtnHome;

	retn = SetHomeMode_Ex(g_cardno, axis, HomeDir[axis - 1], Stop0[axis - 1], Limit[axis - 1], Stop1[axis - 1], BackRange[axis - 1], ZRange[axis - 1], Offset[axis - 1]);
	if (retn < 0)
	{
		TRACE("SetHomeMode_Ex return %d\n", retn);
		return -1;
	}
	retn = SetHomeSpeed_Ex(g_cardno, axis, StartSpeed[axis - 1], SearchSpeed[axis - 1], HomeSpeed[axis - 1], Acc[axis - 1], ZSpeed[axis - 1]);
    if (retn < 0)
    {
		TRACE("SetHomeSpeed_Ex return %d\n", retn);
        return -2;
    }
	retn = HomeProcess_Ex(g_cardno, axis);
    if (retn < 0)
    {
		TRACE("HomeProcess_Ex return %d\n", retn);
        return -3;
    }
	FailFlag = false;
	SuccessFlag = false;

	while (true)
	{
		DoEvent();
		Sleep(1);

		retn = GetHomeStatus_Ex(g_cardno, axis);
		statusRtnHome.Format(_T("%c return Home Status return: %d"), Axis[axis - 1], retn);
		staticProMsg.SetWindowText(statusRtnHome);
		if (retn > 20 || retn < 0)
		{
			FailFlag = true;
		}
		else if (retn == 0)
		{
			SuccessFlag = true;
		}

		if (SuccessFlag)
		{
			msg.Format(_T("%c轴回零成功"), Axis[axis - 1]);
			staticProMsg.SetWindowText(msg);
			break;
		}
		if (FailFlag)
		{
			msg.Format(_T("%c轴回零失败, 返回%d"), Axis[axis - 1], retn);
			staticProMsg.SetWindowText(msg);
			return -4;
		}
	}

	staticProMsg.SetWindowTextW(_T(""));

	return 0;
}

int CCtrlCard::GetSpeed(int cardno, int axis)
{
	long speed = 0;
	int ret = get_speed( cardno, axis, &speed );

	if ( ret )
	{
		return -1;
	}

	return speed;
}
