#include "machikado.h"
#include <sodium.h>
#include <algorithm>
#include <cstring>

namespace machikado {
    const char* to_string(SignError err) {
        switch (err) {
            case SignError::InvalidPrivateKey:   return "invalid private key bytes";
            case SignError::InvalidBlob:         return "invalid signed blob (expected 96 bytes)";
            case SignError::VerificationFailed:  return "signature verification failed";
            case SignError::InvalidModuleId:     return "invalid module id: must match ^[a-zA-Z][a-zA-Z0-9._-]+$";
            case SignError::PublicKeyMismatch:   return "public key mismatch";
        }
        return "unknown error";
    }

    SignedBlob::SignedBlob(const std::array<std::uint8_t, SIGNATURE_SIZE>& sig,
                        const PublicKey& pk)
        : signature_(sig), public_key_(pk) {}

    std::optional<SignedBlob> SignedBlob::from_bytes(const std::uint8_t* data, std::size_t len) {
        if (len != SIGNED_BLOB_SIZE) {
            return std::nullopt;
        }
        SignedBlob blob;
        std::memcpy(blob.signature_.data(), data, SIGNATURE_SIZE);
        std::memcpy(blob.public_key_.data(), data + SIGNATURE_SIZE, PUBLIC_KEY_SIZE);
        return blob;
    }

    std::optional<SignedBlob> SignedBlob::from_bytes(const std::vector<std::uint8_t>& bytes) {
        return from_bytes(bytes.data(), bytes.size());
    }

    const std::array<std::uint8_t, SIGNATURE_SIZE>& SignedBlob::signature() const noexcept {
        return signature_;
    }

    const PublicKey& SignedBlob::public_key() const noexcept {
        return public_key_;
    }

    std::array<std::uint8_t, SIGNED_BLOB_SIZE> SignedBlob::as_bytes() const {
        std::array<std::uint8_t, SIGNED_BLOB_SIZE> bytes{};
        std::memcpy(bytes.data(), signature_.data(), SIGNATURE_SIZE);
        std::memcpy(bytes.data() + SIGNATURE_SIZE, public_key_.data(), PUBLIC_KEY_SIZE);
        return bytes;
    }

    std::vector<std::uint8_t> SignedBlob::to_vec() const {
        auto arr = as_bytes();
        return std::vector<std::uint8_t>(arr.begin(), arr.end());
    }

    bool SignedBlob::operator==(const SignedBlob& other) const {
        return signature_ == other.signature_ && public_key_ == other.public_key_;
    }

    bool SignedBlob::operator!=(const SignedBlob& other) const {
        return !(*this == other);
    }

    static bool is_valid_module_id(const std::string& id) {
        if (id.empty()) return false;
        if (!std::isalpha(static_cast<unsigned char>(id[0]))) return false;
        for (char c : id) {
            auto uc = static_cast<unsigned char>(c);
            if (!std::isalnum(uc) && c != '.' && c != '_' && c != '-') {
                return false;
            }
        }
        return true;
    }

    static std::vector<std::uint8_t> build_signing_data(const std::vector<FileEntry>& entries) {
        std::vector<std::uint8_t> data;
        for (const auto& entry : entries) {
            data.insert(data.end(), entry.relative_path.begin(), entry.relative_path.end());
            data.push_back(0x00);
            std::uint64_t size = static_cast<std::uint64_t>(entry.content.size());
            for (int i = 0; i < 8; i++) {
                data.push_back(static_cast<std::uint8_t>((size >> (i * 8)) & 0xFF));
            }
            data.insert(data.end(), entry.content.begin(), entry.content.end());
        }
        return data;
    }

    std::optional<SignedBlob> sign_file_entries(const std::vector<FileEntry>& entries,
                                                const PrivateKey& private_key) {
        if (sodium_init() < 0) {
            return std::nullopt;
        }

        PublicKey pk;
        std::memcpy(pk.data(), private_key.data() + 32, 32);

        auto signing_data = build_signing_data(entries);

        std::array<std::uint8_t, SIGNATURE_SIZE> sig{};
        unsigned long long sig_len = 0;

        if (crypto_sign_ed25519_detached(
                sig.data(), &sig_len,
                signing_data.data(), signing_data.size(),
                private_key.data()) != 0) {
            return std::nullopt;
        }

        return SignedBlob(sig, pk);
    }

