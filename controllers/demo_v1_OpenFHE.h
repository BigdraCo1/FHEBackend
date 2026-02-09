#pragma once

#include <cstdint>
#include <drogon/HttpController.h>
#include "utils/redis_interface.h"
#include "utils/fhe_compute.h"

using namespace drogon;

namespace demo
{
namespace v1
{
class OpenFHE : public drogon::HttpController<OpenFHE>, public RedisInterface
{
  public:
    METHOD_LIST_BEGIN
    // use METHOD_ADD to add your custom processing function here;
    // METHOD_ADD(OpenFHE::get, "/{2}/{1}", Get); // path is /demo/v1/OpenFHE/{arg2}/{arg1}
    // METHOD_ADD(OpenFHE::your_method_name, "/{1}/{2}/list", Get); // path is /demo/v1/OpenFHE/{arg1}/{arg2}/list
    // ADD_METHOD_TO(OpenFHE::your_method_name, "/absolute/path/{1}/{2}/list", Get); // path is /absolute/path/{arg1}/{arg2}/list
    METHOD_ADD(OpenFHE::rot_key, "/input", Post);
    METHOD_ADD(OpenFHE::get_rot_key, "/input/{1}", Get);
    METHOD_ADD(OpenFHE::compute, "/compute/{1}", Get);
    METHOD_LIST_END
    // your declaration of processing function maybe like this:
    // void get(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, int p1, std::string p2);
    // void your_method_name(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, double p1, int p2) const;
    void rot_key(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback);
    void get_rot_key(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, uint32_t user);
    void compute(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, uint32_t user);
    // void compute(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, uint32_t user);
};
}
}
