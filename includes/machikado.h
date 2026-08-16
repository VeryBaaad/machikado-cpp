#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace machikado {
    constexpr std::size_t PUBLIC_KEY_SIZE  = 32;
    constexpr std::size_t PRIVATE_KEY_SIZE = 64;
    constexpr std::size_t SIGNATURE_SIZE   = 64;
    constexpr std::size_t SIGNED_BLOB_SIZE = 96;

    using PublicKey  = std::array<std::uint8_t, PUBLIC_KEY_SIZE>;
    using PrivateKey = std::array<std::uint8_t, PRIVATE_KEY_SIZE>;

    enum class SignError {
        InvalidPrivateKey,
        InvalidBlob,
        VerificationFailed,
        InvalidModuleId,
        PublicKeyMismatch,
        SodiumInitFailed,
    };

    const char* to_string(SignError err);

    struct Ed25519KeyPair {
        PublicKey  public_key;
        PrivateKey private_key;
    };

    class SignedBlob {
    public:
        SignedBlob() = default;
        SignedBlob(const std::array<std::uint8_t, SIGNATURE_SIZE>& sig,
                const PublicKey& pk);

        static std::optional<SignedBlob> from_bytes(const std::uint8_t* data, std::size_t len);
        static std::optional<SignedBlob> from_bytes(const std::vector<std::uint8_t>& bytes);

        const std::array<std::uint8_t, SIGNATURE_SIZE>& signature() const noexcept;
        const PublicKey& public_key() const noexcept;

        std::array<std::uint8_t, SIGNED_BLOB_SIZE> as_bytes() const;
        std::vector<std::uint8_t> to_vec() const;

        bool operator==(const SignedBlob& other) const;
        bool operator!=(const SignedBlob& other) const;

    private:
        std::array<std::uint8_t, SIGNATURE_SIZE> signature_{};
        PublicKey public_key_{};
    };

    struct FileEntry {
        std::string relative_path;
        std::vector<std::uint8_t> content;
    };

    class FileMapping {
    public:
        FileMapping() = default;

        void insert(const std::string& target_path, const std::string& source_path);

        std::size_t size() const noexcept;
        bool empty() const noexcept;

        using iterator = std::map<std::string, std::string>::iterator;
        using const_iterator = std::map<std::string, std::string>::const_iterator;
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        const_iterator cbegin() const;
        const_iterator cend() const;

        const std::map<std::string, std::string>& map() const noexcept;

        static FileMapping from_pair(const std::string& target, const std::string& source);
        static FileMapping from_pairs(std::initializer_list<std::pair<std::string, std::string>> pairs);

    private:
        std::map<std::string, std::string> map_;
    };

    std::optional<Ed25519KeyPair> generate_keypair();

    std::optional<SignedBlob> sign_file_entries(const std::vector<FileEntry>& entries,
                                                const PrivateKey& private_key);

    std::optional<SignedBlob> sign_mazoku(const std::string& module_id,
                                        const PublicKey& project_public_key,
                                        const PrivateKey& org_private_key);

    std::pair<bool, std::optional<SignError>>
    verify(const std::vector<std::uint8_t>& machikado_blob,
        const std::vector<std::uint8_t>& mazoku_blob,
        const std::vector<FileEntry>& entries,
        const std::string& module_id,
        const PublicKey& expected_org_pk);

    std::pair<bool, std::optional<SignError>>
    verify_machikado(const std::vector<std::uint8_t>& machikado_blob,
                    const std::vector<FileEntry>& entries,
                    const PublicKey& expected_pk);

    std::optional<std::vector<FileEntry>> load_folder_files(
        const std::filesystem::path& folder,
        const std::vector<std::string>& ignore_prefixes = {},
        const std::vector<std::string>& ignore_names = {},
        const FileMapping* mapping = nullptr);

} // namespace machikado
