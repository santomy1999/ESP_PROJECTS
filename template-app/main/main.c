/*Sample program two test working of servo motor, buttons, leds using esp32
requires 2 leds, 2 buttons , 2 1k ohm resistors and a servo.
Communication to servo is established using mcpwm . 
*/



#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/mcpwm_prelude.h"

#define LED_PIN  21
#define LED_PIN_1  23
#define PUSH_BUTTON_PIN  18
#define PUSH_BUTTON_PIN_D 12

// Please consult the datasheet of your servo before changing the following parameters
#define SERVO_MIN_PULSEWIDTH_US 500  // Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH_US 2500  // Maximum pulse width in microsecond
#define SERVO_MIN_DEGREE        -100   // Minimum angle
#define SERVO_MAX_DEGREE        100    // Maximum angle

#define SERVO_PULSE_GPIO             2        // GPIO connects to the PWM signal line
#define SERVO_TIMEBASE_RESOLUTION_HZ 1000000  // 1MHz, 1us per tick
#define SERVO_TIMEBASE_PERIOD        20000    // 20000 ticks, 20ms


static const char *TAG = "example";

static inline uint32_t example_angle_to_compare(int angle)
{
    return (angle - SERVO_MIN_DEGREE) * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / (SERVO_MAX_DEGREE - SERVO_MIN_DEGREE) + SERVO_MIN_PULSEWIDTH_US;
}

void app_main(void)
{
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT); 
    gpio_set_direction(LED_PIN_1, GPIO_MODE_OUTPUT);   
    gpio_set_direction(PUSH_BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PUSH_BUTTON_PIN, GPIO_PULLUP_ONLY);
    gpio_set_direction(PUSH_BUTTON_PIN_D, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PUSH_BUTTON_PIN_D, GPIO_PULLUP_ONLY);


    ESP_LOGI(TAG, "Create timer and operator");
    mcpwm_timer_handle_t timer = NULL;
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = SERVO_TIMEBASE_RESOLUTION_HZ,
        .period_ticks = SERVO_TIMEBASE_PERIOD,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer));

    mcpwm_oper_handle_t operator = NULL;
    mcpwm_operator_config_t operator_config = {
        .group_id = 0, // operator must be in the same group to the timer
    };
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &operator));

    ESP_LOGI(TAG, "Connect timer and operator");
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(operator, timer));

    ESP_LOGI(TAG, "Create comparator and generator from the operator");
    mcpwm_cmpr_handle_t comparator = NULL;
    mcpwm_comparator_config_t comparator_config = {
        .flags.update_cmp_on_tez = true,
    };
    ESP_ERROR_CHECK(mcpwm_new_comparator(operator, &comparator_config, &comparator));

    mcpwm_gen_handle_t generator = NULL;
    mcpwm_generator_config_t generator_config = {
        .gen_gpio_num = SERVO_PULSE_GPIO,
    };
    ESP_ERROR_CHECK(mcpwm_new_generator(operator, &generator_config, &generator));

    // set the initial compare value, so that the servo will spin to the center position
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, example_angle_to_compare(0)));

    ESP_LOGI(TAG, "Set generator action on timer and compare event");
    // go high on counter empty
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_timer_event(generator,
                    MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),
                    MCPWM_GEN_TIMER_EVENT_ACTION_END()));
    // go low on compare threshold
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(generator,
                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator, MCPWM_GEN_ACTION_LOW),
                    MCPWM_GEN_COMPARE_EVENT_ACTION_END()));

    ESP_LOGI(TAG, "Enable and start timer");
    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));

    int angle = 0;
    int step = 2;
    while (1) {
        ESP_LOGI(TAG, "Angle of rotation: %d", angle);
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, example_angle_to_compare(angle)));
        //Add delay, since it takes time for servo to rotate, usually 200ms/60degree rotation under 5V power supply
        // vTaskDelay(pdMS_TO_TICKS(1));
        // if ((angle + step) > 60 || (angle + step) < -60) {
        //     step *= -1;
        // }
        // angle += step;

        if (gpio_get_level(PUSH_BUTTON_PIN) == 0)
        {  
            ESP_LOGI(TAG ,"Button1 state1: %d",gpio_get_level(PUSH_BUTTON_PIN));
            // if ((angle + step) > 60 || (angle + step) < -60) {
            // step *= -1;
            // }
            angle=angle+2;
            if ((angle) >=  90) {
                angle = 90;``
            }
            gpio_set_level(LED_PIN_1, 1);
            vTaskDelay(50);
            gpio_set_level(LED_PIN_1, 0);        
        }
        else if (gpio_get_level(PUSH_BUTTON_PIN_D) == 0)
        {  
            ESP_LOGI(TAG ,"Button2 state: %d",gpio_get_level(PUSH_BUTTON_PIN_D));
            // if ((angle + step) > 60 || (angle + step) < -60) {
            // step *= -1;
            // }
            angle=angle-2;
            if ((angle ) <= -90) {
                angle = -90;
            }
            gpio_set_level(LED_PIN, 1);
            vTaskDelay(50);
            gpio_set_level(LED_PIN, 0);        
        } 
        else
        {
            // ESP_LOGI(TAG ,"Button state2: %d",gpio_get_level(PUSH_BUTTON_PIN_D));
            gpio_set_level(LED_PIN, 0);
            gpio_set_level(LED_PIN_1, 0);        
        }

        vTaskDelay(10);
    }
} 
