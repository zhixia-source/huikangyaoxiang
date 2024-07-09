#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

#include "hi_io.h"
#include "hi_stdlib.h"
#include "hi_time.h"
#include "hi_gpio.h"
#include "hi_uart.h"
#include "hi_errno.h"

#include "init.h"
#include "kv_store.h"

#include "hal_bsp_structAll.h"
#include "oled.h"

osThreadId_t main_task_id;
osThreadId_t uart_task_id;
osThreadId_t at_task_id;
osThreadId_t key_init_id;

#define TASK_STACK_SIZE (1024 * 5)

#define SEC_MAX 60
#define MIN_MAX 60
#define HOUR_MAX 24

#define TASK_DELAY_TIME (200 * 1000)
#define KEY HI_IO_NAME_GPIO_5
hi_gpio_value val, val_last; // GPIO的状态值
bool key_flag=0;
int choose;

extern system_value_t systemValue; // 系统全局变量,系统时间校准,增删药品
extern uint8_t numOfDrugs;         // 药品数量
extern Medicine1 Leixing1;         // 第一种药结构体
extern Medicine2 Leixing2;         // 第二种药结构体
extern Medicine_t ZZZ;             // 药品信息接受中转站
extern bool del_flag, time_flag;
extern bool med_name_flag, med_num_flag, med_time_flag, med_DayFreq_flag, med_numFreq_flag,red_flag1,red_flag2;
extern uint32_t red_time1,red_time2;

uint8_t oled_show_time;
uint8_t oledShowBuff[20] = {0};
#define ONE_MIN 58 // 一分钟计时
uint8_t hour = 00;
uint8_t min = 00;
uint8_t day = 00;
uint8_t month = 00;
uint8_t M;
uint8_t buzz_time;
uint16_t year = 0000;
uint8_t timeString[20];
bool ChiYaotime_flag = 0,chiyao_over_flag=0; // 吃药时间标志位
bool l610_flag=false;
const uint8_t daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
int isLeapYear(int year)
{
    // 如果年份能被4整除但不能被100整除，或者能被400整除，则为闰年
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}
bool compareTime(const char *TimeString, const char *med_time)
{
    int len = strlen(med_time);
    int i = 0;
    char HeDui_time[5];
    for (i = 0; i < len; i += 4)
    {
        strncpy_s(HeDui_time, sizeof(HeDui_time), &med_time[i], 4);
        int result = strcmp(TimeString, HeDui_time);
        if (result == 0)
        {
            return true; // 时间匹配
        }
    }
    return false; // 时间不匹配
}
/***********************************  解释  *********************************/
// SSD1306_ShowStr(0, 1,"1", 16);
// 列数（0~128 8大小为）   行数（0-4）8大小为0-7  显示的东西   字体大小   16大小下8列为一个字符
/********************************** 逻辑部分 **********************************/

/***********************************  OLED显示  *********************************/
void oled_show(void)
{
    printf("ENTER OLED_SHOW\r\n");
    OLED_ShowString(40, 0, "huikang", 8);
    sprintf_s((char *)oledShowBuff, sizeof(oledShowBuff), "Name:%s num:%d", Leixing1.med_name, Leixing1.med_num);
    OLED_ShowString(0, 1, oledShowBuff, 8);
    sprintf_s((char *)oledShowBuff, sizeof(oledShowBuff), "Df:%d     Nf:%d", Leixing1.med_DayFreq, Leixing1.med_numFreq);
    OLED_ShowString(0, 2, oledShowBuff, 8);
    sprintf_s((char *)oledShowBuff, sizeof(oledShowBuff), "Name:%s num:%d", Leixing2.med_name, Leixing2.med_num);
    OLED_ShowString(0, 4, oledShowBuff, 8);
    sprintf_s((char *)oledShowBuff, sizeof(oledShowBuff), "Df:%d     Nf:%d", Leixing2.med_DayFreq, Leixing2.med_numFreq);
    OLED_ShowString(0, 5, oledShowBuff, 8);
    sprintf_s((char *)oledShowBuff, sizeof(oledShowBuff), "  %4d-%02d-%02d", year, month, day);
    OLED_ShowString(0, 6, oledShowBuff, 8);
    sprintf_s((char *)oledShowBuff, sizeof(oledShowBuff), "    %02d:%02d   ", hour, min);
    OLED_ShowString(0, 7, oledShowBuff, 8);
}

