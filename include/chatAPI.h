#ifndef CHAT_API_H
#define CHAT_API_H

#include <string>
#include <curl/curl.h>
#include <stdexcept>
#include "inih/INIReader.h"
// 声明函数
std::string getAPIKey(const std::string &filename);
std::string getSecretKey(const std::string &filename);
std::string extract_value(const std::string &json, const std::string &key);
std::string get_access_token(const std::string &api_key, const std::string &secret_key);
std::string chat_with_ai(const std::string &user_input);

// 声明常量
extern const std::string CONFIG_FILE_API;

#endif // CHAT_API_H
