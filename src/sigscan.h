#pragma once

#define AOB_PATTERN(str) \
  CreateAOBPattern<sigscan_helpers::AOBCount(std::string_view(str))>(std::string_view(str))

namespace sigscan_helpers {

constexpr byte HexDigit(char c) {
  if (c >= '0' && c <= '9') return static_cast<byte>(c - '0');
  if (c >= 'a' && c <= 'f') return static_cast<byte>(c - 'a' + 10);
  if (c >= 'A' && c <= 'F') return static_cast<byte>(c - 'A' + 10);
  throw std::invalid_argument("invalid hex digit");
}
constexpr byte HexByte(char hi, char lo) {
  return static_cast<byte>((HexDigit(hi) << 4) | HexDigit(lo));
}
constexpr size_t AOBCount(std::string_view s) {
  size_t count = 0;
  size_t i = 0;
  while (i < s.size()) {
    while (i < s.size() && s[i] == ' ') ++i;  // skip spaces
    if (i >= s.size()) break;
    // consume one token (run of non-space chars)
    while (i < s.size() && s[i] != ' ') ++i;
    ++count;
  }
  return count;
}
}  // namespace sigscan_helpers

typedef unsigned char byte;

template <size_t N>
struct Pattern {
  std::array<byte, N> needle{0};
  std::array<bool, N> wildcard{0};  // wildcard[i] == true means byte i matches anything
  bool has_wildcard{false};
  constexpr auto size() const { return N; }
};

// Depending on your input it will include a null terminator
template <size_t N>
constexpr Pattern<N> CreateTextPattern(const char (&text)[N]) {
  Pattern<N> pat;
  std::memcpy((void*)pat.needle.data(), text, N);
  return pat;
}

//   "44 55 66 ? 99 AA ??"
//   Wildcards: "?" or "??" (both treated identically)
template <size_t N>
constexpr Pattern<N> CreateAOBPattern(std::string_view s) {
  Pattern<N> pat{};
  size_t idx = 0;
  size_t i = 0;

  while (i < s.size() && idx < N) {
    while (i < s.size() && s[i] == ' ') ++i;
    if (i >= s.size()) break;

    if (s[i] == '?') {
      // wildcard: "?" or "??"
      pat.needle[idx] = 0x00;
      pat.wildcard[idx] = true;
      pat.has_wildcard = true;
      ++i;
      if (i < s.size() && s[i] == '?') ++i;  // consume optional second '?'
    } else {
      // concrete byte: exactly two hex digits
      if (i + 1 >= s.size()) throw std::invalid_argument("truncated hex byte in AOB pattern");
      pat.needle[idx] = sigscan_helpers::HexByte(s[i], s[i + 1]);
      pat.wildcard[idx] = false;
      i += 2;
    }
    ++idx;
  }

  if (idx != N) throw std::invalid_argument("AOB token count does not match N");

  return pat;
}

//   needle: "\x44\x55\x66\xCC\x99\xAA\xCC"
//   mask:   "xx???xx"        ('x' = concrete, '?' = wildcard)
//   N is deduced from the mask length.
template <size_t N>
constexpr Pattern<N> CreateBinaryPattern(const char (&needle)[N + 1], std::string_view mask) {
  if (mask.size() != N) throw std::invalid_argument("mask length must equal needle length");

  Pattern<N> pat{};
  for (size_t i = 0; i < N; ++i) {
    pat.needle[i] = static_cast<byte>(needle[i]);
    pat.wildcard[i] = (mask[i] == '?');
    if (pat.wildcard[i]) pat.has_wildcard = true;
  }
  return pat;
}

// http://0x80.pl/notesen/2016-11-28-simd-strfind.html#generic-sse-avx2
template <size_t N>
size_t avx2_strstr_anysize(const byte* s, size_t n, const Pattern<N>& pat) {
  // Find first and last non-wildcard bytes for the AVX2 candidate filter.
  // Fall back to index 0 / len-1 if those are wildcards.
  size_t first_idx = 0;
  size_t last_idx = pat.size() - 1;

  if (pat.has_wildcard) {
    // Advance first_idx to the first concrete byte
    while (first_idx < pat.size() && pat.wildcard[first_idx]) ++first_idx;
    // Retreat last_idx to the last concrete byte
    while (last_idx > first_idx && pat.wildcard[last_idx]) --last_idx;

    // Degenerate: all-wildcard pattern matches at offset 0
    if (first_idx == last_idx && pat.wildcard[first_idx])
      return (n >= pat.size()) ? 0 : std::string::npos;
  }

  const __m256i first = _mm256_set1_epi8(pat.needle[first_idx]);
  const __m256i last = _mm256_set1_epi8(pat.needle[last_idx]);

  for (size_t i = 0; i + pat.size() <= n + 32; i += 32) {
    const __m256i block_first =
        _mm256_loadu_si256(reinterpret_cast<const __m256i*>(s + i + first_idx));
    const __m256i block_last =
        _mm256_loadu_si256(reinterpret_cast<const __m256i*>(s + i + last_idx));

    const __m256i eq_first = _mm256_cmpeq_epi8(first, block_first);
    const __m256i eq_last = _mm256_cmpeq_epi8(last, block_last);

    uint32_t mask = _mm256_movemask_epi8(_mm256_and_si256(eq_first, eq_last));

    while (mask != 0) {
      const uint32_t bitpos = __builtin_ctz(mask);
      const size_t candidate = i + bitpos;

      if (candidate + pat.size() <= n) {
        if (wildcard_memcmp(s + candidate, pat)) {
          return candidate;
        }
      }

      mask = mask & (mask - 1);
    }
  }
  return std::string::npos;
}

template <size_t N>
inline bool wildcard_memcmp(const byte* haystack, const Pattern<N>& pat) {
  if (!pat.has_wildcard) {
    return std::memcmp(haystack, pat.needle.data(), pat.size()) == 0;
  }
  for (size_t j = 0; j < pat.size(); ++j) {
    if (!pat.wildcard[j] && haystack[j] != pat.needle[j]) return false;
  }
  return true;
}
