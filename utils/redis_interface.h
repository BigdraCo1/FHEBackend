#pragma once

#include <drogon/drogon.h>
#include <drogon/nosql/RedisClient.h>
#include <functional>

class RedisInterface {
public:
    void set(const std::string& key, const std::string& value,
             std::function<void(bool success)> callback = nullptr);

    void get(const std::string& key,
             std::function<void(const std::string& value, bool success)> callback);

private:
    drogon::nosql::RedisClientPtr getClient() {
        return drogon::app().getRedisClient();
    }
};
