#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/modbus/modbus.h>

#include "web.h"

LOG_MODULE_REGISTER(mb_replacer_main, LOG_LEVEL_DBG);

// LEDs thread define for debug blink
K_THREAD_STACK_DEFINE(led_thread_stack, 512);
struct k_thread led_thread_data;

// STM32F407Disco LEDs Array
static const struct gpio_dt_spec leds[] = {
	GPIO_DT_SPEC_GET(DT_ALIAS(led_serial), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led_status), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led_alert), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led_net), gpios),
};
const size_t leds_count = ARRAY_SIZE(leds);

// Modbus RTU struct
static int mb_rtu_iface;

const static struct modbus_iface_param mb_rtu_iface_param = {
	.mode = MODBUS_MODE_RTU,
	.rx_timeout = 500000,
	.serial = {
		.baud = 4800,
		.parity = UART_CFG_PARITY_NONE,
	},
};

#define MODBUS_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(zephyr_modbus_serial)

// Modbus RTU interface init
static int init_mb_rtu_iface(void) {
	const char mb_rtu_iface_name[] = {DEVICE_DT_NAME(MODBUS_NODE)};
	
	mb_rtu_iface = modbus_iface_get_by_name(mb_rtu_iface_name);
	if (mb_rtu_iface < 0) {
		LOG_ERR("Failed to get iface index for %s", mb_rtu_iface_name);
		return -ENODEV;
	}
	
	return modbus_init_client(mb_rtu_iface, mb_rtu_iface_param);
}

// LEDs thread for debug blink
void led_blink_thread(void *p1, void *p2, void *p3)
{
	size_t current_led = 0;
	while (1) {
		gpio_pin_toggle_dt(&leds[current_led++]);
		if (current_led == leds_count) current_led = 0;
		k_msleep(200);
	}
}

int main(void)
{				
	int ret = 0;

	// Init LEDs for debug blink
	for (size_t i = 0; i < leds_count; i++) {
		if (!gpio_is_ready_dt(&leds[i])) {
			LOG_ERR("GPIO device for led %d (%s) is not ready", (int)i, 
            			leds[i].port != NULL ? leds[i].port->name : "NULL");
			return -EIO;
		}
		
		ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_ACTIVE);
		if (ret < 0) {
			LOG_ERR("Failed to configure pin for led%d (err: %d)", (int)i, ret);
			return ret;
		}
	}
	
	// Create LEDs thread for debug blink
	k_thread_create(
		&led_thread_data, led_thread_stack, K_THREAD_STACK_SIZEOF(led_thread_stack),
		led_blink_thread, NULL, NULL, NULL, 7, 0, K_NO_WAIT
	);

	// MODBUS RTU intenface init call
	if (init_mb_rtu_iface()) {
		LOG_ERR("Modbus iface initialization failed");
		return 0;
	}

	// Reconfigure Modbus RTU UART to STOP BIT 1
	// !!! Remove uart6 from text, replace to get interface name from DT by modbus label
	const struct device *uart_dev = DEVICE_DT_GET(DT_NODELABEL(usart6)); 
	if (device_is_ready(uart_dev)) {
		struct uart_config uart_cfg;
		uart_config_get(uart_dev, &uart_cfg);
		uart_cfg.stop_bits = UART_CFG_STOP_BITS_1; 
		int ret = uart_configure(uart_dev, &uart_cfg);
		if (ret) {
			LOG_ERR("Failed to configure UART_CFG_STOP_BITS_1 (err: %d)",  ret);
		}
	}

	// MODBUS RTU registers buffer
	uint16_t mb_regs_buffer[7];

	// HTTP Server init call
	http_server_start();
	
	// while (1) {

	// 	// Modbus RTU soil sensor regs read cycle
	// 	ret = modbus_read_holding_regs(
	// 		mb_rtu_iface,
	// 		0x01,
	// 		0x00,
	// 		mb_regs_buffer,
	// 		ARRAY_SIZE(mb_regs_buffer)
	// 	);
		
	// 	if (!ret) {
	// 		for (int i = 0; i < 7; i++) {
	// 			LOG_INF("Reg %d: 0x%X",i , mb_regs_buffer[i]);
	// 		}
	// 	} else {
	// 		LOG_ERR("MB Transceive error. Code: %d", ret);
	// 	}
		
	// 	printk("\
	// 		Humidity:       %d.%d %\n\
	// 		Temperature:    %d.%d °C\n\
	// 		EC:             %d uS/cm\n\
	// 		pH:             %d.%d\n\
	// 		Nitrogen (N):   %d mg/kg\n\
	// 		Phosphorus (P): %d mg/kg\n\
	// 		Potassium (K):  %d mg/kg\n",
	// 		mb_regs_buffer[0] / 10, mb_regs_buffer[0] % 10,
	// 		mb_regs_buffer[1] / 10, mb_regs_buffer[1] % 10,
	// 		mb_regs_buffer[2],
	// 		mb_regs_buffer[3] / 10, mb_regs_buffer[3] % 10,
	// 		mb_regs_buffer[4],
	// 		mb_regs_buffer[5],
	// 		mb_regs_buffer[6]
	// 		);
		
	// 	k_msleep(10000);

	// }

	return 0;
}

