#include "server.h"

#include <unordered_map>

#include "conn.h"
#include "request.h"
#include "response.h"
#include "task.h"

namespace grpcxx {
using requests_t = std::unordered_map<int32_t, detail::request>;

server::server() {
	// TODO: error handling
	uv_loop_init(&_loop);
	uv_tcp_init(&_loop, &_handle);

	_handle.data = this;
}

detail::task server::conn(uv_stream_t *stream) {
	std::printf("[info] connection - start\n");

	detail::conn c(stream);
	requests_t   requests;

	auto &session = c.session();
	auto  reader  = c.read();

	while (reader) {
		auto events = session.read(co_await reader);

		for (auto bytes = session.pending(); bytes.size() > 0; bytes = session.pending()) {
			co_await c.write(bytes);
		}

		for (const auto &ev : events) {
			if (ev.stream_id.value_or(0) <=0 ) {
				continue;
			}

			 if (ev.type == h2::event::type_t::stream_close) {
				requests.erase(ev.stream_id.value());
				continue;
			 }

			requests_t::iterator it = requests.emplace(ev.stream_id.value(), ev.stream_id.value()).first;
			auto &req = it->second;

			switch (ev.type) {
			case h2::event::type_t::stream_data: {
				req.read(ev.data);
				break;
			}

			case h2::event::type_t::stream_end: {
				session.headers(
					req.id(),
					{
						{":status", "200"},
						{"content-type", "application/grpc"},
					});

				auto       resp = process(req);
				auto       data = resp.bytes();
				h2::buffer buf(data);
				session.data(req.id(), buf);

				for (auto bytes = session.pending(); bytes.size() > 0; bytes = session.pending()) {
					co_await c.write(bytes);
				}

				session.trailers(
					req.id(),
					{
						{"grpc-status", resp.status()},
					});

				for (auto bytes = session.pending(); bytes.size() > 0; bytes = session.pending()) {
					co_await c.write(bytes);
				}

				break;
			}

			case h2::event::type_t::stream_header: {
				req.header(ev.header->name, ev.header->value);
				break;
			}

			default: {
				std::printf(
					"  [event] stream_id: %d, type: %hhu\n", ev.stream_id.value_or(-1), ev.type);
				break;
			}
			}
		}
	}

	std::printf("[info] connection - end\n");
}

void server::conn_cb(uv_stream_t *stream, int status) {
	if (status < 0) {
		return;
	}

	auto *s = static_cast<server *>(stream->data);
	s->conn(stream);
}

detail::response server::process(const detail::request &req) const noexcept {
	if (!req) {
		return {status::code_t::invalid_argument};
	}

	auto it = _services.find(req.service());
	if (it == _services.end()) {
		return {status::code_t::not_found};
	}

	detail::response resp;
	try {
		auto r = it->second(req.method(), req.data());
		resp.status(std::move(r.first));
		resp.data(std::move(r.second));
	} catch (std::exception &e) {
		return {status::code_t::internal};
	}

	return resp;
}

void server::run(const std::string_view &ip, int port) {
	// TODO: error handling
	struct sockaddr_in addr;
	uv_ip4_addr(ip.data(), port, &addr);

	uv_tcp_bind(&_handle, reinterpret_cast<const sockaddr *>(&addr), 0);
	uv_listen(reinterpret_cast<uv_stream_t *>(&_handle), 128, conn_cb);

	uv_run(&_loop, UV_RUN_DEFAULT);
}
} // namespace grpcxx
