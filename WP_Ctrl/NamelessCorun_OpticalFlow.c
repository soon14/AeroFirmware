/* Copyright (c)  2018-2025 Wuhan Nameless Innovation Technology Co.,Ltd. All rights reserved.*/
/*----------------------------------------------------------------------------------------------------------------------/
*               ������ֻ��������ѧϰʹ�ã���Ȩ����Ȩ���������ƴ��Ŷӣ������ƴ��Ŷӽ��ɿس���Դ���ṩ�������ߣ�
*               ������ҪΪ�����ƴ��Ŷ��ṩ������δ���������ɣ����ý�Դ�����ṩ�����ˣ����ý�Դ����ŵ����Ϲ�����������أ� 
*               �������Դ�����Ĳ�����緢��������Ϊ�������ƴ��Ŷӽ���֮�Է��ɽ��������
-----------------------------------------------------------------------------------------------------------------------/
*               ������Ϣ���ܶ���ֹ��ǰ�����������˳���������
*               ��Դ���ף���ѧ����ϧ��ף������Ϯ�����׳ɹ�������
*               ѧϰ�����ߣ��������Ƽ���DJI��ZEROTECH��XAG��AEE��GDU��AUTEL��EWATT��HIGH GREAT�ȹ�˾��ҵ
*               ��ְ�����뷢�ͣ�15671678205@163.com���豸ע��ְ����λ����λ��������
*               �����ƴ���Դ�ɿ�QQȺ��540707961
*               CSDN���ͣ�http://blog.csdn.net/u011992534
*               �ſ�ID��NamelessCotrun����С��
*               Bվ��ѧ��Ƶ��https://space.bilibili.com/67803559/#/video
*               �ͻ�ʹ���ĵá��Ľ������������http://www.openedv.com/forum.php?mod=viewthread&tid=234214&extra=page=1
*               �Ա����̣�https://shop348646912.taobao.com/?spm=2013.1.1000126.2.5ce78a88ht1sO2
*               �ٶ�����:�����ƴ���Դ�ɿ�
*               ��˾����:www.nameless.tech
*               �޸�����:2019/4/12
*               �汾����Ӯ�ߡ���CarryPilot_V1.0
*               ��Ȩ���У�����ؾ���
*               Copyright(C) 2017-2025 �人�������¿Ƽ����޹�˾ 
*               All rights reserved
*               ��Ҫ��ʾ��
*               �����Ա�����ת�ֵķɿء��������ѡ�����ѧ�ܵĶ����Խ��ۺ�Ⱥѧϰ������
*               ����ֱ�����������������������ϣ��������´�����������Ȩ�����˲��ý�
*               ���ϴ��봫���Ϲ��������أ�������ı��ΪĿ���������ϴ��룬�����д˲�
*               ���ߣ���˾����ǰ��֪����1���ڼ�ʱ�������������ѧУ����λ����������
*               ������ַ��Ϣ�ᱻ�����ڹ�˾�������ٷ�΢�Ź���ƽ̨���ٷ��������͡�֪��
*               ר���Լ��Ա�������ҳ���Թ�ʾ���棬����������Ϊ�����Ϊ�����۵㣬Ӱ��
*               ��ѧ���ҹ���������������ܿ�ͺ������˻����������������ء�
*               �����Ϊ����˾����ش���ʧ�ߣ����Է���;���������л���ĺ�����лл������
----------------------------------------------------------------------------------------------------------------------*/
#include "Headfile.h"
#include "NamelessCorun_OpticalFlow.h"



#include <Time.h>


#define OpticalFlow_Ringbuf  COM6_Rx_Buf

