param(
    [ValidateSet("Release", "Debug")]
    [string]$Configuration = "Release"
)

$ProjectRoot = $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build"
$OutputDir = Join-Path $ProjectRoot "output"

$JavaHome = $env:JAVA_HOME
if (-not $JavaHome) {
    Write-Error "JAVA_HOME is not set. Set it to your JDK path."
    exit 1
}

$JavaInclude = Join-Path $JavaHome "include"
$JavaIncludeWin = Join-Path $JavaHome "include" "win32"

if (-not (Test-Path $JavaInclude)) {
    Write-Error "Java include not found at: $JavaInclude"
    exit 1
}

Write-Host "Building Client ($Configuration)..." -ForegroundColor Cyan

New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

# Collect source files
$Sources = Get-ChildItem -Path (Join-Path $ProjectRoot "src") -Recurse -Filter "*.cpp" | Select-Object -ExpandProperty FullName

# Build CL flags
$ClArgs = @(
    "cl.exe"
    "/nologo"
    "/$Configuration"
    "/std:c++17"
    "/EHsc"
    "/W4"
    "/utf-8"
    "/DUNICODE"
    "/D_UNICODE"
    "/I`"$JavaInclude`""
    "/I`"$JavaIncludeWin`""
    "/I`"$ProjectRoot\src`""
)

if ($Configuration -eq "Release") { $ClArgs += "/O2", "/MT" }
else { $ClArgs += "/MTd" }

# Add sources
$ClArgs += $Sources

# Link flags
$ClArgs += @(
    "/link"
    "/DLL"
    "/OUT:`"$(Join-Path $OutputDir "Client.dll")`""
    "/IMPLIB:`"$(Join-Path $OutputDir "Client.lib")`""
    "/SUBSYSTEM:WINDOWS"
    "user32.lib"
    "gdi32.lib"
    "comctl32.lib"
    "advapi32.lib"
    "opengl32.lib"
)

Write-Host "Compiling Client DLL..." -ForegroundColor Yellow
Push-Location $ProjectRoot
& $ClArgs[0] $ClArgs[1..$ClArgs.Count]
$dllExit = $LASTEXITCODE
Pop-Location

if ($dllExit -eq 0) {
    Write-Host "Client DLL: $(Join-Path $OutputDir "Client.dll")" -ForegroundColor Green
} else {
    Write-Host "Client DLL build failed (exit code: $dllExit)" -ForegroundColor Red
    exit 1
}

# Build Injector
Write-Host "`nBuilding Injector..." -ForegroundColor Cyan

$InjectorArgs = @(
    "cl.exe"
    "/nologo"
    "/$Configuration"
    "/std:c++17"
    "/EHsc"
    "`"$(Join-Path $ProjectRoot "injector\injector.cpp")`""
    "/link"
    "/OUT:`"$(Join-Path $OutputDir "Injector.exe")`""
    "advapi32.lib"
)

Push-Location $ProjectRoot
& $InjectorArgs[0] $InjectorArgs[1..$InjectorArgs.Count]
$injExit = $LASTEXITCODE
Pop-Location

if ($injExit -eq 0) {
    Write-Host "Injector: $(Join-Path $OutputDir "Injector.exe")" -ForegroundColor Green
} else {
    Write-Host "Injector build failed (exit code: $injExit)" -ForegroundColor Red
}

Write-Host "`n=== Build Complete ===" -ForegroundColor Cyan
Get-ChildItem $OutputDir | ForEach-Object { Write-Host "  $($_.Name)" }
