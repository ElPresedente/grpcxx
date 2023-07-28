#pragma once

#include <string_view>
#include <vector>

#include <uv.h>

namespace grpcxx {

// Forward declarations
namespace h2 {
class conn;
struct event;
} // namespace h2

class server {
public:
	server();

	void run(const std::string_view &ip, int port);

private:
	static void alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
	static void close_cb(uv_handle_t *handle);
	static void conn_cb(uv_stream_t *server, int status);
	static void read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
	static void write_cb(uv_write_t *req, int status);

	void   listen(const h2::event &ev);
	size_t write(uv_stream_t *handle, const uint8_t *data, size_t size);

	uv_tcp_t  _handle;
	uv_loop_t _loop;
};
} // namespace grpcxx