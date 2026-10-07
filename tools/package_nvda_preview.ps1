param(
    [Parameter(Mandatory = $true)][string]$Executable,
    [Parameter(Mandatory = $true)][string]$RuntimeDirectory,
    [Parameter(Mandatory = $true)][string]$ControllerDirectory,
    [string]$Destination = 'dist/nvda'
)

$ErrorActionPreference = 'Stop'
$repository = Split-Path -Parent $PSScriptRoot
$target = [IO.Path]::GetFullPath((Join-Path $repository $Destination))
$binary = (Resolve-Path -LiteralPath $Executable).Path
$runtime = (Resolve-Path -LiteralPath $RuntimeDirectory).Path
$controller = (Resolve-Path -LiteralPath $ControllerDirectory).Path
if (-not (Test-Path -LiteralPath (Join-Path $controller 'nvdaControllerClient.dll'))) {
    throw 'Choose the controller directory matching the executable architecture (normally x64).'
}
New-Item -ItemType Directory -Path $target -Force | Out-Null
Copy-Item -LiteralPath $binary -Destination (Join-Path $target 'cataclysm-tiles.exe')
foreach ($directory in @('data', 'gfx', 'lang')) {
    Copy-Item -LiteralPath (Join-Path $repository $directory) -Destination $target -Recurse -Force
}

# Resolve the MinGW import graph instead of relying on a user's global PATH.
$objdump = Join-Path $runtime 'objdump.exe'
if (-not (Test-Path -LiteralPath $objdump)) {
    throw 'The runtime directory must contain the MinGW objdump tool.'
}
$pending = [Collections.Generic.Queue[string]]::new()
$seen = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
$pending.Enqueue($binary)
while ($pending.Count -gt 0) {
    $current = $pending.Dequeue()
    $imports = & $objdump -p $current
    if ($LASTEXITCODE -ne 0) { throw "Could not inspect imports of $current" }
    foreach ($line in $imports) {
        if ($line -match 'DLL Name:\s*(\S+)') {
            $name = $Matches[1]
            if ($seen.Add($name)) {
                $dependency = Join-Path $runtime $name
                if (Test-Path -LiteralPath $dependency) {
                    Copy-Item -LiteralPath $dependency -Destination $target -Force
                    $pending.Enqueue($dependency)
                } elseif (-not (Test-Path -LiteralPath (Join-Path $env:WINDIR "System32/$name")) -and
                          $name -notlike 'api-ms-*' -and $name -notlike 'ext-ms-*') {
                    throw "Missing runtime dependency: $name"
                }
            }
        }
    }
}

Copy-Item -LiteralPath (Join-Path $controller 'nvdaControllerClient.dll') -Destination $target -Force
$controllerLicense = Join-Path (Split-Path -Parent $controller) 'license.txt'
Copy-Item -LiteralPath $controllerLicense -Destination (Join-Path $target 'NVDA-CONTROLLER-LICENSE.txt') -Force
foreach ($license in @('LICENSE.txt', 'LICENSE-Apache-Robot-Font.txt', 'LICENSE-OFL-Terminus-Font.txt')) {
    Copy-Item -LiteralPath (Join-Path $repository $license) -Destination $target -Force
}
Copy-Item -LiteralPath (Join-Path $repository 'doc/NVDA_ACCESSIBILITY.md') -Destination $target -Force

# A separate user directory keeps preview saves and options apart from the normal release.
$previewUser = Join-Path $target 'user'
New-Item -ItemType Directory -Path (Join-Path $previewUser 'config') -Force | Out-Null
$settings = Join-Path $previewUser 'config/options.json'
if (-not (Test-Path -LiteralPath $settings)) {
    [IO.File]::WriteAllText($settings, '[{"name":"SCREEN_READER_MODE","value":"true"}]',
                          [Text.UTF8Encoding]::new($false))
}
$launcher = @'
@echo off
cd /d "%~dp0"
start "" "cataclysm-tiles.exe" --basepath "%~dp0." --userdir "%~dp0user/"
'@
[IO.File]::WriteAllText((Join-Path $target 'Run-Accessible-Game.bat'), $launcher.Replace("`n", "`r`n"),
                      [Text.Encoding]::ASCII)
Write-Output "NVDA preview packaged in $target"
