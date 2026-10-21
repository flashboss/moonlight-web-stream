// Links the x86_64 CPU-dispatch symbols Apple clang does not provide.
const CPU_MODEL_C: &str = "build/macos_x86_64_cpu_model.c";

include!("build/link_macos_x86_64_cpu_model.rs");
