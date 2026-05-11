# RauMa release automation

## CI

`.github/workflows/ci.yml` runs on pull requests and pushes to `main`.

The validation matrix covers:

1. Windows x64
2. Windows ARM64
3. Linux x64
4. Linux ARM64
5. macOS ARM64

Each job builds `rmb`, verifies the deterministic self-host chain with
`scripts/release/verify-self-host.ps1`, and runs the platform-appropriate
installer dry-run. The full bootstrap test suite runs on Windows x64, Windows
ARM64, Linux x64, and Linux ARM64.

## Release workflow

`.github/workflows/release.yml` runs on tags matching `v*` and on manual
`workflow_dispatch`.

The workflow uses `GITHUB_TOKEN` with `contents: write`, builds per-platform
release packages in a matrix, uploads them as workflow artifacts, then publishes
all assets from a single final job to avoid release creation/upload races.

## Release assets

For v0.1.0 the release produces:

- `rauma-v0.1.0-windows-x64.zip`
- `rauma-v0.1.0-windows-arm64.zip`
- `rauma-v0.1.0-linux-x64.tar.gz`
- `rauma-v0.1.0-linux-arm64.tar.gz`
- `rauma-v0.1.0-macos-arm64.tar.gz`

Binary names inside the archives:

- `windows-x64`: `rmc-windows-x64.exe`, `rmb-windows-x64.exe`
- `windows-arm64`: `rmc-windows-arm64.exe`, `rmb-windows-arm64.exe`
- `linux-x64`: `rmc-linux-x64`, `rmb-linux-x64`
- `linux-arm64`: `rmc-linux-arm64`, `rmb-linux-arm64`
- `macos-arm64`: `rmc-macos-arm64`, `rmb-macos-arm64`

Every archive also includes:

- `rauma-setup.sh`
- `rauma-setup.ps1`

Release publication also uploads a combined `SHA256SUMS.txt`.

Do not upload x64 binaries under arm64 names.

## Cut a release

1. ensure `main` is clean and all local verification passes
2. update versions and release docs
3. push the release commit
4. create the tag: `git tag v0.1.0`
5. push the tag: `git push origin v0.1.0`
6. wait for GitHub Actions to create the release assets
7. download the archive and smoke-test `rmc version`, installer dry-run, and
   `rmc build` on a small program

Do not commit generated files from `rmb/build/` or `dist/`.

## Local verification

From `rmb/`:

```bash
make self-host-test
```

Or from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/release/verify-self-host.ps1
```
