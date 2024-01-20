param(
    [string]$ConfigFile = 'BuildConfig.json',
    [switch]$Clean
)

$config = Get-Content $ConfigFile | ConvertFrom-Json

Write-Host $config | Format-Table

if ($Clean) {
    Remove-Item -Path $config.BuildDirectory -Recurse -Force | Out-Null
}

if (-not (Test-Path $config.BuildDirectory -PathType Container)) {
    New-Item -ItemType Directory -Path $config.BuildDirectory -Force | Out-Null
}

cmake `
    -G 'Ninja' `
    -DCMAKE_TOOLCHAIN_FILE="$($config.VcpkgToolchain)" `
    -B $config.BuildDirectory `
    -S .


ninja -C $config.BuildDirectory
