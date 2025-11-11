/*
 * Comunicação UART entre duas placas, sem loop de eco
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <string.h>

#define UART_DEVICE_NODE_SHELL DT_CHOSEN(zephyr_shell_uart)
#define UART_DEVICE_NODE_CHAT  DT_NODELABEL(uart0)

#define MSG_SIZE 64

K_MSGQ_DEFINE(uart_msgq_shell, MSG_SIZE, 10, 4);
K_MSGQ_DEFINE(uart_msgq_chat,  MSG_SIZE, 10, 4);

static const struct device *const uart_dev_shell = DEVICE_DT_GET(UART_DEVICE_NODE_SHELL);
static const struct device *const uart_dev_chat  = DEVICE_DT_GET(UART_DEVICE_NODE_CHAT);

static char rx_buf_shell[MSG_SIZE];
static int rx_buf_pos_shell;
static char rx_buf_chat[MSG_SIZE];
static int rx_buf_pos_chat;

/* ------------------- Funções auxiliares ------------------- */
void print_uart(const struct device *dev, const char *buf)
{
	int len = strlen(buf);
	for (int i = 0; i < len; i++) {
		uart_poll_out(dev, buf[i]);
	}
}

/* ------------------- Callbacks UART ------------------- */
void serial_cb_shell(const struct device *dev, void *user_data)
{
	uint8_t c;

	if (!uart_irq_update(dev) || !uart_irq_rx_ready(dev))
		return;

	while (uart_fifo_read(dev, &c, 1) == 1) {
		if ((c == '\n' || c == '\r') && rx_buf_pos_shell > 0) {
			rx_buf_shell[rx_buf_pos_shell] = '\0';
			k_msgq_put(&uart_msgq_shell, &rx_buf_shell, K_NO_WAIT);
			rx_buf_pos_shell = 0;
		} else if (rx_buf_pos_shell < (sizeof(rx_buf_shell) - 1)) {
			rx_buf_shell[rx_buf_pos_shell++] = c;
		}
	}
}

void serial_cb_chat(const struct device *dev, void *user_data)
{
	uint8_t c;

	if (!uart_irq_update(dev) || !uart_irq_rx_ready(dev))
		return;

	while (uart_fifo_read(dev, &c, 1) == 1) {
		if ((c == '\n' || c == '\r') && rx_buf_pos_chat > 0) {
			rx_buf_chat[rx_buf_pos_chat] = '\0';
			k_msgq_put(&uart_msgq_chat, &rx_buf_chat, K_NO_WAIT);
			rx_buf_pos_chat = 0;
		} else if (rx_buf_pos_chat < (sizeof(rx_buf_chat) - 1)) {
			rx_buf_chat[rx_buf_pos_chat++] = c;
		}
	}
}

/* ------------------- Threads ------------------- */

/* Envia do shell (PC) para a outra placa */
void shell_to_chat_thread(void)
{
	char msg[MSG_SIZE];
	while (1) {
		if (k_msgq_get(&uart_msgq_shell, &msg, K_FOREVER) == 0) {
			print_uart(uart_dev_chat, msg);
			print_uart(uart_dev_chat, "\r\n");
		}
	}
}

/* Recebe da outra placa e mostra no shell (somente exibe) */
void chat_to_shell_thread(void)
{
	char msg[MSG_SIZE];
	while (1) {
		if (k_msgq_get(&uart_msgq_chat, &msg, K_FOREVER) == 0) {
			print_uart(uart_dev_shell, "\r\n[Remote]: ");
			print_uart(uart_dev_shell, msg);
			print_uart(uart_dev_shell, "\r\n> ");
		}
	}
}

/* Criação das threads */
K_THREAD_DEFINE(shell_to_chat_tid, 1024, shell_to_chat_thread, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(chat_to_shell_tid, 1024, chat_to_shell_thread, NULL, NULL, NULL, 7, 0, 0);

/* ------------------- Função principal ------------------- */
int main(void)
{
	if (!device_is_ready(uart_dev_shell)) {
		printk("UART shell device not found!\n");
		return 0;
	}
	if (!device_is_ready(uart_dev_chat)) {
		printk("UART chat device not found!\n");
		return 0;
	}

	int ret = uart_irq_callback_user_data_set(uart_dev_shell, serial_cb_shell, NULL);
	if (ret < 0) {
		printk("Error setting shell callback: %d\n", ret);
		return 0;
	}
	uart_irq_rx_enable(uart_dev_shell);

	ret = uart_irq_callback_user_data_set(uart_dev_chat, serial_cb_chat, NULL);
	if (ret < 0) {
		printk("Error setting chat callback: %d\n", ret);
		return 0;
	}
	uart_irq_rx_enable(uart_dev_chat);

	print_uart(uart_dev_shell, "System ready. Type a message and press Enter.\r\n> ");

	return 0;
}
