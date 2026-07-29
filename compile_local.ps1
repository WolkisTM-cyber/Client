$env:PATH = "C:\Users\ak\.gemini\antigravity\scratch\w64devkit\bin;" + $env:PATH

$outputDir = "C:\Users\ak\.gemini\antigravity\scratch\client\output"

if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
}

Write-Host "Compiling Client.dll locally with GCC..." -ForegroundColor Cyan

$sources = @(
    "src/dllmain.cpp",
    "src/GUI.cpp",
    "src/command/CommandManager.cpp",
    "src/config/ConfigManager.cpp",
    "src/gui/ClickGUI.cpp",
    "src/modules/JNIHelper.cpp",
    "src/modules/Module.cpp",
    "src/modules/ModuleManager.cpp",
    "src/modules/packet/PacketUtil.cpp",
    "src/render/Renderer.cpp",
    "src/render/ShaderEngine.cpp"
)

$cmdArgs = @(
    "-shared",
    "-O2",
    "-std=c++17",
    "-fpermissive",
    "-DUNICODE",
    "-D_UNICODE",
    "-Iinclude",
    "-Iinclude/win32",
    "-Isrc",
    "-o", "$outputDir\Client.dll"
) + $sources + @(
    "-luser32",
    "-lgdi32",
    "-lcomctl32",
    "-ladvapi32",
    "-lopengl32",
    "-lglu32"
)

& g++.exe $cmdArgs

if ($LASTEXITCODE -eq 0) {
    Write-Host "Client.dll built successfully at: $outputDir\Client.dll" -ForegroundColor Green
} else {
    Write-Host "Client.dll build failed with code $LASTEXITCODE" -ForegroundColor Red
    exit 1
}

Write-Host "`nCompiling Injector.exe locally..." -ForegroundColor Cyan
& g++.exe -O2 -std=c++17 -municode -o "$outputDir\Injector.exe" injector/injector.cpp -ladvapi32

if ($LASTEXITCODE -eq 0) {
    Write-Host "Injector.exe built successfully at: $outputDir\Injector.exe" -ForegroundColor Green
} else {
    Write-Host "Injector.exe build failed with code $LASTEXITCODE" -ForegroundColor Red
}
