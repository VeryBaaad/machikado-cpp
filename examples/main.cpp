#include "machikado.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

static void write_file(const fs::path& path, const std::string& content) {
    fs::create_directories(path.parent_path());
    std::ofstream ofs(path, std::ios::binary);
    ofs << content;
}

static void print_hex(const char* label, const auto& data) {
    std::cout << label << ": ";
    for (auto b : data) {
        printf("%02x", b);
    }
    std::cout << "\n";
}

int main() {
    try {
        auto dir = fs::temp_directory_path() / "machikado_cpp_test";
        fs::remove_all(dir);
        fs::create_directories(dir);

        write_file(dir / "module.prop", "id=test\nname=Test\nversion=v1.0\n");
        write_file(dir / "lib" / "arm64" / "libzygisk.so", "binary_content");
        write_file(dir / "bin" / "daemon", "daemon_binary");

        std::cout << "=== Generating keypairs ===\n";
        auto org_kp = machikado::generate_keypair();
        auto member_kp = machikado::generate_keypair();
        print_hex("org public key   ", org_kp.public_key);
        print_hex("member public key", member_kp.public_key);

        std::cout << "\n=== Loading files ===\n";
        auto entries = machikado::load_folder_files(
            dir, {}, {"machikado", "mazoku"}, nullptr);
        for (const auto& e : entries) {
            std::cout << "  " << e.relative_path
                      << " (" << e.content.size() << " bytes)\n";
        }

        std::cout << "\n=== Signing files (machikado) ===\n";
        auto machikado = machikado::sign_file_entries(entries, member_kp.private_key);
        print_hex("machikado blob", machikado.as_bytes());

        std::cout << "\n=== Signing mazoku ===\n";
        auto mazoku = machikado::sign_mazoku(
            "test_module", member_kp.public_key, org_kp.private_key);
        print_hex("mazoku blob   ", mazoku.as_bytes());

        std::cout << "\n=== Verifying (two-tier) ===\n";
        auto [ok, err] = machikado::verify(
            machikado.to_vec(), mazoku.to_vec(),
            entries, "test_module", org_kp.public_key);

        if (ok) {
            std::cout << "Verification PASSED!\n";
        } else {
            std::cout << "Verification FAILED: "
                      << machikado::to_string(err.value()) << "\n";
        }

        std::cout << "\n=== Verifying (machikado-only) ===\n";
        auto [ok2, err2] = machikado::verify_machikado(
            machikado.to_vec(), entries, member_kp.public_key);
        std::cout << (ok2 ? "PASSED" : "FAILED") << "\n";

        std::cout << "\n=== Tamper test ===\n";
        auto bad_entries = entries;
        bad_entries[0].content = {'H', 'A', 'C', 'K', 'E', 'D'};
        auto [ok3, err3] = machikado::verify(
            machikado.to_vec(), mazoku.to_vec(),
            bad_entries, "test_module", org_kp.public_key);
        std::cout << "Tampered verify: "
                  << (ok3 ? "PASSED (BAD!)" : "FAILED (expected)")
                  << "\n";

        std::cout << "\n=== FileMapping test ===\n";
        auto mapping = machikado::FileMapping::from_pair(
            "bin/zygiskd64", "bin/daemon");
        auto mapped_entries = machikado::load_folder_files(
            dir, {}, {}, &mapping);
        for (const auto& e : mapped_entries) {
            std::cout << "  " << e.relative_path << "\n";
        }
        fs::remove_all(dir);
        std::cout << "\nDone.\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
