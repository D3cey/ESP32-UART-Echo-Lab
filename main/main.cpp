#include "sdkconfig.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

#if defined(CONFIG_ROLE_SENDER)
    static const char *TAG = "UART-SENDER";
#elif defined(CONFIG_ROLE_ECHO)
    static const char *TAG = "UART-ECHO";
#endif

/* uart pins */
constexpr int UART_RX_PIN = 16;
constexpr int UART_TX_PIN = 17;

/* uart config*/
constexpr int UART_BAUD_RATE = 115200;
constexpr uart_port_t UART_PORT_NUM = UART_NUM_2;

constexpr int BUF_SIZE = 1024;
constexpr int TASK_STACK_SIZE = 3072;

class UARTController
{
private:
    int m_RX_pin;
    int m_TX_pin;
    uart_port_t m_UART_PORT_NUM;
    uint8_t m_data[BUF_SIZE];

public:
    UARTController(int RX_pin, int TX_pin) : m_RX_pin(RX_pin), m_TX_pin(TX_pin), m_UART_PORT_NUM(UART_PORT_NUM)
    {
        uart_config_t uart_config = {};
        uart_config.baud_rate = UART_BAUD_RATE;
        uart_config.data_bits = UART_DATA_8_BITS;
        uart_config.parity = UART_PARITY_DISABLE;
        uart_config.stop_bits = UART_STOP_BITS_1;
        uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;

        int intr_alloc_flags = 0;

        ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
        ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
        ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, m_TX_pin, m_RX_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    }

    static void task_entry(void *arg)
    {
        UARTController *self = static_cast<UARTController *>(arg);

#if defined(CONFIG_ROLE_SENDER)
        self->sender_task();
#elif defined(CONFIG_ROLE_ECHO)
        self->echo_task();
#endif
    }

    void sender_task()
    {
        const char *my_message = "PING";
        int my_message_len = strlen(my_message);
        TickType_t wait_ticks = pdMS_TO_TICKS(1000);
        TickType_t xLastWakeTime = xTaskGetTickCount();

        while (1)
        {
            uart_write_bytes(m_UART_PORT_NUM, (const char *)my_message, my_message_len);

            int len = uart_read_bytes(m_UART_PORT_NUM, m_data, (BUF_SIZE - 1), pdMS_TO_TICKS(20));

            if (len > 0)
            {
                m_data[len] = '\0';
                ESP_LOGI(TAG, "Recv str: %s", (char *)m_data);
            }

            vTaskDelayUntil(&xLastWakeTime, wait_ticks);
        }
    }

    void echo_task()
    {
        while (1)
        {
            int len = uart_read_bytes(m_UART_PORT_NUM, m_data, (BUF_SIZE - 1), pdMS_TO_TICKS(20));

            if (len > 0)
            {
                uart_write_bytes(m_UART_PORT_NUM, (const char *)m_data, len);
                m_data[len] = '\0';
                ESP_LOGI(TAG, "Recv str: %s", (char *)m_data);
            }
        }
    }
};

extern "C" void app_main(void)
{
    static UARTController uart_echo(UART_RX_PIN, UART_TX_PIN);

    xTaskCreate(UARTController::task_entry, "uart_echo_task", TASK_STACK_SIZE, &uart_echo, 10, NULL);
}