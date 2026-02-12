#include "fhe_compute.h"
#include "cryptocontextfactory.h"
#include "lattice/hal/lat-backend.h"
#include "trantor/utils/Logger.h"
#include "utils/sertype.h"
#include <cstring>
#include <drogon/drogon.h>
#include <string>

using namespace lbcrypto;

// Constructor
FHECompute::FHECompute(const FHEKeyPaths &paths) {
  this->cc->ClearEvalMultKeys();
  this->cc->ClearEvalAutomorphismKeys();
  CryptoContextFactory<DCRTPoly>::ReleaseAllContexts();

  if (!Serial::DeserializeFromFile(paths.crypto_context_path, this->cc,
                                   SerType::BINARY)) {
    LOG_ERROR << "Failed to deserialize crypto context";
    return;
  }

  if (!Serial::DeserializeFromFile(paths.public_key_path, this->pk,
                                   SerType::BINARY)) {
    LOG_ERROR << "Failed to deserialize public key";
    return;
  }

  std::ifstream mult_key_i_stream(paths.eval_mult_path,
                                  std::ios::in | std::ios::binary);
  if (!mult_key_i_stream.is_open()) {
    LOG_ERROR << "Cannot read serialization from " << paths.eval_mult_path;
    return;
  }
  if (!this->cc->DeserializeEvalMultKey(mult_key_i_stream, SerType::BINARY)) {
    LOG_ERROR << "Could not deserialize eval mult key file";
    return;
  }

  std::ifstream rotKeyIStream(paths.eval_rot_path,
                              std::ios::in | std::ios::binary);
  if (!rotKeyIStream.is_open()) {
    LOG_ERROR << "Cannot read serialization from " << paths.eval_rot_path;
    return;
  }
  if (!this->cc->DeserializeEvalAutomorphismKey(rotKeyIStream,
                                                SerType::BINARY)) {
    LOG_ERROR << "Could not deserialize eval rot key file";
    return;
  }
}

// Destructor
FHECompute::~FHECompute() {
  cc->ClearEvalMultKeys();
  cc->ClearEvalSumKeys();
}

// Encode values
Plaintext FHECompute::encoded(const std::vector<double> &values) {
  return cc->MakeCKKSPackedPlaintext(values);
}

// Encrypt values
Ciphertext<DCRTPoly> FHECompute::encrypt(const std::vector<double> &values) {
  return cc->Encrypt(pk, this->encoded(values));
}

// Load ciphertext from file
Ciphertext<DCRTPoly> FHECompute::load(const std::string &cipherPath) {
  Ciphertext<DCRTPoly> cipher;
  if (!Serial::DeserializeFromFile(cipherPath, cipher, SerType::BINARY)) {
    LOG_ERROR << "Failed to deserialize ciphertext " << cipherPath;
    return nullptr;
  }
  return cipher;
}

void FHECompute::mult(const std::string &cipherPath) {
  auto c1 = load(cipherPath);
  LOG_INFO << "Loaded ciphertext";
  auto c_mul = this->cc->EvalMult(c1, 2.5);
  LOG_INFO << "Evaluated multiplication";

  if (!Serial::SerializeToFile("mult_result.txt", c_mul, SerType::BINARY)) {
    LOG_ERROR << "Error serializing result";
    return;
  }
  LOG_INFO << "Serialized result";
}

void FHECompute::invertNorm(const std::string &cipherPath) {
  auto c1 = load(cipherPath);
  LOG_INFO << "Loaded ciphertext";
  auto c_inv = this->cc->EvalSub(1, c1);
  LOG_INFO << "Evaluated inversion";

  if (!Serial::SerializeToFile("invert_result.txt", c_inv, SerType::BINARY)) {
    LOG_ERROR << "Error serializing result";
    return;
  }
  LOG_INFO << "Serialized result";
}

void FHECompute::edgeDetection(const std::string &cipherPath, int width,
                               int height, uint8_t flag) {
  // Mask boundary
  std::vector<double> maskVector(width * height, 1.0);
  memset(&maskVector[0], 0, sizeof(double) * width);
  memset(&maskVector[width * (height - 1)], 0, sizeof(double) * width);
  for (int i = 1; i < height; i++) {
    maskVector[i * width] = 0;
    maskVector[i * width + width - 1] = 0;
  }
  auto c_mask = this->encrypt(maskVector);
  switch (flag) {
  case 0:
    this->verticalEdge(cipherPath, width, height, c_mask);
    break;
  case 1:
    this->horizontalEdge(cipherPath, width, height, c_mask);
    break;
  default:
    LOG_ERROR << "Invalid flag";
    break;
  }

  return;
}

// Prewitt vertical edge detect
void FHECompute::verticalEdge(const std::string &cipherPath, int width,
                              int height, Ciphertext<DCRTPoly> c_mask) {
  auto c1 = load(cipherPath);

  // Mask boundary
  std::vector<double> maskVector(width * height, 1.0);
  memset(&maskVector[0], 0, sizeof(double) * width);
  memset(&maskVector[width * (height - 1)], 0, sizeof(double) * width);
  for (int i = 1; i < height; i++) {
    maskVector[i * width] = 0;
    maskVector[i * width + width - 1] = 0;
  }

  auto c_left = this->cc->EvalRotate(c1, -1);
  auto c_right = this->cc->EvalRotate(c1, 1);
  auto c_diff = this->cc->EvalSub(c_left, c_right);
  auto c_edge = this->cc->EvalMult(c_diff, c_mask);

  if (!Serial::SerializeToFile("edge_result.txt", c_edge, SerType::BINARY)) {
    LOG_ERROR << "Error serializing result";
    return;
  }
  LOG_INFO << "Serialized result";
}

// Prewitt horizontal edge detect
void FHECompute::horizontalEdge(const std::string &cipherPath, int width,
                                int height, Ciphertext<DCRTPoly> c_mask) {
  auto c1 = load(cipherPath);

  // Mask boundary
  std::vector<double> maskVector(width * height, 1.0);
  memset(&maskVector[0], 0, sizeof(double) * width);
  memset(&maskVector[width * (height - 1)], 0, sizeof(double) * width);
  for (int i = 1; i < height; i++) {
    maskVector[i * width] = 0;
    maskVector[i * width + width - 1] = 0;
  }

  auto c_up = this->cc->EvalRotate(c1, -width);
  auto c_down = this->cc->EvalRotate(c1, width);
  auto c_diff = this->cc->EvalSub(c_up, c_down);
  auto c_edge = this->cc->EvalMult(c_diff, c_mask);

  if (!Serial::SerializeToFile("edge_result.txt", c_edge, SerType::BINARY)) {
    LOG_ERROR << "Error serializing result";
    return;
  }
  LOG_INFO << "Serialized result";
}
