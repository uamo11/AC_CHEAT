$ErrorActionPreference = 'Stop'
New-Item -ItemType Directory -Force -Path src\imgui
$files = @('imgui.h', 'imgui.cpp', 'imgui_draw.cpp', 'imgui_tables.cpp', 'imgui_widgets.cpp', 'imgui_internal.h', 'imstb_rectpack.h', 'imstb_textedit.h', 'imstb_truetype.h')
foreach ($f in $files) {
    Invoke-WebRequest -Uri "https://raw.githubusercontent.com/ocornut/imgui/master/$f" -OutFile "src\imgui\$f"
}
$backends = @('imgui_impl_win32.h', 'imgui_impl_win32.cpp', 'imgui_impl_opengl2.h', 'imgui_impl_opengl2.cpp')
foreach ($b in $backends) {
    Invoke-WebRequest -Uri "https://raw.githubusercontent.com/ocornut/imgui/master/backends/$b" -OutFile "src\imgui\$b"
}
Write-Host "Download complete"
