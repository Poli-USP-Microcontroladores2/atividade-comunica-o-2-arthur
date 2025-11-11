/*
 * Copyright (c) 2022 Libre Solar Technologies GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

#include <string.h>

/* change this to any other UART peripheral if desired */
#define UART_DEVICE_NODE_SHELL DT_CHOSEN(zephyr_shell_uart)
#define UART_DEVICE_NODE_CHAT DT_NODELABEL(uart0)

#define MSG_SIZE 32

/* queue to store up to 10 messages (aligned to 4-byte boundary) */
K_MSGQ_DEFINE(uart_msgq_shell, MSG_SIZE, 10, 4);
K_MSGQ_DEFINE(uart_msgq_chat, MSG_SIZE, 10, 4);

static const struct device *const uart_dev_shell = DEVICE_DT_GET(UART_DEVICE_NODE_SHELL);
static const struct device *const uart_dev_chat = DEVICE_DT_GET(UART_DEVICE_NODE_CHAT);

/* receive buffer used in UART ISR callback */
static char rx_buf_shell[MSG_SIZE];
static int rx_buf_pos_shell;
static char rx_buf_chat[MSG_SIZE];
static int rx_buf_pos_chat;

/*
 * Read characters from UART until line end is detected. Afterwards push the
 * data to the message queue.
 */
void serial_cb_shell(const struct device *dev, void *user_data)
{
	uint8_t c;

	if (!uart_irq_update(dev)) {
		return;
	}

	if (!uart_irq_rx_ready(dev)) {
		return;
	}

	/* read until FIFO empty */
	while (uart_fifo_read(dev, &c, 1) == 1) {
		if ((c == '\n' || c == '\r') && rx_buf_pos_shell > 0) {
			/* terminate string */
			rx_buf_shell[rx_buf_pos_shell] = '\0';

			/* if queue is full, message is silently dropped */
			k_msgq_put(&uart_msgq_shell, &rx_buf_shell, K_NO_WAIT);

			/* reset the buffer (it was copied to the msgq) */
			rx_buf_pos_shell = 0;
		} else if (rx_buf_pos_shell < (sizeof(rx_buf_shell) - 1)) {
			rx_buf_shell[rx_buf_pos_shell++] = c;
		}
		/* else: characters beyond buffer size are dropped */
	}
}

void serial_cb_chat(const struct device *dev, void *user_data)
{
	uint8_t c;

	if (!uart_irq_update(dev)) {
		return;
	}

	if (!uart_irq_rx_ready(dev)) {
		return;
	}

	/* read until FIFO empty */
	while (uart_fifo_read(dev, &c, 1) == 1) {
		if ((c == '\n' || c == '\r') && rx_buf_pos_chat > 0) {
			/* terminate string */
			rx_buf_chat[rx_buf_pos_chat] = '\0';

			/* if queue is full, message is silently dropped */
			k_msgq_put(&uart_msgq_chat, &rx_buf_chat, K_NO_WAIT);

			/* reset the buffer (it was copied to the msgq) */
			rx_buf_pos_chat = 0;
		} else if (rx_buf_pos_chat < (sizeof(rx_buf_chat) - 1)) {
			rx_buf_chat[rx_buf_pos_chat++] = c;
		}
		/* else: characters beyond buffer size are dropped */
	}
}


/*
 * Print a null-terminated string character by character to the UART interface
 */
void print_uart(const struct device *dev, char *buf)
{
	int msg_len = strlen(buf);

	for (int i = 0; i < msg_len; i++) {
		uart_poll_out(dev, buf[i]);
	}
}

void shell_to_chat_thread(void)
{
    char tx_buf[MSG_SIZE];
    while (1) {
        if (k_msgq_get(&uart_msgq_shell, &tx_buf, K_FOREVER) == 0) {
            print_uart(uart_dev_chat, tx_buf);
			print_uart(uart_dev_chat, "\r\n");
        }
    }
}

void chat_to_shell_thread(void)
{
    char tx_buf[MSG_SIZE];
    while (1) {
        if (k_msgq_get(&uart_msgq_chat, &tx_buf, K_FOREVER) == 0) {
            print_uart(uart_dev_shell, "Received: ");
            print_uart(uart_dev_shell, tx_buf);
            print_uart(uart_dev_shell, "\r\n");
        }
    }
}

K_THREAD_DEFINE(shell_to_chat_tid, 1024, shell_to_chat_thread, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(chat_to_shell_tid, 1024, chat_to_shell_thread, NULL, NULL, NULL, 7, 0, 0);


int main(void)
{
	if (!device_is_ready(uart_dev_shell)) {
		printk("UART shell device not found!");
		return 0;
	}

	if (!device_is_ready(uart_dev_chat)) {
		printk("UART chat device not found!");
		return 0;
	}

	int ret = uart_irq_callback_user_data_set(uart_dev_shell, serial_cb_shell, NULL);
	if (ret < 0) {
		if (ret == -ENOTSUP) {
			printk("Interrupt-driven UART API support not enabled for shell UART\n");
		} else if (ret == -ENOSYS) {
			printk("UART shell device does not support interrupt-driven API\n");
		} else {
			printk("Error setting UART shell callback: %d\n", ret);
		}
		return 0;
	}
	uart_irq_rx_enable(uart_dev_shell);

	ret = uart_irq_callback_user_data_set(uart_dev_chat, serial_cb_chat, NULL);
	if (ret < 0) {
		if (ret == -ENOTSUP) {
			printk("Interrupt-driven UART API support not enabled for chat UART\n");
		} else if (ret == -ENOSYS) {
			printk("UART chat device does not support interrupt-driven API\n");
		} else {
			printk("Error setting UART chat callback: %d\n", ret);
		}
		return 0;
	}
	uart_irq_rx_enable(uart_dev_chat);

    print_uart(uart_dev_shell, "System ready. Type a message and press enter.\r\n");

	return 0;
}