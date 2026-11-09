# =============================================================================
# build_armcc.ps1  (ASCII only - Windows PowerShell 5.1 safe)
# Build this Keil MDK project with the bundled ARMCC 5.06 toolchain
# (armcc / armasm / armlink / fromelf) from the command line.
#
# Purpose: verify that the project compiles and links even when the uVision
# GUI (UV4.exe) cannot be started in the current session.
#
# Output: MDK-ARM\Objects_armcc_<Mode>\*.o, *.axf, *.hex, *.map
# Usage : powershell -ExecutionPolicy Bypass -File .\_tools\build_armcc.ps1 -Mode I2C
# =============================================================================
param(
    [ValidateSet('I2C','RVC')]
    [string]$Mode = 'I2C'
)

$ErrorActionPreference = 'Continue'

$projectDir = Resolve-Path (Join-Path $PSScriptRoot '..')
$mdkDir     = Join-Path $projectDir 'MDK-ARM'
$projFile   = Join-Path $mdkDir 'MPU_C86T_BNO08X.uvprojx'
$outDir     = Join-Path $mdkDir ('Objects_armcc_' + $Mode)
$keilRoot   = 'C:\Keil_v5\ARM\ARMCC\bin'
$armcc      = Join-Path $keilRoot 'armcc.exe'
$armasm     = Join-Path $keilRoot 'armasm.exe'
$armlink    = Join-Path $keilRoot 'armlink.exe'
$fromelf    = Join-Path $keilRoot 'fromelf.exe'

if (-not (Test-Path $projFile))   { throw "Project file not found: $projFile" }
foreach ($tool in @($armcc, $armasm, $armlink, $fromelf)) {
    if (-not (Test-Path $tool))   { throw "Tool not found: $tool" }
}

New-Item -ItemType Directory -Force -Path $outDir | Out-Null

# ---- read source list / include paths / defines from uvprojx ----------------
[xml]$xml = Get-Content -LiteralPath $projFile
$target   = $xml.Project.Targets.Target
$cads     = $target.TargetOption.TargetArmAds.Cads
$groups   = $target.Groups.Group

$includeRel = $cads.VariousControls.IncludePath -split ';' | Where-Object { $_ -ne '' }
$includeArgs = @()
foreach ($inc in $includeRel) {
    $full = (Resolve-Path (Join-Path $mdkDir $inc)).Path
    $includeArgs += ('-I' + $full)
}
$defineArgs = @()
foreach ($d in ($cads.VariousControls.Define -split ',')) {
    if ($d -ne '') { $defineArgs += ('-D' + $d) }
}
if ($Mode -eq 'RVC') { $defineArgs += '-DBNO08X_MODE=2' } else { $defineArgs += '-DBNO08X_MODE=1' }

$cSources   = New-Object System.Collections.Generic.List[string]
$asmSources = New-Object System.Collections.Generic.List[string]
$objectList = New-Object System.Collections.Generic.List[string]

foreach ($group in $groups) {
    foreach ($file in $group.Files.File) {
        $rel  = $file.FilePath
        $type = [int]$file.FileType
        $full = Join-Path $mdkDir $rel
        if (-not (Test-Path $full)) { throw "Source file missing: $full" }
        $base = [System.IO.Path]::GetFileNameWithoutExtension($full)
        if ($type -eq 1) {
            $cSources.Add($full)
            $objectList.Add((Join-Path $outDir ($base + '.o')))
        }
        elseif ($type -eq 2) {
            $asmSources.Add($full)
            $objectList.Add((Join-Path $outDir ($base + '.o')))
        }
    }
}

Write-Host ("C sources : {0}" -f $cSources.Count)
Write-Host ("ASM files : {0}" -f $asmSources.Count)
Write-Host ("Output    : {0}" -f $outDir)
Write-Host ("Mode      : {0}" -f $Mode)

# ---- compile C sources ------------------------------------------------------
$commonArgs = @(
    '--cpu', 'Cortex-M3',
    '--c99',
    '--apcs=interwork',
    '-O1',
    '--split_sections',
    '-c',
    '-D__MICROLIB'
) + $defineArgs + $includeArgs

