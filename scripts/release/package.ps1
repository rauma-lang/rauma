param(
    [string]$Version = "0.1.0",
    [string]$PlatformOs,
    [string]$PlatformArch,
    [ValidateSet("zip", "tar.gz")]
    [string]$ArchiveFormat
)

$ErrorActionPreference = "Stop"

function Repo-Root {
    return (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
}

function Native-Exe([string]$PathNoExt) {
    if (Test-Path "$PathNoExt.exe") {
        return (Resolve-Path "$PathNoExt.exe").Path
    }
    if (Test-Path $PathNoExt) {
        return (Resolve-Path $PathNoExt).Path
    }
    throw "missing executable: $PathNoExt"
}

if (-not $PlatformOs) {
    throw "PlatformOs is required"
}
if (-not $PlatformArch) {
    throw "PlatformArch is required"
}

$root = Repo-Root
$dist = Join-Path $root "dist"
if (Test-Path $dist) {
    Remove-Item -Recurse -Force $dist
}
New-Item -ItemType Directory -Force $dist | Out-Null

& (Join-Path $PSScriptRoot "verify-self-host.ps1")

Set-Location (Join-Path $root "rmb")
$rmb = Native-Exe "build/rmb"
$rmc = Native-Exe "build/rmc3-real"

$exeSuffix = if ($PlatformOs -eq "windows") { ".exe" } else { "" }
$target = "$PlatformOs-$PlatformArch"
$rmbAsset = "rmb-$target$exeSuffix"
$rmcAsset = "rmc-$target$exeSuffix"

Copy-Item -Force $rmb (Join-Path $dist $rmbAsset)
Copy-Item -Force $rmc (Join-Path $dist $rmcAsset)
Copy-Item -Force (Join-Path $root "scripts/install/rauma-setup.ps1") (Join-Path $dist "rauma-setup.ps1")
Copy-Item -Force (Join-Path $root "scripts/install/rauma-setup.sh") (Join-Path $dist "rauma-setup.sh")

Set-Location $dist
$archive = if ($ArchiveFormat -eq "zip") {
    "rauma-v$Version-$target.zip"
} else {
    "rauma-v$Version-$target.tar.gz"
}

$archiveMembers = @(
    $rmbAsset,
    $rmcAsset,
    "rauma-setup.ps1",
    "rauma-setup.sh"
)

if ($ArchiveFormat -eq "zip") {
    Compress-Archive -Force -Path $archiveMembers -DestinationPath $archive
} else {
    & tar -czf $archive @archiveMembers
    if ($LASTEXITCODE -ne 0) {
        throw "tar archive creation failed"
    }
}

$checksumFile = "SHA256SUMS-$target.txt"
$assetNames = @($archiveMembers + $archive)
$lines = @()
foreach ($name in $assetNames) {
    $hash = (Get-FileHash -Algorithm SHA256 $name).Hash.ToLowerInvariant()
    $lines += "$hash  $name"
}
Set-Content -Encoding ASCII -Path $checksumFile -Value $lines

Write-Host "release_package= PASS"
Write-Host "dist=$dist"
Get-ChildItem $dist | ForEach-Object { Write-Host $_.Name }
