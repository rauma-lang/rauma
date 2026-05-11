param(
    [string]$Version = "latest",
    [string]$Prefix = "$env:USERPROFILE\.rauma\bin",
    [switch]$AddToPath,
    [switch]$DryRun,
    [switch]$Help
)

$ErrorActionPreference = "Stop"
$Owner = $env:RAUMA_GITHUB_OWNER
if (-not $Owner) { $Owner = "rauma-lang" }
$Repo = $env:RAUMA_GITHUB_REPO
if (-not $Repo) { $Repo = "rauma" }

if ($Help) {
    Write-Host "rauma-setup.ps1 [-Version v0.1.0] [-Prefix DIR] [-AddToPath] [-DryRun]"
    exit 0
}

$archRaw = $env:PROCESSOR_ARCHITECTURE
switch ($archRaw) {
    "AMD64" { $arch = "x64-gcc" }
    "ARM64" { $arch = "arm64-gcc" }
    default { throw "unsupported architecture: $archRaw" }
}

$asset = "rmc-windows-$arch.exe"
$archive = "rauma-v0.1.0-windows-$arch.zip"
if ($Version -eq "latest") {
    $baseUrl = "https://github.com/$Owner/$Repo/releases/latest/download"
} else {
    $baseUrl = "https://github.com/$Owner/$Repo/releases/download/$Version"
}
$url = "$baseUrl/$archive"

Write-Host "platform: windows-$arch"
Write-Host "asset: $asset"
Write-Host "archive: $archive"
Write-Host "install dir: $Prefix"
Write-Host "download URL: $url"

$gcc = Get-Command gcc -ErrorAction SilentlyContinue
if ($gcc) {
    Write-Host "gcc: $($gcc.Source)"
} else {
    Write-Host "gcc: not found"
    Write-Host "Install MSYS2/MinGW-w64 and ensure gcc is on PATH before using rmc build."
}

if ($DryRun) {
    Write-Host "dry-run: no download performed"
    exit 0
}

$tmp = Join-Path ([System.IO.Path]::GetTempPath()) ("rauma-setup-" + [System.Guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Force $tmp | Out-Null
try {
    $zip = Join-Path $tmp "rauma.zip"
    Invoke-WebRequest -Uri $url -OutFile $zip
    Expand-Archive -Force -Path $zip -DestinationPath $tmp
    New-Item -ItemType Directory -Force $Prefix | Out-Null
    Copy-Item -Force (Join-Path $tmp $asset) (Join-Path $Prefix "rmc.exe")

    if ($AddToPath) {
        $current = [Environment]::GetEnvironmentVariable("Path", "User")
        if (-not (($current -split ";") -contains $Prefix)) {
            [Environment]::SetEnvironmentVariable("Path", "$current;$Prefix", "User")
            Write-Host "added to User PATH: $Prefix"
        }
    } else {
        Write-Host "add this to PATH if needed: $Prefix"
    }

    & (Join-Path $Prefix "rmc.exe") version
    Write-Host "rauma setup complete"
} finally {
    if (Test-Path $tmp) {
        Remove-Item -Recurse -Force $tmp
    }
}
