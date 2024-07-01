# Set the necessary paths
$freeRTOSPath = "C:\FreeRTOSv202212.01\FreeRTOS\Source"
$projectPath = "C:\Users\PMLS\Documents\Projects\RTOS_arduinoBlink"
$openCVIncludePath = "C:\OpenCV\include"
$openCVLibPath = "C:\OpenCV\build\x64\vc16\lib"

# Compile the code
gcc "$projectPath\rtos-draft.c" -o "$projectPath\rtos-draft.exe" `
    -I "$freeRTOSPath\include" `
    -I "$freeRTOSPath\portable\MSVC-MingW" `
    -I "$projectPath" `
    -I "$openCVIncludePath" `
    -L "$openCVLibPath" `
    -lopencv_core -lopencv_imgproc -lopencv_highgui

# Output a message indicating the result
if ($?) {
    Write-Host "Compilation succeeded."
} else {
    Write-Host "Compilation failed." -ForegroundColor Red
}