/***********************************  时间处理  *********************************/
void time_task(void)
{
    if (time_flag)
    {
        printf("time_flag coming:\r\n");
        sscanf(systemValue.time, "%4u%2u%2u%2u%2u", &year, &M, &day, &hour, &min);
        month = M;
        time_flag = false;
    }
    uint8_t daysInCurrentMonth = daysInMonth[month - 1];
    if (hi_get_real_time() > ONE_MIN)
    {
        hi_set_real_time(0);
        min++;
        sprintf_s((char *)timeString, sizeof(timeString), "%02d%02d", hour, min);
        printf("nowTime:%s\r\n", timeString);
        ChiYaotime_flag = 1;
        if (min > (MIN_MAX - 1))
        {
            min = 0;
            hour++;
            sprintf_s((char *)timeString, sizeof(timeString), "%02d%02d", hour, min);
            printf("nowTime:%s\r\n", timeString);
        }

        if (hour > (HOUR_MAX - 1))
        {
            hour = 0;
            sprintf_s((char *)timeString, sizeof(timeString), "%02d%02d", hour, min);
            printf("nowTime:%s\r\n", timeString);
            day++;
            if (day > daysInCurrentMonth)
            {
                month++;
                if (month == 2 && isLeapYear(year))
                    daysInCurrentMonth++;
                day = 1;
                if (month > 12)
                {
                    year++;
                    month = 1;
                }
            }
        }
    }
}

/**
 * @brief 按键中断回调函数
 * @note   当按键按下的时候才会触发
 * @param  *arg:
 * @retval None
 */
hi_void gpio_callback(void)
{
    printf("key down...\r\n");
    hi_gpio_get_input_val(KEY, &val); // 获取GPIO引脚的状态
    key_flag=1;
    printf("gpio_callback: %s\r\n", val ? "HI_GPIO_VALUE_1" : "HI_GPIO_VALUE_0");
}

