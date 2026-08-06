#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/net/http/server.h>
#include <zephyr/net/http/service.h>

#define LED_NODE DT_ALIAS(led_http)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

static const uint8_t index_html[] =
	"<!DOCTYPE html>"
	"<html><head><meta charset=\"utf-8\"><title>STM32F4</title></head>"
	"<body><h1>Hello from Zephyr on STM32F4Discovery!</h1></body></html>";

static struct http_resource_detail_static index_html_resource_detail = {
	.common = {
		.type = HTTP_RESOURCE_TYPE_STATIC,
		.bitmask_of_supported_http_methods = BIT(HTTP_GET),
		.content_type = "text/html",
	},
	.static_data = index_html,
	.static_data_len = sizeof(index_html) - 1,
};

static uint16_t http_service_port = 80;

HTTP_SERVICE_DEFINE(
	modbus_replacer_http_service,
	NULL,
	&http_service_port,
	CONFIG_HTTP_SERVER_MAX_CLIENTS,
	10,
	NULL,
	NULL,
	NULL
);

HTTP_RESOURCE_DEFINE(
	index_html_resource,
	modbus_replacer_http_service,
	"/",
	&index_html_resource_detail
);
	

int main(void)
{
	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}
	gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);

	http_server_start();

	return 0;
}

