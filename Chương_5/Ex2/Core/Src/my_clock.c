#include "my_clock.h"

// Biến toàn cục
ClockState_t clock_state = FSM_DISPLAY;
ButtonState_t button_state[3] = {BTN_RELEASED};
BlinkState_t blink_state = BLINK_OFF;
TimeChange_t change_state = CH_SECOND;

Time_t current_time = {23, 11, 20, 6, 20, 10, 23};
Time_t alarm_time = {12, 20, 6, 20, 10, 23};

/**
 * @brief FSM chính quản lý đồng hồ
 */
void Clock_FSM(void) {
    static uint8_t update_step = 0; // Bước cập nhật thời gian

    switch (clock_state) {
        case FSM_DISPLAY:
            Update_DS3231();
            Display_Clock();
            Alarm_Check();
            if (Button_Mode_FSM()) {
                clock_state = FSM_CHANGE_TIME;
                change_state = CH_SECOND;
            }
            if (is_button_pressed(1)) {  // Thêm nút bấm để vào chế độ cập nhật RS232
                clock_state = FSM_UPDATE_TIME;
                update_step = 0; // Reset bước cập nhật
            }
            break;

        case FSM_CHANGE_TIME:
            Blink_Display((uint8_t *)&current_time.sec, 59);
            if (Button_Set_FSM()) {
                clock_state = FSM_ALARM;
                change_state = CH_SECOND;
            }
            break;

        case FSM_ALARM:
            Blink_Display((uint8_t *)&alarm_time.min, 59);
            if (Button_Mode_FSM()) {
                clock_state = FSM_DISPLAY;
            }
            break;

        case FSM_UPDATE_TIME:
            switch (update_step) {
                case 0:
                    lcd_ShowStr(10, 10, "Updating hours...", WHITE, BLACK, 16, 0);
                    UART_Request_Time("Hours");
                    current_time.hour = UART_Receive_Response();
                    update_step++;
                    break;

                case 1:
                    lcd_ShowStr(10, 10, "Updating minutes...", WHITE, BLACK, 16, 0);
                    UART_Request_Time("Minutes");
                    current_time.min = UART_Receive_Response();
                    update_step++;
                    break;

                case 2:
                    lcd_ShowStr(10, 10, "Updating seconds...", WHITE, BLACK, 16, 0);
                    UART_Request_Time("Seconds");
                    current_time.sec = UART_Receive_Response();
                    update_step++;
                    break;

                case 3:
                    lcd_ShowStr(10, 10, "Updating day...", WHITE, BLACK, 16, 0);
                    UART_Request_Time("Day");
                    current_time.day = UART_Receive_Response();
                    update_step++;
                    break;

                case 4:
                    lcd_ShowStr(10, 10, "Updating month...", WHITE, BLACK, 16, 0);
                    UART_Request_Time("Month");
                    current_time.month = UART_Receive_Response();
                    update_step++;
                    break;

                case 5:
                    lcd_ShowStr(10, 10, "Updating year...", WHITE, BLACK, 16, 0);
                    UART_Request_Time("Year");
                    current_time.year = UART_Receive_Response();
                    update_step++;
                    break;

                default:
                    // Lưu dữ liệu vào DS3231
                    ds3231_Write(ADDRESS_SEC, current_time.sec);
                    ds3231_Write(ADDRESS_MIN, current_time.min);
                    ds3231_Write(ADDRESS_HOUR, current_time.hour);
                    ds3231_Write(ADDRESS_DAY, current_time.day);
                    ds3231_Write(ADDRESS_DATE, current_time.date);
                    ds3231_Write(ADDRESS_MONTH, current_time.month);
                    ds3231_Write(ADDRESS_YEAR, current_time.year);
                    lcd_ShowStr(10, 10, "Update complete", WHITE, BLACK, 16, 0);
                    clock_state = FSM_DISPLAY; // Quay lại trạng thái hiển thị
                    break;
            }
            break;
    }
}


/**
 * @brief Cập nhật dữ liệu từ DS3231
 */
void Update_DS3231(void) {
    current_time.sec = ds3231_sec;
    current_time.min = ds3231_min;
    current_time.hour = ds3231_hours;
    current_time.day = ds3231_day;
    current_time.date = ds3231_date;
    current_time.month = ds3231_month;
    current_time.year = ds3231_year;
}

/**
 * @brief Hiển thị thời gian hiện tại
 */
void Display_Clock(void) {
    dis_sec(current_time.sec, 1);
    dis_min(current_time.min, 1);
    dis_hour(current_time.hour, 1);
    dis_day(current_time.day, 1);
    dis_date(current_time.date, 1);
    dis_month(current_time.month, 1);
    dis_year(current_time.year, 1);
}

/**
 * @brief Hiển thị thời gian báo thức
 */
void Display_Alarm(void) {
    dis_sec(alarm_time.sec, 1);
    dis_min(alarm_time.min, 1);
    dis_hour(alarm_time.hour, 1);
    dis_day(alarm_time.day, 1);
    dis_date(alarm_time.date, 1);
    dis_month(alarm_time.month, 1);
    dis_year(alarm_time.year, 1);
}

/**
 * @brief Kiểm tra báo thức
 */
void Alarm_Check(void) {
    if (memcmp(&current_time, &alarm_time, sizeof(Time_t)) == 0) {
        lcd_ShowStr(10, 200, "WAKE UP", WHITE, RED, 24, 0);
    } else {
        lcd_ShowStr(10, 200, "WAKE UP", BLACK, BLACK, 24, 0);
    }
}

/**
 * @brief FSM xử lý nút chuyển chế độ
 */
bool Button_Mode_FSM(void) {
    if (is_button_pressed(0) == 1) {
        button_state[0] = BTN_PRESSED;
        return true;
    } else if (is_button_pressed(0) == 0) {
        button_state[0] = BTN_RELEASED;
    }
    return false;
}

/**
 * @brief FSM xử lý nút thiết lập giá trị
 */
bool Button_Set_FSM(void) {
    if (is_button_pressed(2) == 1) {
        // Lưu dữ liệu vào DS3231
        ds3231_Write(ADDRESS_SEC, current_time.sec);
        ds3231_Write(ADDRESS_MIN, current_time.min);
        ds3231_Write(ADDRESS_HOUR, current_time.hour);
        ds3231_Write(ADDRESS_DAY, current_time.day);
        ds3231_Write(ADDRESS_DATE, current_time.date);
        ds3231_Write(ADDRESS_MONTH, current_time.month);
        ds3231_Write(ADDRESS_YEAR, current_time.year);
        return true;
    }
    return false;
}

/**
 * @brief Hiển thị giá trị nhấp nháy
 */
void Blink_Display(uint8_t *value, uint8_t max_val) {
    if (blink_state == BLINK_ON) {
        dis_sec(*value, 1);
        blink_state = BLINK_OFF;
    } else {
        dis_sec(*value, 0);
        blink_state = BLINK_ON;
    }
    if (is_button_pressed(1)) {
        (*value)++;
        if (*value > max_val) *value = 0;
    }
}

void UART_Request_Time(const char *request) {
    char buffer[50];
    sprintf(buffer, "Request: %s\n", request); // Chuỗi yêu cầu gửi qua UART
    HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
}
uint8_t UART_Receive_Response(void) {
    char response[10];
    uint8_t value = 0;
    
    // Chờ dữ liệu từ máy tính
    HAL_UART_Receive(&huart1, (uint8_t *)response, sizeof(response), HAL_MAX_DELAY);
    
    // Chuyển chuỗi nhận được sang số nguyên
    value = atoi(response);
    return value;
}

