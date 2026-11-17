/*
 * Firmware update over a serial port (demonstrator only).
 *
 * Receives a signed image, stores it in slot1 and requests MCUboot to install
 * it. Host side: scripts/dfu.py
 *
 * Stop-and-wait protocol: the host sends one frame and waits for the response
 * before sending the next one. All multi-byte values are little endian.
 *
 * Request frame (host -> device):
 *   0xA5 | command (u8) | length (u16) | payload (length bytes) | crc32 (u32)
 *   crc32: CRC-32/IEEE (same as Python's zlib.crc32) over command, length and payload
 *
 * Response (device -> host):
 *   0x5A | status (u8)
 *
 * Commands:
 *   START  (0x01): payload = image size (u32). Erases slot1.
 *   DATA   (0x02): payload = offset (u32) + 1..512 data bytes. Offsets must be
 *                  sequential. A repeated last chunk is acknowledged again.
 *   FINISH (0x03): no payload. Marks the image for a test upgrade and reboots.
 *
 * Incomplete frames are discarded after 500 ms without receiving data.
 */

#include "serial_dfu.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/dfu/flash_img.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/crc.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(serial_dfu);

#define FRAME_START    0xA5
#define RESPONSE_START 0x5A

#define CMD_START  0x01
#define CMD_DATA   0x02
#define CMD_FINISH 0x03

#define MAX_DATA_SIZE    512
#define MAX_PAYLOAD_SIZE (sizeof(uint32_t) + MAX_DATA_SIZE)

enum status {
	STATUS_OK = 0x00,
	STATUS_FRAME_ERROR = 0x01,     /* invalid length or CRC, frame discarded */
	STATUS_INVALID_COMMAND = 0x02, /* unknown command or invalid payload length */
	STATUS_INVALID_STATE = 0x03,   /* DATA/FINISH without START, or FINISH too early */
	STATUS_INVALID_OFFSET = 0x04,  /* DATA offset does not match the bytes received */
	STATUS_INVALID_SIZE = 0x05,    /* image does not fit into slot1, or too much data */
	STATUS_FLASH_ERROR = 0x06,     /* erasing/writing flash or requesting the upgrade failed */
};

#define INTER_BYTE_TIMEOUT K_MSEC(500)

static const struct device *const uart = DEVICE_DT_GET(DT_NODELABEL(dfu_uart));

/* filled by the UART ISR, drained by the thread */
RING_BUF_DECLARE(rx_ring, 1024);
static K_SEM_DEFINE(rx_sem, 0, 1);

/* frame parser */
enum parser_state {
	STATE_WAIT_START,
	STATE_COMMAND,
	STATE_LENGTH_LOW,
	STATE_LENGTH_HIGH,
	STATE_PAYLOAD,
	STATE_CRC,
};

static struct {
	enum parser_state state;
	uint8_t command;
	uint16_t length;
	uint8_t payload[MAX_PAYLOAD_SIZE];
	size_t index;
	uint32_t crc;
	uint32_t received_crc;
} frame;

/* image transfer */
static struct flash_img_context image;
static bool receiving;
static uint32_t image_size;
static uint32_t bytes_received;

static void send_response(enum status status)
{
	uart_poll_out(uart, RESPONSE_START);
	uart_poll_out(uart, status);
}

static enum status handle_start(void)
{
	if (frame.length != sizeof(uint32_t)) {
		return STATUS_INVALID_COMMAND;
	}

	/* (re)start the transfer from scratch */
	receiving = false;

	int ret = flash_img_init(&image);
	if (ret != 0) {
		LOG_ERR("flash_img_init failed (%d)", ret);
		return STATUS_FLASH_ERROR;
	}

	/* the end of the slot is reserved for the MCUboot trailer */
	const size_t slot_size = image.flash_area->fa_size;
	const ssize_t max_size = boot_get_trailer_status_offset(slot_size);
	const uint32_t size = sys_get_le32(frame.payload);

	if (size == 0 || max_size <= 0 || size > (size_t)max_size) {
		LOG_ERR("Invalid image size %u (max %d)", size, (int)max_size);
		return STATUS_INVALID_SIZE;
	}

	LOG_INF("Receiving image of %u bytes, erasing slot1", size);
	ret = flash_area_erase(image.flash_area, 0, slot_size);
	if (ret != 0) {
		LOG_ERR("Erasing slot1 failed (%d)", ret);
		return STATUS_FLASH_ERROR;
	}

	image_size = size;
	bytes_received = 0;
	receiving = true;
	return STATUS_OK;
}

static enum status handle_data(void)
{
	if (frame.length <= sizeof(uint32_t)) {
		return STATUS_INVALID_COMMAND;
	}
	if (!receiving) {
		return STATUS_INVALID_STATE;
	}

	const uint32_t offset = sys_get_le32(frame.payload);
	const uint8_t *data = frame.payload + sizeof(uint32_t);
	const uint32_t len = frame.length - sizeof(uint32_t);

	/* the host repeats a chunk if our response got lost */
	if (offset + len == bytes_received) {
		return STATUS_OK;
	}
	if (offset != bytes_received) {
		LOG_ERR("Unexpected offset %u (expected %u)", offset, bytes_received);
		return STATUS_INVALID_OFFSET;
	}
	if (bytes_received + len > image_size) {
		return STATUS_INVALID_SIZE;
	}

	const int ret = flash_img_buffered_write(&image, data, len, false);
	if (ret != 0) {
		LOG_ERR("Writing image failed (%d)", ret);
		receiving = false;
		return STATUS_FLASH_ERROR;
	}