void key_init(void)
{
    printf("enter Task 1.......\r\n");

    uart_init();
    hi_gpio_init();                                            // GPIO初始化
    hi_io_set_pull(KEY, HI_IO_PULL_UP);                        // 设置GPIO上拉
    hi_io_set_func(KEY, HI_IO_FUNC_GPIO_5_GPIO);              // 设置IO14为GPIO功能
    hi_gpio_set_dir(KEY, HI_GPIO_DIR_IN);                      // 设置GPIO为输入模式
    hi_gpio_register_isr_function(KEY,                         // KEY按键引脚
                                  HI_INT_TYPE_EDGE,            // 下降沿检测
                                  HI_GPIO_EDGE_FALL_LEVEL_LOW, // 低电平时触发
                                  &gpio_callback,              // 触发后调用的回调函数
                                  NULL);                       // 回调函数的传参值
    char buff_call[255];
    while(1){
    hi_gpio_get_input_val(KEY, &val); // 获取GPIO引脚的状态
    if (val != val_last&&key_flag==1) {
            choose=(choose+1)%2;
            switch(choose)
            {
                case 0 :
                {
                     sprintf(buff_call, "ATH\r\n");  
                     hi_uart_write(HI_UART_IDX_2, buff_call, strlen(buff_call));  
                }
                break;
                case 1 :
                {
                         printf("keyValue: %s\r\n", val ? "HI_GPIO_VALUE_1" : "HI_GPIO_VALUE_0");
                    // if (!isAtSent1) {  
                        sprintf(buff_call, "AT\r\n");  
                        hi_uart_write(HI_UART_IDX_2, buff_call, strlen(buff_call));  
                        // isAtSent1 = true;  
                    // }  
                    sleep(1);
                    // if (!isCpin) {  
                        sprintf(buff_call, "AT+CPIN?\r\n");  
                        hi_uart_write(HI_UART_IDX_2, buff_call, strlen(buff_call));  
                        // isCpin = true;  
                    // }  
                    sleep(1);
                    // if (!isCsq) {  
                        sprintf(buff_call, "AT+CSQ\r\n");  
                        hi_uart_write(HI_UART_IDX_2, buff_call, strlen(buff_call));  
                    //     isCsq = true;  
                    // }  
                    sleep(1);
                    // if (!isCgreg) {  
                        sprintf(buff_call, "AT+CGREG?\r\n");  
                        hi_uart_write(HI_UART_IDX_2, buff_call, strlen(buff_call));  
                    //     isCgreg = true;  
                    // }  
                    sleep(1);  
                    // if (!isCavims) {  
                        sprintf(buff_call, "AT+CAVIMS=1\r\n");  
                        hi_uart_write(HI_UART_IDX_2, buff_call, strlen(buff_call));  
                    //     isCavims = true;  
                    // }  
                    sleep(1);
                    // if (!isGtsetgprs) {  
                        sprintf(buff_call, "AT+GTSET=\"GPRSFIRST\",0\r\n");  
                        hi_uart_write(HI_UART_IDX_2, buff_call, strlen(buff_call));  
                    //     isGtsetgprs = true;  
                    // }  
                    sleep(1);
                    // if (!isGtsetcall) {  
                        sprintf(buff_call, "AT+GTSET=\"CALLBREAK\",1\r\n");  
                        hi_uart_write(HI_UART_IDX_2, buff_call, strlen(buff_call));  
                    //     isGtsetcall = true;  
                    // }  
                    sleep(1);
                    // if (!isclip) {  
                        sprintf(buff_call, "AT+CLIP=1\r\n");  
                        hi_uart_write(HI_UART_IDX_2, buff_call, strlen(buff_call));  
                    //     isCgreg = true;  
                    // }  
                    sleep(1); 
                    // if (!isatd) {  
                        sprintf(buff_call, "ATD17832842632\r\n");  
                        hi_uart_write(HI_UART_IDX_2, buff_call, strlen(buff_call));  
                    //     isatd = true;  
                    // }  
                }  
                break;       
            }           
    val_last = val;
    key_flag=0;
    }
    usleep(TASK_DELAY_TIME); // 200ms sleep
  }
}

/***********************************  判断吃药  *********************************/
void chiyao(void)
{
    if ((hi_get_real_time() - buzz_time) > 5)
    {
        set_buzzer(false);
    }
    if (ChiYaotime_flag)
    {
        if (compareTime(timeString, Leixing1.med_time))
        {
            QuYao_Left(Leixing1.med_numFreq);
            Leixing1.med_num -= Leixing1.med_numFreq;
            buzz_time = hi_get_real_time();
            set_buzzer(true);
            red_flag1=true;
            red_time1=0;
            write_nvram();
        }
        if (compareTime(timeString, Leixing2.med_time))
        {
            QuYao_Right(Leixing2.med_numFreq);
            Leixing2.med_num -= Leixing2.med_numFreq;
            buzz_time = hi_get_real_time();
            set_buzzer(true);
            red_flag2=true;
            red_time2=0;
            write_nvram();
        }
        ChiYaotime_flag = false;    
    }
}
/***********************************  l610通讯  *********************************/
void at(void)
{
    static bool isAtSent = false;
    static bool isMipCall0Sent = false;
    static bool isMipCall1Sent = false;
    static bool isHmconSent = false;
    
    char buffer[256]; // 足够大的缓冲区来容纳所有AT指令
      if(!l610_flag)
   {
        sleep(7);
        if (!isAtSent) {  
            sprintf(buffer, "AT\r\n");  
            hi_uart_write(HI_UART_IDX_2, buffer, strlen(buffer));  
            isAtSent = true;  
        }  
        sleep(1);
        if (!isMipCall0Sent) {  
            sprintf(buffer, "AT+MIPCALL=0\r\n");  
            hi_uart_write(HI_UART_IDX_2, buffer, strlen(buffer));  
            isMipCall0Sent = true;  
        }  
        sleep(1);
        if (!isMipCall1Sent) {  
            sprintf(buffer, "AT+MIPCALL=1\r\n");  
            hi_uart_write(HI_UART_IDX_2, buffer, strlen(buffer));  
            isMipCall1Sent = true;  
        }  
        sleep(1);
        if (!isHmconSent) {  
            //恐雾121.36.42.100    117.78.5.125
            sprintf(buffer, "AT+HMCON=0,60,\"121.36.42.100\",\"1883\",\"66597e1e2ca97925e05df9b7_123456\",\"123456123456\",0\r\n");  
            hi_uart_write(HI_UART_IDX_2, buffer, strlen(buffer));  
            isHmconSent = true;  
            l610_flag=true;
        }  
        sleep(1);
        //恐雾 topic订阅 AT+HMSUB=0,"$oc/devices/66597e1e2ca97925e05df9b7_123456/sys/properties/report"
  }
}

