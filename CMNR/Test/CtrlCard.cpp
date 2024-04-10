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
/***********************��ʼ������**************************
    �ú����а����˿��ƿ���ʼ�����õĿ⺯�������ǵ���    
    ���������Ļ��������Ա�����ʾ�����������ȵ���
    ����ֵ<=0��ʾ��ʼ��ʧ�ܣ�����ֵ>0��ʾ��ʼ���ɹ�
***********************************************************/
int CCtrlCard::Init_Board()
{
	Result = HY5610V_initial() ;         //����ʼ������    
	
	if (Result <= 0) return Result;	
    
    for (int i = 1; i<=MAXAXIS; i++)
	{
		for(int j=0;j<Result;j++)
		{
		
			set_range (j, i, 8000000 / 5);    //�趨��Χ,���ó�ʼ����Ϊ5
			
		//		set_limit_mode (0, i, 0,  0); //�趨��λģʽ����������λ��Ч���͵�ƽ��Ч
			
			set_command_pos (j, i, 0);        //���߼�������
			
			set_actual_pos (j, i, 0);         //��ʵλ������
			
			set_startv (j, i, 100);           //�趨��ʼ�ٶ�
			
			set_speed (j, i, 100);            //�趨�����ٶ�

			set_acc (j, i, 0);              //�趨���ٶ�

			set_actualcount_mode(j,i,0,0,0);  //����ʵ��λ�ü������Ĺ���ģʽ

		//		set_pulse_mode(0,i,1,1,0);        //�����������Ĺ�����ʽ
		}
    }    
	inp_dec_enable(cardno);

    return Result;	
}


/**********************�ͷſ��ƿ�����***********************

    �ú����а������ͷſ��ƿ��Ŀ⺯�������Ǵ˺���Ӧ��

	���������ʱ�����   
***********************************************************/

int CCtrlCard::End_Board ()
{
	Result = HY5610V_end();
	
	return Result;
}


/********************����stop0�źŷ�ʽ**********************

   �ú��������趨stop0�źŵ�ģʽ

   ������ axis�����
          value   0����Ч  1����Ч
		  logic   0���͵�ƽֹͣ  1���ߵ�ƽֹͣ
   Ĭ��ģʽΪ����Ч

   ����ֵ=0��ȷ������ֵ=1����
  *********************************************************/
int CCtrlCard::Setup_Stop0Mode(int axis, int value, int logic)
{
	Result = set_stop0_mode(cardno, axis, value ,logic);

	return Result;
}


/********************����stop1�źŷ�ʽ**********************

   �ú��������趨stop1�źŵ�ģʽ

   ������ axis�����
          value   0����Ч  1����Ч
		  logic   0���͵�ƽֹͣ  1���ߵ�ƽֹͣ
   Ĭ��ģʽΪ����Ч

   ����ֵ=0��ȷ������ֵ=1����
  *********************************************************/
int CCtrlCard::Setup_Stop1Mode(int axis, int value, int logic)
{
	Result = set_stop1_mode(cardno, axis, value, logic);

	return Result;

}

/********************����stop2�źŷ�ʽ**********************

   �ú��������趨stop2�źŵ�ģʽ

   ������ axis�����
          value   0����Ч  1����Ч
		  logic   0���͵�ƽֹͣ  1���ߵ�ƽֹͣ
   Ĭ��ģʽΪ����Ч

   ����ֵ=0��ȷ������ֵ=1����
***********************************************************/
int CCtrlCard::Setup_Stop2Mode(int axis, int value, int logic)
{
	Result = set_stop2_mode(cardno, axis, value, logic);

	return Result;

}

/*********************����ʵ��λ�ü�����********************
cardno	   ����
axis	   ��ţ�1-4��
value	   �������巽ʽ
0��        A/B��������		1����/�£�PPIN/PMIN����������
dir		  ��������
0��	      A��ǰB��PPIN�����������ϼ���
		  B��ǰA��PMIN�����������¼���
1��       B��ǰA��PMIN�����������ϼ���
		  A��ǰB��PPIN�����������¼���
freq	  A/B��������ʱ�ı�Ƶ����/����������ʱ��Ч
0��4��Ƶ     1��2��Ƶ        2������Ƶ
����ֵ	     0����ȷ	   	 1������
��ʼ��ʱ״̬Ϊ��A/B���������룬����Ϊ0��4��Ƶ
***********************************************************/
int CCtrlCard::Actualcount_Mode(int axis,int value, int dir,int freq)
{
	Result = set_actualcount_mode(cardno, axis, value,dir,freq);
	
	return Result;
}

/********************�������������ʽ***********************
      
	�ú���������������Ĺ�����ʽ

	������axis-��ţ� value-���巽ʽ 0�����士���巽ʽ 1�����士����ʽ

    ����ֵ=0��ȷ������ֵ=1����

    Ĭ�����巽ʽΪ���士����ʽ

    ���������Ĭ�ϵ����߼�����ͷ�������ź����߼�

***********************************************************/
int CCtrlCard::Setup_PulseMode(int axis, int value)
{
	Result = set_pulse_mode(cardno, axis, value, 0, 0);
	
	return Result;	

}

/*********************������λ�źŷ�ʽ**********************

   �ú��������趨��/��������λ����nLMT�źŵ�ģʽ

   ������ axis�����

          value   0����Чʱ����ֹͣ  1����Чʱ����ֹͣ
		  
		  logic    0���͵�ƽ��Ч  1���ߵ�ƽ��Ч

   Ĭ��ģʽΪ������λ��Ч������λ��Ч���͵�ƽ��Ч

   ����ֵ=0��ȷ������ֵ=1����
***********************************************************/
int CCtrlCard::Setup_LimitMode(int axis, int value, int logic)
{
	Result = set_limit_mode(cardno, axis, value,  logic);

	return Result;

}

/******************����COMP+�Ĵ������������****************
cardno	   ����

axis		��ţ�1-4��

value		0����Ч				1����Ч

����ֵ	    0����ȷ				1������

��ʼ��ʱ״̬Ϊ����Ч
ע�⣺�����λ���Ǽ���ֹͣ����ô����ֵ�Ϳ��ܻس����趨ֵ��
������趨��ΧʱӦ���ǵ���һ�㡣
***********************************************************/
int CCtrlCard::Setsoft_LimitMode1(int axis, int value)
{
    Result = set_softlimit_mode1(cardno, axis, value);

	return Result;

}

/******************����COMP-�Ĵ������������****************
cardno	   ����

axis		��ţ�1-4��

value		0����Ч				1����Ч

����ֵ	    0����ȷ				1������

��ʼ��ʱ״̬Ϊ����Ч

ע�⣺�����λ���Ǽ���ֹͣ����ô����ֵ�Ϳ��ܻس����趨ֵ��
������趨��ΧʱӦ���ǵ���һ�㡣
***********************************************************/
int CCtrlCard::Setsoft_LimitMode2(int axis, int value)
{
    Result = set_softlimit_mode2(cardno, axis, value);

	return Result;

}

/****************����COMP+/-�Ĵ������������****************
cardno	   ����

axis		��ţ�1-4��

value		0���߼�λ�ü�����		1��ʵ��λ�ü�����

����ֵ	    0����ȷ				1������

Ĭ��ģʽΪ : �߼�λ�ü�����
�˺������趨�����λ�ıȽ϶���
***********************************************************/
int CCtrlCard::Setsoft_LimitMode3(int axis, int value)
{
    Result = set_softlimit_mode3(cardno, axis, value);

	return Result;

}

/****************�ŷ���λ�ź�nINPOS���趨*******************
cardno	   ����

axis		��ţ�1-4��

value		0����Ч				1����Ч

logic		0���͵�ƽ��Ч		1���ߵ�ƽ��Ч

����ֵ	    0����ȷ				1������
Ĭ��ģʽΪ : ��Ч���͵�ƽ��Ч
***********************************************************/
int CCtrlCard::Inpos_Mode(int axis, int value, int logic)
{
	Result = set_inpos_mode(cardno, axis,value,logic);
	
	return Result;
}

