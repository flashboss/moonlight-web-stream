/* Apple clang turns __builtin_cpu_supports() into a reference to __cpu_model,
 * which libgcc provides on Linux and Apple's compiler-rt does not. Reed-Solomon
 * in moonlight-common-c uses that builtin on x86_64, so the Intel macOS link
 * fails unless this object is linked into the final binary.
 *
 * Feature bits match GCC/Clang's processor_features enum.
 */

#if defined(__APPLE__) && defined(__x86_64__)

struct __processor_model {
    unsigned int __cpu_vendor;
    unsigned int __cpu_type;
    unsigned int __cpu_subtype;
    unsigned int __cpu_features[1];
};

struct __processor_model __cpu_model = {0};
unsigned int __cpu_features2 = 0;

static void cpuid(unsigned leaf, unsigned subleaf, unsigned *eax, unsigned *ebx, unsigned *ecx, unsigned *edx) {
    __asm__ volatile("cpuid"
                     : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                     : "a"(leaf), "c"(subleaf));
}

int __cpu_indicator_init(void) {
    unsigned eax, ebx, ecx, edx;
    unsigned max_leaf;
    unsigned features = 0;
    unsigned xcr0 = 0;

    cpuid(0, 0, &max_leaf, &ebx, &ecx, &edx);
    if (max_leaf < 1) {
        return 0;
    }

    cpuid(1, 0, &eax, &ebx, &ecx, &edx);
    if (edx & (1u << 15)) features |= 1u << 0;  /* cmov */
    if (edx & (1u << 23)) features |= 1u << 1;  /* mmx */
    if (ecx & (1u << 23)) features |= 1u << 2;  /* popcnt */
    if (edx & (1u << 25)) features |= 1u << 3;  /* sse */
    if (edx & (1u << 26)) features |= 1u << 4;  /* sse2 */
    if (ecx & (1u << 0)) features |= 1u << 5;   /* sse3 */
    if (ecx & (1u << 9)) features |= 1u << 6;   /* ssse3 */
    if (ecx & (1u << 19)) features |= 1u << 7;  /* sse4.1 */
    if (ecx & (1u << 20)) features |= 1u << 8;  /* sse4.2 */

    if ((ecx & (1u << 27)) != 0) {
        unsigned eax_xcr;
        __asm__ volatile("xgetbv" : "=a"(eax_xcr) : "c"(0) : "edx");
        xcr0 = eax_xcr;
    }

    if ((ecx & (1u << 28)) != 0 && (xcr0 & 0x6) == 0x6) {
        features |= 1u << 9; /* avx */
        if (ecx & (1u << 12)) features |= 1u << 14; /* fma */

        if (max_leaf >= 7) {
            unsigned eax7, ebx7, ecx7, edx7;
            cpuid(7, 0, &eax7, &ebx7, &ecx7, &edx7);
            if (ebx7 & (1u << 3)) features |= 1u << 16; /* bmi */
            if (ebx7 & (1u << 5)) features |= 1u << 10; /* avx2 */
            if (ebx7 & (1u << 8)) features |= 1u << 17; /* bmi2 */
            if ((ebx7 & (1u << 16)) != 0 && (xcr0 & 0xe6) == 0xe6) {
                features |= 1u << 15; /* avx512f */
                if (ebx7 & (1u << 31)) features |= 1u << 20; /* avx512vl */
                if (ebx7 & (1u << 30)) features |= 1u << 21; /* avx512bw */
            }
        }
    }

    __cpu_model.__cpu_features[0] = features;
    return 0;
}

#endif
