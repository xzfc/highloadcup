#pragma once
void parse(const char *dir);

bool json_parse_single(const std::string &json, User &, UserMask &);
bool json_parse_single(const std::string &json, Loct &, LoctMask &);
bool json_parse_single(const std::string &json, Vist &, VistMask &);
