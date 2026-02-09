#include "demo_v1_OpenFHE.h"
#include "drogon/HttpResponse.h"
#include "trantor/utils/Logger.h"
#include <cstdint>
#include <unordered_map>

using namespace demo::v1;

void OpenFHE::rot_key(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
    MultiPartParser fileUpload;
    if (fileUpload.parse(req) != 0 || fileUpload.getFiles().size() != 3)
    {
        Json::Value json_resp;
        json_resp["error"] = "Must only be 3 files";
        auto resp = HttpResponse::newHttpJsonResponse(json_resp);
        resp->setStatusCode(k403Forbidden);
        callback(resp);
        return;
    }

    auto& params = fileUpload.getParameters();
    auto it = params.find("metadata");

    std::string key;
    if (it != params.end())
    {
        Json::Reader reader;
        Json::Value metadataJson;
        if (reader.parse(it->second, metadataJson)) {
            key = "user_" + metadataJson["key"].asString();
        }
    }
    else
    {
        Json::Value json_resp;
        json_resp["error"] = "Missing metadata field";
        auto resp = HttpResponse::newHttpJsonResponse(json_resp);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    std::unordered_map<std::string, std::string> data;

    for (auto& file : fileUpload.getFiles())
    {
        auto md5 = file.getMd5();
        std::string filename = key + "_" + md5 + "_" + file.getFileName();
        file.saveAs(filename);
        data.insert({file.getFileName().substr(0, file.getFileName().length() - 5), filename});
        LOG_INFO << "The uploaded file has been saved to the ./uploads/" + filename;
    }

    auto resp = HttpResponse::newHttpResponse();
    Json::Value json;
    Json::Value data_json;

    data_json["cc"]         = data["cc"];
    data_json["pub_key"]    = data["pub_key"];
    data_json["cipher"]     = data["cipher"];

    json["user"] = key;
    json["file"] = data_json;

    Json::FastWriter writer;
    std::string data_str = writer.write(data_json);

    this->set(key, data_str, [callback, json](bool success) {
        Json::Value resp;
        if (success) {
            resp["success"] = true;
            resp["data"] = json;
        } else {
            resp["success"] = false;
            resp["error"] = "Failed to store in Redis";
        }
        callback(HttpResponse::newHttpJsonResponse(resp));
    });
}

void OpenFHE::get_rot_key(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, uint32_t user) {
    std::string key = "user_" + std::to_string(user);

    this->get(key, [callback](const std::string& value, bool success) {
        Json::Value resp;
        if (!value.empty()) {
            resp["success"] = true;
            Json::Value val_json;
            Json::Reader reader;
            if (reader.parse(value, val_json)) {
                resp["data"] = val_json;
            } else {
                resp["error"] = "Failed to parse JSON";
            }
        } else {
            resp["success"] = false;
            resp["error"] = "Key not found";
        }
        callback(HttpResponse::newHttpJsonResponse(resp));
    });
}

void OpenFHE::compute(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, uint32_t user) {
    // FHECompute fhe(user);
    // std::string key = "user_" + std::to_string(user);

    std::string base = "/Users/bellian/CEPP/FHEClient/data";
    std::string pubKeyPath = base + "/key_pub.txt";
    std::string cryptoContextPath = base + "/cryptocontext.txt";
    std::string cipherPath1 = base + "/ciphertext1.txt";
    std::string cipherPath2 = base + "/ciphertext2.txt";
    std::string evalMultPath = base + "/key_mult.txt";
    std::string evalRotPath = base + "/key_rot.txt";
    FHECompute fhe(pubKeyPath, cryptoContextPath, evalMultPath, evalRotPath);
    LOG_INFO << "Initializing FHECompute for user_" << user;
    fhe.mult(cipherPath1, cipherPath2);
    Json::Value resp;
    resp["success"] = true;
    callback(HttpResponse::newHttpJsonResponse(resp));

    // this->get(key, [callback](const std::string& value, bool success) {
    //     Json::Value resp;
    //     if (!value.empty()) {
    //         resp["success"] = true;
    //         Json::Value val_json;
    //         Json::Reader reader;
    //         if (reader.parse(value, val_json)) {
    //             resp["data"] = val_json;
    //         } else {
    //             resp["error"] = "Failed to parse JSON";
    //         }
    //     } else {
    //         resp["success"] = false;
    //         resp["error"] = "Key not found";
    //     }
    //     callback(HttpResponse::newHttpJsonResponse(resp));
    // });
}
