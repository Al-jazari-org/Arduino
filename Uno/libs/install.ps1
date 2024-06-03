# Get the current user's Documents path
$documentsPath = [System.Environment]::GetFolderPath('MyDocuments')
# Define the target path for Arduino libraries
$targetPath = Join-Path -Path $documentsPath -ChildPath 'Arduino\libraries'

# Get the current script directory
$scriptDirectory = Split-Path -Parent $MyInvocation.MyCommand.Definition

# Ensure the target directory exists
if (!(Test-Path -Path $targetPath)) {
    New-Item -ItemType Directory -Path $targetPath | Out-Null
}

# Get all directories in the current script directory
$directories = Get-ChildItem -Path $scriptDirectory -Directory

# Copy each directory to the target path
foreach ($directory in $directories) {
    $sourcePath = $directory.FullName
    $destinationPath = Join-Path -Path $targetPath -ChildPath $directory.Name

    # Copy the directory to the destination
    Copy-Item -Path $sourcePath -Destination $destinationPath -Recurse
}

Write-Output "All directories copied to $targetPath"
