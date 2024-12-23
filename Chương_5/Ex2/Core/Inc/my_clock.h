#ifndef CLOCK_FSM_H
#define CLOCK_FSM_H

#include "stm32f4xx_hal.h"
#include "lcd.h"
#include "ds3231.h"
#include <stdbool.h>

// Enum định nghĩa các trạng thái chính
typedef enum {
    FSM_DISPLAY,
    FSM_CHANGE_TIME,
    FSM_ALARM
} ClockState_t;

typedef enum {
    BTN_RELEASED,
    BTN_PRESSED,
    BTN_LONG_PRESSED
} ButtonState_t;

typedef enum {
    CH_SECOND,
    CH_MINUTE,
    CH_HOUR,
    CH_DAY,
    CH_DATE,
    CH_MONTH,
    CH_YEAR
} TimeChange_t;

typedef enum {
    BLINK_ON,
    BLINK_OFF
} BlinkState_t;

// Cấu trúc dữ liệu lưu thời gian và báo thức
typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint8_t year;
} Time_t;
typedef enum {
    FSM_DISPLAY,
    FSM_CHANGE_TIME,
    FSM_ALARM,
    FSM_UPDATE_TIME,  // Thêm trạng thái cập nhật thời gian
} ClockState_t;



// Biến toàn cục
extern ClockState_t clock_state;
extern ButtonState_t button_state[3];
extern BlinkState_t blink_state;
extern TimeChange_t change_state;

extern Time_t current_time;
extern Time_t alarm_time;

// Hàm FSM chính
void Clock_FSM(void);

// Hàm liên quan đến FSM
void Update_DS3231(void);
void Display_Clock(void);
void Display_Alarm(void);
void Alarm_Check(void);
// Hàm mới
void UART_Request_Time(const char *request); // Gửi yêu cầu qua UART
uint8_t UART_Receive_Response(void);         // Nhận phản hồi từ UART


// Hàm xử lý nút nhấn
bool Button_Mode_FSM(void);
bool Button_Set_FSM(void);
bool Button_Increase_FSM(uint8_t *value);

// Hàm hỗ trợ khác
void Blink_Display(uint8_t *value, uint8_t max_val);

#endif
