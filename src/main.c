#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/net/http/server.h>
#include <zephyr/net/http/service.h>

LOG_MODULE_REGISTER(app_main, LOG_LEVEL_ERR);

static const struct gpio_dt_spec leds[] = {
	GPIO_DT_SPEC_GET(DT_ALIAS(led_serial), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led_status), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led_alert), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led_net), gpios),
};

static uint8_t index_html_gz[] = {
#include "index.html.gz.inc"
};

static struct http_resource_detail_static http_index_gz_resource_detail = {
	.common = {
		.type = HTTP_RESOURCE_TYPE_STATIC,
		.bitmask_of_supported_http_methods = BIT(HTTP_GET),
		.content_encoding = "gzip",
		.content_type = "text/html",
	},
	.static_data = index_html_gz,
	.static_data_len = sizeof(index_html_gz),
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
	http_index_gz_resource,
	modbus_replacer_http_service,
	"/",
	&http_index_gz_resource_detail
);
	

int main(void)
{			
	const size_t leds_count = ARRAY_SIZE(leds);
	
	for (size_t i = 0; i < leds_count; i++) {
		if (!gpio_is_ready_dt(&leds[i])) {
			LOG_ERR("GPIO device for led %d (%s) is not ready", (int)i, 
            			leds[i].port != NULL ? leds[i].port->name : "NULL");
			return -EIO;
		}
		
		int ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_ACTIVE);
		if (ret < 0) {
			LOG_ERR("Failed to configure pin for led%d (err: %d)", (int)i, ret);
			return ret;
		}
	}

	http_server_start();
	
	while (1) {
		for (size_t i = 0; i < leds_count; i++) {
			gpio_pin_toggle_dt(&leds[i]);
			k_msleep(200);
		}
	}

	return 0;
}

