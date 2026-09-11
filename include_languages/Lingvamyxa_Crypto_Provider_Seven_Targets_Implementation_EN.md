# Lingvamyxa Crypto Provider — Implementation Notes for the Seven Planned Targets

**Status:** implementation planning note  
**Date:** 2026-09-11  
**Normative contract:** [Lingvamyxa_spec.txt](../Lingvamyxa_spec.txt), sections 19.32.10–19.32.21 (integrated from `Lingvamyxa_L3_Crypto_Provider_Contract_Proposal_EN.md`).  
**Current port list:** reconstructed from the Revision-2 specification and the discussed VM porting plan.

---

# 0. Seven targets

The planned targets divide into two groups.

## Native/C-machine route

```text
1. MIR
2. WebAssembly linear memory
3. RISC-V
```

These reuse the C-machine route:

```text
L2 -> Translator-L2 -> L1 -> Translator-L1 -> C99 -> target toolchain
```

Therefore they can use the C99/libsodium Crypto Provider contract implementation.

They still differ in linking, entropy, memory-protection and execution-environment details, but they do not need a new L3 crypto semantics.

## Direct L3 VM route

```text
4. JVM classfiles
5. WebAssembly GC
6. Lua 5.4 bytecode
7. .NET CIL
```

These require target-specific Crypto Provider adapters.

The L3 contract remains the same.

---

# 1. Common rule for all seven targets

The portable layer must not be:

```text
libsodium API
```

It must be:

```text
L3 Crypto Provider contract
```

For example:

```text
Signature.sign(Ed25519, privateKeyRef, message)
```

may map to:

```text
MIR/RISC-V/Wasm-linear -> libsodium
JVM                    -> JCA/provider
Wasm-GC/browser        -> WebCrypto host
Lua                    -> host/engine/libsodium module
.NET CIL               -> System.Security.Cryptography / selected provider
```

`privateKeyRef` is a target/provider handle, not a portable raw pointer or mandatory raw key byte array.

---

# 2. Capability matrix at a glance

This table describes the most practical first implementation, not language semantics.

| Capability | MIR / C99 | Wasm linear / C99 | RISC-V / C99 | JVM | Wasm GC | Lua 5.4 | .NET CIL |
|---|---|---|---|---|---|---|---|
| Secure random | libsodium | libsodium + host entropy | libsodium + OS/target entropy | `SecureRandom` | host/WebCrypto | host/C module | `RandomNumberGenerator` |
| SHA-256 | libsodium | libsodium | libsodium | JCA | WebCrypto/host | host/C module | BCL |
| HMAC-SHA-256 | libsodium | libsodium | libsodium | JCA | WebCrypto/host | host/C module | BCL |
| HKDF-SHA-256 | provider wrapper | provider wrapper | provider wrapper | JCA on recent JDK / wrapper | WebCrypto/host | host/C module | wrapper/provider |
| Ed25519 | libsodium | libsodium | libsodium | JCA | WebCrypto/host | host/C module | selected provider; BC is practical |
| X25519 | libsodium | libsodium | libsodium | JCA | WebCrypto/host | host/C module | selected provider; BC is practical |
| Argon2id | libsodium | libsodium, memory-sensitive | libsodium | external provider such as BC | not WebCrypto baseline | host/C module | external provider such as BC |
| AEAD | libsodium | libsodium | libsodium | JCA | WebCrypto AES-GCM / host | host/C module | BCL/provider |
| Constant-time compare | libsodium | libsodium code, Wasm timing caveat | libsodium | provider helper | host crypto preferred | C/host provider | `FixedTimeEquals` |
| Opaque/non-exported key | runtime wrapper | runtime wrapper | runtime wrapper | `PrivateKey`/`SecretKey` provider object | `CryptoKey`/host handle | full userdata/host handle | provider object/wrapper |
| Strong secret-memory controls | host-dependent | weak/limited | OS-dependent | provider/VM-dependent | host-dependent | host-dependent | provider/managed-runtime-dependent |

Important:

> A row saying that an algorithm exists does not mean every runtime version or every embedder has the same security properties. Target profiles must state their provider/minimum-runtime requirements.

---

# 3. Targets 1–3: one C99/libsodium provider

The first three targets are comparatively uninteresting at the L3-contract level because they use the same C provider implementation.

The differences are primarily target/runtime integration.

---

# 4. Target 1 — MIR

## 4.1. Compilation route