/********************************** 主函数 **********************************/
void main_task(void)
{
    strcpy_s(systemValue.time, sizeof(systemValue.time), "202407021800");
    sscanf(systemValue.time,"%4u%2u%2u%2u%2u", &year, &M, &day, &hour, &min);
    month=M;
    while (1)
    {
        at();
        time_task();
        oled_show();
        chiyao();
        chiyao_panduan1(red_flag1);
        chiyao_panduan2(red_flag2);
        sleep(1);
    }
}

/********************************** 初始化 **********************************/
void SC_peripheral_init(void)
{
    /********************************** 外设初始化 **********************************/
    
    oled_init();
    OLED_Init();
    OLED_Clear();
    uart_init();   // 串口2初始化  GPIO 11  12
    Servos_Init(); // 舵机初始化 GPIO13bb       
    Servos_mid();  // 舵机归中
    read_nvram();  // 不掉电存储读取
    Buzzer_red_init(); //红外对管与蜂鸣器初始化

}
static void huikang(void)
{
    SC_peripheral_init();
    hi_set_real_time(0);
    /********************************** 创建线程 **********************************/
    osThreadAttr_t options;
    options.attr_bits = 0;
    options.cb_mem = NULL;
    options.cb_size = 0;
    options.stack_mem = NULL;
    options.stack_size = TASK_STACK_SIZE;

    options.name = "main_task";
    options.priority = osPriorityNormal1;
    main_task_id = osThreadNew((osThreadFunc_t)main_task, NULL, &options);
    if (main_task_id != NULL)
    {
        printf("ID = %d, Create main_task_id is OK!\r\n", main_task_id);
    }

    options.name = "uart_recv_task";
    options.priority = osPriorityNormal1;
    uart_task_id = osThreadNew((osThreadFunc_t)uart_recv_task, NULL, &options);
    if (uart_task_id != NULL)
    {
        printf("ID = %d, Create uart_task_id is OK!\r\n", uart_task_id);
    }
    
    options.name = "at";
    options.priority = osPriorityNormal1;
    at_task_id = osThreadNew((osThreadFunc_t)at, NULL, &options);
    if (at_task_id != NULL)
    {
        printf("ID = %d, Create at_task_id is OK!\r\n", at_task_id);
    }

    options.name = "key_init";
    options.priority = osPriorityNormal;
    key_init_id = osThreadNew((osThreadFunc_t)key_init, NULL, &options);
    if (key_init_id != NULL)
    {
        printf("ID = %d, Create key_init_id is OK!\r\n", key_init_id);
    }
}
SYS_RUN(huikang);