/****************�ŷ������ź�nALARM���趨*******************
cardno	   ����

axis		��ţ�1-4��

value		0����Ч					1����Ч

logic		0���͵�ƽ��Ч			1���ߵ�ƽ��Ч

����ֵ	    0����ȷ					1������
Ĭ��ģʽΪ : ��Ч���͵�ƽ��Ч
***********************************************************/
int CCtrlCard:: Setup_AlarmMode(int axis,int value,int logic)
{
	Result = set_alarm_mode(cardno, axis,value,logic);
	
	return Result;
}


/************************�����ٶ�ģ��***********************
    ���ݲ�����ֵ���ж������ٻ��ǼӼ���
    ���÷�Χ���Ǿ������ʵĲ���  	
	������ĳ�ʼ�ٶȡ������ٶȺͼ��ٶ�
    ������axis   -���
	      startv -��ʼ�ٶ�
		  speed  -�����ٶ�
          add    -���ٶ�
          dec    -���ٶ�
		  ratio  -����
          mode   -ģʽ    
    ����ֵ=0��ȷ������ֵ=1����
***********************************************************/
int CCtrlCard::Setup_Speed(int axis, long startv, long speed, long add ,long dec,long ratio,int mode)
{
	//�����˶�
	if (startv - speed >= 0) 
	{
		Result = set_range(cardno, axis, 8000000/ratio);
		
		set_startv(cardno, axis, startv/ratio);
		
		set_speed (cardno, axis, startv/ratio);
	}
	else//��/�����˶�
	{
		if (mode == 0)//��ѡ��ֱ�߼Ӽ���ʱ������Ӧ�Ĵ���
		{
            set_dec1_mode(cardno,axis,0);//��Ϊ�ԳƷ�ʽ
			
			set_dec2_mode(cardno,axis,0);//����Ϊ�Զ�����

            set_ad_mode(cardno,axis,0);//����Ϊֱ�߼Ӽ��ٷ�ʽ
			
			Result = set_range(cardno, axis, 8000000/ratio);
			
			set_startv(cardno, axis, startv/ratio);
			
			set_speed (cardno, axis, speed/ratio);
			
			set_acc (cardno, axis, add/125/ratio);
					
			
		}
		else if(mode==1)//��ѡ��ֱ�߼Ӽ���ʱ������Ӧ�Ĵ���
		{
			
			set_dec1_mode(cardno,axis,1);//��Ϊ�ǶԳƷ�ʽ
			
			set_dec2_mode(cardno,axis,0);//����Ϊ�Զ�����

		    set_ad_mode(cardno,axis,0);////����Ϊֱ�߼Ӽ��ٷ�ʽ	

			Result = set_range(cardno, axis, 8000000/ratio);
			
			set_startv(cardno, axis, startv/ratio);
			
			set_speed (cardno, axis, speed/ratio);
			
			set_acc (cardno, axis, add/125/ratio);
			
			set_dec (cardno, axis, dec/125/ratio);
						
		
		}
		else if(mode==2)//��ѡ��S���߼Ӽ���ʱ������Ӧ�Ĵ���
		{
			float time;//ʱ��Ķ���

			float addvar;//���ٶȱ仯��

			long k;//Ҫ������Ľ��

            time = (float)(speed-startv)/(add/2);//�������ٶ�ʱ��

			addvar=add/(time/2);//���ٶȱ仯��

			k=(long)(62500000/addvar)*ratio;

            set_dec2_mode(cardno,axis,0);//����Ϊ�Զ�����

            set_ad_mode(cardno,axis,1);////����ΪS���߼Ӽ��ٷ�ʽ	

			Result = set_range(cardno, axis, 8000000/ratio);
			
			set_startv(cardno, axis, startv/ratio);
			
			set_speed (cardno, axis, speed/ratio);
			
			set_acc (cardno, axis, add/125/ratio);
			
			set_acac (cardno, axis,k );					
			
		}		
		
	}
	
	return Result;
	
}

/************************������������***********************

    �ú����������������˶����˶�

    ������ axis-��ţ�value-���������
    
    ����ֵ=0��ȷ������ֵ=1����

***********************************************************/
int CCtrlCard::Axis_Pmove(int axis, long value)
{
	Result = pmove(cardno, axis, value);

	TRACE ("%ld",cardno);
	
	return Result;

}


/************************���ắ����������***********************

    �ú����������������˶����˶�

    ������ axis-��ţ�value-���巽��
    
    ����ֵ=0��ȷ������ֵ=1����

***********************************************************/
int CCtrlCard::Axis_Cmove(int axis, long value)
{
	Result = continue_move(cardno, axis, value);
	
	return Result;

}


/**********************����岹����*********************

     �ú�����������XY��ZA������в岹�˶�
	 no ->   1: X-Y       2:Z-A
    
    ����ֵ=0��ȷ������ֵ=1����

***********************************************************/
int CCtrlCard::Interp_Move2(int axis1,int axis2, long value1, long value2)
{
	Result = inp_move2(cardno, axis1,axis2, value1, value2);

	return Result;

}


/*********************����岹����**********************

    �ú�����������XYZ������в岹�˶�
    
    ����ֵ=0��ȷ������ֵ=1����

***********************************************************/
int CCtrlCard::Interp_Move3(int axis1,int axis2,int axis3,long value1, long value2, long value3)
{
	Result = inp_move3(cardno,axis1,axis2,axis3, value1, value2, value3);

	return Result;
}


/**********************˳ʱ��CWԲ���岹����*****************

 axis1,axis2	   ����岹�����    1��X   2:Y   3��Z  4:A

 x,y		Բ���岹���յ�λ�ã��������㣩
				
 i,j		Բ���岹��Բ�ĵ�λ�ã��������㣩

            �ú�����������XY��ZW�����Բ���岹�˶�
    
             ����ֵ=0��ȷ������ֵ=1����

***********************************************************/
int CCtrlCard::Interp_Arc(int axis1,int axis2, long x, long y, long i,long j)
{
	Result = inp_cw_arc(cardno,axis1,axis2,x,y,i,j);

	return Result;
}


/**********************��ʱ��CCWԲ���岹����****************
axis1,axis2    	����岹�����  1��X    2��Y    3��Z   4��W

x,y		Բ���岹���յ�λ�ã��������㣩
				
i,j		Բ���岹��Բ�ĵ�λ�ã��������㣩

    �ú����������������Բ���岹�˶�
    
    ����ֵ=0��ȷ������ֵ=1����

***********************************************************/
int CCtrlCard::Interp_CcwArc(int axis1,int axis2, long x, long y, long i,long j)
{
	Result = inp_ccw_arc(cardno,axis1,axis2,x,y,i,j);

	return Result;
}


/*****************�������Ŀɱ价���ܵ��趨******************
      �ɱ价�����ǿ��԰�������μ������������ֵ��ѡ�趨��
	  �����λ����ֱ�߶�����ת�˶��Ļ����ô˹��ܿ���λ��	  
    �ܷ���.
    ����ֵ	0����ȷ					1������
***********************************************************/
int CCtrlCard::SetCircle_Mode(int axis, int value)
{
	Result = set_circle_mode(cardno,  axis, value);
	
	return Result;
}


/********************�����ź��˲���������*******************

  	value		0���˲���Ч			1���˲���Ч

	Ĭ��ģʽΪ : ��Ч

***********************************************************/
int CCtrlCard::Setup_InputFilter(int axis,int number,int value)
{	
	Result = set_input_filter(cardno, axis, number, value);

	return Result;
}


/*******************�����ź��˲�ʱ�䳣������****************
axis		��ţ�1-4��

value      ���Գ�ȥ�����������      �����ӳ�
***********************************************************/
int CCtrlCard::Setup_FilterTime(int axis,int value)
{
	
	Result = set_filter_time(cardno, axis, value);
	
	return Result;
}

/****************************λ���������ú���**********************
����:���õ�λ�źŹ���,������������߼�λ�ú�ʵ��λ��
����:
	axis��������
	regi��������ģʽ  |0:�߼�λ��
					  |1:ʵ��λ�� 
	logical����ƽ�ź� |0:�ɸߵ��� 
				      |1:�ɵ͵���
����ֵ         0����ȷ          1������
	˵��:ʹ��ָ����axis��IN�ź���Ϊ�����ź�						  
*******************************************************************/
int CCtrlCard::Setup_LockPosition(int axis,int regi,int logical)
{
	Result = set_lock_position(cardno, axis, regi , logical);
	
	return Result;
}