Conceptually:

```text
L2
 -> L1
 -> generated C99
 -> MIR-capable C toolchain / C-to-MIR route
```

The crypto provider remains the existing C99/libsodium adapter.

L3 code that calls a high-level crypto operation still reaches the same provider contract when it is compiled through the native/C-machine route.

## 4.2. Provider implementation

Recommended:

```text
Lingvamyxa Crypto Provider C ABI
        |
        v
libsodium
```

The adapter owns initialization, algorithm/profile mapping, `KeyRef` runtime handles, provider-owned secret buffers, libsodium error conversion, and Lmx/public-value construction.

The L3 layer never calls `crypto_*` symbols directly.

## 4.3. Linking

The target needs one of:

```text
static libsodium
shared libsodium
host-resolved libsodium symbols
```

depending on the MIR execution model.

This is backend/toolchain configuration, not source semantics.

## 4.4. `KeyRef`

A practical native representation:

```text
KeyRef Lmx leaf / checked wrapper
        ->
provider handle record
        ->
provider-owned secret storage
```

The handle contract defines retain/release and whether more than one Message may use the same key capability.

No raw key pointer is exposed on L3.

## 4.5. Secure memory

Where the MIR program runs as an ordinary native host process, the C provider can use libsodium's native memory helpers where supported by the OS.

Do not make this an unconditional L3 guarantee.

The provider capability should distinguish `key is provider-owned` from stronger claims such as locked pages or guard pages.

---

# 5. Target 2 — WebAssembly linear memory

This target still uses the C99 provider path.

It is therefore different from target 5 (Wasm GC).

## 5.1. Compilation route

Conceptually:

```text
L2
 -> L1
 -> C99
 -> WebAssembly linear-memory toolchain
```

The same C libsodium provider can be compiled into the Wasm module/component.

Current libsodium has supported WebAssembly build paths. As of 2026, upstream libsodium has Emscripten/WebAssembly support and the current release line recommends Zig for WebAssembly/WASI(X) builds.

## 5.2. Functional provider mapping

The provider can keep almost exactly the native adapter:

```text
Random
PasswordVerifier
Hash/MAC
Signature
KeyAgreement
AEAD
```

and call the same libsodium C API compiled to WebAssembly.

This is the strongest reason to classify this target with MIR/RISC-V rather than with Wasm GC.

## 5.3. Entropy

A Wasm module has no ambient OS access by itself.

The libsodium build/runtime must obtain secure randomness through the target environment:

```text
browser host
WASI host
custom embedder
```

If secure entropy is unavailable, the Random provider capability is unavailable and key generation must fail.

No weak fallback PRNG.

## 5.4. Secret memory

Linear memory is not equivalent to native protected secret memory.

Even if the C provider calls a sodium secure-memory helper, the Wasm execution environment may not provide the same `mlock`, guard-page or native-process-memory properties as a native OS process.

Therefore distinguish functional crypto support from native-quality secret-memory hardening.

## 5.5. Timing side channels

This is a serious caveat.

WebAssembly core execution does not currently give a general guarantee that arbitrary compiled crypto code preserves constant-time behavior.

The WebAssembly constant-time proposal existed specifically because this guarantee was missing; the current WebAssembly proposal tracker lists that proposal as inactive.

Therefore:

- libsodium compiled to Wasm is useful and functionally compatible;
- the target profile must not automatically claim the strongest constant-time side-channel guarantee solely because the source C implementation was constant-time;
- high-assurance profiles may prefer host-provided crypto operations.

This limitation is target-specific and does not change the L3 contract.

## 5.6. Argon2

Argon2 is memory-hard.

This matters on Wasm because initial/max linear-memory limits can be tighter and browser environments may impose practical memory limits.

Therefore `PasswordVerifier/Argon2id` should be a declared capability, not assumed merely because some other libsodium algorithms are present.

---

# 6. Target 3 — RISC-V

## 6.1. Compilation route

Conceptually:

```text
L2
 -> L1
 -> C99
 -> RISC-V C toolchain
 -> native RISC-V artifact
```

The provider remains the C99/libsodium implementation.

## 6.2. Provider mapping

No new L3 crypto design is required.

Use libsodium portable C / selected optimized paths and the ordinary native `KeyRef` wrapper.

## 6.3. Main port issues

The interesting issues are not the high-level API but the target environment:

