# machikado-cpp

ED25519 signing for the Machikado Mazoku module ecosystem.
Two-tier: **machikado** (files) + **mazoku** (org auth).
Supports machikado-only mode for single-keypair scenarios.

## Concepts

| Term | Description |
|------|-------------|
| **org key** | Organization/team key pair. Authorizes projects via mazoku. |
| **member key** | Project key pair. Signs module files (machikado). |
| **machikado** | 96 bytes: `signature(64) ‖ member_pk(32)`. |
| **mazoku** | 96 bytes: `signature(64) ‖ org_pk(32)`, over `module_id ‖ 0x00 ‖ member_pk`. |

## Usage

### Generate keys

```cpp
#include "machikado.h"
#include <fstream>
#include <iostream>

auto org_kp = *machikado::generate_keypair();
auto member_kp = *machikado::generate_keypair();

std::ofstream("org_sk", std::ios::binary)
    .write(reinterpret_cast<const char*>(org_kp.private_key.data()), 64);
std::ofstream("member_sk", std::ios::binary)
    .write(reinterpret_cast<const char*>(member_kp.private_key.data()), 64);
```

### Sign (build time)

```cpp
#include "machikado.h"

std::string module_id = "my_module";
auto entries = *machikado::load_folder_files(
    module_dir, {".git"}, {}, nullptr);

auto machikado = *machikado::sign_file_entries(entries, member_sk);
std::ofstream(module_dir / "machikado", std::ios::binary)
    .write(reinterpret_cast<const char*>(machikado.as_bytes().data()), 96);

auto mazoku = *machikado::sign_mazoku(module_id, member_kp.public_key, org_sk);
std::ofstream(module_dir / "mazoku", std::ios::binary)
    .write(reinterpret_cast<const char*>(mazoku.as_bytes().data()), 96);
```

### Verify two-tier (device side)

```cpp
#include "machikado.h"

std::string module_id = "my_module";

auto entries = *machikado::load_folder_files(
    dir, {}, {"machikado", "mazoku"}, nullptr);

std::ifstream mf(dir / "machikado", std::ios::binary);
std::vector<uint8_t> machikado_bytes(
    (std::istreambuf_iterator<char>(mf)), std::istreambuf_iterator<char>());

std::ifstream mk(dir / "mazoku", std::ios::binary);
std::vector<uint8_t> mazoku_bytes(
    (std::istreambuf_iterator<char>(mk)), std::istreambuf_iterator<char>());

auto [ok, err] = machikado::verify(
    machikado_bytes, mazoku_bytes, entries, module_id, expected_org_pk);
if (!ok) {}
```

### Verify machikado-only (single keypair)

```cpp
#include "machikado.h"

auto kp = *machikado::generate_keypair();
auto entries = *machikado::load_folder_files(dir, {}, {"machikado"}, nullptr);

auto machikado = *machikado::sign_file_entries(entries, kp.private_key);

auto [ok, err] = machikado::verify_machikado(
    machikado.to_vec(), entries, kp.public_key);
if (!ok) {}
```

### File mapping

Map source paths to signed paths — for when `customize.sh` moves files at install time.

#### Load from all files

```cpp
#include "machikado.h"

// Single pair
auto mapping = machikado::FileMapping::from_pair(
    "bin/zygiskd64", std::optional<std::string>("bin/arm64-v8a/zygiskd"));

// Array of pairs
auto mapping = machikado::FileMapping::from_pairs({
    {"bin/zygiskd64", std::optional<std::string>("bin/arm64-v8a/zygiskd")},
    {"bin/zygiskd32", std::optional<std::string>("bin/armeabi-v7a/zygiskd")},
    {"module.prop", std::nullopt},
    {"action.sh", std::nullopt},
});

// Iteration
for (const auto& [target, source_opt] : mapping) {
    if (source_opt) {
        std::cout << target << " -> " << *source_opt << "\n";
    } else {
        std::cout << target << "\n";
    }
}

auto entries = machikado::load_folder_files(dir, {}, {}, &mapping);
```

#### Load from file mapping

```cpp
#include "machikado.h"

// Single pair
auto mapping = machikado::FileMapping::from_pair(
    "bin/zygiskd64", std::optional<std::string>("bin/arm64-v8a/zygiskd"));

// Array of pairs
auto mapping = machikado::FileMapping::from_pairs({
    {"bin/zygiskd64", std::optional<std::string>("bin/arm64-v8a/zygiskd")},
    {"bin/zygiskd32", std::optional<std::string>("bin/armeabi-v7a/zygiskd")},
    {"module.prop", std::nullopt},
    {"action.sh", std::nullopt},
});

// Iteration
for (const auto& [target, source_opt] : mapping) {
    if (source_opt) {
        std::cout << target << " -> " << *source_opt << "\n";
    } else {
        std::cout << target << "\n";
    }
}

auto entries = machikado::load_from_mapping(dir, mapping);
```

## API

| Function | Returns |
|----------|---------|
| `generate_keypair()` | `std::optional<Ed25519KeyPair>` |
| `sign_file_entries(entries, private_key)` | `std::optional<SignedBlob>` |
| `sign_mazoku(module_id, project_pk, org_sk)` | `std::optional<SignedBlob>` |
| `verify(machikado_blob, mazoku_blob, entries, module_id, expected_org_pk)` | `pair<bool, optional<SignError>>` |
| `verify_machikado(machikado_blob, entries, expected_pk)` | `pair<bool, optional<SignError>>` |
| `load_folder_files(folder, ignore_prefixes, ignore_names, mapping)` | `std::optional<std::vector<FileEntry>>` |
| `load_from_mapping(folder, mapping)` | `std::optional<std::vector<FileEntry>>` |

`SignedBlob` is a 96-byte newtype with `.as_bytes()`, `.to_vec()`, `.signature()`, `.public_key()`.

## Signing protocol

Compatible with ZygiskNext. Each file feeds into the signature as:

```
relative_path ‖ 0x00 ‖ file_size(LE u64) ‖ file_content
```

Accumulated in lexicographic order, signed once.

**mazoku** signing data: `module_id ‖ 0x00 ‖ project_public_key`, where `module_id` must match `^[a-zA-Z][a-zA-Z0-9._-]+$`.

Both `verify` and `verify_machikado` compare the embedded public key against a hardcoded expected key for integrity.

## License

* [Apache 2.0 license](https://www.apache.org/licenses/LICENSE-2.0)
