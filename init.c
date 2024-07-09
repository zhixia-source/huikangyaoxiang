#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "init.h"

#include "hi_uart.h"
#include "hi_io.h"
#include "hi_gpio.h"
#include "hi_time.h"
#include "hi_stdlib.h"

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "cJSON.h"

#include "kv_store.h"

#include "hal_bsp_structAll.h"
#define Servos_gpio13   HI_IO_NAME_GPIO_13
#define Servos_gpio_idx_13   HI_GPIO_IDX_13
#define servos_time 600 * 1000
#define buzzer_gpio7 HI_IO_NAME_GPIO_7
#define buzzer_gpio_idx_7 HI_GPIO_IDX_7
#define red_gpio10 HI_IO_NAME_GPIO_10
#define red_gpio_idx_10 HI_GPIO_IDX_10
char recvBuff[512] = {0};
// char JsonBuff[512] = {0};
hi_u8 *pbuff = recvBuff;
uint32_t red_time1,red_time2;
system_value_t systemValue={0};
Medicine_t ZZZ = {0};
Medicine1 Leixing1 = {0};
Medicine2 Leixing2 = {0};
bool med_name_flag=0,med_num_flag=0,med_time_flag=0,med_DayFreq_flag=0,med_numFreq_flag=0,red_flag1=false,red_flag2=false;
bool del_flag=false,time_flag=false,search_flag=false;
uint8_t numOfDrugs= 0;
void uart_recv_task(void)
{
    printf("enter uart.\r\n");
    char uart_buff[40] = {0};
    hi_u8 last_len = 0;
    while (1) 
    {
        printf("enter uart panduan.\r\n");
        if (memset_s((char *)uart_buff, sizeof(recvBuff), 0, sizeof(uart_buff)) == 0) 
        {
            printf("enter uart 1.\r\n");
            hi_u32 len = hi_uart_read(HI_UART_IDX_2, uart_buff, sizeof(uart_buff));
            if (len > 0) 
            {
                printf("enter uart 2.\r\n");
                memcpy_s((char *)pbuff, len, (char *)uart_buff, len);
                pbuff += len;
                if (len < last_len) 
                {
                    pbuff = recvBuff; // 回到recvBuff的头位置
                    printf("buff: %s\r\n", recvBuff);       
                    parse_json_data(extract_json((const char *)recvBuff));
                
                    memset_s((char *)recvBuff, sizeof(recvBuff), 0, sizeof(recvBuff));
    
                }
                last_len = len;
            }
        }
    }
}

char* extract_json(const char* input)
 {
    // 查找字符串中第一个大括号的位置
    const char* start = strchr(input, '{');
    if (start == NULL) {
        printf("未找到JSON部分\n");
        return NULL;
    }
    // 查找字符串中最后一个大括号的位置
    const char* end = strrchr(input, '}');
    if (end == NULL) {
        printf("未找到JSON部分\n");
        return NULL;
    }
    // 计算JSON部分的长度
    size_t length = end - start + 1;
    // 分配内存用于保存提取的JSON部分
    char* json = (char*)malloc(length + 1);
    if (json == NULL) {
        printf("内存分配失败\n");
        return NULL;
    }
    // 复制JSON部分到新分配的内存中
    strncpy(json, start, length);
    json[length] = '\0'; // 结尾添加字符串结束符
    return json;
}