- libc availability;
- OS versus bare-metal runtime;
- secure entropy source;
- page locking / virtual memory availability;
- dynamic versus static linking;
- optional assembly/vector optimization availability;
- alignment and toolchain correctness.

A hosted Linux RISC-V target is comparatively straightforward.

A bare-metal RISC-V target needs an explicit entropy/device provider and cannot assume OS secure-memory facilities.

## 6.4. Security capability reporting

For bare metal, distinguish:

```text
crypto primitive available
```

from:

```text
secure entropy available
secret-memory hardening available
persistent secure key store available
```

Those are separate provider capabilities.

---

# 7. Target 4 — JVM classfiles

The JVM target should not use L2 or call the C libsodium implementation as its semantic baseline.

It binds the same L3 contract to Java crypto providers.

## 7.1. Primary provider

Recommended first provider:

```text
Java Cryptography Architecture (JCA/JCE)
```

Optional extension/fallback provider:

```text
Bouncy Castle
```

Recent Java standards include standard algorithm names for `SecureRandom`, SHA-256, HMAC-SHA-256, Ed25519/EdDSA, X25519/XDH, ChaCha20-Poly1305, AES-GCM and PBKDF2. Newer JDKs also define a `KDF` API with HKDF-SHA256.

The target profile should declare its minimum JVM/JDK provider requirements rather than assume every historical JVM has identical algorithms.

## 7.2. `KeyRef`

Natural mapping:

```text
L3 KeyRef
    ->
Java provider object:
    PrivateKey
    SecretKey
    provider-specific key object
```

This is a particularly clean fit.

Private keys need not be converted to byte arrays merely because L3 stores or passes a `KeyRef`.

Some Java private-key types implement/extend destruction interfaces; provider behavior differs, so the target must not claim stronger erasure than its actual provider supplies.

## 7.3. Public keys

Public key export/import can use explicit formats such as RAW where supported, X.509 SubjectPublicKeyInfo, or profile-specific canonical bytes.

The L3 interoperability profile determines the format.

JCA object serialization is not the wire format.

## 7.4. Ed25519/X25519

For a modern JVM provider:

```text
Signature("Ed25519")
KeyAgreement("X25519")
KeyPairGenerator / KeyFactory
```

map naturally to the contract.

Bouncy Castle can serve as a provider when the baseline JVM/provider lacks a required algorithm or when a specific cross-version implementation is wanted.

## 7.5. Password hashing

Standard JCA does not make Argon2id a universal baseline.

If the standard Lingvamyxa password-verifier profile remains Argon2id, an external provider is required.

Bouncy Castle provides Argon2 support.

If an application selects a PBKDF2 profile, JCA can provide that without Argon2.

Do not silently map an Argon2id verifier to PBKDF2.

## 7.6. HKDF

Modern JDKs have a KDF API and standard HKDF algorithm names.

For older target baselines, HKDF-SHA256 can be implemented inside the JVM provider using HMAC-SHA-256 without exposing raw L2 operations.

This is provider implementation code, not L3 crypto logic.

## 7.7. AEAD

Natural choices include:

```text
ChaCha20-Poly1305
AES-GCM
```

A protocol requiring exact interoperability must name one.

Do not make provider choice select the wire algorithm.

## 7.8. Secret memory

Managed Java heap does not provide the same physical-memory model as `sodium_malloc`.

Preferred policy:

- keep private keys inside provider key objects;
- avoid calling `getEncoded()` unless explicit export is requested;
- keep raw secret arrays short-lived when unavoidable;
- use provider/HSM support for stronger non-extractable storage.

L3 should see `KeyRef`, not the temporary Java bytes.

---

# 8. Target 5 — WebAssembly GC

This is a direct L3 backend and must not be confused with target 2.

Wasm 3.0 includes managed reference/GC types, which makes it suitable for the L3 graph model.

But core Wasm deliberately defines no ambient cryptography or OS service.

## 8.1. Core rule

A Wasm-GC module should import a Crypto Provider capability from its embedder.

Conceptually:

```text
L3 Wasm-GC code
    ->
imported crypto provider
    ->
browser / WASI host / game host / custom runtime
```

Do not compile an assumed C libsodium ABI into the L3 object model.

## 8.2. Browser profile: WebCrypto

For a browser-hosted target, WebCrypto is the natural primary provider.

Web Cryptography Level 2 defines operations for:

- Ed25519;
- X25519;
- AES-GCM;
- HMAC;
- SHA-256/384/512;
- HKDF;
- PBKDF2;
- key generation/import/export.

