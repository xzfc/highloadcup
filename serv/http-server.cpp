#include <netinet/ip.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <thread>
#include <mutex>
#include <vector>
#include <fcntl.h>

#include "http.hpp"

namespace {

static std::mutex mut;

static void check(const char *file, int line, const char *func, const char *cond_str, int cond) {
	if (!cond) {
		fprintf(stderr, "%s:%d: %s: Assertion `%s` failed.\n", file, line, func, cond_str);
		fprintf(stderr, "%s\n", strerror(errno));
		exit(1);
	}
}

#define $check(COND) check(__FILE__, __LINE__, __PRETTY_FUNCTION__, #COND, COND)
#define $warn(TEXT, ...)  \
	fprintf(stderr, "%s:%d: %s: " TEXT ".\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__);

template <class T>
struct Pool {
	typedef typename std::vector<T>::size_type size_type;
	Pool(size_type size) : size(size), all(size) {
		free.reserve(size);
		for (size_type i = 0; i < size; i++)
			free.push_back(&all[size - i - 1]);
	}
	T *get() {
		if (free.size() == 0)
			return nullptr;
		auto e = free.back();
		free.pop_back();
		return e;
	}
	void put(T *e) { free.push_back(e);  }
	auto begin()   { return all.begin(); }
	auto end()     { return all.end(); }
	const size_type size;
private:
	std::vector<T>  all;
	std::vector<T*> free;
};

struct Client;
struct Server;

static Pool<Client> client_pool(2048);

struct Input {
	struct Client &client;

	HttpRequestLine http_requst_line;

	int    state; // '\r\n'
	char   buf[2048];
	size_t size;

	int line_state;

	size_t line_start;

	size_t content_length;
	size_t content_read;
	size_t post_start;

	Input(Client &);
	void reset();
	void read();
	void process_line();
};

struct Output {
	struct Client &client;

	char   buf[8192];
	size_t start;
	size_t done;
	size_t size;

	Output(Client &);
	void write();
};

struct Client {
	Server *server;

	int fd;
	Input  input;
	Output output;

	Client();

	void accept(Server *server);
	void close();
	void epollctl(int op, bool write = false);

	void handle_request();

	void process_line(const char *line);
	void on_read();
	void on_write();

	size_t handle_buf(const char *buf, size_t len);
};

struct Server {
	int fd;
	int epollfd;
	int thread;
	int port;
	HttpHandlerFun *handle_request;

	Server(int thread, int port, HttpHandlerFun handle_request);
	void run();
	void accept();
};

Server::Server(int thread, int port, HttpHandlerFun handle_request)
	: thread(thread), port(port), handle_request(handle_request)
{
}

void Server::run() {
	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((uint16_t)port);

	fd = socket(AF_INET, SOCK_STREAM, 0);
	$check(fd >= 0);

	int optval = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (const void *)&optval , sizeof optval);

	$check(bind(fd, (struct sockaddr *)&serveraddr, sizeof serveraddr) == 0);
	$check(listen(fd, 128) == 0);

	epollfd = epoll_create1(0);
	$check(epollfd >= 0);

	struct epoll_event ev;
	ev.data.ptr = NULL;
	ev.events = EPOLLIN;
	$check(epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == 0);

	while (1) {
		struct epoll_event events[64];
		int count = epoll_wait(epollfd, events, sizeof events / sizeof *events, -1);
		$check(count > 0);
		for (int i = 0; i < count; i++) {
			if (events[i].data.ptr == nullptr)
				accept();
			else {
				auto client = (Client *) events[i].data.ptr;
				if (events[i].events & EPOLLIN)
					client->on_read();
				if (events[i].events & EPOLLOUT)
					client->on_write();
			}
		}
	}
}

void Server::accept() {
	mut.lock();
	Client *client = client_pool.get();
	mut.unlock();
	if (client) {
		client->accept(this);
	} else {
		$warn("No free clients!");
		int fd = ::accept(this->fd, NULL, NULL);
		if (fd < 0) {
			$warn("Cant accept: %s", strerror(errno));
			return;
		}
		close(fd);
	}
}

Client::Client() : server(nullptr), input(*this), output(*this) {
	fd = -1;
}

void Client::accept(Server *server) {
	$check(this->server == nullptr);

	this->server = server;

	sockaddr_in clientaddr;
	socklen_t clientlen = sizeof clientaddr;
	fd = ::accept(server->fd, (struct sockaddr *)&clientaddr, &clientlen);
	$check(fd >= 0);

	epollctl(EPOLL_CTL_ADD, false);

	input.reset();
}

void Client::close() {
	if (fd < 0) return;
	epollctl(EPOLL_CTL_DEL);
	::close(fd);
	fd = -1;
	server = nullptr;
	mut.lock();
	client_pool.put(this);
	mut.unlock();
}

void Client::epollctl(int op, bool write) {
	struct epoll_event ev;
	ev.data.ptr = this;
	ev.events = write ? EPOLLOUT : EPOLLIN;
	$check(epoll_ctl(server->epollfd, op, fd, &ev) == 0);
}

void Client::handle_request() {
	epollctl(EPOLL_CTL_MOD, true);

	static const char
		HTTP[]     = "HTTP/1.1 ",
		CODE_200[] = "200 OK",
		CODE_400[] = "400 Bad Request",
		CODE_404[] = "404 Not Found",
		CODE_500[] = "500 Internal Server Error",
		REST[]     = "\r\n"
		             "Connection: Keep-Alive\r\n"
			     "Server: B\r\n"
		             "Content-Length: ",
		RNRN[]     = "\r\n\r\n";
	size_t pad = 128, prepended = 0;
	char none = 0;

	HttpRequest req;
	req.req = &input.http_requst_line;
	req.post_data = input.post_start ? input.buf + input.post_start : &none;
	req.post_data_length = input.content_length;
	req.status = 500;
	req.reply = output.buf + pad;
	req.reply_length = 0;
	req.reply_length_max = sizeof output.buf - pad;

	server->handle_request(req);

	char length_str[16];
	int length_str_len = sprintf(length_str, "%zu", req.reply_length);

	#define $prepend(STR, SIZE) \
		memcpy(output.buf + pad - prepended - (SIZE), STR, (SIZE)); \
		prepended += (SIZE)
	#define $prepend_const(STR) $prepend(STR, sizeof STR - 1)
	$prepend_const(RNRN);
	$prepend(length_str, length_str_len);
	$prepend_const(REST);
	switch (req.status) {
	case 200: $prepend_const(CODE_200); break;
	case 400: $prepend_const(CODE_400); break;
	case 404: $prepend_const(CODE_404); break;
	default:  $prepend_const(CODE_500); break;
	}
	$prepend_const(HTTP);
	#undef $prepend
	#undef $prepend_const

	output.start = pad - prepended;
	output.size = prepended + req.reply_length;
	output.done = 0;
}

void Client::on_read() {
	if (fd < 0) return;
	input.read();
}

void Client::on_write() {
	output.write();
}

Output::Output(Client &client) : client(client) {
}

void Output::write() {
	ssize_t s = ::write(client.fd, buf + done + start, size - done);
	if (s < 0) {
		// if (errno == EAGAIN || errno == EWOULDBLOCK)
		// 	return;
		$warn("write: %s", strerror(errno));
		return;
	}

	done += s;

	if (done >= size) {
#ifdef KEEPALIVE
		if (client.input.http_requst_line.keep_alive) {
#endif
			client.epollctl(EPOLL_CTL_MOD, false);
			client.input.reset();
#ifdef KEEPALIVE
		} else {
			client.close();
		}
#endif
	}
}

Input::Input(Client &client) : client(client) {
}

void Input::reset() {
	state      = 0;
	size       = 0;

	line_state = 0;

	line_start = 0;

	content_length = content_read = 0;
	post_start = 0;
}

void Input::read() {
	ssize_t new_size =
		::read(client.fd, buf + size, sizeof buf - size);
	if (new_size <= 0) {
		client.close();
		return;
	}

	if (line_state != 2) {
		for (size_t pos = size; pos < size + new_size; pos++) {
			char *c = buf + pos;
			switch (state) {
			case 0: if (*c == '\r')
					state = 1;
				break;
			case 1:
				if (*c == '\n') {
					state = 0;
					c[0] = c[-1] = 0;
					process_line();
					if (line_state == 2)
						break;
					line_start = pos + 1;
				} else {
					state = 0;
				}
				break;
			}
			if (line_state == 2)
				break;
		}
	}
	if (line_state == 2) {
		content_read += new_size;
		if (content_read >= content_length)
			client.handle_request();
	}
	size += new_size;
}

void Input::process_line() {
	static const char ContentLength[] = "Content-Length: ";
#ifdef KEEPALIVE
	static const char KeepAlive[] = "Connection: Keep-Alive";
#endif
	// std::cout << "Line: `" << buf+line_start << "`\n";

	if (line_state == 0) {
		http_parse_requst_line(buf + line_start, http_requst_line);
		line_state = 1;
		return;
	}
	if (line_state == 1) {
		if (buf[line_start] == 0) {
			if (content_length == 0) {
				client.handle_request();
				line_state = 4;
			} else {
				state = 2;
				line_state = 2;
				post_start = line_start + 2;
			}
		} else if (memcmp(buf + line_start,
				  ContentLength,
				  sizeof ContentLength - 1) == 0) {
			content_length = atoi(
				buf + line_start + sizeof ContentLength - 1);
		}
#ifdef KEEPALIVE
		else if (!http_requst_line.keep_alive &&
		           strcmp(buf + line_start, KeepAlive) == 0)
			http_requst_line.keep_alive = true;
#endif
		return;
	}
}

}

void run_http_server(int port, HttpHandlerFun f) {
	
	std::thread threads[4];
	int i = 0;
	for (auto &thread : threads) {
		thread = std::thread([=] {
			auto server = new Server(i, port, f);
			server->run();
			free(server);
		});
		i++;
	}
	for (auto &thread : threads)
		thread.join();
}