uint8_t OpticalFlow_Is_Work=0;
float opticalflow_high=1000;//Ĭ��1m=100cm=1000mm
flow_float opt_data;
flow_integral_frame opt_origin_data;
Vector2f opt_filter_data; //�˲��������λ��
Vector2f opt_gyro_data;   //�������ٶ�
Vector2f opt_gyro_filter_data;//����������ת�����˲���Ľ��ٶ�
Vector2f gyro_filter_data;//��ͨͬ����λ�Ľ��ٶ�
Butter_Parameter OpticalFlow_Parameter,OpticalFlow_Gyro_Parameter;
Butter_BufferData Buffer_OpticalFlow[2],Buffer_OpticalFlow_Gyro[2];
void  OpticalFlow_CF(float flow_height,Vector2f accel,Vector2f flow);
void OpticalFlow_Init()
{
  Set_Cutoff_Frequency(50, 20,&OpticalFlow_Parameter);
  Set_Cutoff_Frequency(50, 8,&OpticalFlow_Gyro_Parameter);//ͬ����λ
}
uint8_t Optflow_Prase()//50hz
{
  if(FLOW_BUF_ONE_FRAME==14)//����LC306����ģ��
  {
    for(uint16_t i=0;i<=FLOW_BUF_ONE_FRAME;i++)
    {
      if(OpticalFlow_Ringbuf.Ring_Buff[i]==0xfe//֡ͷ
         &&OpticalFlow_Ringbuf.Ring_Buff[i+1]==0x0a//֡��
           &&OpticalFlow_Ringbuf.Ring_Buff[i+13]==0x55)//֡β
      {
        opt_origin_data.pixel_flow_x_integral=(int16_t)(OpticalFlow_Ringbuf.Ring_Buff[i+3]<<8)|OpticalFlow_Ringbuf.Ring_Buff[i+2];
        opt_origin_data.pixel_flow_y_integral=(int16_t)(OpticalFlow_Ringbuf.Ring_Buff[i+5]<<8)|OpticalFlow_Ringbuf.Ring_Buff[i+4];
        opt_origin_data.integration_timespan= (int16_t)(OpticalFlow_Ringbuf.Ring_Buff[i+7]<<8)|OpticalFlow_Ringbuf.Ring_Buff[i+6];
        opt_origin_data.qual=OpticalFlow_Ringbuf.Ring_Buff[i+12];      
        opt_filter_data.x=LPButterworth(opt_origin_data.pixel_flow_x_integral,&Buffer_OpticalFlow[0],&OpticalFlow_Parameter);
        opt_filter_data.y=LPButterworth(opt_origin_data.pixel_flow_y_integral,&Buffer_OpticalFlow[1],&OpticalFlow_Parameter);   
        opt_data.x=(opt_origin_data.pixel_flow_x_integral*opticalflow_high)/10000.0f;//��λ:���Ը߶ȵ�λmm��Ϊʵ��λ��mm
        opt_data.y=(opt_origin_data.pixel_flow_y_integral*opticalflow_high)/10000.0f;//��λ:���Ը߶ȵ�λmm��Ϊʵ��λ��mm     
        opt_data.dt=(int16_t)(opt_origin_data.integration_timespan*0.001f);//��λms
        opt_data.qual=opt_origin_data.qual;    
        opt_gyro_data.x=(opt_filter_data.x)/200.0f;//�������ٶ�rad/s
        opt_gyro_data.y=(opt_filter_data.y)/200.0f;//�������ٶ�rad/s         
        gyro_filter_data.x=LPButterworth(Roll_Gyro,&Buffer_OpticalFlow_Gyro[0],&OpticalFlow_Gyro_Parameter)/57.3f;//��������λͬ�����ٶ�
        gyro_filter_data.y=LPButterworth(Pitch_Gyro,&Buffer_OpticalFlow_Gyro[1],&OpticalFlow_Gyro_Parameter)/57.3f;//��������λͬ�����ٶ�
        opt_gyro_filter_data.x=OpticalFlow_Rotate_Complementary_Filter(opt_gyro_data.x,gyro_filter_data.x,'x');//�������ٶ��������ǽ��ٶ��ں� 
        opt_gyro_filter_data.y=OpticalFlow_Rotate_Complementary_Filter(opt_gyro_data.y,gyro_filter_data.y,'y'); //�������ٶ��������ǽ��ٶ��ں� 
        return 1;
      }
    }
  }
  else if(FLOW_BUF_ONE_FRAME==9)//��¥�Ƽ�����ģ��
  {
    for(uint16_t i=0;i<=FLOW_BUF_ONE_FRAME;i++)
    {
      if(OpticalFlow_Ringbuf.Ring_Buff[i]==0xfe//֡ͷ
         &&OpticalFlow_Ringbuf.Ring_Buff[i+1]==0x04//֡��
           &&OpticalFlow_Ringbuf.Ring_Buff[i+8]==0xAA)//֡β
      {
        opt_origin_data.sum=OpticalFlow_Ringbuf.Ring_Buff[i+2]+OpticalFlow_Ringbuf.Ring_Buff[i+3]
          +OpticalFlow_Ringbuf.Ring_Buff[i+4]+OpticalFlow_Ringbuf.Ring_Buff[i+5]; 
        if(opt_origin_data.sum==OpticalFlow_Ringbuf.Ring_Buff[i+6])
        {							
          opt_origin_data.pixel_flow_y_integral=(int16_t)(OpticalFlow_Ringbuf.Ring_Buff[i+3]<<8)|OpticalFlow_Ringbuf.Ring_Buff[i+2];
          opt_origin_data.pixel_flow_x_integral=(int16_t)(OpticalFlow_Ringbuf.Ring_Buff[i+5]<<8)|OpticalFlow_Ringbuf.Ring_Buff[i+4];
          opt_origin_data.pixel_flow_y_integral*=(-1);
          //opt_origin_data.integration_timespan= (int16_t)(OpticalFlow_Ringbuf.Ring_Buff[i+7]<<8)|OpticalFlow_Ringbuf.Ring_Buff[i+6];
          opt_origin_data.qual=OpticalFlow_Ringbuf.Ring_Buff[i+7]; 
        }
        
        opt_filter_data.x=LPButterworth(opt_origin_data.pixel_flow_x_integral,&Buffer_OpticalFlow[0],&OpticalFlow_Parameter);
        opt_filter_data.y=LPButterworth(opt_origin_data.pixel_flow_y_integral,&Buffer_OpticalFlow[1],&OpticalFlow_Parameter);   
        opt_data.x=(opt_origin_data.pixel_flow_x_integral*opticalflow_high)/10000.0f;//��λ:���Ը߶ȵ�λmm��Ϊʵ��λ��mm
        opt_data.y=(opt_origin_data.pixel_flow_y_integral*opticalflow_high)/10000.0f;//��λ:���Ը߶ȵ�λmm��Ϊʵ��λ��mm     
        //opt_data.dt=(int16_t)(opt_origin_data.integration_timespan*0.001f);//��λms
        opt_data.qual=opt_origin_data.qual;    
        //opt_gyro_data.x=0.5f*(opt_filter_data.x/11.914f) *2.54f;//�������ٶ�rad/s
        //opt_gyro_data.y=0.5f*(opt_filter_data.y/11.914f) *2.54f;//�������ٶ�rad/s  
        opt_gyro_data.x=opt_filter_data.x*0.106597f;//�������ٶ�rad/s
        opt_gyro_data.y=opt_filter_data.y*0.106597f;//�������ٶ�rad/s 				
        gyro_filter_data.x=LPButterworth(Roll_Gyro,&Buffer_OpticalFlow_Gyro[0],&OpticalFlow_Gyro_Parameter)/57.3f;//��������λͬ�����ٶ�
        gyro_filter_data.y=LPButterworth(Pitch_Gyro,&Buffer_OpticalFlow_Gyro[1],&OpticalFlow_Gyro_Parameter)/57.3f;//��������λͬ�����ٶ�
        opt_gyro_filter_data.x=OpticalFlow_Rotate_Complementary_Filter(opt_gyro_data.x,gyro_filter_data.x,'x');//�������ٶ��������ǽ��ٶ��ں� 
        opt_gyro_filter_data.y=OpticalFlow_Rotate_Complementary_Filter(opt_gyro_data.y,gyro_filter_data.y,'y'); //�������ٶ��������ǽ��ٶ��ں� 
        return 1;
      }
    }
  }
  return 0;
}