This maps very well to the proposed contract.

## 8.3. `KeyRef` mapping

WebCrypto's `CryptoKey` is almost exactly the desired concept.

A private key can be non-extractable and still be usable for the operations allowed by its usage policy.

Possible Wasm-side representation:

```text
externref to host CryptoKey
```

or:

```text
host-side capability id
```

if the embedder does not expose the object directly.

L3 must not depend on which representation is used.

## 8.4. Async provider

WebCrypto `SubtleCrypto` operations are asynchronous.

Therefore the Wasm-GC provider adapter may need Message request/result, a host callback bridge, or a future/continuation adapter rather than a direct synchronous instruction sequence.

This is why the Crypto Provider contract must be sync/async neutral.

## 8.5. Password hashing

WebCrypto baseline has PBKDF2 but not Argon2id.

Therefore an Argon2id Lingvamyxa verifier requires one of:

```text
host extension/provider
separate audited crypto service
explicit auxiliary Wasm/native module
```

or the target reports Argon2id unsupported.

It must not silently reinterpret the verifier as PBKDF2.

## 8.6. AEAD

Browser WebCrypto naturally supports AES-GCM.

It does not provide the same libsodium-first XChaCha20-Poly1305 API.

Therefore the AEAD family must remain separate from a concrete interoperability profile.

A WebCrypto-oriented protocol can choose AES-GCM.

A protocol that requires XChaCha20-Poly1305 must supply another provider.

## 8.7. Why host crypto is preferred over pure Wasm crypto

Wasm core 3.0 gives managed references but does not itself provide cryptographic side-channel guarantees.

The WebAssembly constant-time extension was proposed specifically to address this, and the current proposal tracker lists it as inactive.

Therefore for secret operations the host crypto capability is the safer default architecture.

L3 protocol/key-management logic still compiles normally to Wasm GC.

## 8.8. Standalone/non-browser Wasm GC

Core Wasm has no ambient OS API.

A non-browser embedder must explicitly supply crypto imports.

Do not assume a universal standardized WASI crypto API.

The target profile should name the host capability set it requires.

---

# 9. Target 6 — Lua 5.4

Lua is the most host-dependent target in this list.

Lua 5.4's standard libraries do not include cryptography.

This is not a problem for the L3 contract; it means the provider belongs to the Lua host/engine.

## 9.1. Three deployment cases

### Case A — Lingvamyxa controls the Lua host

Best implementation:

```text
Lua L3 runtime
    ->
C module / built-in host module
    ->
libsodium
```

This can reuse most of the existing C provider implementation.

In that case Lua becomes surprisingly easy.

### Case B — game engine allows native Lua extensions

Use an engine-approved native module backed by libsodium or the engine crypto API.

The Lingvamyxa Lua backend binds the high-level contract to that module.

### Case C — sandboxed engine does not allow native extensions

The only conforming choices are:

```text
engine-provided crypto capability
```

or:

```text
required crypto capability unavailable
```

Do not silently add a pure-Lua private-key implementation merely to make the build succeed.

## 9.2. `KeyRef`

When native modules are available, the clean representation is:

```text
Lua full userdata
    ->
provider key handle
```

Full userdata can carry controlled lifetime/finalization metadata.

Avoid exposing a native secret pointer as ordinary lightuserdata without an explicit lifetime wrapper.

For an engine-provided provider, `KeyRef` may instead be an engine object/capability proxy.

## 9.3. Secret bytes and Lua strings

Lua strings are ordinary immutable language values.

They are appropriate for public keys, signatures, ciphertext, nonces and hashes.

They are a poor default representation for long-lived private-key material because the runtime controls copying/storage and the L3 program cannot guarantee physical erasure.

Therefore private/symmetric keys should remain behind `KeyRef`/userdata where possible.

## 9.4. Do not implement core crypto in Lua bytecode

A pure-Lua implementation may be mathematically correct but is a poor security baseline because the Lua VM does not provide a portable constant-time execution contract or secure secret-memory model.

Therefore:

```text
key management / protocol logic -> L3/Lua
cryptographic primitives        -> host provider
```

is the intended split.

## 9.5. Game-engine portability

This provider abstraction is especially useful for Lua because "Lua support" does not imply one common host API.

Different engines can bind the same L3 Crypto Provider contract to an engine crypto API, bundled C module, platform security service, or remote crypto service.