	bytes_received += len;
	return STATUS_OK;
}

static enum status handle_finish(void)
{
	if (frame.length != 0) {
		return STATUS_INVALID_COMMAND;
	}
	if (!receiving || bytes_received != image_size) {
		return STATUS_INVALID_STATE;
	}
	receiving = false;

	int ret = flash_img_buffered_write(&image, NULL, 0, true);
	if (ret != 0) {
		LOG_ERR("Writing image failed (%d)", ret);
		return STATUS_FLASH_ERROR;
	}

	/* MCUboot verifies the signature on the next boot and swaps the images */
	ret = boot_request_upgrade(BOOT_UPGRADE_TEST);
	if (ret != 0) {
		LOG_ERR("Requesting upgrade failed (%d)", ret);
		return STATUS_FLASH_ERROR;
	}

	LOG_INF("Image received, rebooting to install it");
	send_response(STATUS_OK);

	/* give USB and the deferred logger time to send the last data */
	k_sleep(K_MSEC(500));
	sys_reboot(SYS_REBOOT_COLD);

	return STATUS_OK; /* not reached */
}

static enum status handle_frame(void)
{
	switch (frame.command) {
	case CMD_START:
		return handle_start();
	case CMD_DATA:
		return handle_data();
	case CMD_FINISH:
		return handle_finish();
	default:
		return STATUS_INVALID_COMMAND;
	}
}

static void update_crc(uint8_t byte)
{
	frame.crc = crc32_ieee_update(frame.crc, &byte, 1);
}

/* feeds one received byte into the frame parser, handles complete frames */
static void process_byte(uint8_t byte)
{
	switch (frame.state) {
	case STATE_WAIT_START:
		/* ignore anything between frames */
		if (byte == FRAME_START) {
			frame.crc = 0;
			frame.state = STATE_COMMAND;
		}
		break;

	case STATE_COMMAND:
		update_crc(byte);
		frame.command = byte;
		frame.state = STATE_LENGTH_LOW;
		break;

	case STATE_LENGTH_LOW:
		update_crc(byte);
		frame.length = byte;
		frame.state = STATE_LENGTH_HIGH;
		break;

	case STATE_LENGTH_HIGH:
		update_crc(byte);
		frame.length |= (uint16_t)byte << 8;
		if (frame.length > MAX_PAYLOAD_SIZE) {
			LOG_WRN("Invalid frame length %u", frame.length);
			frame.state = STATE_WAIT_START;
			send_response(STATUS_FRAME_ERROR);
			break;
		}
		frame.index = 0;
		frame.received_crc = 0;
		frame.state = (frame.length > 0) ? STATE_PAYLOAD : STATE_CRC;
		break;

	case STATE_PAYLOAD:
		update_crc(byte);
		frame.payload[frame.index++] = byte;
		if (frame.index == frame.length) {
			frame.index = 0;
			frame.state = STATE_CRC;
		}
		break;

	case STATE_CRC:
		frame.received_crc |= (uint32_t)byte << (8 * frame.index++);
		if (frame.index == sizeof(uint32_t)) {
			frame.state = STATE_WAIT_START;
			if (frame.received_crc == frame.crc) {
				send_response(handle_frame());
			} else {
				LOG_WRN("Invalid frame CRC");
				send_response(STATUS_FRAME_ERROR);
			}
		}
		break;
	}
}

static void uart_isr(const struct device *dev, void *user_data)
{
	ARG_UNUSED(user_data);

	while (uart_irq_update(dev) && uart_irq_rx_ready(dev)) {
		uint8_t buffer[64];
		const int len = uart_fifo_read(dev, buffer, sizeof(buffer));

		if (len <= 0) {
			break;
		}
		/* cannot overflow with the stop-and-wait protocol (one frame in flight) */
		ring_buf_put(&rx_ring, buffer, len);
	}
	k_sem_give(&rx_sem);
}

static void serial_dfu_thread(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	LOG_INF("Waiting for firmware image on %s", uart->name);

	while (true) {
		if (k_sem_take(&rx_sem, INTER_BYTE_TIMEOUT) != 0) {
			if (frame.state != STATE_WAIT_START) {
				LOG_WRN("Timeout, discarding incomplete frame");
				frame.state = STATE_WAIT_START;
			}
			continue;
		}

		uint8_t buffer[64];
		uint32_t len;

		while ((len = ring_buf_get(&rx_ring, buffer, sizeof(buffer))) > 0) {
			for (uint32_t i = 0; i < len; i++) {
				process_byte(buffer[i]);
			}
		}
	}
}

K_THREAD_DEFINE(serial_dfu_tid, 2048, serial_dfu_thread, NULL, NULL, NULL, 7, 0,
		SYS_FOREVER_MS);

/*
 * After a test upgrade, MCUboot reverts to the previous image on the next reset
 * unless the new image is confirmed. A real product would only confirm after a
 * successful self test; this demonstrator confirms right away.
 */
static void confirm_running_image(void)
{
	if (boot_is_img_confirmed()) {
		return;
	}

	const int ret = boot_write_img_confirmed();

	if (ret == 0) {
		LOG_INF("Running image confirmed");
	} else {
		LOG_ERR("Failed to confirm running image (%d)", ret);
	}
}

void serial_dfu_start(void)
{
	__ASSERT(device_is_ready(uart), "dfu_uart not ready");

	confirm_running_image();

	uart_irq_callback_user_data_set(uart, uart_isr, NULL);
	uart_irq_rx_enable(uart);

	k_thread_start(serial_dfu_tid);
}
