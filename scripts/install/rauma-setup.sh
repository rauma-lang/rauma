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
    *) echo "unsupported OS: $os" >&2; exit 1 ;;
esac

case "$arch" in
    x86_64|amd64) platform_arch="x64" ;;
    aarch64|arm64) platform_arch="arm64" ;;
    *) echo "unsupported architecture: $arch" >&2; exit 1 ;;
esac

case "$platform_os-$platform_arch" in
    linux-x64|linux-arm64|macos-arm64) ;;
    macos-x64)
        echo "macos-x64 release asset is not produced by the current CI" >&2
        exit 1
        ;;
esac

asset="rmc-$platform_os-$platform_arch"
version_tag="$(normalize_tag "$VERSION")"
version_number="$(normalize_number "$VERSION")"
archive="rauma-v$version_number-$platform_os-$platform_arch.tar.gz"
if [ "$VERSION" = "latest" ]; then
    base_url="https://github.com/$OWNER/$REPO/releases/latest/download"
else
    base_url="https://github.com/$OWNER/$REPO/releases/download/$version_tag"
fi
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
    curl -fsSL "$url" -o "$tmp/rauma.tar.gz"
elif command -v wget >/dev/null 2>&1; then
    wget -q "$url" -O "$tmp/rauma.tar.gz"
else
    echo "curl or wget is required" >&2
    exit 1
fi

mkdir -p "$tmp/pkg" "$PREFIX"
tar -xzf "$tmp/rauma.tar.gz" -C "$tmp/pkg"
cp "$tmp/pkg/$asset" "$PREFIX/rmc"
chmod +x "$PREFIX/rmc"

case ":$PATH:" in
    *":$PREFIX:"*) ;;
    *) echo "add this to PATH: $PREFIX" ;;
esac

"$PREFIX/rmc" version
echo "rauma setup complete"