A game-specific backend module does not change the language contract.

---

# 10. Target 7 — .NET CIL

This target is structurally close to JVM.

Primary provider:

```text
System.Security.Cryptography
```

with an optional additional provider such as Bouncy Castle when the required algorithm/profile is not uniformly available in the selected .NET/runtime/OS combination.

## 10.1. Strong BCL coverage

The .NET cryptography APIs provide useful direct mappings including:

```text
RandomNumberGenerator
SHA-256 and other hashes
HMAC-SHA-256
AesGcm
ChaCha20Poly1305
CryptographicOperations.FixedTimeEquals
CryptographicOperations.ZeroMemory
```

This is enough for a large portion of the provider contract without libsodium.

## 10.2. `KeyRef`

Possible provider representations:

```text
AsymmetricAlgorithm-derived provider object
provider-specific private-key object
SafeHandle to native/provider key
secret-key wrapper owned by runtime
Bouncy Castle key object
```

The L3 reference remains opaque.

Where a type implements `IDisposable`, deterministic provider release can be mapped to the `KeyRef` lifetime contract.

## 10.3. Ed25519/X25519

Support details vary with .NET version, OS provider, and API surface.

For a Lingvamyxa target that requires stable cross-platform Ed25519/X25519 behavior, a dedicated selected provider is safer than assuming every historical .NET BCL/runtime combination exposes the same API.

Bouncy Castle C# is a practical provider option and supports modern Edwards/Montgomery-curve operations.

Therefore the target profile should state required provider/minimum runtime rather than encode a fragile assumption into L3 semantics.

## 10.4. Argon2id

Argon2 is not a universal `System.Security.Cryptography` baseline.

If the standard Lingvamyxa verifier profile is Argon2id, bind an additional provider.

Bouncy Castle C# includes Argon2 functionality.

Again:

```text
unsupported != silently use PBKDF2
```

## 10.5. Secret buffers

When raw secret bytes are unavoidable, `CryptographicOperations.ZeroMemory` can clear runtime-owned spans/buffers.

This does not prove that no other managed/native copy exists.

Therefore the preferred model remains opaque `KeyRef` rather than persistent L3 byte arrays.

## 10.6. Constant-time comparison

.NET provides `CryptographicOperations.FixedTimeEquals`, whose documented behavior for equal-length inputs is independent of byte values.

That maps directly to the contract's constant-time equality operation.

## 10.7. AEAD

Useful native BCL profiles include:

```text
AES-GCM
ChaCha20-Poly1305
```

Choose the exact algorithm in the Lingvamyxa interoperability profile.

Do not choose it implicitly from whichever class exists first on a machine.

---

# 11. Cross-target algorithm strategy

The most important practical conclusion is:

> Do not define "libsodium algorithms" as the language contract.

Define operation families and explicit interoperability profiles.

## 11.1. Good common primitive profiles

The following are relatively easy to provide across targets:

```text
SHA-256
HMAC-SHA-256
Ed25519
X25519
```

with provider additions where necessary.

HKDF-SHA-256 is also practical across targets, although some runtimes need a small provider wrapper rather than one built-in call.

## 11.2. Password verifier

If strong password storage is the goal, Argon2id is a good standard profile for native/JVM/.NET/provider-enabled Lua/Wasm.

But it is not a bare WebCrypto operation.

Therefore Wasm-GC/browser either needs an added provider or explicitly lacks that profile.

A separate WebCrypto-only profile may use PBKDF2-HMAC-SHA-256 where that tradeoff is intentionally selected by the application.

## 11.3. AEAD has no perfect built-in intersection

Current convenient defaults differ:

```text
libsodium    -> ChaCha20/XChaCha20-Poly1305
JVM          -> ChaCha20-Poly1305, AES-GCM
WebCrypto    -> AES-GCM
.NET         -> ChaCha20-Poly1305, AES-GCM
Lua          -> whatever host provider supplies
```

Therefore the base Crypto Provider contract should say `AEAD`, while wire protocols select one exact profile.

If all seven targets must decrypt exactly the same ciphertext, require the same algorithm provider on all seven.

For example, a project could choose AES-GCM and require an AES-GCM provider everywhere; choose ChaCha20-Poly1305 and add a non-WebCrypto browser provider; or choose XChaCha20-Poly1305 and require libsodium/compatible providers on every participant.

The language must not silently translate ciphertext between algorithms.

---

# 12. Async behavior by target

