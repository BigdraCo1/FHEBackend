#include "ciphertext-fwd.h"
#include "key/keypair.h"
#include "key/publickey-fwd.h"
#include "lattice/hal/lat-backend.h"
#include "openfhe.h"
#include "ciphertext-ser.h"
#include "cryptocontext-ser.h"
#include "key/key-ser.h"
#include "scheme/ckksrns/ckksrns-ser.h"

using namespace lbcrypto;

class FHECompute {
public:
    FHECompute(const std::string& publicKeyPath, const std::string& cryptoContextPath, const std::string& evalMultPath, const std::string& evalRotPath);
    ~FHECompute();

    void mult(const std::string& cipherPath1, const std::string& cipherPath2);

private:
    CryptoContext<DCRTPoly>     cc; // CryptoContext
    PublicKey<DCRTPoly>         pk; // Public Key
    Ciphertext<DCRTPoly> encrypt(const std::vector<double>& values);
    Plaintext encoded(const std::vector<double>& values);
    Ciphertext<DCRTPoly> load(const std::string& cipherPath);
};