/*****************ֹͣ������********************************

    �ú����������������ֹͣ�������

	������axis-��š�mode-���ٷ�ʽ(0������ֹͣ, 1������ֹͣ)
    
    ����ֵ=0��ȷ������ֵ=1����

***********************************************************/
int CCtrlCard::StopRun(int axis, int mode)
{
	if(mode == 0)       //����ֹͣ
    {
        Result = sudden_stop(cardno, axis);
	}   
    else                 //����ֹͣ
    {
        Result = dec_stop(cardno, axis);
	}   
    return Result;
	
}


/*****************��ȡ�������״̬**************************

    �ú������ڻ�ȡ���������״̬��岹����״̬

    ������axis -��ţ�value-״ָ̬��(0-������������0����������)
	  
		  mode(0-��ȡ��������״̬��1����ȡ�岹����״̬)
    
    ����ֵ=0��ȷ������ֵ=1����

***********************************************************/
int CCtrlCard::Get_Status(int axis, int &value, int mode)
{
	if (mode==0)          //��ȡ��������״̬

		Result=get_status(cardno,axis,&value);

	else                  //��ȡ�岹����״̬

		Result=get_inp_status(cardno,&value);

	return Result;

}


/*************************��ȡͬ��������״̬***********************
����:��ȡͬ��������״̬
����:
	cardno       ����
	axis         ���
	status      0|δִ��ͬ������
			    1|ִ�й�ͬ������
����ֵ         0����ȷ          1������
	˵��:���øú������Բ�׽λ�������Ƿ�ִ��		
******************************************************************/
int CCtrlCard::Get_LockStatus(int axis,int &status)
{
      Result=get_lock_status(cardno,  axis, &status);

	  return Result;
}


/**************************ԭ������״̬����************************
����:��ȡ��ԭ���Ƿ����,�Լ��ж��Ƿ��д���
������
	cardno      ����
	axis        ���
    status������״̬,0|��������
	                 1|��������
	err�������־,   0|��ȷ
	                 1|����  
����ֵ         0����ȷ          1������
******************************************************************/
int CCtrlCard::Get_HomeStatus(int axis,int &status,int &err)
{
      Result=get_home_status(cardno,  axis, &status,&err);

	  return Result;
}


/************************��ȡ��ԭ�������Ϣ************************
����:��ȡ��ԭ�����ԭ��    
����:	        
      err�������־ 
				0  |��ȷ
	            ��0|����  
				D0:comp+��λ
				D1:comp-��λ
				D2:LMT+��λ
				D3:LMT-��λ
				D4:�ŷ�����
				D5:����ֹͣ
				D6:Z���źų�ǰ����
����ֵ         0����ȷ          1������
******************************************************************/
int CCtrlCard::Get_HomeError(int axis,int &err)
{
      Result=get_home_error(cardno,  axis, &err);

	  return Result;
}


/*****************************������ٺ���**************************

      �ú������������������������

      ����ֵ��0��ȷ ����ֵ��1����

*******************************************************************/
int CCtrlCard::AllowDec()
{
      Result=inp_dec_enable(cardno);

	  return Result;
}


/**************************��ֹ���ٺ���******************************

      �ú����������������н�ֹ����

      ����ֵ��0��ȷ ����ֵ��1����

********************************************************************/
int CCtrlCard::ForbidDec( )
{
	Result=inp_dec_disable(cardno);

	return Result;

}


/**************************��ȡ��Ĵ���ֹͣ��Ϣ**********************
       �ú������ڻ�ȡ��ֹͣ����Ϣ
     
	   value:����״̬��ָ��  0���޴���  1��Ϊ���ֽڳ��ȵ�ֵ

	   ����ֵ��0��ȷ ����ֵ��1����

*********************************************************************/
int CCtrlCard::Get_ErrorInf(int axis, int &value)
{
	Result=get_stopdata(cardno,axis,&value);

	return Result;
}


/***************************��ȡ�����岹״̬*************************
       �ú������ڻ�ȡ�����岹����д��״̬

       value���岹״̬��ָ��  0��������д��  1������д��
     
	   ����ֵ��0��ȷ ����ֵ��1����

********************************************************************/
int CCtrlCard::Get_AllowInpStatus( int &value)
{
	Result=get_inp_status2(cardno,&value);

	return Result;
}


/******************************�趨���ٷ�ʽ***************************
       �ú��������趨�Գƻ�ǶԳƺ��Զ����ֶ�����

	   ����ֵ��0��ȷ ����ֵ��1����
*********************************************************************/
int CCtrlCard::Set_DecMode(int axis, int mode1, int mode2)
{   
	int result1,result2;

	result1=set_dec1_mode(cardno,axis,mode1);

	result2=set_dec2_mode(cardno,axis,mode2);

	Result=result1 && result2;

	return Result;
}


/******************************�趨���ٵ�*****************************
       �ú��������趨�ֶ����ٹ����еļ��ٵ�

	   ����ֵ��0��ȷ ����ֵ��1����
*********************************************************************/
int CCtrlCard::Set_DecPos(int axis, long value, long startv, long speed, long add)
{
	float addtime;

	long  DecPulse;   //����ʱ���ѵ�����

	addtime=float(speed-startv)/add;

	DecPulse=long((startv+speed)*addtime)/2;

	Result=set_dec_pos(cardno,axis,value-DecPulse);

    return Result;
}

/****************************��ȡ�����*******************************

     �ú������ڶ�ȡ���������

     ������number-�����(0 ~ 31)

     ����ֵ��0 �� �͵�ƽ��1 �� �ߵ�ƽ��-1 �� ����

*********************************************************************/
int CCtrlCard::Read_Input(int number)
{
	Result = read_bit(cardno, number);
    
	return Result;
}


/***************************������㺯��******************************

    �ú���������������ź�

    ������ number-�����(0 ~ 31),value 0-�͵�ƽ��1���ߵ�ƽ

    ����ֵ=0��ȷ������ֵ=1����
**********************************************************************/

int CCtrlCard::Write_Output(int number, int value)
{
	Result = write_bit(cardno, number, value);

	return Result;
}

/*************************����λ�ü�����*******************************

     �ú������������߼�λ�ú�ʵ��λ��

     ������axis-���,pos-���õ�λ��ֵ

      mode 0�������߼�λ��,��0������ʵ��λ��

     ����ֵ=0��ȷ������ֵ=1����
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


/*************************����COMP+�Ĵ���******************************
      cardno	 ����
	  axis		 ���
      value	     ��Χֵ��-2147483648~+2147483647��
	 ����ֵ	     0����ȷ				1������
**********************************************************************/
int CCtrlCard::Setup_Comp1(int axis, long value)
{
	Result= set_comp1(cardno, axis, value);

    return Result;
}


/**************************����COMP-�Ĵ���*****************************

      cardno	 ����
	  axis		 ���
      value	     ��Χֵ��-2147483648~+2147483647��
	 ����ֵ	     0����ȷ				1������

**********************************************************************/
int CCtrlCard::Setup_Comp2(int axis, long value)
{
	Result= set_comp2(cardno, axis, value);
	
    return Result;
}


/***************************�̶����ٶ�ģʽ����************************
����:�̶����ٶ�ģʽ����
����:
	cardno ����   
	mode��0|�����ù̶����ٶ�
		  1|���ù̶����ٶ�
����ֵ         0����ȷ          1������
	˵��:���ٶ���ָʸ���ٶ�,�̶����ٶȿ��Ա�֤�岹ʱ�ϳ��ٶȹ̶�
*******************************************************************/
int CCtrlCard::Setup_VectorSpeed(int mode)
{
	Result= set_vector_speed(cardno, mode);
	
    return Result;
}