| Target | Typical crypto execution |
|---|---|
| MIR | synchronous native call |
| Wasm linear | synchronous Wasm call; host entropy/import may be involved |
| RISC-V | synchronous native call |
| JVM | usually synchronous provider call |
| Wasm GC/browser | commonly asynchronous WebCrypto |
| Lua | host-dependent; synchronous C module or async engine/service |
| .NET CIL | usually synchronous primitive calls; external/HSM provider may be async |

Therefore the high-level contract must not require a physical C-style synchronous ABI.

A provider adapter may complete through Message/result/service machinery.

---

# 13. KeyRef representation by target

| Target | Suggested physical representation |
|---|---|
| MIR | C runtime provider handle |
| Wasm linear | integer/offset handle to provider-owned Wasm linear-memory state; never exposed as L3 raw address |
| RISC-V | C runtime provider handle |
| JVM | `PrivateKey`, `SecretKey`, provider key object |
| Wasm GC | `externref`/host capability to `CryptoKey` or equivalent |
| Lua 5.4 | full userdata / engine capability |
| .NET CIL | crypto provider object / wrapper / `SafeHandle` |

This is the same semantic `KeyRef`.

---

# 14. What remains in portable L3 code

The following should be shared unchanged among JVM, Wasm GC, Lua and .NET ports, and also usable on the native route:

```text
key ids
public-key directories
key ownership relationships
key versioning
rotation
expiry
revocation
allowed-use policy
verifier realms
delegated tickets
AuthEvidence
which public key belongs to which peer
which key protects which object
protocol state
algorithm/profile selection
wire envelope construction
authorization based on verified evidence
```

This is the main portability gain.

---

# 15. What must remain provider-specific

```text
secure RNG
private-key generation
private-key physical storage
signature primitive
key agreement primitive
password KDF implementation
AEAD primitive
constant-time byte comparison
secure zeroization/page locking
HSM/OS keystore integration
hardware acceleration
```

Some targets may implement a few of these in ordinary managed code, but they still belong behind the provider contract.

---

# 16. Recommended provider layout

A practical source tree could conceptually separate:

```text
crypto/
    contract/               # L3-visible contracts/policy Structures
    profiles/               # Ed25519, X25519, Argon2id, AEAD profiles
    native-sodium/          # C99/L2 provider for targets 1–3
    jvm/                    # JCA + optional BC
    wasm-gc-host/           # WebCrypto/custom host bridge
    lua-host/               # native module / engine bridge
    dotnet/                 # BCL + optional BC
```

These names are illustrative.

No directory layout is normative.

---

# 17. Build/link behavior

If an application requires:

```text
Signature Ed25519
PasswordVerifier Argon2id
AEAD AES-GCM
```

the target builder resolves those requirements against the selected provider set.

Possible result:

```text
JVM:
    Ed25519        -> JCA
    Argon2id       -> BC
    AES-GCM        -> JCA
    build succeeds

Wasm-GC/browser, WebCrypto-only:
    Ed25519        -> WebCrypto
    Argon2id       -> missing
    AES-GCM        -> WebCrypto
    build fails: missing Argon2id provider
```

It must not turn the second target into PBKDF2 instead of Argon2id without explicit application/profile selection.

---

# 18. Testing strategy

The same provider conformance suite should run on all seven targets.

For each claimed profile:

```text
known test vector
    -> provider operation
    -> exact expected output/result
```

Test at least:

- SHA-256;
- HMAC-SHA-256;
- Ed25519 verify/sign vectors;
- X25519 agreement vectors;
- KDF vectors;
- AEAD vectors for each selected profile;
- password verifier import/create/verify;
- invalid signature;
- invalid AEAD tag;
- unsupported algorithm;
- non-exportable private key;
- provider unavailable;
- KeyRef lifetime/release.

Additionally verify target-specific security claims separately:

```text
constant-time guarantee
secure erase
non-extractability
host entropy
```

Functional vector success alone does not prove those properties.

---

# 19. Target-specific risk summary

## MIR

Main risk: toolchain/link/runtime integration. Crypto semantics are straightforward.

## Wasm linear memory

Main risks: entropy integration, memory limits for Argon2, no universal native-style secure memory, constant-time/side-channel guarantees.

## RISC-V

Main risks: bare-metal entropy, OS/security facilities, toolchain/optimization coverage.

## JVM

Main risks: minimum JDK/provider version, Argon2 external provider, secret-memory guarantees weaker than native secure allocation.

