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
    if ($text.Contains("build failed") -or $text.Contains("check failed") -or $text.Contains("emit failed")) {
        Write-Host $text
        throw "compiler command reported failure: $File $($CommandArgs -join ' ')"
    }
    return $text
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
Write-Host (Run-Cmd $rmb @("version")).TrimEnd()

Write-Host (Run-Cmd $rmb @("build", "../rmc/main.rm")).TrimEnd()
$rmc = Native-Exe "build/debug/native/bin/main"

Write-Host (Run-Cmd $rmc @("version")).TrimEnd()
Write-Host (Run-Cmd $rmc @("help")).Split("`n")[0]
Write-Host (Run-Cmd $rmc @("lsp", "--check-exits-immediately")).TrimEnd()

$moduleChecks = @(
    "../rmc/hir/types.rm",
    "../rmc/hir/nodes.rm",
    "../rmc/hir/gen.rm",
    "../rmc/mir/types.rm",
    "../rmc/mir/nodes.rm",
    "../rmc/mir/gen.rm",
    "../rmc/mir/validate.rm",
    "../rmc/mir/opt.rm",
    "../rmc/type/types.rm",
    "../rmc/type/symtab.rm",
    "../rmc/type/checker.rm",
    "../rmc/lsp/protocol.rm",
    "../rmc/lsp/message.rm",
    "../rmc/lsp/docs.rm",
    "../rmc/lsp/handlers.rm",
    "../rmc/lsp/dispatch.rm",
    "../rauma/main.rm",
    "../std/core/option.rm",
    "../std/core/result.rm",
    "../std/str/str.rm",
    "../std/io/io.rm",
    "../std/io/print.rm",
    "../std/conv/conv.rm",
    "../std/collections/vec.rm",
    "../std/math/math.rm"
)

foreach ($path in $moduleChecks) {
    Run-Cmd $rmb @("check", $path) | Out-Null
}
Write-Host "module_checks= PASS"

Write-Host (Run-Cmd $rmb @("build", "../std/smoke.rm")).TrimEnd()
$smoke = Native-Exe "build/debug/native/bin/smoke"
$smokeOut = Run-Cmd $smoke @()
$want = "std ok`n7`ntrue`ntrue`n7`ntrue`n"
if ($smokeOut -ne $want) {
    Write-Host $smokeOut
    throw "stdlib smoke output mismatch"
}
Write-Host "stdlib_smoke= PASS"

Write-Host "product_verify= PASS"
