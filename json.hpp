#pragma once
void parse(const char *dir);
bool parse_user(const std::string &json, User &user, uint8_t &mask);
bool parse_location(const std::string &json, Location &location, uint8_t &mask);
bool parse_visit(const std::string &json, Visit &visit, uint8_t &mask);