static void parse_json_data(const char *payload)
{
    /* 解析JSON数据 */
    cJSON *root = cJSON_Parse(extract_json((const char *)recvBuff));
    if (root == NULL) {
        printf("Failed to parse JSON data.\n");
    }
    if (root) 
    {
/****************************药品信息解析**********************************/
        cJSON *paras = cJSON_GetObjectItem(root, "paras");
        if (paras) 
        {
            // // 判断药品查询
            // cJSON *jion_search = cJSON_GetObjectItem(paras, "search");
            // if (jion_search) {
            //     if (cJSON_IsBool(jion_search)) {
            //          search_flag = cJSON_IsTrue(jion_search);
            //         printf("jion_med_delete: %s\n", search_flag ? "true" : "false");
            //         char buffer[256]; 
            //         if(search_flag){
            //             if(numOfDrugs==2)
            //         {
            //             sprintf(buffer, "AT+HMPUB=1,$oc/devices/66597e1e2ca97925e05df9b7_123456/sys/properties/report",76,
            //             "{\"services\":[{\"service_id\":\"medicine\",\"properties\":{\"med_name1\":%s},{\"med_num1\":%d},{\"med_time1\":%s},{\"med_numFreq1\":%d},{\"med_DayFreq1\":%d}}]},0\r\n"
            //             ,Leixing1.med_name,Leixing1.med_num,Leixing1.med_time,Leixing1.med_numFreq,Leixing1.med_DayFreq);  
            //             hi_uart_write(HI_UART_IDX_2, buffer, strlen(buffer));  
            //             sleep(1);
            //             sprintf(buffer, "AT+HMPUB=1,$oc/devices/66597e1e2ca97925e05df9b7_123456/sys/properties/report",76,
            //             "{\"services\":[{\"service_id\":\"medicine\",\"properties\":{\"med_name2\":%s},{\"med_num2\":%d},{\"med_time2\":%s},{\"med_numFreq2\":%d},{\"med_DayFreq2\":%d}}]},0\r\n"
            //             ,Leixing2.med_name,Leixing2.med_num,Leixing2.med_time,Leixing2.med_numFreq,Leixing2.med_DayFreq);
            //             hi_uart_write(HI_UART_IDX_2, buffer, strlen(buffer));  
            //         }
            //         else if(numOfDrugs==1)
            //         {
            //             sleep(1);
            //             sprintf(buffer, "AT+HMPUB=1,$oc/devices/66597e1e2ca97925e05df9b7_123456/sys/properties/report",76,
            //             "{\"services\":[{\"service_id\":\"medicine\",\"properties\":{\"med_name1\":%s},{\"med_num1\":%d},{\"med_time1\":%s},{\"med_numFreq1\":%d},{\"med_DayFreq1\":%d}}]},0\r\n"
            //             ,Leixing1.med_name,Leixing1.med_num,Leixing1.med_time,Leixing1.med_numFreq,Leixing1.med_DayFreq); 
            //             hi_uart_write(HI_UART_IDX_2, buffer, strlen(buffer));  
            //         }else search_flag = false;
            //         }                   
            //     }
            // }
            
            //判断时间校准
            cJSON *json_time = cJSON_GetObjectItem(paras, "time");
            if (json_time != NULL) {
            strcpy_s(systemValue.time,sizeof(systemValue.time),json_time->valuestring);
            time_flag=1;
            printf("time: %s\r\n",systemValue.time );
            json_time = NULL;
            }
            //判断是否删除药品
            cJSON *jion_delete = cJSON_GetObjectItem(paras, "delete");
            if (jion_delete) {
                if (cJSON_IsBool(jion_delete)) {
                     del_flag = cJSON_IsTrue(jion_delete);
                    printf("jion_med_delete: %s\n", del_flag ? "true" : "false"); 
                            if(del_flag){
            if(numOfDrugs==2)
                {
                    memset_s(&Leixing2,sizeof(Leixing2),0,sizeof(Leixing2));
                    del_flag = false;
                    numOfDrugs--;
                    write_nvram();
                }
                else if(numOfDrugs==1)
                {
                    memset_s(&Leixing1,sizeof(Leixing1),0,sizeof(Leixing1));
                    del_flag = false;
                    numOfDrugs--;
                    write_nvram();
                }else del_flag = false;
                }
                        }
            }
            // 解析药名
            cJSON* med_name = cJSON_GetObjectItem(paras, "med_name");
            if (med_name) {
                strcpy_s(ZZZ.name,sizeof(ZZZ.name),med_name->valuestring);
                printf("med_name: %s\r\n", med_name->valuestring);
                med_name_flag=1;
            }
            // 解析数量
            cJSON* med_num = cJSON_GetObjectItem(paras, "med_num");
            if (med_num) {
                ZZZ.num = med_num->valueint;
                printf("med_num: %d\r\n", med_num->valueint);
                med_num_flag=1;
            }
            // 解析吃药时间
            cJSON* med_time = cJSON_GetObjectItem(paras, "med_time");
            if (med_time) {
                strcpy_s(ZZZ.timing,sizeof(ZZZ.timing),med_time->valuestring);
                printf("med_time: %s\r\n", med_time->valuestring);
                med_time_flag=1;
            }
            // 解析每天吃几次
            cJSON* med_DayFreq = cJSON_GetObjectItem(paras, "med_DayFreq");
            if (med_DayFreq) {
                ZZZ.DayFreq = med_DayFreq->valueint;
                printf("med_DayFreq: %d\r\n", med_DayFreq->valueint);
                med_DayFreq_flag=1;
            }
            // 解析一次吃几颗
            cJSON* med_numFreq = cJSON_GetObjectItem(paras, "med_numFreq");
            if (med_numFreq) {
                ZZZ.numFreq = med_numFreq->valueint;
                printf("med_numFreq: %d\r\n", med_numFreq->valueint);
                med_numFreq_flag=1;
            }
            paras = NULL;
        }
    }   
    cJSON_Delete(root);
    if(numOfDrugs<2)
    {
        if(med_name_flag==1&&med_num_flag==1&&med_time_flag==1&&med_DayFreq_flag==1&&med_numFreq_flag==1)
        {
            numOfDrugs++;
            med_name_flag=0;
            med_num_flag=0;
            med_time_flag=0;
            med_DayFreq_flag=0;
            med_numFreq_flag=0;
            if(numOfDrugs==1)
            {
                strcpy_s(Leixing1.med_name,sizeof(Leixing1.med_name),ZZZ.name);
                Leixing1.med_num=ZZZ.num;
                strcpy_s(Leixing1.med_time,sizeof(Leixing1.med_time),ZZZ.timing);
                Leixing1.med_numFreq=ZZZ.numFreq;
                Leixing1.med_DayFreq=ZZZ.DayFreq;
                Medicine_t ZZZ = {0};
                write_nvram();
            }
            if(numOfDrugs==2)
            {
                strcpy_s(Leixing2.med_name,sizeof(Leixing2.med_name),ZZZ.name);
                Leixing2.med_num=ZZZ.num;
                strcpy_s(Leixing2.med_time,sizeof(Leixing2.med_time),ZZZ.timing);
                Leixing2.med_numFreq=ZZZ.numFreq;
                Leixing2.med_DayFreq=ZZZ.DayFreq;
                Medicine_t ZZZ = {0};
                write_nvram();
            }
        }  
    }else if (numOfDrugs>2)memset_s(&ZZZ,sizeof(ZZZ),0,sizeof(ZZZ));
    
    root = NULL;
}

