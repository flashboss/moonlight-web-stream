use std::env;
use std::path::PathBuf;
use std::process::Command;

fn main() {
    let target_arch = env::var("CARGO_CFG_TARGET_ARCH").unwrap_or_default();
    let target_os = env::var("CARGO_CFG_TARGET_OS").unwrap_or_default();
    if target_arch != "x86_64" || target_os != "macos" {
        return;
    }

    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    let source = manifest_dir.join(CPU_MODEL_C);
    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());
    let object = out_dir.join("macos_x86_64_cpu_model.o");

    println!("cargo:rerun-if-changed={}", source.display());

    let cc = env::var("CC").unwrap_or_else(|_| "cc".to_string());
    let deployment_target = env::var("MACOSX_DEPLOYMENT_TARGET").unwrap_or_else(|_| "11.0".to_string());
    let status = Command::new(cc)
        .arg("-target")
        .arg("x86_64-apple-darwin")
        .arg(format!("-mmacosx-version-min={deployment_target}"))
        .arg("-c")
        .arg(&source)
        .arg("-O2")
        .arg("-o")
        .arg(&object)
        .status()
        .expect("failed to spawn the C compiler for the macOS x86_64 CPU model");
    if !status.success() {
        panic!("failed to compile {}", source.display());
    }

    println!("cargo:rustc-link-arg={}", object.display());
}
