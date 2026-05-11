param(
    [switch]$SkipBootstrapBuild
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
    return $text
}

function Copy-Stage([string]$Name, [string]$CName) {
    $built = Native-Exe "build/rmc_build_out"
    $suffix = if ($built.EndsWith(".exe")) { ".exe" } else { "" }
    Copy-Item -Force $built "build/$Name$suffix"
    Copy-Item -Force "build/rmc_build_out.c" "build/$CName"
}

function Build-Rmb {
    if (Get-Command make -ErrorAction SilentlyContinue) {
        make clean
        if ($LASTEXITCODE -ne 0) { throw "make clean failed" }
        make
        if ($LASTEXITCODE -ne 0) { throw "make failed" }
        return
    }

    if (-not (Get-Command gcc -ErrorAction SilentlyContinue)) {
        throw "neither make nor gcc is available"
    }

    if (Test-Path "build") {
        Remove-Item -Recurse -Force "build"
    }
    New-Item -ItemType Directory -Force "build" | Out-Null

    $sources = @(
        "src/main.c",
        "src/common.c",
        "src/arena.c",
        "src/source.c",
        "src/span.c",
        "src/token.c",
        "src/lexer.c",
        "src/string.c",
        "src/vec.c",
        "src/diag.c",
        "src/ast.c",
        "src/parser.c",
        "src/type.c",
        "src/checker.c",
        "src/cgen.c",
        "src/build.c"
    )

    & gcc -std=c11 -Wall -Wextra -Werror -pedantic -fno-strict-aliasing -g -O0 -Iinclude @sources -o build/rmb
    if ($LASTEXITCODE -ne 0) {
        throw "gcc fallback build failed"
    }
}

$root = Repo-Root
$rmbRoot = Join-Path $root "rmb"
Set-Location $rmbRoot

if (-not $SkipBootstrapBuild) {
    Build-Rmb
}

$rmb = Native-Exe "build/rmb"
$rmbVersion = Run-Cmd $rmb @("version")
Write-Host $rmbVersion.TrimEnd()

$rmc0Build = Run-Cmd $rmb @("build", "../rmc/main.rm")
Write-Host $rmc0Build.TrimEnd()

$rmc0 = Native-Exe "build/debug/native/bin/main"
$rmc0Version = Run-Cmd $rmc0 @("version")
Write-Host $rmc0Version.TrimEnd()

$stageA = Run-Cmd $rmc0 @("build", "../rmc/main.rm")
Write-Host $stageA.TrimEnd()
Copy-Stage "rmc1-real" "real_from_rmc0.c"
$rmc1 = Native-Exe "build/rmc1-real"
Write-Host (Run-Cmd $rmc1 @("version")).TrimEnd()

$stageB = Run-Cmd $rmc1 @("build", "../rmc/main.rm")
Write-Host $stageB.TrimEnd()
Copy-Stage "rmc2-real" "real_from_rmc1.c"
$rmc2 = Native-Exe "build/rmc2-real"
Write-Host (Run-Cmd $rmc2 @("version")).TrimEnd()

$stageC = Run-Cmd $rmc2 @("build", "../rmc/main.rm")
Write-Host $stageC.TrimEnd()
Copy-Stage "rmc3-real" "real_from_rmc2.c"
$rmc3 = Native-Exe "build/rmc3-real"
Write-Host (Run-Cmd $rmc3 @("version")).TrimEnd()

Set-Location $rmbRoot
$files = @(
    (Join-Path $rmbRoot "build/real_from_rmc0.c"),
    (Join-Path $rmbRoot "build/real_from_rmc1.c"),
    (Join-Path $rmbRoot "build/real_from_rmc2.c")
)
$base = [System.IO.File]::ReadAllBytes($files[0])
$hashes = @()
foreach ($f in $files) {
    $bytes = [System.IO.File]::ReadAllBytes($f)
    $hash = (Get-FileHash -Algorithm SHA256 $f).Hash.ToLowerInvariant()
    $hashes += $hash
    Write-Host "$(Split-Path $f -Leaf) $hash $($bytes.Length)"
    if ($bytes.Length -ne $base.Length) {
        throw "generated C size mismatch: $f"
    }
    for ($i = 0; $i -lt $bytes.Length; $i++) {
        if ($bytes[$i] -ne $base[$i]) {
            throw "generated C mismatch at byte $i in $f"
        }
    }
}
Write-Host "generated_c_compare= PASS"

$bins = @($rmc1, $rmc2, $rmc3)
$commands = @(
    @(),
    @("version"),
    @("help"),
    @("demo-lex"),
    @("demo-parse"),
    @("lex", "tests/rmc_lex_input.rm"),
    @("parse", "tests/rmc_parse_input.rm"),
    @("check", "tests/rmc_check_ok.rm"),
    @("emit-c", "tests/rmc_emit_workflow_add.rm"),
    @("wat")
)

foreach ($cmd in $commands) {
    $outs = @()
    foreach ($bin in $bins) {
        $outs += (Run-Cmd $bin $cmd)
    }
    if (($outs[0] -ne $outs[1]) -or ($outs[0] -ne $outs[2])) {
        throw "behavior mismatch: $($cmd -join ' ')"
    }
    $name = if ($cmd.Count -eq 0) { "no-args" } else { $cmd -join " " }
    Write-Host "$name PASS"
}

$candidateOuts = @()
foreach ($bin in $bins) {
    $candidateOuts += (Run-Cmd $bin @("build", "tests/rmc_candidate/main.rm"))
    $candidate = Native-Exe "build/rmc_build_out"
    $candidateOuts += (Run-Cmd $candidate @("self-test"))
}
if (($candidateOuts[0] -ne $candidateOuts[2]) -or ($candidateOuts[0] -ne $candidateOuts[4])) {
    throw "candidate build behavior mismatch"
}
if (($candidateOuts[1] -ne $candidateOuts[3]) -or ($candidateOuts[1] -ne $candidateOuts[5])) {
    throw "candidate self-test behavior mismatch"
}
Write-Host "build tests/rmc_candidate/main.rm PASS"
Write-Host "candidate self-test PASS"
Write-Host "behavior_compare= PASS"
Write-Host "self_host_verify= PASS"
