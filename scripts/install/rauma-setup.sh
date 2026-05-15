#!/bin/sh
set -eu

OWNER="${RAUMA_GITHUB_OWNER:-rauma-lang}"
REPO="${RAUMA_GITHUB_REPO:-rauma}"
VERSION="latest"
PREFIX="$HOME/.rauma/bin"
DRY_RUN=0

usage() {
    cat <<'USAGE'
rauma-setup.sh [--version v0.1.0] [--prefix DIR] [--dry-run]

Installs the rmc compiler from GitHub Releases.
USAGE
}

normalize_tag() {
    case "$1" in
        latest) printf '%s\n' latest ;;
        v*) printf '%s\n' "$1" ;;
        *) printf 'v%s\n' "$1" ;;
    esac
}

normalize_number() {
    case "$1" in
        latest) printf '%s\n' latest ;;
        v*) printf '%s\n' "${1#v}" ;;
        *) printf '%s\n' "$1" ;;
    esac
}

fetch_url() {
    if command -v curl >/dev/null 2>&1; then
        curl -fsSL "$1"
    elif command -v wget >/dev/null 2>&1; then
        wget -q "$1" -O -
    else
        echo "curl or wget is required" >&2
        exit 1
    fi
}

resolve_version_tag() {
    case "$1" in
        latest)
            api_url="https://api.github.com/repos/$OWNER/$REPO/releases/latest"
            tag="$(fetch_url "$api_url" | sed -n 's/.*"tag_name"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p' | head -n 1)"
            if [ -z "$tag" ]; then
                echo "could not resolve latest release tag from $api_url" >&2
                exit 1
            fi
            printf '%s\n' "$tag"
            ;;
        *)
            normalize_tag "$1"
            ;;
    esac
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --version)
            VERSION="$2"
            shift 2
            ;;
        --prefix)
            PREFIX="$2"
            shift 2
            ;;
        --dry-run)
            DRY_RUN=1
            shift
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        *)
            echo "unknown argument: $1" >&2
            usage
            exit 1
            ;;
    esac
done

os="$(uname -s | tr '[:upper:]' '[:lower:]')"
arch="$(uname -m | tr '[:upper:]' '[:lower:]')"

case "$os" in
    linux*) platform_os="linux" ;;
    darwin*) platform_os="macos" ;;
    mingw*|msys*|cygwin*) platform_os="windows" ;;
    *) echo "unsupported OS: $os" >&2; exit 1 ;;
esac

case "$arch" in
    x86_64|amd64) platform_arch="x64" ;;
    aarch64|arm64) platform_arch="arm64" ;;
    *) echo "unsupported architecture: $arch" >&2; exit 1 ;;
esac

case "$platform_os-$platform_arch" in
    windows-x64|windows-arm64|linux-x64|linux-arm64|macos-arm64) ;;
    macos-x64)
        echo "macos-x64 release asset is not produced by the current CI" >&2
        exit 1
        ;;
    *)
        echo "unsupported target: $platform_os-$platform_arch" >&2
        exit 1
        ;;
esac

exe_suffix=""
archive_suffix="tar.gz"
if [ "$platform_os" = "windows" ]; then
    exe_suffix=".exe"
    archive_suffix="zip"
fi

asset="rmc-$platform_os-$platform_arch$exe_suffix"
version_tag="$(resolve_version_tag "$VERSION")"
version_number="$(normalize_number "$version_tag")"
archive="rauma-v$version_number-$platform_os-$platform_arch.$archive_suffix"
base_url="https://github.com/$OWNER/$REPO/releases/download/$version_tag"
url="$base_url/$archive"

echo "platform: $platform_os-$platform_arch"
echo "asset: $asset"
echo "archive: $archive"
echo "install dir: $PREFIX"
echo "download URL: $url"

if [ "$DRY_RUN" -eq 1 ]; then
    echo "dry-run: no download performed"
    exit 0
fi

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

if command -v curl >/dev/null 2>&1; then
    curl -fsSL "$url" -o "$tmp/rauma.$archive_suffix"
elif command -v wget >/dev/null 2>&1; then
    wget -q "$url" -O "$tmp/rauma.$archive_suffix"
else
    echo "curl or wget is required" >&2
    exit 1
fi

mkdir -p "$tmp/pkg" "$PREFIX"
if [ "$archive_suffix" = "zip" ]; then
    if command -v unzip >/dev/null 2>&1; then
        unzip -q "$tmp/rauma.$archive_suffix" -d "$tmp/pkg"
    else
        echo "unzip is required for Windows release archives" >&2
        exit 1
    fi
else
    tar -xzf "$tmp/rauma.$archive_suffix" -C "$tmp/pkg"
fi

cp "$tmp/pkg/$asset" "$PREFIX/rmc$exe_suffix"
chmod +x "$PREFIX/rmc$exe_suffix"

case ":$PATH:" in
    *":$PREFIX:"*) ;;
    *) echo "add this to PATH: $PREFIX" ;;
esac

"$PREFIX/rmc$exe_suffix" version
echo "rauma setup complete"
