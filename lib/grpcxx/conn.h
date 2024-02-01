#pragma once

#include <forward_list>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include <uv.h>

#include "h2/session.h"

#include "reader.h"
#include "request.h"
#include "response.h"
#include "task.h"
#include "writer.h"

namespace grpcxx {
namespace detail {
class conn {
public:
	using buffer_t   = std::string;
	using handle_t   = std::shared_ptr<uv_tcp_t>;
	using requests_t = std::forward_list<request>;
	using streams_t  = std::unordered_map<int32_t, request>;

	conn(const conn &) = delete;
	conn(uv_stream_t *stream);

	task flush();

	requests_t   read(std::string_view bytes);
	class reader reader() const noexcept;

	void write(response resp) noexcept;

private:
	struct deleter {
		void operator()(void *handle) const noexcept {
			uv_close(static_cast<uv_handle_t *>(handle), [](uv_handle_t *handle) {
				delete reinterpret_cast<uv_tcp_t *>(handle);
			});
		}
	};

	void   buffer() noexcept;
	writer write(std::string_view bytes) const noexcept;

	buffer_t    _buffer;
	handle_t    _handle;
	h2::session _session;
	streams_t   _streams;
};
} // namespace detail
} // namespace grpcxx
