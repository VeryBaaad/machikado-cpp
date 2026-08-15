#include "machikado.h"
#include <sodium.h>

namespace machikado {

Ed25519KeyPair generate_keypair() {
    if (sodium_init() < 0) {
        throw std::runtime_error("Failed to initialize libsodium");
    }

    Ed25519KeyPair kp;

    std::array<unsigned char, 32> seed;
    randombytes_buf(seed.data(), seed.size());

    if (crypto_sign_ed25519_seed_keypair(
            kp.public_key.data(),
            kp.private_key.data(),
            seed.data()) != 0) {
        throw std::runtime_error("Failed to generate Ed25519 keypair");
    }

    return kp;
}

} // namespace machikado
