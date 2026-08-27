$ProjectRoot = Split-Path -Parent $PSScriptRoot
$DefaultGameIni = Join-Path $ProjectRoot "Config\DefaultGame.ini"
$RequiredEntries = @(
    "BuildConfiguration=PPBC_Shipping",
    "bUsePakFile=True",
    "bUseIoStore=True",
    "bCompressed=True",
    "bIncludePrerequisites=True",
    '+MapsToCook=(FilePath="/Game/Maps/Main/L_Main")',
    '+DirectoriesToAlwaysCook=(Path="/GF_Geojunggi")',
    '+DirectoriesToAlwaysCook=(Path="/GF_Gongsimdon")',
    '+DirectoriesToAlwaysCook=(Path="/GF_Singijeon")',
    '+DirectoriesToAlwaysCook=(Path="/GF_OngseongCrossbow")'
)

$Content = Get-Content -LiteralPath $DefaultGameIni -Raw
$Missing = $RequiredEntries | Where-Object { -not $Content.Contains($_) }
if ($Missing) {
    throw "Packaging configuration is incomplete:`n$($Missing -join "`n")"
}

Write-Host "PACKAGING CONFIGURATION VERIFIED: Win64 Shipping, L_Main, and Game Feature cook directories are configured."