/******************************ԭ��ģʽ����**************************
���ܣ�����ָ�����ԭ���ģʽ
����:
	   logical0��stop0|0:�͵�ƽֹͣ
					  |1:�ߵ�ƽֹͣ
					  |-1:��Ч
	   logical1��stop1|0:�͵�ƽֹͣ
					  |1:�ߵ�ƽֹͣ
					  |-1:��Ч	
	   logical2��stop2|0:�͵�ƽֹͣ
					  |1:�ߵ�ƽֹͣ
					  |-1:��Ч	   
       0ffset�� 0|��ƫ��ԭ��
	            1|ƫ��ԭ��
	   dir0����ʾ����|0:����
	                 |1:���� 
	   dir1����ʾ����|0:����
	                 |1:���� 
	   dir2����ʾ����|0:����
	                 |1:���� 	   
	   offsetdir����ʾ����|0:����
	                      |1:���� 				 
	   speed��������Ѱ�ٶȣ�Ҫ����ڸ��ٳ�ʼ�ٶ�
	   clear���Ƿ����������|0:���Ϊ��
	                        |1:����� 
����ֵ         0����ȷ          1������
   ˵��:
		(1)��ԭ���Ϊ�Ĵ�:
		   |��һ��:���ٽӽ�stop0(logical0��ԭ������);
		   |�ڶ���:���ٽӽ�stop1(logical1ԭ������);
		   |������:���ٽӽ�stop2(logical2������Z��);
		   |���Ĳ�:ƫ�ƾ���(���ڹ���ԭ��);
        (2)�����Ĳ�����ѡ���Ƿ�ִ��,ͨ��logical0��logical1��logical2��offset��ѡ��
		(3)����ʹ��һ���ӽ����س䵱�����ź�ʹ��
******************************************************************/
int CCtrlCard::Setup_HomeMode(int axis,long speed,int logical0, int logical1, int logical2,int offset,int dir0, int dir1, int dir2,int offsetdir,int clear,long pulse)
{
	Result= set_home_mode(cardno, axis,speed,logical0,logical1,logical2,offset,dir0,dir1,dir2,offsetdir,clear,pulse);
	
    return Result;
}

/************************��ȡ�˶���Ϣ***********************
�ú������ڷ����ᵱǰ���߼�λ�ã�ʵ��λ�ú������ٶ�
������axis-���,LogPos-�߼�λ��,ActPos-ʵ��λ��,Speed-�����ٶ�   
����ֵ=0��ȷ������ֵ=1����
***********************************************************/
int CCtrlCard::Get_CurrentInf(int axis, long &LogPos, long &ActPos, long &Speed)
{
	Result = get_command_pos(cardno, axis, &LogPos);
    
    get_actual_pos (cardno, axis, &ActPos);
    
    get_speed (cardno, axis, &Speed);

	return Result;
}

/**************************��ȡ������λ��**************************
����:��ȡ������λ��
����:
	cardno      ����
	axis         ���
	pos         �����λ��
����ֵ         0����ȷ          1������
******************************************************************/
int CCtrlCard::Get_LockPosition(int axis,long &pos)
{
	Result= get_lock_position(cardno, axis, &pos);
	
    return Result;
}


/************************�⺯���汾�ź���**************************
*����:��ȡ�⺯���汾
*����:
      ver-�汾��(ǰ��λΪ���汾��,����λΪ�ΰ汾��)
����ֵ         0����ȷ          1������
******************************************************************/
int CCtrlCard::Get_LibVision(int &ver)
{
	Result= get_lib_vision( &ver);
	
    return Result;
}


/************************�ⲿ�źŶ�����������**********************
����:�ⲿ�źŶ�����������
����:
	cardno      ����
	axis        ���
	pulse       ����
����ֵ         0����ȷ          1������
	˵��:(1)�����������壬������û���������У���Ҫ�ȵ��ⲿ�źŵ�ƽ�����仯
		 (2)����ʹ����ͨ��ť,Ҳ���Խ�����
******************************************************************/
int CCtrlCard::Manu_Pmove(int axis, long pulse)
{
	Result= manual_pmove(cardno, axis, pulse);

    return Result;
}

/************************�ⲿ�ź�������������**********************
����:�ⲿ�ź�������������
����:
	cardno     ����
	axis       ���
����ֵ         0����ȷ          1������
	˵��:(1)�����������壬������û���������У���Ҫ�ȵ��ⲿ�źŵ�ƽ�����仯
		 (2)����ʹ����ͨ��ť,Ҳ���Խ�����
******************************************************************/
int CCtrlCard::Manu_Continue(int axis)
{
	Result= manual_continue(cardno, axis);

    return Result;
}

/***********************�ر��ⲿ�ź�����ʹ��***********************
����:�ر��ⲿ�ź�����ʹ��
������
	cardno      ����
	axis        ���
����ֵ         0����ȷ          1������
******************************************************************/
int CCtrlCard::Manu_Disable(int axis)
{
	Result= manual_disable(cardno, axis);

    return Result;
}

/*********************����������岹���ú���*********************	
���ܣ�������������ʽ�����岹������
����:
	������岹������ͬ
����ֵ         0����ȷ          1������
	����˵��:���������岹�����ݣ�������û�н���,�ȴ�����������������
******************************************************************/
int CCtrlCard::Inp_Command2(int axis1,int axis2,long pulse1,long pulse2)
{
	Result= inp_step_command2(cardno, axis1,axis2,pulse1,pulse2);

    return Result;
}

/*********************����������岹���ú���*********************
���ܣ�������������ʽ�����岹������
����:
	������岹������ͬ
����ֵ         0����ȷ          1������
	����˵��:���������岹�����ݣ�������û�н���,�ȴ�����������������
******************************************************************/
int CCtrlCard::Inp_Command3(int axis1,int axis2,int axis3,long pulse1,long pulse2, long pulse3)
{
	Result= inp_step_command3(cardno, axis1,axis2,axis3,pulse1,pulse2,pulse3);

    return Result;
}


/***********************�����岹������������***********************
����:����ִ��������岹
����:
	cardno      ����
����ֵ         0����ȷ          1������
	����˵��:�������ú����趨�Ĳ���,������ķ�ʽ���������˶�
******************************************************************/
int CCtrlCard::Inp_StepMove()
{
	Result= inp_step_move(cardno);

    return Result;
}

/*********************�����źŲ����岹���ú���**********************
���ܣ����������ź�ʽ�����岹������
����:
	������岹������ͬ
����ֵ         0����ȷ          1������
	����˵��:���������岹�����ݣ�������û�н���,�ȴ��ⲿ�źŵ�ƽ�½����͵�ƽ
*******************************************************************/
int CCtrlCard::Inp_Signal2(int axis1,int axis2,long pulse1,long pulse2)
{
	Result= inp_step_signal2(cardno, axis1,axis2,pulse1,pulse2);

    return Result;
}


/*********************�����źŲ����岹���ú���*********************
����:�������Ჽ���岹������
����:
	������岹������ͬ
����ֵ         0����ȷ          1������
	����˵��:���������岹�����ݣ�������û�н���,�ȴ��ⲿ�źŵ�ƽ�½����͵�ƽ
******************************************************************/
int CCtrlCard::Inp_Signal3(int axis1,int axis2,int axis3,long pulse1,long pulse2, long pulse3)
{
	Result= inp_step_signal3(cardno,  axis1,axis2,axis3,pulse1,pulse2,pulse3);

    return Result;
}


/*************************�����岹ֹͣ����*************************	
����:ֹͣ�����岹��ִ�й���
����:
	cardno      ����
	axis        ���
����ֵ         0����ȷ          1������
	
	ע��:���ڲ����岹�������ʹ�ò����岹ֹͣ�����,���ܽ�����������
******************************************************************/
int CCtrlCard::Inp_Stop(int axis)
{
	Result= inp_step_stop(cardno, axis);

    return Result;
}


/****************************ԭ����������**************************
����:�����趨�ķ�ʽ,ʵ�ֻ�ԭ��Ĳ�����
����:
	cardno      ����
	axis        ���
����ֵ         0����ȷ          1������
******************************************************************/
int CCtrlCard::Move_Home(int axis)
{
	Result= home(cardno, axis);

    return Result;
}


/***************************���ԭ�������Ϣ***********************
����:���ԭ�������Ϣ
����:
	cardno      ����
	axis        ���
����ֵ         0����ȷ          1������
******************************************************************/
int CCtrlCard::Clear_HomeError(int axis)
{
	Result= clear_home_error(cardno, axis);

    return Result;
}


