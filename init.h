#ifndef INIT_H
#define INIT_H

#include "cmsis_os2.h"
#include <stdbool.h>

typedef struct _system_value {
    char time[20];               //时间校准
    bool add_med;               //添加药品标识符
    bool delete_med;               //删除药品标识符
    // char name[50];    //药名
    // int num;          //药的数量
    // char timing[50];    //吃药的时间
    // int DayFreq;      //一天吃几次
    // int numFreq;      //一次吃几颗 
} system_value_t;


//第一种药
typedef struct{
    char med_name[20];
    int med_num;
    char med_time[50];
    int med_DayFreq;
    int med_numFreq;
} Medicine1; 
//第二种药
typedef struct {
    char med_name[20];
    int med_num;
    char med_time[50];
    int med_DayFreq;
    int med_numFreq;
} Medicine2;
typedef struct Medicine{
    char name[20];      //药名
    int num;                     //药的数量
    char timing[50];    //吃药的时间
    int DayFreq;                 //一天吃几次
    int numFreq;                 //一次吃几颗 
    // int udp_socket_fd;           // UDP通信的套接字
} Medicine_t;

extern system_value_t systemValue; // 系统全局变量
extern uint8_t numOfDrugs;
extern Medicine1 Leixing1;
extern Medicine2 Leixing2;
extern Medicine_t ZZZ;
// extern system_value_t systemValue[];


void Servos_Init(void);
void SetAngle(unsigned int angle);
void Servos_mid(void);
void QuYao_Right(unsigned int i);
void QuYao_Left(unsigned int i);
void uart_init(void);
void uart_send_buff(unsigned char *str, unsigned short len);
void uart_recv_task(void);
static void parse_json_data(const char *payload);
char* extract_json(const char* input);
char* extract_json(const char* input);
void read_nvram(void);
void write_nvram(void);
void Buzzer_red_init(void);
void set_buzzer(bool a);
void chiyao_panduan1(bool a);
void chiyao_panduan2(bool a);
void chiyao_over1(void);
void chiyao_over2(void);

#endif