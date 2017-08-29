#include <iostream>
#include <cstring>
#include <shared_mutex>
#include "rapidjson/writer.h"

#include "all.hpp"
#include "http.hpp"
#include "json.hpp"

extern std::shared_mutex mut;

struct Buffer {
    typedef char Ch;

    Buffer(char *buf, size_t max_size)
    : buf(buf), size(0), max_size(max_size)
    { }

    void Put(Ch c)              { buf[size++] = c; }
    void Flush()                { }
    void Clear()                { size = 0; }
    void ShrinkToFit()          { }
    Ch* Push(size_t count)      { size_t ret = size; size += count; return buf + ret; }
    void Pop(size_t count)      { size -= count; }
    const Ch* GetBuffer() const { return buf; }
    size_t GetSize()      const { return size; }

private:
    char *buf;
    size_t size;
    size_t max_size;
};

typedef rapidjson::Writer<Buffer> Writer;

static void json_serialize(Writer &writer, const User &user) {
	writer.StartObject();

	writer.String("id");         writer.Uint(user.id);
	writer.String("email");      writer.String(user.email);
	writer.String("first_name"); writer.String(user.first_name);
	writer.String("last_name");  writer.String(user.last_name);
	writer.String("gender");     writer.String(user.gender_is_male ? "m" : "f");
	writer.String("birth_date"); writer.Int(user.birth_date);

	writer.EndObject();
}

static void json_serialize(Writer &writer, const Loct &loct) {
	writer.StartObject();

	writer.String("id");         writer.Uint  (loct.id);
	writer.String("place");      writer.String(loct.place.c_str());
	writer.String("country");    writer.String(loct.country);
	writer.String("city");       writer.String(loct.city);
	writer.String("distance");   writer.Uint  (loct.distance);

	writer.EndObject();
}

static void json_serialize(Writer &writer, const Vist &vist) {
	writer.StartObject();

	writer.String("id");         writer.Uint(vist.id);
	writer.String("location");   writer.Uint(vist.loct);
	writer.String("user");       writer.Uint(vist.user);
	writer.String("visited_at"); writer.Uint(vist.visited);
	writer.String("mark");       writer.Uint(vist.mark);

	writer.EndObject();
}

static void json_serialize(Writer &writer, const std::vector<VistData> &data) {
	writer.StartObject();
	writer.String("visits");

	writer.StartArray();

	for (const auto &i : data) {
		writer.StartObject();
		writer.String("mark");       writer.Uint(i.mark);
		writer.String("visited_at"); writer.Uint(i.visited);
		writer.String("place");      writer.String(i.place.c_str());
		writer.EndObject();
	}

	writer.EndArray();
	writer.EndObject();
}

static void json_serialize(Writer &writer, const char *str) {
	writer.String(str);
}

static void json_raw(Writer &writer, const char *str) {
	size_t len = strlen(str);
	memcpy(writer.GetStream()->Push(len), str, len);
}

template <class Data>
static void server_get(HttpRequest &req, Writer &writer) {
	mut.lock_shared();
	Data *data = All::get<Data>(req.req->id);
	if (data) {
		req.status = 200;
		json_serialize(writer, *data);
	} else {
		req.status = 404;
		json_serialize(writer, "NONE");
	}
	mut.unlock_shared();
}

template <class Data>
static void server_new(HttpRequest &req, Writer &writer) {
	Data data;
	typename Data::Mask mask;
	if (!json_parse_single(req.post_data, data, mask) || !mask.is_full()) {
		req.status = 400;
		json_raw(writer, "Bad param");
		return;
	}
	mut.lock();
	bool ok = All::add(data);
	mut.unlock();
	if (ok) {
		req.status = 200;
		json_raw(writer, "{}");
	} else {
		req.status = 400;
		json_raw(writer, "Bad param");
	}
}

template <class Data>
static void server_update(HttpRequest &req, Writer &writer) {
	Data data;
	typename Data::Mask mask;
	if (!json_parse_single(req.post_data, data, mask) || mask.id) {
		req.status = 400;
		json_raw(writer, "Bad param");
		return;
	}
	data.id = req.req->id;
	mut.lock();
	bool ok = All::update(data, mask);
	mut.unlock();
	if (ok) {
		req.status = 200;
		json_raw(writer, "{}");
	} else {
		req.status = 404;
		json_raw(writer, "Not found");
	}
}

void http_hande_request(HttpRequest &req) {
	Buffer buffer(req.reply, req.reply_length_max);
	Writer writer(buffer);
	req.post_data[req.post_data_length] = 0;

	switch (req.req->type) {
	case HttpRequestLineType::error:
		req.status = 500;
		req.reply_length = sprintf(req.reply, ":>");
		break;
	case HttpRequestLineType::not_found:
		req.status = 404;
		req.reply_length = sprintf(req.reply, "Not found");
		break;
	case HttpRequestLineType::bad_param:
		req.status = 400;
		req.reply_length = sprintf(req.reply, "Bad param");
		break;
	case HttpRequestLineType::get_user: server_get<User>(req, writer); break;
	case HttpRequestLineType::get_loct: server_get<Loct>(req, writer); break;
	case HttpRequestLineType::get_vist: server_get<Vist>(req, writer); break;
	case HttpRequestLineType::get_loct_avg:
	{
		mut.lock_shared();
		auto avg = All::get_avg(
			req.req->id,
			req.req->from_date,
			req.req->to_date,
			req.req->from_age,
			req.req->to_age,
			req.req->gender);
		mut.unlock_shared();
		if (avg >= 0) {
			buffer.Push(sprintf(req.reply, "{\"avg\":%.5lf}", avg));
			req.status = 200;
		} else {
			json_serialize(writer, "Not found");
			req.status = 404;
		}
	}
		break;
	case HttpRequestLineType::get_user_vists:
	{
		mut.lock_shared();
		static thread_local std::vector<VistData> out;
		bool ok = All::get_vists(
			out,
			req.req->id,
			req.req->from_date,
			req.req->to_date,
			req.req->country,
			req.req->to_distance);
		mut.unlock_shared();
		if (ok) {
			json_serialize(writer, out);
			req.status = 200;
		} else {
			json_serialize(writer, "Not found");
			req.status = 404;
		}
	}

		break;
	case HttpRequestLineType::post_user: server_update<User>(req, writer); break;
	case HttpRequestLineType::post_loct: server_update<Loct>(req, writer); break;
	case HttpRequestLineType::post_vist: server_update<Vist>(req, writer); break;
	case HttpRequestLineType::post_user_new: server_new<User>(req, writer); break;
	case HttpRequestLineType::post_loct_new: server_new<Loct>(req, writer); break;
	case HttpRequestLineType::post_vist_new: server_new<Vist>(req, writer); break;
	}

	req.reply_length = buffer.GetSize();
}