/**************************�����涯���ú���************************
����:INͬ���˶����ú���
����:
		axis��������
		axis1���Ӷ���
		pulse������������
		pulse1���Ӷ�������
		logical����ƽ�ź� |0:�ɸߵ��� 
					      |1:�ɵ͵���					  
		mode���������Ƿ�����˶� 0|�����᲻��������
								 1|�����ᶯ��
����ֵ         0����ȷ          1������
	˵��:ʹ��������axis��IN�ź���Ϊ�����ź�						  
*******************************************************************/
int CCtrlCard::Set_InMove1(int axis,int axis1,long pulse,long pulse1,int logical,int mode)
{
	Result= set_in_move1(cardno, axis,axis1,pulse,pulse1,logical,mode);

    return Result;
}


/**************************�����涯���ú���*************************
����:����INͬ������
����:
		axis��������
		axis1���Ӷ���
		axis2���Ӷ���
		pulse1������
		pulse2������
		logical����ƽ�ź� |0:�ɸߵ��� 
					      |1:�ɵ͵���					  
		mode���������Ƿ�����˶� 0|�����᲻��������
								 1|�����ᶯ��
����ֵ         0����ȷ          1������
	˵��:ʹ���������IN�ź���Ϊ�����ź�						  
*******************************************************************/
int CCtrlCard::Set_InMove2(int axis, int axis1,int axis2 ,long pulse,long pulse1,long pulse2,int logical,int mode)
{
	Result= set_in_move2(cardno, axis,axis1,axis2,pulse,pulse1,pulse2,logical,mode);

    return Result;
}


/*************************�����涯���ú���*************************	
����:����INͬ������
����:
		axis��������
		axis1���Ӷ���1
		axis2���Ӷ���2
		axis3���Ӷ���3
		pulse�������������
		pulse1���Ӷ���1������
		pulse2���Ӷ���2������
		pulse3���Ӷ���3������
		logical����ƽ�ź� |0:�ɸߵ��� 
					      |1:�ɵ͵���	
		mode���������Ƿ�����˶� 0|�����᲻��������
								 1|�����ᶯ��
����ֵ         0����ȷ          1������
	˵��:ʹ���������IN�ź���Ϊ�����ź�						  
********************************************************************/
int CCtrlCard::Set_InMove3(int axis,int axis1,int axis2,int axis3 ,long pulse,long pulse1,long pulse2, long pulse3,int logical,int mode)
{
	Result= set_in_move3(cardno, axis,axis1,axis2,axis3,pulse,pulse1,pulse2,pulse3,logical, mode);

    return Result;
}

/**************************������ͣ���ú���*************************
����:����INͬ������
����:
		axis��������
		axis1���Ӷ���1
		logical����ƽ�ź� |0:�ɸߵ��� 
					      |1:�ɵ͵���
        mode������ֹͣ |0:ֹͣ
		              1|:��ͣ
����ֵ         0����ȷ          1������
	˵��:
		��⵽�źű仯,�Ӷ���ֹͣ������������״̬��������
*******************************************************************/
int CCtrlCard::Set_InStop1(int axis, int axis1 ,int logical, int mode)
{
	Result= set_in_stop1(cardno, axis,axis1,logical,mode);

    return Result;
}


/*************************������ͣ���ú���*************************
����:����INͬ������
����:
		axis��������
		axis1���Ӷ���1
		axis1���Ӷ���2
		logical����ƽ�ź� |0:�ɸߵ��� 
					      |1:�ɵ͵���
        mode������ֹͣ |0:ֹͣ
		              1|:��ͣ
����ֵ         0����ȷ          1������
	˵��:
		��⵽�źű仯,�Ӷ���ֹͣ������������״̬��������
******************************************************************/
int CCtrlCard::Set_InStop2(int axis, int axis1,int axis2,int logical, int mode)
{
	Result= set_in_stop2(cardno, axis,axis1,axis2,logical,mode);

    return Result;
}

/*************************������ͣ���ú���************************
����:����INͬ������
����:
		axis��������		
		logical����ƽ�ź� |0:�ɸߵ��� 
					      |1:�ɵ͵���
        mode������ֹͣ |0:ֹͣ
		              1|:��ͣ
����ֵ         0����ȷ          1������
˵��:
		��⵽�źű仯,�Ӷ���ֹͣ������������״̬��������

*******************************************************************/
int CCtrlCard::Set_InStop3(int axis,int logical, int mode)
{
	Result= set_in_stop3(cardno, axis,logical,mode);

    return Result;
}

/********************����Ŀ��λ�õ�����������*********************
����:����Ŀ��λ�õ�����������
����:
		axis��������
		axis2���Ӷ���
		pulse1���������Ŀ��λ��
		pulse2���Ӷ������������
		regi��0|comp+  ѡ��ȽϼĴ���
			  1|comp-
        term��0|>=  ѡ��ȽϼĴ��� 
			  1|<   
����ֵ         0����ȷ          1������
******************************************************************/
int CCtrlCard::Setup_Pmove1(int axis, int axis1, long pulse, long pulse1, int regi, int term)
{
	Result= set_comp_pmove1(cardno, axis,axis1,pulse,pulse1,regi,term);

    return Result;
}

/********************����Ŀ��λ��������������**********************
����:����Ŀ��λ��������������
����:
		axis��������
		axis1���Ӷ���1
		axis2���Ӷ���2
		pulse���������Ŀ��λ��
		pulse1���Ӷ���1����������
		pulse2���Ӷ���2����������
		regi��0|comp+  ѡ��ȽϼĴ���
			  1|comp-
        term��0|>=  ѡ��ȽϼĴ��� 
			  1|<    
����ֵ         0����ȷ          1������
*****************************************************************/
int CCtrlCard::Setup_Pmove2(int axis, int axis1,int axis2, long pulse, long pulse1,long pulse2, int regi, int term)
{
	Result= set_comp_pmove2(cardno, axis,axis1,axis2,pulse,pulse1,pulse2,regi,term);

    return Result;
}


/*********************����Ŀ��λ��������������*********************
����:����Ŀ��λ��������������
����:
		axis��������
		axis1���Ӷ���1
		axis2���Ӷ���2
		axis2���Ӷ���3
		pulse���������Ŀ��λ��
		pulse1���Ӷ���1����������
		pulse2���Ӷ���2����������
		pulse3���Ӷ���3����������
		regi��0|comp+  ѡ��ȽϼĴ���
			  1|comp-
        term��0|>=  ѡ��ȽϼĴ��� 
			  1|<       
����ֵ         0����ȷ          1������
*******************************************************************/
int CCtrlCard::Setup_Pmove3(int axis, int axis1,int axis2, int axis3, long pulse, long pulse1,long pulse2,long pulse3, int regi, int term)
{
	Result= set_comp_pmove3(cardno, axis,axis1,axis2,axis3,pulse,pulse1,pulse2,pulse3,regi,term);

    return Result;
}

/**********************����Ŀ��λ��ֹͣ��������*********************
����:����Ŀ��λ��ֹͣ��������
����:
		axis��������
		axis1���Ӷ���1		
		pulse���������Ŀ��λ��		
		regi��0|comp+  ѡ��ȽϼĴ���
			  1|comp-
        term��0|>=  ѡ��ȽϼĴ��� 
			  1|<
        mode��|0:����ֹͣ
			  |1:���᲻ͣ
����ֵ         0����ȷ          1������
*******************************************************************/
int CCtrlCard::Setup_Stop1(int axis,int axis1,long pulse,int regi,int term,int mode)
{
	Result= set_comp_stop1(cardno, axis,axis1,pulse,regi,term,mode);

    return Result;
}


/*********************����Ŀ��λ��ֹͣ��������**********************
����:����Ŀ��λ��ֹͣ��������
����:
		axis��������
		axis1���Ӷ���1	
		axis2���Ӷ���2
		pulse���������Ŀ��λ��		
		regi��0|comp+  ѡ��ȽϼĴ���
			  1|comp-
        term��0|>=  ѡ��ȽϼĴ��� 
			  1|<
        mode��|0:����ֹͣ
			  |1:���᲻ͣ	
����ֵ         0����ȷ          1������
******************************************************************/
int CCtrlCard::Setup_Stop2(int axis,int axis1,int axis2,long pulse,int regi,int term,int mode)
{
	Result= set_comp_stop2(cardno, axis,axis1,axis2,pulse,regi,term,mode);

    return Result;
}


