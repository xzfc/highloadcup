
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

const char *json_serialize(const Location &location) {
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
	sb.Clear();
	writer.StartObject();

	writer.String("id");         writer.Uint(location.id);
	writer.String("place");      writer.String(location.place.c_str());
	writer.String("country");    writer.String(location.country);
	writer.String("city");       writer.String(location.city);
	writer.String("distance");   writer.Uint(location.distance);

	writer.EndObject();
	return sb.GetString();
}

const char *json_serialize(const Visit &visit) {
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
	sb.Clear();
	writer.StartObject();

	writer.String("id");         writer.Uint(visit.id);
	writer.String("location");   writer.Uint(visit.location);
	writer.String("user");       writer.Uint(visit.user);
	writer.String("visited_at"); writer.Uint(visit.visited_at);
	writer.String("mark");       writer.Uint(visit.mark);

	writer.EndObject();
	return sb.GetString();
}

const char *json_serialize(const std::vector<VisitData> &data) {
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
	sb.Clear();

	writer.StartObject();
	writer.String("visits");

	writer.StartArray();

	for (const auto &i : data) {
		writer.StartObject();
		writer.String("mark");       writer.Uint(i.mark);
		writer.String("visited_at"); writer.Uint(i.visited_at);
		writer.String("place");      writer.String(i.place.c_str());
		writer.EndObject();
	}

	writer.EndArray();
	writer.EndObject();

	return sb.GetString();
}
