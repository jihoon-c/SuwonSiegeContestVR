param(
    [switch]$Clean,
    [string]$OutputDirectory = ""
)

# Packages the same Win64 Shipping configuration that is stored in
# Config/DefaultGame.ini.  Run from the project root in PowerShell.
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$ProjectFile = Join-Path $ProjectRoot "SuwonSiegeContestVR.uproject"
$RunUAT = "F:\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat"

if (-not (Test-Path -LiteralPath $RunUAT)) {
    throw "Unreal Automation Tool was not found: $RunUAT"
}

$Arguments = @(
    "BuildCookRun",
    "-project=$ProjectFile",
    "-noP4",
    "-platform=Win64",
    "-clientconfig=Shipping",
    "-build",
    "-cook",
    "-stage",
    "-pak",
    "-iostore",
    "-package",
    "-prereqs",
    "-utf8output"
)

if ($Clean) {
    $Arguments += "-clean"
}

if ($OutputDirectory) {
    $ResolvedOutput = [System.IO.Path]::GetFullPath($OutputDirectory)
    $Arguments += "-archivedirectory=$ResolvedOutput"
    $Arguments += "-archive"
}

& $RunUAT @Arguments
if ($LASTEXITCODE -ne 0) {
    throw "Win64 Shipping package failed with exit code $LASTEXITCODE"
}
