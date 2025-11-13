#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>

#define TX_PERIOD_MS 5000

/* ----------------------------------------------------
 * Configuração das GPIOs
 * ---------------------------------------------------- */
static const struct gpio_dt_spec leds[3] = {
    GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios),
};

/* Entradas PTB0, PTB1, PTB2 (ativo LOW, pull-up) */
static const struct gpio_dt_spec inputs[3] = {
    { .port = DEVICE_DT_GET(DT_NODELABEL(gpiob)), .pin = 0, .dt_flags = GPIO_ACTIVE_HIGH },
    { .port = DEVICE_DT_GET(DT_NODELABEL(gpiob)), .pin = 1, .dt_flags = GPIO_ACTIVE_HIGH },
    { .port = DEVICE_DT_GET(DT_NODELABEL(gpiob)), .pin = 2, .dt_flags = GPIO_ACTIVE_HIGH },
};

/* ----------------------------------------------------
 * UART
 * ---------------------------------------------------- */
static const struct device *uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

/* ----------------------------------------------------
 * Variáveis globais
 * ---------------------------------------------------- */
static atomic_t rx_state = ATOMIC_INIT(0);  // Valor hexadecimal recebido (0–F)
static struct k_mutex led_mutex;

/* ----------------------------------------------------
 * Função utilitária: converter caractere hex para número (0–15)
 * ---------------------------------------------------- */
static int hex_to_int(uint8_t c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    return 0;
}

/* ----------------------------------------------------
 * Callback de recepção UART
 * ---------------------------------------------------- */
static void uart_rx_cb(const struct device *dev, void *user_data)
{
    uint8_t c;
    while (uart_fifo_read(dev, &c, 1)) {
        /* Cada byte recebido é um valor hexadecimal de 0–F */
        int val = hex_to_int(c);
        atomic_set(&rx_state, val);
    }
}

/* ----------------------------------------------------
 * Thread de transmissão UART
 * ---------------------------------------------------- */
void uart_tx_thread(void *a, void *b, void *c)
{
    for (;;) {
        /* Monta o valor local (0–F) baseado nos PTBn (ativo LOW) */
        uint8_t val = 0;

        for (int i = 0; i < 3; i++) {
            int pin_val = gpio_pin_get_dt(&inputs[i]);
            bool local_active = (pin_val == 0);  // ativo LOW
            if (local_active) {
                val |= (1 << i);
            }
        }

        /* Envia como caractere hexadecimal */
        uint8_t hex_char;
        if (val < 10)
            hex_char = '0' + val;
        else
            hex_char = 'A' + (val - 10);

        uart_fifo_fill(uart_dev, &hex_char, 1);

        k_msleep(TX_PERIOD_MS);
    }
}

/* Criação da thread de transmissão */
K_THREAD_DEFINE(uart_tx_tid, 512, uart_tx_thread, NULL, NULL, NULL, 2, 0, 0);

/* ----------------------------------------------------
 * Função principal
 * ---------------------------------------------------- */
int main(void)
{
    printk("Iniciando sincronismo 3 canais via UART (ativo-LOW, pull-up)\n");

    k_mutex_init(&led_mutex);

    /* Configura LEDs */
    for (int i = 0; i < 3; i++) {
        gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
    }

    /* Configura entradas */
    for (int i = 0; i < 3; i++) {
        gpio_pin_configure(inputs[i].port, inputs[i].pin, GPIO_INPUT | GPIO_PULL_UP);
    }

    /* Inicializa UART */
    if (!device_is_ready(uart_dev)) {
        printk("UART não está pronta!\n");
        return 0;
    }

    uart_irq_callback_user_data_set(uart_dev, uart_rx_cb, NULL);
    uart_irq_rx_enable(uart_dev);

    uint8_t last_state = 0xFF;

    for (;;) {
        uint8_t led_mask = 0;
        uint8_t rx_val = atomic_get(&rx_state);

        /* Monta estado final: prioridade local */
        for (int i = 0; i < 3; i++) {
            int pin_val = gpio_pin_get_dt(&inputs[i]);
            bool local_active = (pin_val == 0);

            if (local_active) {
                led_mask |= (1 << i);
            } else {
                /* Se local inativo, segue remoto */
                if (rx_val & (1 << i))
                    led_mask |= (1 << i);
            }
        }

        /* Atualiza LEDs apenas se houver mudança */
        if (led_mask != last_state) {
            k_mutex_lock(&led_mutex, K_FOREVER);
            for (int i = 0; i < 3; i++) {
                gpio_pin_set_dt(&leds[i], (led_mask & (1 << i)) ? 1 : 0);
            }
            k_mutex_unlock(&led_mutex);
            last_state = led_mask;
        }

        k_msleep(10);
    }

    return 0;
}
