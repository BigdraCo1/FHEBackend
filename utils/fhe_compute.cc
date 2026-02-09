#include "fhe_compute.h"
#include "cryptocontextfactory.h"
#include "lattice/hal/lat-backend.h"
#include "trantor/utils/Logger.h"
#include "utils/sertype.h"
#include <drogon/drogon.h>

using namespace lbcrypto;

// Constructor
FHECompute::FHECompute(
    const std::string& publicKeyPath,
    const std::string& cryptoContextPath,
    const std::string& evalMultPath,
    const std::string& evalRotPath
)
{
    this->cc->ClearEvalMultKeys();
    this->cc->ClearEvalAutomorphismKeys();
    CryptoContextFactory<DCRTPoly>::ReleaseAllContexts();

    if (!Serial::DeserializeFromFile(cryptoContextPath, this->cc, SerType::BINARY)) {
        LOG_ERROR << "Failed to deserialize crypto context";
        return;
    }

    if (!Serial::DeserializeFromFile(publicKeyPath, this->pk, SerType::BINARY)) {
        LOG_ERROR << "Failed to deserialize public key";
        return;
    }

    std::ifstream multKeyIStream(evalMultPath, std::ios::in | std::ios::binary);
    if (!multKeyIStream.is_open()) {
        LOG_ERROR << "Cannot read serialization from " << evalMultPath;
        return;
    }
    if (!this->cc->DeserializeEvalMultKey(multKeyIStream, SerType::BINARY)) {
        LOG_ERROR << "Could not deserialize eval mult key file";
        return;
    }
}

// Destructor
FHECompute::~FHECompute()
{
    cc->ClearEvalMultKeys();
    cc->ClearEvalSumKeys();
}

// Encode values
Plaintext FHECompute::encoded(const std::vector<double>& values)
{
    return cc->MakeCKKSPackedPlaintext(values);
}


// Encrypt values
Ciphertext<DCRTPoly> FHECompute::encrypt(const std::vector<double>& values)
{
    return cc->Encrypt(pk, this->encoded(values));
}

// Load ciphertext from file
Ciphertext<DCRTPoly> FHECompute::load(const std::string& cipherPath)
{
    Ciphertext<DCRTPoly> cipher;
    if (!Serial::DeserializeFromFile(cipherPath, cipher, SerType::BINARY)) {
        LOG_ERROR << "Failed to deserialize ciphertext " << cipherPath;
        return nullptr;
    }
    return cipher;
}

void FHECompute::mult(const std::string& cipherPath1, const std::string& cipherPath2) {
    auto c1 = this->load(cipherPath1);
    LOG_INFO << "Loaded ciphertext 1";
    // auto c2 = this->load(cipherPath2);
    // LOG_INFO << "Loaded ciphertext 2";
    auto cMul = this->cc->EvalMult(c1, 2.5);
    LOG_INFO << "Evaluated multiplication";

    if (!Serial::SerializeToFile("result.txt", cMul, SerType::BINARY)) {
        LOG_ERROR << "Error serializing result";
        return;
    }
    LOG_INFO << "Serialized result";
}
