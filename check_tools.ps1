Get-ItemProperty HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall\* | ForEach-Object { 
    if ($_.DisplayName -match 'Visual|SDK|C\+\+|MinGW|Build|Java|JDK') { 
        [PSCustomObject]@{
            Name = $_.DisplayName
            Location = $_.InstallLocation
        }
    } 
}
Get-ItemProperty HKLM:\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\* | ForEach-Object { 
    if ($_.DisplayName -match 'Visual|SDK|C\+\+|MinGW|Build|Java|JDK') { 
        [PSCustomObject]@{
            Name = $_.DisplayName
            Location = $_.InstallLocation
        }
    } 
}
