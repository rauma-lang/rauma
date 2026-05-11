param(
    [string]$Version = "0.1.0"
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

function Run-Cmd([string]$File, [string[]]$CommandArgs) {
    $out = & $File @CommandArgs 2>&1
    $text = ($out | Out-String).Replace("`r`n", "`n")
    if ($LASTEXITCODE -ne 0) {
        Write-Host $text
        throw "command failed: $File $($CommandArgs -join ' ')"
    }
    Write-Host $text.TrimEnd()
    return $text
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

Copy-Item -Force $rmb (Join-Path $dist "rauma-rmb-windows-x64.exe")
Copy-Item -Force $rmc (Join-Path $dist "rauma-rmc-windows-x64.exe")

Run-Cmd $rmc @("build", "../examples/setup/rauma-setup.rm") | Out-Null
$setup = Native-Exe "build/rmc_build_out"
Copy-Item -Force $setup (Join-Path $dist "rauma-setup-windows-x64.exe")

Set-Location $dist
$zip = "rauma-v$Version-windows-x64.zip"
Compress-Archive -Force -Path `
    "rauma-rmb-windows-x64.exe", `
    "rauma-rmc-windows-x64.exe", `
    "rauma-setup-windows-x64.exe" `
    -DestinationPath $zip

$assetNames = @(
    "rauma-rmb-windows-x64.exe",
    "rauma-rmc-windows-x64.exe",
    "rauma-setup-windows-x64.exe",
    $zip
)

$lines = @()
foreach ($name in $assetNames) {
    $hash = (Get-FileHash -Algorithm SHA256 $name).Hash.ToLowerInvariant()
    $lines += "$hash  $name"
}
Set-Content -Encoding ASCII -Path "SHA256SUMS.txt" -Value $lines

Write-Host "release_package= PASS"
Write-Host "dist=$dist"
Get-ChildItem $dist | ForEach-Object { Write-Host $_.Name }
