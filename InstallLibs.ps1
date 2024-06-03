# Define the target directory
$targetDir = "$env:USERPROFILE\Documents\libraries"

# Ensure the target directory exists
if (-Not (Test-Path -Path $targetDir)) {
    New-Item -Path $targetDir -ItemType Directory
}

# Function to download and copy submodules
function Download-Submodules {
    # Read the .gitmodules file
    $gitmodulesPath = ".gitmodules"

    if (-Not (Test-Path -Path $gitmodulesPath)) {
        Write-Error "No .gitmodules file found in the current directory."
        return
    }

    # Parse the .gitmodules file
    $gitmodulesContent = Get-Content -Path $gitmodulesPath
    $submodules = @()
    $currentSubmodule = @{}
    
    foreach ($line in $gitmodulesContent) {
        if ($line -match "^\[submodule \"(.*)\"\]$") {
            if ($currentSubmodule) {
                $submodules += [PSCustomObject]$currentSubmodule
                $currentSubmodule = @{}
            }
            $currentSubmodule.Name = $matches[1]
        }
        elseif ($line -match "^\s*path\s*=\s*(.*)$") {
            $currentSubmodule.Path = $matches[1]
        }
        elseif ($line -match "^\s*url\s*=\s*(.*)$") {
            $currentSubmodule.Url = $matches[1]
        }
    }

    if ($currentSubmodule) {
        $submodules += [PSCustomObject]$currentSubmodule
    }

    # Clone each submodule and copy to the target directory
    foreach ($submodule in $submodules) {
        $submoduleDir = "$targetDir\$($submodule.Name)"
        $submoduleUrl = $submodule.Url

        # Clone the submodule if it doesn't already exist
        if (-Not (Test-Path -Path $submoduleDir)) {
            git clone $submoduleUrl $submoduleDir
        }

        # Verify cloning success
        if (-Not (Test-Path -Path $submoduleDir)) {
            Write-Error "Failed to clone submodule '$($submodule.Name)' from '$submoduleUrl'."
        } else {
            Write-Output "Submodule '$($submodule.Name)' cloned to '$submoduleDir'."
        }
    }
}

# Run the function
Download-Submodules
