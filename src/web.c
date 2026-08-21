#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/http/server.h>
#include <zephyr/net/http/service.h>

LOG_MODULE_REGISTER(mb_replacer_web, LOG_LEVEL_DBG);

static uint8_t index_html_gz[] = {
#include "index.html.gz.inc"
};

static uint8_t main_js_gz[] = {
#include "main.js.gz.inc"
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

static struct http_resource_detail_static main_js_gz_resource_detail = {
	.common = {
			.type = HTTP_RESOURCE_TYPE_STATIC,
			.bitmask_of_supported_http_methods = BIT(HTTP_GET),
			.content_encoding = "gzip",
			.content_type = "text/javascript",
		},
	.static_data = main_js_gz,
	.static_data_len = sizeof(main_js_gz),
};


static int uptime_handler(
	struct http_client_ctx *client, 
	enum http_transaction_status status,
	const struct http_request_ctx *request_ctx,
	struct http_response_ctx *response_ctx,
	void *user_data
)
{
	int ret;
	static uint8_t uptime_buf[sizeof(STRINGIFY(INT64_MAX))];

	LOG_DBG("Uptime handler status %d", status);

	/* A payload is not expected with the GET request. Ignore any data and wait until
	 * final callback before sending response
	 */
	if (status == HTTP_SERVER_REQUEST_DATA_FINAL) {
		ret = snprintf(uptime_buf, sizeof(uptime_buf), "%" PRId64, k_uptime_get());
		if (ret < 0) {
			LOG_ERR("Failed to snprintf uptime, err %d", ret);
			return ret;
		}

		response_ctx->body = uptime_buf;
		response_ctx->body_len = ret;
		response_ctx->final_chunk = true;
	}

	return 0;
}

static struct http_resource_detail_dynamic uptime_resource_detail = {
	.common = {
			.type = HTTP_RESOURCE_TYPE_DYNAMIC,
			.bitmask_of_supported_http_methods = BIT(HTTP_GET),
		},
	.cb = uptime_handler,
	.user_data = NULL,
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

HTTP_RESOURCE_DEFINE(
	main_js_gz_resource,
	modbus_replacer_http_service,
	"/main.js",
	&main_js_gz_resource_detail
);


HTTP_RESOURCE_DEFINE(
	uptime_resource, 
	modbus_replacer_http_service, 
	"/uptime", 
	&uptime_resource_detail
);
