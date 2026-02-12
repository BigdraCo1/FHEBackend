#include "ciphertext-fwd.h"
#include "ciphertext-ser.h"
#include "cryptocontext-ser.h"
#include "key/key-ser.h"
#include "key/keypair.h"
#include "key/publickey-fwd.h"
#include "lattice/hal/lat-backend.h"
#include "openfhe.h"
#include "scheme/ckksrns/ckksrns-ser.h"

using namespace lbcrypto;

struct FHEKeyPaths {
  std::string public_key_path;
  std::string crypto_context_path;
  std::string eval_mult_path;
  std::string eval_rot_path;
};

class FHECompute {
public:
  explicit FHECompute(const FHEKeyPaths &paths);
  ~FHECompute();

  void mult(const std::string &cipherPath);
  void invertNorm(const std::string &cipherPath);
  void edgeDetection(const std::string &cipherPath, int width, int height,
                     uint8_t flag);

private:
  CryptoContext<DCRTPoly> cc; // CryptoContext
  PublicKey<DCRTPoly> pk;     // Public Key
  Ciphertext<DCRTPoly> encrypt(const std::vector<double> &values);
  Plaintext encoded(const std::vector<double> &values);
  static Ciphertext<DCRTPoly> load(const std::string &cipherPath);
  void verticalEdge(const std::string &cipherPath, int width, int height,
                    Ciphertext<DCRTPoly> c_mask);
  void horizontalEdge(const std::string &cipherPath, int width, int height,
                      Ciphertext<DCRTPoly> c_mask);
};
