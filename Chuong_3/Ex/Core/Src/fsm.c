#include "fsm.h"

/* Enum định nghĩa các trạng thái chính */
typedef enum {
    TRAFFIC_LIGHT,
    RED_ADJUSTMENT,
    YELLOW_ADJUSTMENT,
    GREEN_ADJUSTMENT,
    SET_VALUE,
    INCREASE_BY_1,
    INCREASE_BY_1_OVER_TIME
} SystemState;

typedef enum {
    RED_GREEN,
    RED_YELLOW,
    GREEN_RED,
    YELLOW_RED
} TrafficLightState;

typedef enum {
    RELEASED,
    PRESSED,
    LONG_PRESSED
} ButtonState;

/* Trạng thái hiện tại và trước đó */
static SystemState current_state = TRAFFIC_LIGHT;
static SystemState previous_state = TRAFFIC_LIGHT;

/* Trạng thái đèn giao thông */
static TrafficLightState tl_state = RED_GREEN;

/* Trạng thái nút bấm */
static ButtonState button_states[3] = { RELEASED, RELEASED, RELEASED };

/* Cấu hình thời gian ban đầu cho đèn */
#define RED_TIME_INIT       5
#define GREEN_TIME_INIT     3
#define YELLOW_TIME_INIT    2

/* Biến lưu thời gian */
static uint32_t red_time = RED_TIME_INIT;
static uint32_t green_time = GREEN_TIME_INIT;
static uint32_t yellow_time = YELLOW_TIME_INIT;

static uint32_t red_buffer = RED_TIME_INIT;
static uint32_t green_buffer = GREEN_TIME_INIT;
static uint32_t yellow_buffer = YELLOW_TIME_INIT;

static uint32_t timer1 = RED_TIME_INIT;
static uint32_t timer2 = GREEN_TIME_INIT;

/* Định nghĩa các khoảng thời gian */
#define TOGGLE_TIME         250   // 250ms
#define INCREASE_TIME       250   // 250ms
#define ONE_SECOND          1000  // 1s

/* Hàm điều khiển đèn giao thông */
void control_traffic_light1(uint8_t light_id, uint8_t red, uint8_t yellow, uint8_t green) {
    if (light_id == 0) {
        // Điều khiển đèn chính
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, red);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, yellow);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, green);
    } else if (light_id == 1) {
        // Điều khiển đèn phụ
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, red);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, yellow);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, green);
    }
}

/* FSM điều khiển đèn giao thông */
void traffic_light_fsm(void) {
    switch (tl_state) {
        case RED_GREEN:
            control_traffic_light(0, 1, 0, 0); // Đèn chính: đỏ
            control_traffic_light(1, 0, 0, 1); // Đèn phụ: xanh
            if (timer2 == 0) {
                timer2 = yellow_time;
                tl_state = RED_YELLOW;
            }
            break;

        case RED_YELLOW:
            control_traffic_light(0, 1, 0, 0); // Đèn chính: đỏ
            control_traffic_light(1, 0, 1, 0); // Đèn phụ: vàng
            if (timer2 == 0) {
                timer1 = green_time;
                timer2 = red_time;
                tl_state = GREEN_RED;
            }
            break;

        case GREEN_RED:
            control_traffic_light(0, 0, 0, 1); // Đèn chính: xanh
            control_traffic_light(1, 1, 0, 0); // Đèn phụ: đỏ
            if (timer1 == 0) {
                timer1 = yellow_time;
                tl_state = YELLOW_RED;
            }
            break;

        case YELLOW_RED:
            control_traffic_light(0, 0, 1, 0); // Đèn chính: vàng
            control_traffic_light(1, 1, 0, 0); // Đèn phụ: đỏ
            if (timer1 == 0) {
                timer1 = red_time;
                timer2 = green_time;
                tl_state = RED_GREEN;
            }
            break;
    }
}

/* FSM nút bấm */
void button_fsm(uint8_t button_id) {
    switch (button_states[button_id]) {
        case RELEASED:
            if (is_button_pressed(button_id)) {
                previous_state = current_state;
                switch (button_id) {
                    case 0: // Nút chuyển chế độ
                        if (current_state == TRAFFIC_LIGHT)
                            current_state = RED_ADJUSTMENT;
                        else if (current_state == RED_ADJUSTMENT)
                            current_state = YELLOW_ADJUSTMENT;
                        else if (current_state == YELLOW_ADJUSTMENT)
                            current_state = GREEN_ADJUSTMENT;
                        else
                            current_state = TRAFFIC_LIGHT;
                        break;

                    case 1: // Nút tăng giá trị
                        current_state = INCREASE_BY_1;
                        break;

                    case 2: // Nút xác nhận giá trị
                        current_state = SET_VALUE;
                        break;
                }
                button_states[button_id] = PRESSED;
            }
            break;

        case PRESSED:
            if (!is_button_pressed(button_id))
                button_states[button_id] = RELEASED;
            else if (is_button_long_pressed(button_id))
                button_states[button_id] = LONG_PRESSED;
            break;

        case LONG_PRESSED:
            if (!is_button_pressed(button_id))
                button_states[button_id] = RELEASED;
            break;
    }
}

/* FSM tổng */
void traffic_run(void) {
    switch (current_state) {
        case TRAFFIC_LIGHT:
            if (red_time != (green_time + yellow_time)) {
                // Báo lỗi nếu thời gian không hợp lệ
                control_traffic_light(0, 0, 0, 0);
                control_traffic_light(1, 0, 0, 0);
                lcd_ShowStr(40, 130, "SYSTEM ERROR!", RED, WHITE, 32, 1);
            } else {
                lcd_ShowStr(40, 130, "SYSTEM RUNNING", WHITE, BLACK, 32, 1);
                if (!is_timer_on(1)) {
                    if (timer1 > 0) timer1--;
                    if (timer2 > 0) timer2--;
                    set_timer(1, ONE_SECOND);
                }
                traffic_light_fsm();
            }
            break;

        case RED_ADJUSTMENT:
        case YELLOW_ADJUSTMENT:
        case GREEN_ADJUSTMENT:
            // Điều chỉnh thời gian cho từng màu
            button_fsm(1); // Xử lý nút tăng
            button_fsm(2); // Xử lý nút xác nhận
            break;

        case SET_VALUE:
            // Lưu giá trị mới
            if (previous_state == RED_ADJUSTMENT) red_time = red_buffer;
            else if (previous_state == YELLOW_ADJUSTMENT) yellow_time = yellow_buffer;
            else if (previous_state == GREEN_ADJUSTMENT) green_time = green_buffer;
            current_state = previous_state;
            break;

        case INCREASE_BY_1:
            if (previous_state == RED_ADJUSTMENT) red_buffer++;
            else if (previous_state == YELLOW_ADJUSTMENT) yellow_buffer++;
            else if (previous_state == GREEN_ADJUSTMENT) green_buffer++;
            current_state = previous_state;
            break;

        case INCREASE_BY_1_OVER_TIME:
            if (!is_timer_on(4)) {
                button_fsm(1); // Kiểm tra nút giữ
                if (previous_state == RED_ADJUSTMENT) red_buffer++;
                else if (previous_state == YELLOW_ADJUSTMENT) yellow_buffer++;
                else if (previous_state == GREEN_ADJUSTMENT) green_buffer++;
                set_timer(4, INCREASE_TIME);
            }
            break;
    }
    LCD_DisplayNum();
}