/***********************����Ŀ��λ��ֹͣ��������*******************
����:����Ŀ��λ��ֹͣ��������
����:
		axis��������		
		pulse���������Ŀ��λ��		
		regi��0|comp+  ѡ��ȽϼĴ���
			  1|comp-
        term��0|>=  ѡ��ȽϼĴ��� 
			  1|<
        mode��|0:����ֹͣ
			  |1:���᲻ͣ	
����ֵ         0����ȷ          1������
*******************************************************************/
int CCtrlCard::Setup_Stop3(int axis,long pulse,int regi,int term,int mode)
{
	Result= set_comp_stop3(cardno, axis,pulse,regi,term,mode);

    return Result;
}

//------------------------����������--------------------------
//˵��:���º�����Ϊ�˷���ͻ���ʹ�ö����ӵĺ���
//------------------------------------------------------------

/*****************************����Գ�����˶�*********************
*����:���յ�ǰλ��,�ԶԳƼӼ��ٽ��ж����ƶ�
*����:
      cardno-����
	  axis---���
	  pulse--����
	  lspd---����
	  hspd---����
      tacc---����ʱ��(��λ:��)
	  vacc---���ٶȱ仯��
	  mode---ģʽ(����(0)����S����(1))
����ֵ         0����ȷ          1������
*******************************************************************/
int CCtrlCard::Symmetry_RelativeMove(int axis, long pulse, long lspd ,long hspd, double tacc, long vacc, int mode)
{
	Result= symmetry_relative_move(cardno,axis, pulse, lspd ,hspd, tacc, vacc, mode);

    return Result;
}

/***************************����Գƾ����ƶ�************************
*����:�������λ��,�ԶԳƼӼ��ٽ��ж����ƶ�
*����:
      cardno-����
	  axis---���
	  pulse--����
	  lspd---����
	  hspd---����
      tacc---����ʱ��(��λ:��)
	  vacc---���ٶȱ仯��
	  mode---ģʽ(����(0)����S����(1))
����ֵ         0����ȷ          1������
********************************************************************/
int CCtrlCard::Symmetry_AbsoluteMove(int axis, long pulse, long lspd ,long hspd, double tacc, long vacc, int mode)
{
	Result= symmetry_absolute_move(cardno,axis, pulse, lspd ,hspd, tacc, vacc, mode);

    return Result;
}


/**************************����ǶԳ�����˶�************************
*����:���յ�ǰλ��,�ԷǶԳƼӼ��ٽ��ж����ƶ�
*����:
      cardno-����
	  axis---���
	  pulse--����
	  lspd---����
	  hspd---����
      tacc---����ʱ��(��λ:��)
	  tdec---����ʱ��(��λ:��)
	  vacc---��/���ٶȱ仯��
	  mode---ģʽ(����(0)����S����(1))
����ֵ         0����ȷ          1������
*********************************************************************/
int CCtrlCard::Unsymmetry_RelativeMove( int axis, long pulse, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode)
{
	Result= unsymmetry_relative_move(cardno,  axis, pulse, lspd ,hspd,tacc, tdec, vacc, mode);

    return Result;
}


/*************************����ǶԳƾ����˶�*************************
*����:�������λ��,�ԷǶԳƼӼ��ٽ��ж����ƶ�
*����:
      cardno-����
	  axis---���
	  pulse--����
	  lspd---����
	  hspd---����
      tacc---����ʱ��(��λ:��)
	  tdec---����ʱ��(��λ:��)
	  vacc---���ٶȱ仯��
	  mode---ģʽ(����(0)����S����(1))
����ֵ         0����ȷ          1������
********************************************************************/
int CCtrlCard::Unsymmetry_AbsoluteMove(int axis, long pulse, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode)
{
	Result= unsymmetry_absolute_move(cardno, axis, pulse, lspd ,hspd, tacc, tdec, vacc, mode);

    return Result;
}

/**********************����Գ�ֱ�߲岹����ƶ�********************
*����:���յ�ǰλ��,�ԶԳƼӼ��ٽ���ֱ�߲岹
*����:
      cardno-����
	  axis1---���1
	  axis2---���2	
	  pulse1--����1
	  pulse2--����2
	  lspd---����
	  hspd---����
      tacc---����ʱ��(��λ:��)
	  vacc---���ٶȱ仯��
	  mode---ģʽ(����(0)����S����(1))
����ֵ         0����ȷ          1������
******************************************************************/
int CCtrlCard::Symmetry_RelativeLine2(int axis1, int axis2, long pulse1, long pulse2, long lspd ,long hspd, double tacc, long vacc, int mode)
{
	Result= symmetry_relative_line2(cardno, axis1, axis2, pulse1,pulse2, lspd ,hspd,tacc, vacc,mode);

    return Result;
}

/********************����Գ�ֱ�߲岹�����ƶ�**********************
*����:�������λ��,�ԶԳƼӼ��ٽ���ֱ�߲岹
*����:
      cardno-����
	  axis1---���1
	  axis2---���2	
	  pulse1--����1
	  pulse2--����2
	  lspd---����
	  hspd---����
      tacc---����ʱ��(��λ:��)
	  vacc---���ٶȱ仯��
	  mode---ģʽ(����(0)����S����(1))
����ֵ         0����ȷ          1������
******************************************************************/
int CCtrlCard::Symmetry_AbsoluteLine2(int axis1, int axis2, long pulse1, long pulse2, long lspd ,long hspd, double tacc, long vacc, int mode)
{
	Result= symmetry_absolute_line2(cardno, axis1, axis2, pulse1, pulse2, lspd ,hspd, tacc, vacc,mode);

    return Result;
}

/********************����ǶԳ�ֱ�߲岹����ƶ�********************
*����:���յ�ǰλ��,�ԷǶԳƼӼ��ٽ���ֱ�߲岹
*����:
      cardno-����
	  axis1---���1
	  axis2---���2	
	  pulse1--����1
	  pulse2--����2
	  lspd---����
	  hspd---����
      tacc---����ʱ��(��λ:��)
	  tdec---����ʱ��(��λ:��)
	  vacc---��/���ٶȱ仯��
	  mode---ģʽ(����(0)����S����(1))
����ֵ         0����ȷ          1������
******************************************************************/
int CCtrlCard::Unsymmetry_RelativeLine2(int axis1, int axis2, long pulse1, long pulse2, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode)
{
	Result= unsymmetry_relative_line2(cardno, axis1,axis2, pulse1, pulse2, lspd , hspd, tacc,  tdec,  vacc,  mode);

    return Result;
}


/*******************����ǶԳ�ֱ�߲岹�����ƶ�*********************
*����:�������λ��,�ԷǶԳƼӼ��ٽ���ֱ�߲岹
*����:
      cardno-����
	  axis1---���1
	  axis2---���2	
	  pulse1--����1
	  pulse2--����2
	  lspd---����
	  hspd---����
      tacc---����ʱ��(��λ:��)
	  tdec---����ʱ��(��λ:��)
	  vacc---���ٶȱ仯��
	  mode---ģʽ(����(0)����S����(1))
����ֵ         0����ȷ          1������
******************************************************************/
int CCtrlCard::Unsymmetry_AbsoluteLine2(int axis1, int axis2, long pulse1, long pulse2, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode)
{
	Result= unsymmetry_absolute_line2(cardno, axis1, axis2, pulse1, pulse2, lspd ,hspd, tacc, tdec, vacc, mode);

    return Result;
}


/**********************����Գ�ֱ�߲岹����˶�********************
*����:���յ�ǰλ��,�ԶԳƼӼ��ٽ���ֱ�߲岹
*����:
      cardno-����
	  axis1---���1
	  axis2---���2	
	  axis3---���3	
	  pulse1--����1
	  pulse2--����2
	  pulse3--����3
	  lspd---����
	  hspd---����
      tacc---����ʱ��(��λ:��)
	  vacc---���ٶȱ仯��
	  mode---ģʽ(����(0)����S����(1))
����ֵ         0����ȷ          1������
******************************************************************/
int CCtrlCard::Symmetry_RelativeLine3(int axis1, int axis2, int axis3, long pulse1, long pulse2, long pulse3, long lspd ,long hspd, double tacc, long vacc, int mode)
{
	Result= symmetry_relative_line3(cardno,axis1, axis2, axis3, pulse1, pulse2, pulse3, lspd ,hspd, tacc, vacc, mode);

    return Result;
}