    std::optional<SignedBlob> sign_mazoku(const std::string& module_id,
                                        const PublicKey& project_public_key,
                                        const PrivateKey& org_private_key) {
        if (!is_valid_module_id(module_id)) {
            return std::nullopt;
        }

        if (sodium_init() < 0) {
            return std::nullopt;
        }

        PublicKey org_pk;
        std::memcpy(org_pk.data(), org_private_key.data() + 32, 32);

        std::vector<std::uint8_t> data;
        data.insert(data.end(), module_id.begin(), module_id.end());
        data.push_back(0x00);
        data.insert(data.end(), project_public_key.begin(), project_public_key.end());

        std::array<std::uint8_t, SIGNATURE_SIZE> sig{};
        unsigned long long sig_len = 0;

        if (crypto_sign_ed25519_detached(
                sig.data(), &sig_len,
                data.data(), data.size(),
                org_private_key.data()) != 0) {
            return std::nullopt;
        }

        return SignedBlob(sig, org_pk);
    }

    std::pair<bool, std::optional<SignError>>
    verify(const std::vector<std::uint8_t>& machikado_blob,
        const std::vector<std::uint8_t>& mazoku_blob,
        const std::vector<FileEntry>& entries,
        const std::string& module_id,
        const PublicKey& expected_org_pk) {
        if (sodium_init() < 0) {
            return {false, SignError::VerificationFailed};
        }

        auto machikado_opt = SignedBlob::from_bytes(machikado_blob);
        if (!machikado_opt) {
            return {false, SignError::InvalidBlob};
        }

        auto mazoku_opt = SignedBlob::from_bytes(mazoku_blob);
        if (!mazoku_opt) {
            return {false, SignError::InvalidBlob};
        }

        const auto& machikado = *machikado_opt;
        const auto& mazoku = *mazoku_opt;

        if (mazoku.public_key() != expected_org_pk) {
            return {false, SignError::PublicKeyMismatch};
        }

        {
            std::vector<std::uint8_t> mazoku_data;
            mazoku_data.insert(mazoku_data.end(), module_id.begin(), module_id.end());
            mazoku_data.push_back(0x00);
            mazoku_data.insert(mazoku_data.end(),
                            machikado.public_key().begin(), machikado.public_key().end());

            if (crypto_sign_ed25519_verify_detached(
                    mazoku.signature().data(),
                    mazoku_data.data(), mazoku_data.size(),
                    mazoku.public_key().data()) != 0) {
                return {false, SignError::VerificationFailed};
            }
        }

        {
            auto file_data = build_signing_data(entries);
            if (crypto_sign_ed25519_verify_detached(
                    machikado.signature().data(),
                    file_data.data(), file_data.size(),
                    machikado.public_key().data()) != 0) {
                return {false, SignError::VerificationFailed};
            }
        }

        return {true, std::nullopt};
    }

    std::pair<bool, std::optional<SignError>>
    verify_machikado(const std::vector<std::uint8_t>& machikado_blob,
                    const std::vector<FileEntry>& entries,
                    const PublicKey& expected_pk) {
        if (sodium_init() < 0) {
            return {false, SignError::VerificationFailed};
        }

        auto machikado_opt = SignedBlob::from_bytes(machikado_blob);
        if (!machikado_opt) {
            return {false, SignError::InvalidBlob};
        }

        const auto& machikado = *machikado_opt;

        if (machikado.public_key() != expected_pk) {
            return {false, SignError::PublicKeyMismatch};
        }

        auto file_data = build_signing_data(entries);
        if (crypto_sign_ed25519_verify_detached(
                machikado.signature().data(),
                file_data.data(), file_data.size(),
                machikado.public_key().data()) != 0) {
            return {false, SignError::VerificationFailed};
        }

        return {true, std::nullopt};
    }
} // namespace machikado
