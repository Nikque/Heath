# Quick checks of a change (one logical CPU): builds, fforum-20-39 solved with one thread and compared with the scores
# of the problem file (tests/check-solve.py), the regression cases, the C API test.
# usage: pwsh -File quick_check.ps1 <output directory> -Eval <eval.dat> [-Cpu n] [-Asm]
#   -Eval: an eval.dat of Heath (64 slots: the builds of this script have 64 slots). bin\data\eval.dat of the
#   repository is the one of eval2, which the builds read with a warning (their evaluation is then wrong).
#   -Cpu: the logical CPU of the builds and the runs (default 0).
#   -Asm also writes the assembly listing (MSVC x64 AVX-512, /O2 /FAs) to <output directory>\asm\all.asm.
# The exit code is the number of the checks that failed (a build, a run that does not end with 0, a result that differs).
param([Parameter(Mandatory)][string]$Out, [Parameter(Mandatory)][string]$Eval, [int]$Cpu = 0, [switch]$Asm)

$ErrorActionPreference = 'Continue'
$repo = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
if (-not (Test-Path -LiteralPath $Eval -PathType Leaf)) { "no eval.dat: $Eval"; exit 1 }
$Eval = (Resolve-Path -LiteralPath $Eval).Path
New-Item -ItemType Directory -Force $Out | Out-Null
$env:PATH = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer;$env:PATH"
$vsdev = 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat'
$failed = 0

function Run1([string]$file, [string[]]$arguments, [string]$cwd, [string]$stdout) {
	$p = Start-Process -FilePath $file -ArgumentList $arguments -WorkingDirectory $cwd -NoNewWindow -PassThru -RedirectStandardOutput $stdout
	$p.ProcessorAffinity = [IntPtr](1L -shl $Cpu)
	$p.WaitForExit()
	return $p.ExitCode
}

foreach ($c in @(@('x64v4', 'x64', ''), @('x86sse', 'x86', '/D hasSSE2'))) {
	$exe = Join-Path $Out "edax-$($c[0]).exe"
	$code = Run1 'cmd.exe' @('/c', "$PSScriptRoot\build-msvc.cmd $($c[1]) $exe $($c[2])") $repo (Join-Path $Out "build-$($c[0]).log")
	$warnings = @(Select-String -Path (Join-Path $Out "build-$($c[0]).log") -Pattern 'warning|error' | Where-Object { $_.Line -notmatch 'C4805' })
	if ($code -ne 0) { "$($c[0]): BUILD FAILED"; $warnings | Select-Object -First 5 | ForEach-Object { '  ' + $_.Line }; $failed++; continue }
	if ($warnings.Count) { "$($c[0]): $($warnings.Count) new warnings"; $warnings | Select-Object -First 5 | ForEach-Object { '  ' + $_.Line } }
	# check-solve.py runs the program, checks its exit code and compares each result with the file (exit 0: all right)
	$solve = Join-Path $Out "solve-$($c[0]).stdout"
	$code = Run1 'py' @('-3', "$repo\tests\check-solve.py", $exe, "$repo\problem\fforum-20-39.obf", '--eval', $Eval, '--cwd', (Join-Path $repo 'bin')) $repo $solve
	if ($code -ne 0) { $failed++ }
	"$($c[0]): size $((Get-Item $exe).Length), solve exit $code, " + (Get-Content $solve | Select-Object -Last 1)
}

$env:EDAX_EVAL_FILE = $Eval
foreach ($s in 'build-regression.cmd', 'build-libedax-test.cmd') {
	$log = Join-Path $Out "$s.log"
	$code = Run1 'cmd.exe' @('/c', "call `"$vsdev`" -arch=x64 -host_arch=x64 >nul && $repo\tests\$s") $repo $log
	if ($code -ne 0) { $failed++ }
	"$s exit $code " + ((Get-Content $log | Select-String 'checks|error C|FAIL') -join ' | ')
}
$env:REGRESSION_CPU = "$Cpu"
$r = py -3 "$PSScriptRoot\run_regression.py" (Join-Path $repo 'tests\regression.exe') (Join-Path $Out 'regression') $Eval (Join-Path $repo 'tests\merge-complete-fixture.dat') | Select-Object -Last 1
if ($LASTEXITCODE -ne 0) { $failed++ }
"regression: $r"

if ($Asm) {
	$asmDir = Join-Path $Out 'asm'
	New-Item -ItemType Directory -Force $asmDir | Out-Null
	$flags = '/std:c++23preview /Zc:__cplusplus /EHs-c- /D _HAS_EXCEPTIONS=0 /GR- /D UNICODE /utf-8 /D _CRT_SECURE_NO_DEPRECATE /I..\include /I. /I..\..\third_party\boost_1_92_0 /O2 /fp:fast /GS- /D NDEBUG /MT /D HAS_CPU_64 /arch:AVX512 /vlen=256 /D POPCOUNT'
	$code = Run1 'cmd.exe' @('/c', "call `"$vsdev`" -arch=x64 -host_arch=x64 >nul && cl /nologo $flags /c /FAs /Fa$asmDir\all.asm /Fo$asmDir\all.obj all.cpp") (Join-Path $repo 'src') (Join-Path $asmDir 'build.log')
	if ($code -ne 0) { $failed++ }
	"asm: exit $code"
}
"checks failed: $failed"
exit $failed
