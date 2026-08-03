<#
.SYNOPSIS
    qt_image_maker를 Release로 빌드하고, Qt/Visual C++가 설치되지 않은 윈도우 PC에서도
    바로 실행 가능하도록 필요한 DLL/플러그인을 모두 모은 배포용 폴더(dist)를 만든다.

.PARAMETER QtDir
    사용할 Qt kit 경로. 기본값: C:\Qt\6.6.3\msvc2019_64

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File .\build_release.ps1
    powershell -ExecutionPolicy Bypass -File .\build_release.ps1 -QtDir "C:\Qt\6.7.0\msvc2019_64"
#>

param(
    [string]$QtDir = "C:\Qt\6.6.3\msvc2019_64"
)

$ErrorActionPreference = "Stop"

$ProjectDir = $PSScriptRoot
$BuildDir   = Join-Path $ProjectDir "build-release"
$DistDir    = Join-Path $ProjectDir "dist"
$ExeName    = "qt_image_maker.exe"

function Write-Step([string]$Message)
{
    Write-Host ""
    Write-Host "== $Message ==" -ForegroundColor Cyan
}

function Fail([string]$Message)
{
    Write-Host "[오류] $Message" -ForegroundColor Red
    exit 1
}

# 1. Qt 설치 확인 --------------------------------------------------------
if (-not (Test-Path $QtDir))
{
    Fail "Qt 경로를 찾을 수 없습니다: $QtDir (-QtDir 옵션으로 실제 설치 경로를 지정하세요)"
}

$WinDeployQt = Join-Path $QtDir "bin\windeployqt6.exe"
if (-not (Test-Path $WinDeployQt))
{
    $WinDeployQt = Join-Path $QtDir "bin\windeployqt.exe"
}
if (-not (Test-Path $WinDeployQt))
{
    Fail "windeployqt(6).exe를 찾을 수 없습니다: $QtDir\bin"
}

# 2. Visual Studio (MSVC) 빌드 도구 위치 탐색 -----------------------------
Write-Step "Visual Studio 빌드 도구 탐색"

$VsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $VsWhere))
{
    Fail "vswhere.exe를 찾을 수 없습니다. Visual Studio(C++ 빌드 도구 포함)가 설치되어 있어야 합니다."
}

$VsInstallPath = & $VsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if ([string]::IsNullOrWhiteSpace($VsInstallPath))
{
    Fail "C++ 빌드 도구가 설치된 Visual Studio를 찾지 못했습니다."
}

$VcVars64 = Join-Path $VsInstallPath "VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path $VcVars64))
{
    Fail "vcvars64.bat를 찾을 수 없습니다: $VcVars64"
}

Write-Host "Visual Studio: $VsInstallPath"

# vcvars64.bat이 설정하는 환경 변수를 현재 PowerShell 세션으로 가져온다.
$VcVarsOutput = cmd /c "`"$VcVars64`" && set"
foreach ($line in $VcVarsOutput)
{
    if ($line -match "^([^=]+)=(.*)$")
    {
        Set-Item -Path "Env:$($Matches[1])" -Value $Matches[2]
    }
}

if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue))
{
    Fail "cl.exe를 찾을 수 없습니다 (vcvars64.bat 적용 실패)."
}

# 3. Ninja 확인 (Qt 설치 시 함께 제공되는 것을 우선 사용) -------------------
$QtRoot = Split-Path (Split-Path $QtDir -Parent) -Parent
$NinjaCandidate = Join-Path $QtRoot "Tools\Ninja\ninja.exe"
if (Test-Path $NinjaCandidate)
{
    $env:PATH = "$(Split-Path $NinjaCandidate);$env:PATH"
}
elseif (-not (Get-Command ninja -ErrorAction SilentlyContinue))
{
    Fail "ninja.exe를 찾을 수 없습니다. Qt Maintenance Tool에서 'Ninja' 도구를 설치하거나 PATH에 추가하세요."
}

# 4. CMake 구성 + 빌드 (Release) -----------------------------------------
Write-Step "CMake 구성 (Release)"

cmake -S "$ProjectDir" -B "$BuildDir" -G Ninja `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_PREFIX_PATH="$QtDir" `
    -DCMAKE_C_COMPILER=cl `
    -DCMAKE_CXX_COMPILER=cl
