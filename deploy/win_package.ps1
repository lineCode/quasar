param (
    [switch]$skipartifact = $false,
    [switch]$portable = $false,
    [switch]$installer = $false,
    [string]$name = (git describe --tags --always),
    [switch]$delete = $false
 )

if (!$skipartifact)
{
    Write-Host "Packing artifacts ($($name))..."

    Copy-Item .\build\x64\Release\ .\Quasar\ -recurse

    Remove-Item .\Quasar\*.ilk
    Remove-Item .\Quasar\*.exp
    Remove-Item .\Quasar\*.iobj
    Remove-Item .\Quasar\*.ipdb
    Remove-Item .\Quasar\plugin-api.lib

    Remove-Item .\Quasar\plugins\*.exp
    Remove-Item .\Quasar\plugins\*.iobj
    Remove-Item .\Quasar\plugins\*.lib
    Remove-Item .\Quasar\plugins\*.ipdb

    Copy-Item .\widgets\ .\Quasar\ -recurse
    Copy-Item README.md .\Quasar\
    Copy-Item LICENSE.txt .\Quasar\

    if (($env:OPENSSL) -and (Test-Path $env:OPENSSL -pathType container))
    {
        Copy-Item $env:OPENSSL\libeay32.dll .\Quasar\
        Copy-Item $env:OPENSSL\ssleay32.dll .\Quasar\
    }
}

if ($portable)
{
    if (Test-Path .\Quasar -pathType container)
    {
        $pkgname = "$($name)_win_x64_portable.7z"

        Write-Host "Packaging $($pkgname)..."

        & 7z a -t7z $pkgname .\Quasar\

        if ($env:APPVEYOR -eq $true)
        {
            Get-ChildItem $pkgname | % { Push-AppveyorArtifact $_.FullName -FileName $_.Name }
        }
    }
    else
    {
        Write-Error "Artifact path not found."
    }
}

if ($installer)
{
    if (Test-Path .\Quasar -pathType container)
    {
        $pkgname = "$($name)_win_x64_installer.msi"

        Write-Host "Packaging $($pkgname)..."

        if ($env:WIX)
        {
            & $env:WIX\bin\candle.exe -arch x64 .\deploy\quasar.wxs
            & $env:WIX\bin\light.exe -ext WixUIExtension -out $pkgname quasar.wixobj

            if ($env:APPVEYOR -eq $true)
            {
                Get-ChildItem $pkgname | % { Push-AppveyorArtifact $_.FullName -FileName $_.Name }
            }
        }
    }
    else
    {
        Write-Error "Artifact path not found."
    }
}

if ($delete)
{
    Remove-Item .\Quasar -Force -Recurse
}
