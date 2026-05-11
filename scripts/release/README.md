# RauMa release automation

## CI

`.github/workflows/ci.yml` runs on pull requests and pushes to `main`.

The Windows job:

1. builds `rmb`
2. checks `rmb 0.0.7`
3. builds real `rmc`
4. checks `rmc 0.1.0`
5. runs `make test`
6. runs deterministic self-host verification with
   `scripts/release/verify-self-host.ps1`
7. builds and smokes `examples/setup/rauma-setup.rm`

## Release workflow

`.github/workflows/release.yml` runs on tags matching `v*` and on manual
`workflow_dispatch`.

The workflow uses `GITHUB_TOKEN` with `contents: write`, builds the Windows
release package, and publishes assets with `gh release create` / `gh release
upload` as `github-actions[bot]`.

## Release assets

For v0.1.0 the Windows release produces:

- `rauma-rmb-windows-x64.exe`
- `rauma-rmc-windows-x64.exe`
- `rauma-setup-windows-x64.exe`
- `rauma-v0.1.0-windows-x64.zip`
- `SHA256SUMS.txt`

`rauma-rmc` is the self-hosted RauMa compiler. `rauma-rmb` is the bootstrap
compiler retained for recovery. `rauma-setup` is a RauMa-written setup helper.

## Cut a release

1. ensure `main` is clean and all local verification passes
2. update versions and release docs
3. push the release commit
4. create the tag: `git tag v0.1.0`
5. push the tag: `git push origin v0.1.0`
6. wait for GitHub Actions to create the release assets
7. download the archive and smoke-test `rauma-rmc version` and
   `rauma-setup doctor`

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