uint8_t Optflow_Is_Okay=0;
void Optflow_Statemachine(void)
{
  static uint16_t Optflow_Cnt=0;
  if(OpticalFlow_Is_Work==1)//����ʼ��ʱ�����ڹ�������
  {
    Optflow_Cnt++;
    if(Optflow_Cnt>=4)//20ms����һ��
    {
      Optflow_Is_Okay=Optflow_Prase();
      Optflow_Cnt=0;
    }
    OpticalFlow_CF(NamelessQuad.Position[_YAW],SINS_Accel_Body,opt_gyro_filter_data);
  }
}


float OpticalFlow_Rotate_Complementary_Filter(float optflow_gyro,float gyro,uint8_t axis)
{
  float optflow_gyro_filter=0;
  if(FLOW_BUF_ONE_FRAME==14)
  {
    if(axis=='x') optflow_gyro_filter=optflow_gyro-constrain_float(gyro,-4.0f,4.0f);//4
    else optflow_gyro_filter=optflow_gyro-constrain_float(gyro,-4.0f,4.0f);
  }
  else
  {
    if(axis=='x') optflow_gyro_filter=optflow_gyro-constrain_float(gyro,-10.0f,10.0f);//4
    else optflow_gyro_filter=optflow_gyro-constrain_float(gyro,-10.0f,10.0f);
  }
  return optflow_gyro_filter;
}