/*********************����Գ�ֱ�߲岹�����˶�*********************
����:�������λ��,�ԶԳƼӼ��ٽ���ֱ�߲岹
����:
      cardno-����
	  axis1---���1
	  axis2---���2	
	  axis3---���3
	  pulse1--����1
	  pulse2--����2
	  pulse3--����3
	  lspd---����
	  hspd---����
      tacc---����ʱ��(��λ:��)
	  vacc---���ٶȱ仯��
	  mode---ģʽ(����(0)����S����(1))
����ֵ         0����ȷ          1������
******************************************************************/
int CCtrlCard::Symmetry_AbsoluteLine3(int axis1, int axis2, int axis3, long pulse1, long pulse2, long pulse3, long lspd ,long hspd, double tacc, long vacc, int mode)
{
	Result= symmetry_absolute_line3(cardno, axis1, axis2,axis3,pulse1, pulse2,  pulse3,  lspd , hspd,  tacc,  vacc,  mode);

    return Result;
}


/*********************����ǶԳ�ֱ�߲岹����˶�*******************
����:���յ�ǰλ��,�ԷǶԳƼӼ��ٽ���ֱ�߲岹
����:
      cardno-����
	  axis1---���1
	  axis2---���2	
	  axis3---���3	
	  pulse1--����1
	  pulse2--����2
	  pulse3--����3
	  lspd---����
	  hspd---����
      tacc---����ʱ��(��λ:��)
	  tdec---����ʱ��(��λ:��)
	  vacc---���ٶȱ仯��
	  mode---ģʽ(����(0)����S����(1))
����ֵ         0����ȷ          1������
******************************************************************/
int CCtrlCard::Unsymmetry_RelativeLine3(int axis1, int axis2, int axis3, long pulse1, long pulse2, long pulse3, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode)
{
	Result= unsymmetry_relative_line3(cardno, axis1, axis2, axis3, pulse1, pulse2, pulse3, lspd ,hspd, tacc, tdec, vacc, mode);

    return Result;
}


/**********************����ǶԳ�ֱ�߲岹�����˶�******************
*����:�������λ��,�ԷǶԳƼӼ��ٽ���ֱ�߲岹
*����:
      cardno-����
	  axis1---���1
	  axis2---���2	
	  axis3---���3
	  pulse1--����1
	  pulse2--����2
	  pulse3--����3
	  lspd---����
	  hspd---����
      tacc---����ʱ��(��λ:��)
	  tdec---����ʱ��(��λ:��)
	  vacc---���ٶȱ仯��
	  mode---ģʽ(����(0)����S����(1))
******************************************************************/
int CCtrlCard::Unsymmetry_AbsoluteLine3(int axis1, int axis2, int axis3, long pulse1, long pulse2, long pulse3, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode)
{
	Result= unsymmetry_absolute_line3(cardno, axis1,  axis2,  axis3,  pulse1,  pulse2,  pulse3,  lspd , hspd,  tacc,  tdec,  vacc,  mode);

    return Result;
}

/********************����Գ�Բ���岹����ƶ�**********************
����:���յ�ǰλ��,�ԶԳƼӼ��ٽ���Բ���岹
����:
      cardno-����
	  axis1---���1
	  axis2---���2	
	  x��y----Բ���յ�����(���յ�ǰ��,��Բ�����)
	  i��j----Բ������(���յ�ǰ��,��Բ�����)
	  dir-----�˶�����(0-˳ʱ��,1-��ʱ��)
	  lspd---����
	  hspd---����
      tacc---����ʱ��(��λ:��)
	  vacc---���ٶȱ仯��
	  mode---ģʽ(����(0)����S����(1))
����ֵ         0����ȷ          1������
******************************************************************/
int CCtrlCard::Symmetry_Relativearc(int axis1, int axis2, long x, long y, long i, long j, int dir, long lspd ,long hspd, double tacc, long vacc, int mode)
{
	Result= symmetry_relative_arc(cardno, axis1,axis2,x,y,i,j, dir,  lspd , hspd,  tacc,  vacc,  mode);

    return Result;
}


/**********************����Գ�Բ���岹�����ƶ�********************
����:�������λ��,�ԶԳƼӼ��ٽ���Բ���岹
����:
      cardno-����
	  axis1---���1
	  axis2---���2	
	  x��y----Բ���յ�����
	  i��j----Բ������
	  dir-----�˶�����(0-˳ʱ��,1-��ʱ��)
	  lspd---����
	  hspd---����
      tacc---����ʱ��(��λ:��)
	  vacc---���ٶȱ仯��
	  mode---ģʽ(����(0)����S����(1))
����ֵ         0����ȷ          1������
******************************************************************/
int CCtrlCard::Symmetry_AbsoluteArc(int axis1, int axis2, long x, long y, long i, long j, int dir, long lspd ,long hspd, double tacc, long vacc, int mode)
{
	Result= symmetry_absolute_arc(cardno, axis1,axis2,x,y,i,j,dir,lspd,hspd,tacc, vacc, mode);

    return Result;
}


/**********************����ǶԳ�Բ���岹����ƶ�******************
����:���յ�ǰλ��,�ԷǶԳƼӼ��ٽ���Բ���岹
����:
      cardno-����
	  axis1---���1
	  axis2---���2	
	  x��y----Բ���յ�����(���յ�ǰ��,��Բ�����)
	  i��j----Բ������(���յ�ǰ��,��Բ�����)
	  dir-----�˶�����(0-˳ʱ��,1-��ʱ��)
	  lspd---����
	  hspd---����
      tacc---����ʱ��(��λ:��)
	  tdec---����ʱ��(��λ:��)
	  vacc---���ٶȱ仯��
	  mode---ģʽ(����(0)����S����(1))
����ֵ         0����ȷ          1������
******************************************************************/
int CCtrlCard::Unsymmetry_RelativeArc(int axis1, int axis2, long x, long y, long i, long j, int dir, long lspd ,long hspd, double tacc, double tdec, long vacc, int mode)
{
	Result= unsymmetry_relative_arc(cardno, axis1,axis2,x,y,i,j,dir,lspd,hspd,tacc, tdec, vacc, mode);

    return Result;
}