## Wasm GC

Main risks: no core crypto API, host capability required, WebCrypto is async, Argon2 absent from WebCrypto, AEAD overlap differs from libsodium.

## Lua 5.4

Main risks: no standard crypto, foreign game host may forbid native modules, pure Lua has unsuitable security/timing guarantees, secret bytes should not live as ordinary Lua strings.

## .NET CIL

Main risks: algorithm availability varies by runtime/provider, Argon2 external provider, Ed25519/X25519 portability should use an explicitly selected provider/profile, managed-memory secret handling.

---

# 20. Recommended first implementation order

1. Define the L3 contract independent of any library.
2. Refactor current C99/libsodium code to implement that contract.
3. Use the same conformance vectors on MIR, Wasm-linear and RISC-V.
4. Implement JVM provider with JCA; add BC only for missing required profiles.
5. Implement Wasm-GC host bridge:
   - WebCrypto browser provider first;
   - explicit missing capability diagnostics.
6. Implement Lua host-provider interface:
   - libsodium C module when Lingvamyxa controls the host;
   - engine provider adapters separately.
7. Implement .NET provider:
   - BCL first;
   - selected external provider for Ed/X/Argon2 profiles where required.
8. Run one cross-target wire-profile suite to verify that provider choice never changes protocol bytes.

---

# 21. External implementation references checked on 2026-09-11

These references are implementation research, not Lingvamyxa normative sources.

## Java / JVM

Oracle Java Security Standard Algorithm Names, Java SE 26:  
https://docs.oracle.com/en/java/javase/26/docs/specs/security/standard-names.html

Oracle `javax.crypto.KDF`, Java SE 26:  
https://docs.oracle.com/en/java/javase/26/docs/api/java.base/javax/crypto/KDF.html

Oracle `PrivateKey`, Java SE 26:  
https://docs.oracle.com/en/java/javase/26/docs/api/java.base/java/security/PrivateKey.html

## WebAssembly GC / WebCrypto

WebAssembly 3.0 core specification:  
https://webassembly.github.io/spec/core/

W3C Web Cryptography Level 2:  
https://www.w3.org/TR/webcrypto-2/

WebAssembly constant-time proposal overview:  
https://github.com/WebAssembly/constant-time/blob/main/proposals/constant-time/Overview.md

Current WebAssembly inactive proposals list:  
https://github.com/WebAssembly/proposals/blob/main/inactive-proposals.md

## Lua 5.4

Lua 5.4 Reference Manual:  
https://www.lua.org/manual/5.4/manual.html

Lua 5.4 source tree:  
https://www.lua.org/source/5.4/

## .NET

`RandomNumberGenerator.Fill`:  
https://learn.microsoft.com/en-us/dotnet/api/system.security.cryptography.randomnumbergenerator.fill?view=net-10.0

`CryptographicOperations.FixedTimeEquals`:  
https://learn.microsoft.com/en-us/dotnet/api/system.security.cryptography.cryptographicoperations.fixedtimeequals?view=net-10.0

`CryptographicOperations.ZeroMemory`:  
https://learn.microsoft.com/en-us/dotnet/api/system.security.cryptography.cryptographicoperations.zeromemory?view=net-10.0

`ChaCha20Poly1305`:  
https://learn.microsoft.com/en-us/dotnet/api/system.security.cryptography.chacha20poly1305?view=net-10.0

`AesGcm`:  
https://learn.microsoft.com/en-us/dotnet/api/system.security.cryptography.aesgcm?view=net-10.0

Bouncy Castle C# downloads/release information:  
https://www.bouncycastle.org/download/bouncy-castle-c/

## libsodium / WebAssembly

libsodium documentation:  
https://doc.libsodium.org/

libsodium WebAssembly/Emscripten build script:  
https://github.com/jedisct1/libsodium/blob/master/dist-build/emscripten.sh

libsodium.js / WebAssembly wrapper project:  
https://github.com/jedisct1/libsodium.js

---

# 22. Final implementation principle

The seven ports should differ here:

```text
how crypto primitives are executed
how secret keys are physically represented
how host security facilities are reached
```

They should not differ here:

```text
what a key means
what a verifier means
what Sign/Verify means
what KeyAgreement means
what AEAD authentication means
how algorithm identity is represented
how policy/rotation/revocation works
how AuthEvidence enters Message validation
```

That shared part belongs in L3.