SINS OpticalFlow_SINS;
Testime Optical_Delta;
Vector2f OpticalFlow_Position={0};
Vector2f OpticalFlow_Speed={0};
Vector2f OpticalFlow_Speed_Err={0};
Vector2f OpticalFlow_Position_Err={0};
uint16_t Optflow_Sync_Cnt=5;
float CF_Parameter[2]={0.05f,0.0f};//���������˲�Ȩ�� 0.1  0.1   0.08  0
//����λ���ں�Ȩ�ظ�Ϊ0����ʾ����������λ����������Ϊ�ͳɱ�����ģ��Ư�ƽϴ�����Ը���Сֵ��0.2f
#define Optical_Output_Dt  0.02f//50hz
void  OpticalFlow_CF(float flow_height,Vector2f accel,Vector2f flow)
{
  float use_height=0;
  float optical_dt=0;
  Vector2f speed_delta={0};
  Test_Period(&Optical_Delta);
  optical_dt=Optical_Delta.Time_Delta/1000.0f;
  use_height = flow_height; 
  if(Optflow_Is_Okay==1)//�������ݹ�������ʱ��20msһ��
  {  
    OpticalFlow_Speed.x=flow.x*use_height;//�����ٶ�
    OpticalFlow_Speed.y=flow.y*use_height;//�����ٶ�
    OpticalFlow_Position.x+=OpticalFlow_Speed.x*Optical_Output_Dt;//����λ��
    OpticalFlow_Position.y+=OpticalFlow_Speed.y*Optical_Output_Dt;//����λ��
    //OpticalFlow_Speed_Err.x=OpticalFlow_Speed.x-OpticalFlow_SINS.Speed[_PITCH];
    //OpticalFlow_Speed_Err.y=OpticalFlow_Speed.y-OpticalFlow_SINS.Speed[_ROLL];
    //OpticalFlow_Position_Err.x=OpticalFlow_Position.x-OpticalFlow_SINS.Position[_PITCH];
    //OpticalFlow_Position_Err.y=OpticalFlow_Position.y-OpticalFlow_SINS.Position[_ROLL];
    Optflow_Is_Okay=0;
    OpticalFlow_Position_Err.x=OpticalFlow_Position.x-OpticalFlow_SINS.Pos_History[_PITCH][Optflow_Sync_Cnt];
    OpticalFlow_Position_Err.y=OpticalFlow_Position.y-OpticalFlow_SINS.Pos_History[_ROLL][Optflow_Sync_Cnt];
		OpticalFlow_Speed_Err.x=OpticalFlow_Speed.x-OpticalFlow_SINS.Vel_History[_PITCH][Optflow_Sync_Cnt];
    OpticalFlow_Speed_Err.y=OpticalFlow_Speed.y-OpticalFlow_SINS.Vel_History[_ROLL][Optflow_Sync_Cnt];
  }
  else
  {
    OpticalFlow_Speed_Err.x=0;
    OpticalFlow_Speed_Err.y=0;
    OpticalFlow_Position_Err.x=0;
    OpticalFlow_Position_Err.y=0;
  }
  
  OpticalFlow_SINS.Acceleration[_PITCH]=-accel.x;//�ߵ����ٶ��������ͷ
  OpticalFlow_SINS.Acceleration[_ROLL]=accel.y;//�ߵ����ٶ�������������ͷ�Ҳࣩ
  
	speed_delta.x=OpticalFlow_SINS.Acceleration[_PITCH]*optical_dt;
  speed_delta.y=OpticalFlow_SINS.Acceleration[_ROLL]*optical_dt;    
  
	OpticalFlow_SINS.Position[_PITCH]+=OpticalFlow_SINS.Speed[_PITCH]*optical_dt
    +0.5f*speed_delta.x*optical_dt+CF_Parameter[1]*OpticalFlow_Position_Err.x;
  OpticalFlow_SINS.Position[_ROLL]+=OpticalFlow_SINS.Speed[_ROLL]*optical_dt
    +0.5f*speed_delta.y*optical_dt+CF_Parameter[1]*OpticalFlow_Position_Err.y;

  OpticalFlow_SINS.Speed[_PITCH]+=OpticalFlow_SINS.Acceleration[_PITCH]*optical_dt+CF_Parameter[0]*OpticalFlow_Speed_Err.x;
  OpticalFlow_SINS.Speed[_ROLL]+=OpticalFlow_SINS.Acceleration[_ROLL]*optical_dt+CF_Parameter[0]*OpticalFlow_Speed_Err.y; 
   
	
	for(uint16_t i=Num-1;i>0;i--)
	{
		OpticalFlow_SINS.Pos_History[_ROLL][i]=OpticalFlow_SINS.Pos_History[_ROLL][i-1];
		OpticalFlow_SINS.Pos_History[_PITCH][i]=OpticalFlow_SINS.Pos_History[_PITCH][i-1];
		OpticalFlow_SINS.Vel_History[_ROLL][i]=OpticalFlow_SINS.Vel_History[_ROLL][i-1];
		OpticalFlow_SINS.Vel_History[_PITCH][i]=OpticalFlow_SINS.Vel_History[_PITCH][i-1]; 		
	}   
	OpticalFlow_SINS.Pos_History[_ROLL][0]=OpticalFlow_SINS.Position[_ROLL];
  OpticalFlow_SINS.Pos_History[_PITCH][0]=OpticalFlow_SINS.Position[_PITCH]; 
  OpticalFlow_SINS.Vel_History[_ROLL][0]=OpticalFlow_SINS.Speed[_ROLL];
  OpticalFlow_SINS.Vel_History[_PITCH][0]=OpticalFlow_SINS.Speed[_PITCH];  	 
}










