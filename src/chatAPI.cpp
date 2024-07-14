#include "chatAPI.h"

const std::string CONFIG_FILE_API = "../config.ini";

std::string getAPIKey(const std::string &filename)
{
    INIReader reader(filename);
    if (reader.ParseError() != 0)
    {
        throw std::runtime_error("无法读取配置文件");
    }
    return reader.Get("API", "API_Key", "0");
}

std::string getSecretKey(const std::string &filename)
{
    INIReader reader(filename);
    if (reader.ParseError() != 0)
    {
        throw std::runtime_error("无法读取配置文件");
    }
    return reader.Get("API", "Secret_Key", "0");
}

std::string getAddress(const std::string &filename)
{
    INIReader reader(filename);
    if (reader.ParseError() != 0)
    {
        throw std::runtime_error("无法读取配置文件");
    }
    return reader.Get("API", "Request_Address", "0");
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

std::string extract_value(const std::string &json, const std::string &key)
{
    std::string search_key = "\"" + key + "\":\"";
    size_t start = json.find(search_key);
    if (start == std::string::npos)
    {
        return "";
    }
    start += search_key.length();
    size_t end = json.find("\"", start);
    if (end == std::string::npos)
    {
        return "";
    }
    return json.substr(start, end - start);
}

std::string get_access_token(const std::string &api_key, const std::string &secret_key)
{
    std::string url = "https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id=" + api_key + "&client_secret=" + secret_key;
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
    return extract_value(readBuffer, "access_token");
}

std::string chat_with_ai(const std::string &user_input)
{
    std::string api_key = getAPIKey(CONFIG_FILE_API);
    std::string secret_key = getSecretKey(CONFIG_FILE_API);
    std::string request_address = getAddress(CONFIG_FILE_API);
    std::string access_token = get_access_token(api_key, secret_key);
    std::string url = request_address + "?access_token=" + access_token;
    std::string payload = "{\"messages\":[{\"role\":\"user\",\"content\":\"" + user_input + "\"}]}";
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    return extract_value(readBuffer, "result");
}
