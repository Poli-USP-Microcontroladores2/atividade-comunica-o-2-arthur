#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>

#define TX_PERIOD_MS 5000

/* ----------------------------------------------------
 * Configuração das GPIOs
 * ---------------------------------------------------- */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec in_ptb0 = {
    .port = DEVICE_DT_GET(DT_NODELABEL(gpiob)),
    .pin  = 0,
    .dt_flags = GPIO_ACTIVE_HIGH
};

/* ----------------------------------------------------
 * UART
 * ---------------------------------------------------- */
static const struct device *uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

/* ----------------------------------------------------
 * Variáveis globais
 * ---------------------------------------------------- */
static atomic_t led_state = ATOMIC_INIT(false);
static struct k_mutex led_mutex;

/* ----------------------------------------------------
 * Callback de recepção UART
 * ---------------------------------------------------- */
static void uart_rx_cb(const struct device *dev, void *user_data)
{
    uint8_t c;
    while (uart_fifo_read(dev, &c, 1)) {
        if (c == '1') {
            atomic_set(&led_state, true);
        } else if (c == '0') {
            atomic_set(&led_state, false);
        }
    }
}

/* ----------------------------------------------------
 * Thread de transmissão UART (a cada 5 s)
 * ---------------------------------------------------- */
void uart_tx_thread(void *a, void *b, void *c)
{
    for (;;) {
        bool current = atomic_get(&led_state);
        uint8_t msg = current ? '1' : '0';
        uart_fifo_fill(uart_dev, &msg, 1);
        k_msleep(TX_PERIOD_MS);
    }
}

/* Criação da thread de transmissão (escopo global!) */
K_THREAD_DEFINE(uart_tx_tid, 512, uart_tx_thread, NULL, NULL, NULL, 2, 0, 0);

/* ----------------------------------------------------
 * Função principal
 * ---------------------------------------------------- */
int main(void)
{
    printk("Iniciando sincronismo via UART + PTB0\n");

    k_mutex_init(&led_mutex);

    /* Configura LED como saída inativa (ativo em nível alto) */
    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);

    /* Configura PTB0 como entrada com pull-down */
    gpio_pin_configure(in_ptb0.port, in_ptb0.pin, GPIO_INPUT | GPIO_PULL_DOWN);

    /* Inicializa UART */
    if (!device_is_ready(uart_dev)) {
        printk("UART não está pronta!\n");
        return 0;
    }

    uart_irq_callback_user_data_set(uart_dev, uart_rx_cb, NULL);
    uart_irq_rx_enable(uart_dev);

    bool last_input = false;

    for (;;) {
        bool input_high = gpio_pin_get_dt(&in_ptb0);

        /* Se PTB0 estiver alto, liga o LED e envia '1' */
        if (input_high && !last_input) {
            atomic_set(&led_state, true);
            uint8_t msg = '1';
            uart_fifo_fill(uart_dev, &msg, 1);
        }

        /* Se PTB0 estiver baixo e o LED estava ligado, envia '0' */
        if (!input_high && last_input) {
            atomic_set(&led_state, false);
            uint8_t msg = '0';
            uart_fifo_fill(uart_dev, &msg, 1);
        }

        last_input = input_high;

        /* Atualiza LED físico conforme estado global */
        bool led_on = atomic_get(&led_state);
        k_mutex_lock(&led_mutex, K_FOREVER);
        gpio_pin_set_dt(&led0, led_on ? 1 : 0);
        k_mutex_unlock(&led_mutex);

        k_msleep(50);
    }

    return 0;
}
