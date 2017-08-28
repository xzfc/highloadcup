
#include "all.hpp"

#include "rapidjson/writer.h"

static thread_local rapidjson::StringBuffer sb;

const char *json_serialize(const User &user) {
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
	sb.Clear();
	writer.StartObject();

	writer.String("id");         writer.Uint(user.id);
	writer.String("email");      writer.String(user.email);
	writer.String("first_name"); writer.String(user.first_name);
	writer.String("last_name");  writer.String(user.last_name);
	writer.String("gender");     writer.String(user.gender_is_male ? "m" : "f");
	writer.String("birth_date"); writer.Int(user.birth_date);

	writer.EndObject();
	return sb.GetString();
}

const char *json_serialize(const Loct &loct) {
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
	sb.Clear();
	writer.StartObject();

	writer.String("id");         writer.Uint  (loct.id);
	writer.String("place");      writer.String(loct.place.c_str());
	writer.String("country");    writer.String(loct.country);
	writer.String("city");       writer.String(loct.city);
	writer.String("distance");   writer.Uint  (loct.distance);

	writer.EndObject();
	return sb.GetString();
}

const char *json_serialize(const Vist &vist) {
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
	sb.Clear();
	writer.StartObject();

	writer.String("id");         writer.Uint(vist.id);
	writer.String("location");   writer.Uint(vist.loct);
	writer.String("user");       writer.Uint(vist.user);
	writer.String("visited_at"); writer.Uint(vist.visited);
	writer.String("mark");       writer.Uint(vist.mark);

	writer.EndObject();
	return sb.GetString();
}

const char *json_serialize(const std::vector<VistData> &data) {
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
	sb.Clear();

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

	return sb.GetString();
}

const char *json_serialize(double val) {
	static thread_local char buf[16];
	sprintf(buf, "{\"avg\":%.5lf}", val);
	return buf;
}