void uart_init(void)
{
    uint32_t ret = 0;
    // 初始化串口
    hi_io_set_func(HI_IO_NAME_GPIO_11, HI_IO_FUNC_GPIO_11_UART2_TXD);
    hi_io_set_func(HI_IO_NAME_GPIO_12, HI_IO_FUNC_GPIO_12_UART2_RXD);

    hi_uart_attribute uart_param = {
        .baud_rate = 115200,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = 0,
    };
    ret = hi_uart_init(HI_UART_IDX_2, &uart_param, NULL);
    if (ret != HI_ERR_SUCCESS) {
        printf("hi uart init is faild.\r\n");
    }
    printf("hi uart init is ok.\r\n");
}

void uart_send_buff(unsigned char *str, unsigned short len)
{
    hi_u32 ret = 0;
    ret = hi_uart_write(HI_UART_IDX_2, (uint8_t *)str, len);
    if (ret == HI_ERR_FAILURE) {
        printf("uart send buff is failed.\r\n");
    }
}

void Servos_Init(void)
{
    hi_gpio_init();
    hi_io_set_func(Servos_gpio13, HI_IO_FUNC_GPIO_13_GPIO);
    hi_gpio_set_dir(Servos_gpio_idx_13, HI_GPIO_DIR_OUT);
    hi_io_set_driver_strength(Servos_gpio13,HI_IO_DRIVER_STRENGTH_0);
}

void SetAngle(unsigned int angle)
{
    uint16_t duty;
    duty = 2000 *angle / 180  + 500; 
    hi_gpio_set_ouput_val(Servos_gpio13, HI_GPIO_VALUE1);
    hi_udelay(duty);
    hi_gpio_set_ouput_val(Servos_gpio13, HI_GPIO_VALUE0);
    hi_udelay(20000 - duty);  
}

