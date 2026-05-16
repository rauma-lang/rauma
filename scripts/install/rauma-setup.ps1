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

function Normalize-Tag([string]$Raw) {
    if ($Raw -eq "latest") { return "latest" }
    if ($Raw.StartsWith("v")) { return $Raw }
    return "v$Raw"
}

function Normalize-Number([string]$Raw) {
    if ($Raw -eq "latest") { return "latest" }
    if ($Raw.StartsWith("v")) { return $Raw.Substring(1) }
    return $Raw
}

function Resolve-Version-Tag([string]$Raw) {
    if ($Raw -ne "latest") { return Normalize-Tag $Raw }

    $apiUrl = "https://api.github.com/repos/$Owner/$Repo/releases/latest"
    $latest = Invoke-RestMethod -Headers @{ "User-Agent" = "rauma-setup" } -Uri $apiUrl
    if (-not $latest.tag_name) {
        throw "could not resolve latest release tag from $apiUrl"
    }
    return $latest.tag_name
}

if ($Help) {
    Write-Host "rauma-setup.ps1 [-Version v0.1.0] [-Prefix DIR] [-AddToPath] [-DryRun]"
    exit 0
}

$archRaw = $env:PROCESSOR_ARCHITECTURE
switch ($archRaw) {
    "AMD64" { $arch = "x64" }
    "ARM64" { $arch = "arm64" }
    default { throw "unsupported architecture: $archRaw" }
}

$asset = "rmc-windows-$arch.exe"
$projectAsset = "rauma-windows-$arch.exe"
$versionTag = Resolve-Version-Tag $Version
$versionNumber = Normalize-Number $versionTag
$archive = "rauma-v$versionNumber-windows-$arch.zip"
$baseUrl = "https://github.com/$Owner/$Repo/releases/download/$versionTag"
$url = "$baseUrl/$archive"

Write-Host "platform: windows-$arch"
Write-Host "asset: $asset"
Write-Host "project manager: $projectAsset"
Write-Host "archive: $archive"
Write-Host "install dir: $Prefix"
Write-Host "download URL: $url"

$gcc = Get-Command gcc -ErrorAction SilentlyContinue
if ($gcc) {
    Write-Host "gcc: $($gcc.Source)"
} else {
    Write-Host "gcc: not found"
    Write-Host "Install MSYS2/MinGW-w64 or LLVM and ensure a C compiler is on PATH before using rmc build."
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
    if (Test-Path (Join-Path $tmp $projectAsset)) {
        Copy-Item -Force (Join-Path $tmp $projectAsset) (Join-Path $Prefix "rauma.exe")
    }

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
    if (Test-Path (Join-Path $Prefix "rauma.exe")) {
        & (Join-Path $Prefix "rauma.exe") version
    }
    Write-Host "rauma setup complete"
} finally {
    if (Test-Path $tmp) {
        Remove-Item -Recurse -Force $tmp
    }
}
