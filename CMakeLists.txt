cmake_minimum_required(VERSION 3.16)
project(machikado LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 99)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(SODIUM_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/libsodium" CACHE PATH "libsodium source dir")

if(NOT DEFINED SODIUM_BINARY_DIR)
    set(SODIUM_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/sodium-build" CACHE PATH "libsodium build dir")
endif()

if(NOT EXISTS "${SODIUM_SOURCE_DIR}/src/libsodium/include/sodium.h")
    message(FATAL_ERROR
        "\nlibsodium submodule not found at: ${SODIUM_SOURCE_DIR}\n"
        "Run: git submodule update --init --recursive\n")
endif()

set(SODIUM_SOURCES
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_aead/aegis128l/aead_aegis128l.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_aead/aegis128l/aegis128l_aesni.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_aead/aegis128l/aegis128l_armcrypto.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_aead/aegis128l/aegis128l_soft.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_aead/aegis256/aead_aegis256.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_aead/aegis256/aegis256_aesni.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_aead/aegis256/aegis256_armcrypto.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_aead/aegis256/aegis256_soft.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_aead/aes256gcm/aead_aes256gcm.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_aead/aes256gcm/aesni/aead_aes256gcm_aesni.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_aead/aes256gcm/armcrypto/aead_aes256gcm_armcrypto.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_aead/chacha20poly1305/aead_chacha20poly1305.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_aead/xchacha20poly1305/aead_xchacha20poly1305.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_auth/crypto_auth.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_auth/hmacsha256/auth_hmacsha256.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_auth/hmacsha512/auth_hmacsha512.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_auth/hmacsha512256/auth_hmacsha512256.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_box/crypto_box.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_box/crypto_box_easy.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_box/crypto_box_seal.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_box/curve25519xchacha20poly1305/box_curve25519xchacha20poly1305.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_box/curve25519xchacha20poly1305/box_seal_curve25519xchacha20poly1305.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_box/curve25519xsalsa20poly1305/box_curve25519xsalsa20poly1305.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_core/ed25519/core_ed25519.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_core/ed25519/core_ristretto255.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_core/ed25519/ref10/ed25519_ref10.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_core/hchacha20/core_hchacha20.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_core/hsalsa20/core_hsalsa20.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_core/hsalsa20/ref2/core_hsalsa20_ref2.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_core/keccak1600/armsha3/keccak1600_armsha3.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_core/keccak1600/keccak1600.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_core/keccak1600/ref/keccak1600_ref.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_core/salsa/ref/core_salsa_ref.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_core/softaes/softaes.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_generichash/blake2b/generichash_blake2.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_generichash/blake2b/ref/blake2b-compress-avx2.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_generichash/blake2b/ref/blake2b-compress-ref.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_generichash/blake2b/ref/blake2b-compress-sse41.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_generichash/blake2b/ref/blake2b-compress-ssse3.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_generichash/blake2b/ref/blake2b-ref.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_generichash/blake2b/ref/generichash_blake2b.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_generichash/crypto_generichash.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_hash/crypto_hash.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_hash/sha256/cp/hash_sha256_cp.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_hash/sha256/hash_sha256.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_hash/sha3/hash_sha3.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_hash/sha512/cp/hash_sha512_cp.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_hash/sha512/hash_sha512.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_ipcrypt/crypto_ipcrypt.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_ipcrypt/ipcrypt_aesni.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_ipcrypt/ipcrypt_armcrypto.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_ipcrypt/ipcrypt_soft.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_kdf/blake2b/kdf_blake2b.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_kdf/crypto_kdf.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_kdf/hkdf/kdf_hkdf_sha256.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_kdf/hkdf/kdf_hkdf_sha512.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_kem/crypto_kem.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_kem/mlkem768/kem_mlkem768.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_kem/mlkem768/ref/kem_mlkem768_ref.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_kem/xwing/kem_xwing.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_kx/crypto_kx.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_onetimeauth/crypto_onetimeauth.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_onetimeauth/poly1305/donna/poly1305_donna.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_onetimeauth/poly1305/onetimeauth_poly1305.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_onetimeauth/poly1305/sse2/poly1305_sse2.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/argon2-core.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/argon2-encoding.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/argon2-fill-block-avx2.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/argon2-fill-block-avx512f.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/argon2-fill-block-neon.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/argon2-fill-block-ref.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/argon2-fill-block-ssse3.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/argon2-fill-block-wasm32.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/argon2.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/blake2b-long.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/pwhash_argon2i.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/pwhash_argon2id.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_pwhash/crypto_pwhash.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_pwhash/scryptsalsa208sha256/crypto_scrypt-common.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_pwhash/scryptsalsa208sha256/nosse/pwhash_scryptsalsa208sha256_nosse.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_pwhash/scryptsalsa208sha256/pbkdf2-sha256.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_pwhash/scryptsalsa208sha256/pwhash_scryptsalsa208sha256.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_pwhash/scryptsalsa208sha256/scrypt_platform.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_pwhash/scryptsalsa208sha256/sse/pwhash_scryptsalsa208sha256_sse.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_scalarmult/crypto_scalarmult.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_scalarmult/curve25519/ref10/x25519_ref10.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_scalarmult/curve25519/sandy2x/curve25519_sandy2x.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_scalarmult/curve25519/sandy2x/fe51_invert.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_scalarmult/curve25519/sandy2x/fe_frombytes_sandy2x.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_scalarmult/curve25519/scalarmult_curve25519.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_scalarmult/ed25519/ref10/scalarmult_ed25519_ref10.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_scalarmult/ristretto255/ref10/scalarmult_ristretto255_ref10.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_secretbox/crypto_secretbox.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_secretbox/crypto_secretbox_easy.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_secretbox/xchacha20poly1305/secretbox_xchacha20poly1305.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_secretbox/xsalsa20poly1305/secretbox_xsalsa20poly1305.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_secretstream/xchacha20poly1305/secretstream_xchacha20poly1305.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_shorthash/crypto_shorthash.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_shorthash/siphash24/ref/shorthash_siphash24_ref.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_shorthash/siphash24/ref/shorthash_siphashx24_ref.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_shorthash/siphash24/shorthash_siphash24.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_shorthash/siphash24/shorthash_siphashx24.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_sign/crypto_sign.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_sign/ed25519/ref10/keypair.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_sign/ed25519/ref10/obsolete.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_sign/ed25519/ref10/open.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_sign/ed25519/ref10/sign.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_sign/ed25519/sign_ed25519.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_stream/chacha20/dolbeau/chacha20_dolbeau-avx2.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_stream/chacha20/dolbeau/chacha20_dolbeau-ssse3.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_stream/chacha20/ref/chacha20_ref.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_stream/chacha20/stream_chacha20.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_stream/crypto_stream.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_stream/salsa20/ref/salsa20_ref.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_stream/salsa20/stream_salsa20.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_stream/salsa20/xmm6/salsa20_xmm6.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_stream/salsa20/xmm6int/salsa20_xmm6int-avx2.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_stream/salsa20/xmm6int/salsa20_xmm6int-sse2.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_stream/salsa2012/ref/stream_salsa2012_ref.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_stream/salsa2012/stream_salsa2012.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_stream/salsa208/ref/stream_salsa208_ref.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_stream/salsa208/stream_salsa208.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_stream/xchacha20/stream_xchacha20.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_stream/xsalsa20/stream_xsalsa20.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_verify/verify.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_xof/shake128/ref/shake128_ref.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_xof/shake128/xof_shake128.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_xof/shake256/ref/shake256_ref.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_xof/shake256/xof_shake256.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_xof/turboshake128/ref/turboshake128_ref.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_xof/turboshake128/xof_turboshake128.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_xof/turboshake256/ref/turboshake256_ref.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/crypto_xof/turboshake256/xof_turboshake256.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/randombytes/internal/randombytes_internal_random.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/randombytes/randombytes.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/randombytes/sysrandom/randombytes_sysrandom.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/sodium/codecs.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/sodium/core.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/sodium/runtime.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/sodium/utils.c
    ${SODIUM_SOURCE_DIR}/src/libsodium/sodium/version.c
)

set(SODIUM_VERSION_H "${CMAKE_CURRENT_BINARY_DIR}/sodium_include/sodium/version.h")
set(SODIUM_VERSION_H_TEMPLATE "${SODIUM_SOURCE_DIR}/src/libsodium/include/sodium/version.h.in")

set(VERSION 1.0.22)
set(SODIUM_LIBRARY_VERSION_MAJOR 26)
set(SODIUM_LIBRARY_VERSION_MINOR 4)
set(SODIUM_LIBRARY_MINIMAL_DEF "/* #undef SODIUM_LIBRARY_MINIMAL */")
configure_file("${SODIUM_VERSION_H_TEMPLATE}" "${SODIUM_BINARY_DIR}/sodium/version.h" @ONLY)

add_library(sodium STATIC ${SODIUM_SOURCES})
target_compile_options(sodium PRIVATE -w)
target_include_directories(sodium
    PUBLIC
        ${SODIUM_SOURCE_DIR}/src/libsodium/include
        ${SODIUM_SOURCE_DIR}/src/libsodium/include/sodium
        ${SODIUM_BINARY_DIR}
    PRIVATE
        ${SODIUM_BINARY_DIR}/sodium
)

target_compile_definitions(sodium
    PUBLIC
        $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:SODIUM_STATIC>
    PRIVATE
        CONFIGURED
        $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

add_library(machikado STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/key.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sign.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.cpp
)

target_include_directories(machikado
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes
)

target_link_libraries(machikado PUBLIC sodium)
add_dependencies(machikado sodium)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU" AND NOT ANDROID)
    add_library(machikado_check OBJECT
        ${CMAKE_CURRENT_SOURCE_DIR}/src/key.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/sign.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.cpp
    )
    target_include_directories(machikado_check PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)
    target_link_libraries(machikado_check PRIVATE sodium)
    add_dependencies(machikado_check sodium)
    target_compile_options(machikado_check PRIVATE
        -fsyntax-only -Wall -Wextra -Wpedantic -Wno-unused-parameter)

    add_custom_target(check DEPENDS machikado_check COMMENT "syntax check passed")
endif()

option(MACHIKADO_BUILD_EXAMPLES "Build examples (desktop only)" OFF)

if(MACHIKADO_BUILD_EXAMPLES AND NOT ANDROID
   AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/main.cpp")
    add_executable(machikado_example ${CMAKE_CURRENT_SOURCE_DIR}/examples/main.cpp)
    target_link_libraries(machikado_example PRIVATE machikado)
    add_dependencies(machikado_example sodium)
endif()