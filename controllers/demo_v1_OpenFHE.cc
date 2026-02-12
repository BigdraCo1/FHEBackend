#include "demo_v1_OpenFHE.h"
#include "drogon/HttpResponse.h"
#include "trantor/utils/Logger.h"
#include <cstdint>
#include <unordered_map>

using namespace demo::v1;

enum class ComputeMethod { UNKNOWN, MULT, INVERT, V_EDGE, H_EDGE, NONE };

static const std::unordered_map<std::string, ComputeMethod> methodMap = {
    {"", ComputeMethod::NONE},
    {"mult", ComputeMethod::MULT},
    {"invert", ComputeMethod::INVERT},
    {"v_edge", ComputeMethod::V_EDGE},
    {"h_edge", ComputeMethod::H_EDGE}};

void OpenFHE::rotKey(const HttpRequestPtr &req,
                     std::function<void(const HttpResponsePtr &)> &&callback) {
  MultiPartParser file_upload;
  if (file_upload.parse(req) != 0 || file_upload.getFiles().size() != 3) {
    Json::Value json_resp;
    json_resp["error"] = "Must only be 3 files";
    auto resp = HttpResponse::newHttpJsonResponse(json_resp);
    resp->setStatusCode(k403Forbidden);
    callback(resp);
    return;
  }

  auto &params = file_upload.getParameters();
  auto it = params.find("metadata");

  std::string key;
  if (it != params.end()) {
    Json::Reader reader;
    Json::Value metadata_json;
    if (reader.parse(it->second, metadata_json)) {
      key = "user_" + metadata_json["key"].asString();
    }
  } else {
    Json::Value json_resp;
    json_resp["error"] = "Missing metadata field";
    auto resp = HttpResponse::newHttpJsonResponse(json_resp);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  std::unordered_map<std::string, std::string> data;

  for (auto &file : file_upload.getFiles()) {
    auto md5 = file.getMd5();
    std::string filename = key + "_" + md5 + "_" + file.getFileName();
    file.saveAs(filename);
    data.insert({file.getFileName().substr(0, file.getFileName().length() - 5),
                 filename});
    LOG_INFO << "The uploaded file has been saved to the ./uploads/" + filename;
  }

  auto resp = HttpResponse::newHttpResponse();
  Json::Value json;
  Json::Value data_json;

  data_json["cc"] = data["cc"];
  data_json["pub_key"] = data["pub_key"];
  data_json["cipher"] = data["cipher"];

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

void OpenFHE::getRotKey(const HttpRequestPtr &req,
                        std::function<void(const HttpResponsePtr &)> &&callback,
                        uint32_t user) {
  std::string key = "user_" + std::to_string(user);

  this->get(key, [callback](const std::string &value, bool success) {
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

void OpenFHE::compute(const HttpRequestPtr &req,
                      std::function<void(const HttpResponsePtr &)> &&callback,
                      uint32_t user) {
  std::string method = req->getParameter("method");
  Json::Value resp;

  std::string base = "/Users/bellian/CEPP/FHEClient/data";
  std::string pub_key_path = base + "/key_pub.txt";
  std::string crypto_context_path = base + "/cryptocontext.txt";
  std::string cipher_path1 = base + "/ciphertext1.txt";
  std::string eval_mult_path = base + "/key_mult.txt";
  std::string eval_rot_path = base + "/key_rot.txt";
  FHECompute fhe(FHEKeyPaths{
      .public_key_path = pub_key_path,
      .crypto_context_path = crypto_context_path,
      .eval_mult_path = eval_mult_path,
      .eval_rot_path = eval_rot_path,
  });

  switch (methodMap.at(method)) {
  case ComputeMethod::MULT:
    fhe.mult(cipher_path1);
    break;
  case ComputeMethod::INVERT:
    fhe.invertNorm(cipher_path1);
    break;
  case ComputeMethod::V_EDGE:
    fhe.edgeDetection(cipher_path1, 226, 226, 0);
    break;
  case ComputeMethod::NONE: {
    resp["error"] = "No method specified";
    auto response = HttpResponse::newHttpJsonResponse(resp);
    response->setStatusCode(k400BadRequest);
    callback(response);
    return;
  }
  default: {
    resp["error"] = "Invalid method. Must be 'mult' or 'invert'";
    auto response = HttpResponse::newHttpJsonResponse(resp);
    response->setStatusCode(k400BadRequest);
    callback(response);
    return;
  }
  }
  resp["success"] = true;
  callback(HttpResponse::newHttpJsonResponse(resp));
}