void Servos_mid()
{
    SetAngle(85);
}

void QuYao_Right(unsigned int i)
{
    unsigned int a = 0;
    while(a<i)
    {
      usleep(servos_time);
      SetAngle(27);
      usleep(servos_time);
      Servos_mid();
      usleep(servos_time);
      a++;
    } 
    usleep(600*1000);
}

void QuYao_Left(unsigned int i)
{   
    unsigned int a = 0;
    while(a<i)
    {
      usleep(servos_time);
      SetAngle(145);
      usleep(servos_time);
      Servos_mid();
      usleep(servos_time);
      a++;
    }   
    usleep(600*1000);
}

void read_nvram(void)
{
     // 从持久化存储中读取药品数据
                    //leixing1
                    char restored_data[256];
                    int len = UtilsGetValue("medicine_data", restored_data, sizeof(restored_data));
                    if (len > 0) {
                    // 将字符串解析回 leixng1 结构体
                    sscanf(restored_data, "%[^,],%d,%[^,],%d,%d",
                    Leixing1.med_name, &Leixing1.med_num, Leixing1.med_time, &Leixing1.med_DayFreq, &Leixing1.med_numFreq);
                    } else {

                    }
                    //leixing2
                    char restored_data2[256];
                    int len2 = UtilsGetValue("medicine_data2", restored_data2, sizeof(restored_data2));
                    if (len2 > 0) {
                    // 将字符串解析回 leixng2 结构体
                    sscanf(restored_data2, "%[^,],%d,%[^,],%d,%d",
                    Leixing2.med_name, &Leixing2.med_num, Leixing2.med_time, &Leixing2.med_DayFreq, &Leixing2.med_numFreq);
                    } else {
                        printf("Failed to restore medicine2 data, error code: %d\n", len2);
                    }

                    char restored_data3[10];
                    int len3 = UtilsGetValue("num_of_drugs", restored_data3, sizeof(restored_data3));
                    if (len3 > 0) {
                    // 将字符串解析回 numOfDrugs 结构体
                    sscanf(restored_data3, "%d",&numOfDrugs);
                    } else {
                        printf("Failed to restore num_of_drugs data, error code: %d\n", len3);
                    }
}

void write_nvram(void)
{
       //leixing1
        char num_of_drugs[10];
        snprintf(num_of_drugs, sizeof(num_of_drugs), "%d",numOfDrugs);
        char medicine_data[256];
        snprintf(medicine_data, sizeof(medicine_data), "%s,%d,%s,%d,%d",Leixing1.med_name, Leixing1.med_num, Leixing1.med_time, Leixing1.med_DayFreq, Leixing1.med_numFreq);
        //leixing2
        char medicine_data2[256];
        snprintf(medicine_data2, sizeof(medicine_data2), "%s,%d,%s,%d,%d",Leixing2.med_name, Leixing2.med_num, Leixing2.med_time, Leixing2.med_DayFreq, Leixing2.med_numFreq);
        // 使用 UtilsSetValue() 函数保存数据到持久化存储
        
        //leixing1
        int ret = UtilsSetValue("medicine_data", medicine_data);
        if (ret != 0) {
            printf("Failed to save medicine data, error code: %d\n", ret);
        } else {
            printf("Medicine data1 saved successfully.\n");
        }
        //keixing2
        int ret2 = UtilsSetValue("medicine_data2", medicine_data2);
        if (ret2 != 0) {
            printf("Failed to save medicine data2, error code: %d\n", ret2);
        } else {
            printf("Medicine data2 saved successfully.\n");
        }
        
        //药品数量
        int ret3 = UtilsSetValue("num_of_drugs", num_of_drugs);
        if (ret3 != 0) {
            printf("Failed to save num_of_drugs data, error code: %d\n", ret3);
        } else {
            printf("Medicine num_of_drugs saved successfully.\n");
        }
}