if ($LASTEXITCODE -ne 0)
{
    Fail "CMake 구성에 실패했습니다."
}

Write-Step "빌드 (Release)"

cmake --build "$BuildDir"
if ($LASTEXITCODE -ne 0)
{
    Fail "빌드에 실패했습니다. (실행 중인 이전 exe가 있다면 종료 후 다시 시도하세요)"
}

$BuiltExe = Join-Path $BuildDir $ExeName
if (-not (Test-Path $BuiltExe))
{
    Fail "빌드 결과 exe를 찾을 수 없습니다: $BuiltExe"
}

# 5. 배포 폴더 구성 --------------------------------------------------------
Write-Step "배포 폴더(dist) 구성"

if (Test-Path $DistDir)
{
    try
    {
        Remove-Item -Path $DistDir -Recurse -Force -ErrorAction Stop
    }
    catch
    {
        Fail "기존 dist 폴더를 지울 수 없습니다 (dist\$ExeName이 실행 중인지 확인하세요): $($_.Exception.Message)"
    }
}
New-Item -ItemType Directory -Path $DistDir | Out-Null
Copy-Item -Path $BuiltExe -Destination $DistDir

# 6. windeployqt로 Qt DLL / 플러그인 동봉 --------------------------------
Write-Step "windeployqt 실행 (Qt DLL / 플러그인 동봉)"

$DistExe = Join-Path $DistDir $ExeName
& $WinDeployQt --release --compiler-runtime "$DistExe"
if ($LASTEXITCODE -ne 0)
{
    Fail "windeployqt 실행에 실패했습니다."
}

# 7. VC++ 런타임 DLL 동봉 ---------------------------------------------------
# windeployqt --compiler-runtime 옵션이 환경에 따라 vcruntime/msvcp DLL을
# 챙기지 못하는 경우가 있어, VS Redist 폴더에서 직접 찾아 복사한다.
Write-Step "VC++ 런타임 DLL 확인"

$RedistRoot = Join-Path $VsInstallPath "VC\Redist\MSVC"
$NeedsRuntimeDll = -not (Test-Path (Join-Path $DistDir "vcruntime140.dll"))

if ($NeedsRuntimeDll -and (Test-Path $RedistRoot))
{
    $RedistVersionDir = Get-ChildItem -Path $RedistRoot -Directory |
        Where-Object { $_.Name -match '^\d+\.\d+\.\d+$' } |
        Sort-Object Name -Descending |
        Select-Object -First 1

    if ($RedistVersionDir)
    {
        $CrtDir = Get-ChildItem -Path (Join-Path $RedistVersionDir.FullName "x64") -Directory -Filter "Microsoft.VC*.CRT" |
            Select-Object -First 1

        if ($CrtDir)
        {
            Copy-Item -Path (Join-Path $CrtDir.FullName "*.dll") -Destination $DistDir -Force
            Write-Host "VC++ 런타임 DLL 복사됨: $($CrtDir.FullName)"
        }
    }
}

if (-not (Test-Path (Join-Path $DistDir "vcruntime140.dll")))
{
    Write-Host "[경고] vcruntime140.dll을 동봉하지 못했습니다. 대상 PC에 VC++ 재배포 패키지가 필요할 수 있습니다." -ForegroundColor Yellow
}

# 8. 완료 -------------------------------------------------------------------
Write-Step "완료"

$DistSize = (Get-ChildItem -Path $DistDir -Recurse | Measure-Object -Property Length -Sum).Sum / 1MB
Write-Host ("배포 폴더: {0}" -f $DistDir) -ForegroundColor Green
Write-Host ("크기: {0:N1} MB" -f $DistSize) -ForegroundColor Green
Write-Host "이 폴더(dist) 전체를 그대로 복사/압축해서 배포하면, Qt가 설치되지 않은 윈도우에서도 실행됩니다."
