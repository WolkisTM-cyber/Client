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

$Sources = Get-ChildItem -Path (Join-Path $ProjectRoot "src") -Recurse -Filter "*.cpp" | Select-Object -ExpandProperty FullName
$SourcePathsStr = @()
foreach ($s in $Sources) {
    $SourcePathsStr += "`"$s`""
}
$SourcePathsArg = $SourcePathsStr -join " "

$Flags = @(
    "/nologo",
    "/$Configuration",
    "/std:c++17",
    "/EHsc",
    "/W4",
    "/utf-8",
    "/DUNICODE",
    "/D_UNICODE",
    "/I`"$JavaInclude`"",
    "/I`"$JavaIncludeWin`"",
    "/I`"$ProjectRoot\src`""
)

if ($Configuration -eq "Release") { $Flags += "/O2", "/MT" }
else { $Flags += "/MTd" }

$OutputDll = Join-Path $OutputDir "Client.dll"
$OutputLib = Join-Path $OutputDir "Client.lib"

$LinkFlags = "/DLL /OUT:`"$OutputDll`" /IMPLIB:`"$OutputLib`" /SUBSYSTEM:WINDOWS user32.lib gdi32.lib comctl32.lib advapi32.lib"

$ClCmd = "cl $Flags $SourcePathsArg /link $LinkFlags"

Write-Host "Compiling..." -ForegroundColor Yellow
Push-Location $ProjectRoot
Invoke-Expression $ClCmd
Pop-Location

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nBuild successful!" -ForegroundColor Green
    Write-Host "DLL: $OutputDll"

    $Desktop = [Environment]::GetFolderPath("Desktop")
    $Target = Join-Path $Desktop "Client.dll"
    Copy-Item -Path $OutputDll -Destination $Target -Force
    Write-Host "Copied to: $Target" -ForegroundColor Green
} else {
    Write-Host "`nBuild failed (exit code: $LASTEXITCODE)" -ForegroundColor Red
    exit 1
}