void Buzzer_red_init(void)
{
    hi_io_set_pull(buzzer_gpio7, HI_IO_PULL_UP);                        // 设置GPIO3为蜂鸣器
    hi_io_set_func(buzzer_gpio7, HI_IO_FUNC_GPIO_7_GPIO);             
    hi_gpio_set_dir(buzzer_gpio7, HI_GPIO_DIR_OUT);
    hi_io_get_driver_strength(buzzer_gpio7,0);
    hi_io_set_pull(red_gpio10, HI_IO_PULL_UP);                           //设置gpio4为红外对管
    hi_io_set_func(red_gpio10, HI_IO_FUNC_GPIO_10_GPIO);             
    hi_gpio_set_dir(red_gpio10, HI_GPIO_DIR_IN);
}

void set_buzzer(bool a)
{
    if(a)
    {
        hi_gpio_set_ouput_val(HI_GPIO_IDX_7,HI_GPIO_VALUE1);
    }
    else if(!a)
    {
        hi_gpio_set_ouput_val(HI_GPIO_IDX_7,HI_GPIO_VALUE0);
    }
}

void chiyao_panduan1(bool a)
{
    int val;
    if(a)
    {
        sleep(2);
        hi_gpio_get_input_val(red_gpio_idx_10,&val);
        if(val==0)
        {
            red_time1++;    
        } 
        if(val==1)
        {
            if(red_time1>5)//药盒取出大于5秒视为吃了药
            {
                chiyao_over1();
                red_flag1=false;
                red_time1=0;
            }else if(red_time1<5)//传感器出问题药盒未被取出
            {
                chiyao_noover1();
                red_flag1=false;
                red_time1=0;
            }
        }
    } 
}

void chiyao_panduan2(bool a)
{
    int asc;
    if(a)
    {
        sleep(2);
        hi_gpio_get_input_val(red_gpio_idx_10,&asc);
        if(asc==0)
        {
            red_time2++;     
        } 
        if(asc==1)
        {
            if(red_time2>5)//药盒取出大于5秒视为吃了药
            {
                chiyao_over2();
                red_flag2=false;
                red_time2=0;
            }else if(red_time2<5)//传感器出问题药盒未被取出
            {
                chiyao_noover2();
                red_flag2=false;
                red_time2=0;
            }
        }
    } 
}


void chiyao_over1(void)
{
char over_buffer[256];
  sprintf(over_buffer, "AT+HMPUB=1,\"$oc/devices/66597e1e2ca97925e05df9b7_123456/sys/properties/report\",68,\"{\\\"services\\\":[{\\\"service_id\\\":\\\"medicine\\\",\\\"properties\\\":{\\\"over1\\\":true}}]}\"\r\n");
  hi_uart_write(HI_UART_IDX_2, over_buffer, strlen(over_buffer)); 
  printf("6.\n"); 
}

void chiyao_noover1(void)
{
char over_buffer[256];
  sprintf(over_buffer, "AT+HMPUB=1,\"$oc/devices/66597e1e2ca97925e05df9b7_123456/sys/properties/report\",69,\"{\\\"services\\\":[{\\\"service_id\\\":\\\"medicine\\\",\\\"properties\\\":{\\\"over1\\\":false}}]}\"\r\n");   
  hi_uart_write(HI_UART_IDX_2, over_buffer, strlen(over_buffer));  
  printf("7.\n");  
}

void chiyao_noover2(void)
{
char over_buffer[256];
  sprintf(over_buffer, "AT+HMPUB=1,\"$oc/devices/66597e1e2ca97925e05df9b7_123456/sys/properties/report\",69,\"{\\\"services\\\":[{\\\"service_id\\\":\\\"medicine\\\",\\\"properties\\\":{\\\"over2\\\":false}}]}\"\r\n");
  hi_uart_write(HI_UART_IDX_2, over_buffer, strlen(over_buffer)); 
  hi_uart_write(HI_UART_IDX_2, over_buffer, strlen(over_buffer));  
}

void chiyao_over2(void)
{
char over_buffer[256];
  sprintf(over_buffer, "AT+HMPUB=1,\"$oc/devices/66597e1e2ca97925e05df9b7_123456/sys/properties/report\",68,\"{\\\"services\\\":[{\\\"service_id\\\":\\\"medicine\\\",\\\"properties\\\":{\\\"over2\\\":true}}]}\"\r\n");
  hi_uart_write(HI_UART_IDX_2, over_buffer, strlen(over_buffer)); 
}