/*********************����ǶԳ�Բ���岹�����ƶ�*******************
����:�������λ��,�ԷǶԳƼӼ��ٽ���Բ���岹
����:
      cardno-����
	  axis1---���1
	  axis2---���2	
	  x��y----Բ���յ�����
	  i��j----Բ������
	  dir-----�˶�����(0-˳ʱ��,1-��ʱ��)
	  lspd---����
	  hspd---����
      tacc---����ʱ��(��λ:��)
	  tdec---����ʱ��(��λ:��)
	  vacc---���ٶȱ仯��
	  mode---ģʽ(����(0)����S����(1))
����ֵ         0����ȷ          1������
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
    int  pulsemode[] = { 0, 0, 0 };    //���������ʽ  0��ʾ������+���壬������������Ƿ�����    

    int  limitmode[] = { 0,0,0 };     //��λģʽ 0����Чʱ����ֹͣ  1����Чʱ����ֹͣ

    int inposmode[] = { 0,0,0 };      //��λ�ź�ģʽ 0����Ч				1����Ч

    int alarmmode[] = { 0,0,0 };      //�ŷ������źŵ�ģʽ 0����Ч					1����Ч

    int  stop0mode[] = { 1,1,1 };     //stop0ģʽ 0����Ч  1����Ч

    int  stop1mode[] = { 0,0,0 };     //stop1ģʽ

    int  stop2mode[] = { 0,0,0 };     //stop2ģʽ

    int  effectlogic[] = { 0,0,0 };   //��ƽģʽ 0���͵�ƽ��Ч  1���ߵ�ƽ��Ч

    int  circlemode[] = { 0,0,0 };    //�ɱ价��ģʽ�趨 0����Ч					1����Ч

    long Rio[] = { 5,5,5 };

    for (int i = 1; i < MAXAXIS; i++)
    {
        //���������ʽ����
        Setup_PulseMode(i, pulsemode[i - 1]);

        //��λģʽ����
        Setup_LimitMode(i, limitmode[i - 1], effectlogic[i - 1]);

        //stop0�ź�ģʽ����
        //Setup_Stop0Mode(i, stop0mode[i - 1], effectlogic[i - 1]);

        //stop1�ź�ģʽ����
        Setup_Stop1Mode(i, stop1mode[i - 1], effectlogic[i - 1]);

        //stop2�ź�ģʽ����
        Setup_Stop2Mode(i, stop2mode[i - 1], effectlogic[i - 1]);

        //comp+�Ĵ�����������趨 0����Ч				1����Ч
        Setsoft_LimitMode1(i, 0);

        //comp-�Ĵ�����������趨 0����Ч				1����Ч
        Setsoft_LimitMode2(i, 0);

        //comp+/-�Ĵ����Ƚ϶�����趨 0���߼�λ�ü�����		1��ʵ��λ�ü�����
        Setsoft_LimitMode3(i, 0);

        //comp+�Ĵ�������ֵ�趨
        Setup_Comp1(i, 1000 / 2);

        //comp-�Ĵ�������ֵ�趨
        Setup_Comp2(i, 1000 / 2);

        //�ŷ���λ�ź��趨 
        g_CtrlCard.Inpos_Mode(i, inposmode[i - 1], effectlogic[i - 1]);

        //�ŷ������ź��趨
        Setup_AlarmMode(i, alarmmode[i - 1], effectlogic[i - 1]);

        //�������Ŀɱ价���ܵ��趨
        SetCircle_Mode(i, circlemode[i - 1]);

        //�����ź��˲��������� 0���˲���Ч			1���˲���Ч
        Setup_InputFilter(i, 1, 0);//�������LMT+,LMT-,STOP0,STOP1Ӳ�������壬������1����LMT+,LMT-,STOP0,STOP1

        //�����ź��˲�ʱ�䳣������
        Setup_FilterTime(i, 1);//������ֵ��Ϊ 1: ���Գ�ȥ�����������  �����ź��ӳ�

        set_range(g_cardno, i, 8000000 / Rio[i - 1]);
    }
}

int CCtrlCard::OnButtonPmove(int axis, long pulse) //ָ��Ŀ��λ��pulse(����)�ƶ���
{
    int status = -1;//����״̬��ʼ��Ϊ��Ч״̬
	int ret = 0;
    long speed = 0;

    g_WorkStatus = 1;//����״̬��ʼ��Ϊ��Ч״̬

	ret = Setup_Speed(axis, 25000, 50000, 25000, 25000, 5, AXIS_MOTION_TYPE);
    if (ret > 0)
    {
        return -1;
    }
    /*****************************************************/
    //                                                  //
    //Pmove ������������������������                    //
    //pulse �����������  ��Χ(-268435455~+268435455)   //
    //		>0���������ƶ� < 0���������ƶ�              //
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

        if ((status == 0) || g_WorkStatus == 0)//��ֹͣ��ť���£���������״̬Ϊ0ʱ������ť�ָ���Ч
        {
            if (speed == 0)
            {
                break;
            }
        }
    }

	return 0;
}

int CCtrlCard::OnButtonCmove(int axis, int direction) //��ָ���ٶ������˶�
{
    int status = -1;//����״̬��ʼ��Ϊ��Ч״̬
    long speed = 0;

    g_WorkStatus = 1;//����״̬��ʼ��Ϊ��Ч״̬
    //	startv - ��ʼ�ٶ� 1000
    //	speed - �����ٶ� 2000
    //	add - ���ٶ� 625
    //	dec - ���ٶ� 625
    //	ratio - ���� 5
    //	mode - ģʽ  0 //�ԳƼӼ���
	status = Setup_Speed(axis, 20000, 80000, 12500, 12500, 5, AXIS_MOTION_TYPE);
	if (status > 0)
	{
		return -1;
	}
    /**************************************************/
    //                                                //
    //�����˶��Ǹ����˶��ķ������ģ�����Ҫ�жϣ�����  //
    //Pmove��ͬ�������Ǹ�����������������������       //
    //                                                //
    /**************************************************/
	status = Axis_Cmove(axis, direction); //�ڶ���������0˵����������, 1��ʾ������
    if (status > 0)
    {
        return -2;
    }
    //����Ҫȥ���ã���Ϊ��һֱ���У�ֱ��ֹͣ�� Down�����˶���upֹͣ�˶���
    //while (true)
    //{
    //	DoEvent();

    //	g_CtrlCard.Get_Status(axis, status, AXIS_MOTION_TYPE);
    //	get_speed(g_cardno, axis, &speed);
    //	status = 0;//����״̬��ʼ��Ϊ��Ч״̬
    //	speed = 0;
    //	if ((status == 0) || g_WorkStatus == 0)//��ֹͣ��ť���£���������״̬Ϊ0ʱ������ť�ָ���Ч
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

void CCtrlCard::OnButtonInp(int axis1, int axis2, long pulse1, long pulse2)//ֱ�߲岹 
{
    int status1 = -1, status2 = -1;//����״̬��ʼ��Ϊ��Ч״̬
    long speed1 = 0, speed2 = 0;

    g_WorkStatus = 1;//����״̬��ʼ��Ϊ��Ч״̬


    //***********����岹**********//	
	//XY����岹
	Setup_Speed(axis1, 2000, 10000, 12500, 12500, 5, AXIS_MOTION_TYPE);
    Interp_Move2(axis1, axis2, pulse1, pulse2);

    //*************��������״̬���********************//
    while (true)
    {
        DoEvent();

        g_CtrlCard.Get_Status(1, status1, AXIS_MOTION_TYPE);
        g_CtrlCard.Get_Status(2, status2, AXIS_MOTION_TYPE);

        get_speed(g_cardno, 1, &speed1);
        get_speed(g_cardno, 1, &speed2);

        if ((status1 == 0 && status2 == 0) || g_WorkStatus == 0)//��ֹͣ��ť���£���������״̬Ϊ0ʱ������ť�ָ���Ч
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
	int		HomeDir[] = { 0,0,0 };              //���㷽�� 0:������ 1:������
	int		Stop0[] = { 0,0,0 };                //stop0 ��Ч��ƽ���ã�0���͵�ƽֹͣ	1���ߵ�ƽֹͣ
	int		Limit[] = { 0,0,0 };                //limit �ź� ��Ч��ƽ���ã�0���͵�ƽֹͣ	1���ߵ�ƽֹͣ
	int		Stop1[] = { -1,-1,-1 };             //stop1 ��Ч��ƽ���ã�0���͵�ƽֹͣ	1���ߵ�ƽֹͣ -1:�����ñ�����Z���ź�
	long	BackRange[] = { 2000,2000,2000 };   //����stop0�źź󣬷����˳�stop0�ľ��� >1
	long	ZRange[] = { 80,80,80 }; //{ 400/5,400/5,400/5}; //������Z�෶Χ(�����תһȦ�����������������ñ�����Z��ʱҲ�����óɴ���1��ֵ) >1
	long	Offset[] = { 50,50,50 };            //ԭ��ƫ����(��ԭ��ɹ����Ƿ���Ҫƫ��һ��λ��)��==0��ƫ�ƣ�>0������ƫ�ƣ�<0������ƫ��
	//ԭ��(STOP0)��Ѱ��ʼ�ٶȲ��ܴ���ԭ����Ѱ�ٶ�; �趨��ֵ��Ҫ���Ա��ʡ�
	long	StartSpeed[] = { 2000,2000,2000 }; //{ 100/5,100/5,100/5 };//ԭ��(STOP0)��Ѱ��ʼ�ٶ�
	long	SearchSpeed[] = { 20000,20000,20000 }; //{ 500/5,500/5,500/5 };//ԭ����Ѱ�ٶ�
	long	HomeSpeed[] = { 2000,2000,2000 }; //{ 200/5,200/5,200/5 };//���ٽӽ�ԭ���ٶ�
	long	Acc[] = { 20,20,20 }; // { 12500 / 5 / 125, 12500 / 5 / 125, 12500 / 5 / 125 };//��ԭ������еļ��ٶ�
	long	ZSpeed[] = { 40,40,40 }; //{ 200/5,200/5,200/5 };//������Z��(STOP1)��Ѱ�ٶ�
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
			msg.Format(_T("%c�����ɹ�"), Axis[axis - 1]);
			staticProMsg.SetWindowText(msg);
			break;
		}
		if (FailFlag)
		{
			msg.Format(_T("%c�����ʧ��, ����%d"), Axis[axis - 1], retn);
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
