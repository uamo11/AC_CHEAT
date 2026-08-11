@echo off
setlocal

set CC=C:\winlibs\mingw32\bin\g++.exe
set SRC=src\dllmain.cpp
set OUT=bin\ac_cheat_v6.dll
set INJ=bin\inyeccion.exe
set GAME_DIR=C:\Program Files (x86)\AssaultCube 1.3.0.2

echo [*] Creando bin\...
if not exist bin mkdir bin

echo [*] Compilando AC Cheat DLL con ImGui...
"%CC%" -shared -m32 -o "%OUT%" src\dllmain.cpp ^
    src\imgui\imgui.cpp src\imgui\imgui_draw.cpp src\imgui\imgui_tables.cpp src\imgui\imgui_widgets.cpp ^
    src\imgui\imgui_impl_win32.cpp src\imgui\imgui_impl_opengl2.cpp ^
    -I"src" -I"src\imgui" -std=c++17 ^
    -lopengl32 -lgdi32 -luser32 -lkernel32 -ldwmapi ^
    -static -static-libgcc -static-libstdc++ ^
    -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive
if errorlevel 1 ( echo [-] Error compilando DLL & pause & exit /b 1 )

echo [*] Compilando Inyector AC...
"%CC%" -O2 -m32 -o "%INJ%" src\injector_ac.cpp ^
  -lkernel32 -luser32 ^
  -static -static-libgcc -static-libstdc++ ^
  -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive
if errorlevel 1 ( echo [-] Error compilando inyector & pause & exit /b 1 )

echo [*] Copiando DLL junto a ac_client...
copy /Y "%OUT%" "%GAME_DIR%\bin_win32\ac_cheat_v6.dll" >nul 2>&1

echo.
echo [OK] Listo!
echo.
echo 1. Abre AssaultCube normalmente
echo 2. Corre bin\inyeccion.exe como Admin
echo 3. INSERT = Menu toggle
echo 4. F1=ESP / F2=Aimbot / F3=NoRecoil / F4=InfAmmo
echo 5. END = descargar limpio
echo.
echo Log: C:\Users\Usuario\Documents\C\AC_CHEAT\cheat_log.txt

endlocal
