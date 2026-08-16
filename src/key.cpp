#include "machikado.h"
#include <sodium.h>

namespace machikado {

std::optional<Ed25519KeyPair> generate_keypair() {
    if (sodium_init() < 0) {
        return std::nullopt;
    }

    Ed25519KeyPair kp;

    std::array<unsigned char, 32> seed;
    randombytes_buf(seed.data(), seed.size());

    if (crypto_sign_ed25519_seed_keypair(
            kp.public_key.data(),
            kp.private_key.data(),
            seed.data()) != 0) {
        return std::nullopt;
    }

    return kp;
}

} // namespace machikado
