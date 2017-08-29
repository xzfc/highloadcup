
#include "all.hpp"



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
