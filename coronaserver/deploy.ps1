# Define the source folder, destination folder, and ZIP file name
$SourceFolder = "..\coronasetup\Release\*"
$DestinationFolder = "..\..\countryvideogameshome\public"
$ZipFileName = "coronaserver.zip"

# Create the full path for the ZIP file
$ZipFilePath = Join-Path -Path $DestinationFolder -ChildPath $ZipFileName

# Compress the source folder into a ZIP file
Compress-Archive -Path $SourceFolder -DestinationPath $ZipFilePath

# Verify if the ZIP file exists and copy it to the destination folder
if (Test-Path $ZipFilePath) {
    Write-Output "The folder has been successfully compressed and saved as $ZipFilePath."
} else {
    Write-Output "Failed to compress the folder. $ZipFilePath"
    Write-Output "Compress-Archive -Path $SourceFolder -DestinationPath $ZipFilePath"
}