$failed = @()
foreach ($src in $cSources) {
    $base = [System.IO.Path]::GetFileNameWithoutExtension($src)
    $obj  = Join-Path $outDir ($base + '.o')
    $log  = Join-Path $outDir ($base + '.cc.log')
    $out  = & $armcc @commonArgs $src -o $obj 2>&1
    [System.IO.File]::WriteAllText($log, ((@($out) | ForEach-Object { $_.ToString() }) -join [Environment]::NewLine))
    if ($LASTEXITCODE -ne 0) {
        $failed += $base
        Write-Host ("[FAIL] {0}" -f $base) -ForegroundColor Red
        $out | Select-Object -First 30 | ForEach-Object { Write-Host ("    " + $_) }
    }
    else {
        $warn = $out | Where-Object { $_ -match 'warning' }
        if ($warn) {
            Write-Host ("[WARN] {0} : {1} warning(s)" -f $base, @($warn).Count) -ForegroundColor Yellow
        }
    }
}

# ---- assemble startup file --------------------------------------------------
$asmArgs = @('--cpu', 'Cortex-M3', '--apcs=interwork', '--pd', '__MICROLIB SETA 1') + $includeArgs + @('-o')
foreach ($src in $asmSources) {
    $base = [System.IO.Path]::GetFileNameWithoutExtension($src)
    $obj  = Join-Path $outDir ($base + '.o')
    $out  = & $armasm @asmArgs $obj $src 2>&1
    [System.IO.File]::WriteAllText((Join-Path $outDir ($base + '.as.log')), ((@($out) | ForEach-Object { $_.ToString() }) -join [Environment]::NewLine))
    if ($LASTEXITCODE -ne 0) {
        $failed += $base
        Write-Host ("[FAIL] asm {0}" -f $base) -ForegroundColor Red
        $out | Select-Object -First 20 | ForEach-Object { Write-Host ("    " + $_) }
    }
}

if ($failed.Count -gt 0) {
    throw ("Build failed: " + ($failed -join ', '))
}

# ---- link -------------------------------------------------------------------
$sct = Join-Path $outDir 'MPU_C86T_BNO08X.sct'
$scatterText = @(
'LR_IROM1 0x08000000 0x00010000  {',
'  ER_IROM1 0x08000000 0x00010000  {',
'   *.o (RESET, +First)',
'   *(InRoot$$Sections)',
'   .ANY (+RO)',
'  }',
'  RW_IRAM1 0x20000000 0x00005000  {',
'   .ANY (+RW +ZI)',
'  }',
'}'
)
$scatterText | Set-Content -LiteralPath $sct -Encoding ASCII

$axf     = Join-Path $outDir 'MPU_C86T_BNO08X.axf'
$map     = Join-Path $outDir 'MPU_C86T_BNO08X.map'
$hex     = Join-Path $outDir 'MPU_C86T_BNO08X.hex'
$linkLog = Join-Path $outDir 'link.log'

$linkArgs = @(
    '--cpu', 'Cortex-M3',
    '--library_type=microlib',
    '--scatter', $sct,
    '--info', 'sizes',
    '--info', 'totals',
    '--list', $map,
    '-o', $axf
) + $objectList.ToArray()

$out = & $armlink @linkArgs 2>&1
[System.IO.File]::WriteAllText($linkLog, ((@($out) | ForEach-Object { $_.ToString() }) -join [Environment]::NewLine))
if ($LASTEXITCODE -ne 0) {
    Write-Host '[LINK FAILED]' -ForegroundColor Red
    $out | Select-Object -First 50 | ForEach-Object { Write-Host ("    " + $_) }
    throw 'Link failed'
}
@($out) | ForEach-Object { Write-Host ("    " + $_.ToString()) }

# ---- generate hex -----------------------------------------------------------
$out = & $fromelf --i32combined --output $hex $axf 2>&1
if ($LASTEXITCODE -ne 0) { throw 'fromelf failed' }

Write-Host ''
Write-Host '========== BUILD OK ==========' -ForegroundColor Green
Get-Item $axf, $hex | Format-Table Name, Length, LastWriteTime -AutoSize
