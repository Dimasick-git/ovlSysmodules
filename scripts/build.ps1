<#
.SYNOPSIS
    Local Docker-based devkitPro build wrapper for ovlSysmodules (Ryazhenka).

.DESCRIPTION
    Wraps `docker run devkitpro/devkita64 make` so you can build on a
    Windows host without installing msys2 / devkitPro pacman. Detects
    Docker and prints msys2 install instructions if it's missing.

    Output: `ovlSysmodules.ovl` in repo root, optional zip in `dist/`.

.PARAMETER Clean
    Run `make clean` before building.

.PARAMETER Dist
    Build, then package the .ovl + lang/ JSONs into the SD-card layout
    zip (`dist/ovlSysmodules-<version>-<sha>.zip`).

.PARAMETER Jobs
    Parallel make jobs. Defaults to number of logical CPUs.

.EXAMPLE
    .\scripts\build.ps1 -Dist
    Build the release zip with default parallelism.

.EXAMPLE
    .\scripts\build.ps1 -Clean
    Clean build, no zip.
#>
[CmdletBinding()]
param(
    [switch]$Clean,
    [switch]$Dist,
    [int]$Jobs = 0
)

$ErrorActionPreference = 'Stop'

$repoRoot = Resolve-Path "$PSScriptRoot\.."
Set-Location $repoRoot

function Write-Section($msg) {
    Write-Host ''
    Write-Host '============================================================' -ForegroundColor Cyan
    Write-Host " $msg" -ForegroundColor Cyan
    Write-Host '============================================================' -ForegroundColor Cyan
}

function Test-Docker {
    try {
        $null = & docker version --format '{{.Server.Version}}' 2>&1
        return $LASTEXITCODE -eq 0
    } catch { return $false }
}

function Show-DockerHelp {
    Write-Host ''
    Write-Host 'Docker is required for the containerised build path.' -ForegroundColor Yellow
    Write-Host ''
    Write-Host '  Option A: install Docker Desktop'
    Write-Host '            https://www.docker.com/products/docker-desktop/'
    Write-Host '  Option B: build natively via msys2 + devkitPro pacman'
    Write-Host '            see docs/RU/build.md for the step-by-step'
    Write-Host ''
}

Write-Section 'ovlSysmodules (Ryazhenka) — local build'

if (-not (Test-Docker)) {
    Show-DockerHelp
    exit 1
}

if ($Jobs -le 0) { $Jobs = [Environment]::ProcessorCount }

# Parse APP_VERSION from Makefile so dist names match CI.
$appVersion = (Select-String -Path "$repoRoot\Makefile" -Pattern '^APP_VERSION\s*:=\s*(\S+)').Matches.Groups[1].Value
$sha = (& git -C $repoRoot rev-parse --short HEAD).Trim()

$image = 'devkitpro/devkita64:latest'
Write-Host "Pulling/refreshing $image..." -ForegroundColor Green
& docker pull $image
if ($LASTEXITCODE -ne 0) { throw "docker pull failed" }

$makeCmd = @()
if ($Clean) { $makeCmd += 'make clean || true' }
$makeCmd += 'git config --global --add safe.directory /project'
$makeCmd += "make -j$Jobs"

if ($Dist) {
    $name = "ovlSysmodules-$appVersion-$sha"
    $makeCmd += "rm -rf dist/$name dist/$name.zip"
    $makeCmd += "mkdir -p dist/$name/switch/.overlays"
    $makeCmd += "mkdir -p dist/$name/config/ryazhahand/ovlSysmodules/lang"
    $makeCmd += "cp ovlSysmodules.ovl dist/$name/switch/.overlays/"
    $makeCmd += "cp lang/*.json dist/$name/config/ryazhahand/ovlSysmodules/lang/"
    $makeCmd += "cd dist && zip -r $name.zip $name"
}
$shellCmd = $makeCmd -join ' && '

Write-Section "Running build (jobs=$Jobs, dist=$Dist, clean=$Clean)"
Write-Host "Command: $shellCmd" -ForegroundColor DarkGray
Write-Host ''

& docker run --rm `
    -v "${repoRoot}:/project" `
    -w /project `
    $image `
    bash -c $shellCmd

if ($LASTEXITCODE -ne 0) {
    Write-Host ''
    Write-Host 'Build failed.' -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Section 'Build complete'

if ($Dist) {
    $zips = Get-ChildItem "$repoRoot\dist" -Filter '*.zip' -ErrorAction SilentlyContinue
    foreach ($z in $zips) {
        $hash = (Get-FileHash $z.FullName -Algorithm SHA256).Hash
        $size = [math]::Round($z.Length / 1KB, 1)
        Write-Host ''
        Write-Host "  $($z.Name)" -ForegroundColor Green
        Write-Host "  Size:   $size KB"
        Write-Host "  SHA256: $hash"
    }
    Write-Host ''
    Write-Host "Artifacts in: $repoRoot\dist" -ForegroundColor Green
} else {
    if (Test-Path "$repoRoot\ovlSysmodules.ovl") {
        $hash = (Get-FileHash "$repoRoot\ovlSysmodules.ovl" -Algorithm SHA256).Hash
        $size = (Get-Item "$repoRoot\ovlSysmodules.ovl").Length / 1KB
        Write-Host "  ovlSysmodules.ovl   $([math]::Round($size, 1)) KB" -ForegroundColor Green
        Write-Host "  SHA256: $hash"
    }
}
