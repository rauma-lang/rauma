# RauMa setup scripts

The official v0.1.0 installer is script-based:

- `rauma-setup.sh` for Linux/macOS
- `rauma-setup.ps1` for Windows

RauMa does not yet have the standard library, process API, HTTP/fetch support,
or archive extraction APIs needed for a native RauMa installer. A native
RauMa-written installer is deferred until those capabilities exist.

## Linux/macOS

```bash
./rauma-setup.sh
./rauma-setup.sh --version v0.1.0
./rauma-setup.sh --prefix "$HOME/.local/bin"
./rauma-setup.sh --dry-run --version v0.1.0
```

The script detects Linux/macOS and x64/arm64, downloads the matching release
archive, installs the compiler as `rmc`, marks it executable, and prints PATH
instructions when needed.

## Windows

```powershell
.\rauma-setup.ps1
.\rauma-setup.ps1 -Version v0.1.0
.\rauma-setup.ps1 -Prefix "$env:USERPROFILE\.rauma\bin"
.\rauma-setup.ps1 -AddToPath
.\rauma-setup.ps1 -DryRun -Version v0.1.0
```

The script detects x64/arm64 Windows gcc assets, downloads the matching archive,
and installs the compiler as `rmc.exe`.

`rmc build` needs `gcc` on PATH. If gcc is missing, install MSYS2/MinGW-w64 and
open a shell where `gcc` is available.

## Release assets

The compiler binary is named `rmc-*`. The bootstrap compiler is named `rmb-*`.
No official v0.1.0 asset is named `rauma-rmc-*`, `rauma-rmb-*`, or
`rauma-setup-*.exe`.
