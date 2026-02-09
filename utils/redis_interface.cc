#include "redis_interface.h"

using namespace drogon::nosql;

void RedisInterface::set(const std::string& key, const std::string& value,
                         std::function<void(bool success)> callback) {
    getClient()->execCommandAsync(
        [callback](const RedisResult& result) {
            if (callback) callback(true);
        },
        [callback](const RedisException& err) {
            LOG_ERROR << "Redis SET error: " << err.what();
            if (callback) callback(false);
        },
        "SET %s %s", key.c_str(), value.c_str()
    );
}

void RedisInterface::get(const std::string& key,
                         std::function<void(const std::string& value, bool success)> callback) {
    getClient()->execCommandAsync(
        [callback](const RedisResult& result) {
            if (result.isNil()) {
                callback("", false);
            } else {
                callback(result.asString(), true);
            }
        },
        [callback](const RedisException& err) {
            LOG_ERROR << "Redis GET error: " << err.what();
            callback("", false);
        },
        "GET %s", key.c_str()
    );
}
