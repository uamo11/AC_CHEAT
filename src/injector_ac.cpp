// injector_ac.cpp — Inyector para AssaultCube
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <string>

DWORD GetPID(const char* name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe; pe.dwSize = sizeof(pe);
    if (Process32First(snap, &pe)) {
        do {
            if (!_stricmp(pe.szExeFile, name)) {
                CloseHandle(snap);
                return pe.th32ProcessID;
            }
        } while (Process32Next(snap, &pe));
    }
    CloseHandle(snap);
    return 0;
}

int main() {
    std::cout << "[*] Esperando ac_client.exe...\n";
    DWORD pid = 0;
    while (!(pid = GetPID("ac_client.exe"))) { Sleep(500); }
    std::cout << "[+] ac_client.exe PID: " << pid << "\n";

    char dllPath[MAX_PATH];
    GetFullPathNameA("ac_cheat_v6.dll", MAX_PATH, dllPath, nullptr);
    if (GetFileAttributesA(dllPath) == INVALID_FILE_ATTRIBUTES) {
        std::cout << "[-] No se encontro ac_cheat_v6.dll en: " << dllPath << "\n";
        system("pause"); return 1;
    }

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProc) { std::cout << "[-] No se pudo abrir proceso. Corre como Admin.\n"; system("pause"); return 1; }

    void* mem = VirtualAllocEx(hProc, nullptr, strlen(dllPath)+1, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    WriteProcessMemory(hProc, mem, dllPath, strlen(dllPath)+1, nullptr);
    HANDLE hThread = CreateRemoteThread(hProc, nullptr, 0,
        (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"),
        mem, 0, nullptr);
    if (hThread) {
        WaitForSingleObject(hThread, 3000);
        CloseHandle(hThread);
        std::cout << "[+] Inyectado! INSERT=Menu F1=ESP F2=Aimbot F3=NoRecoil F4=InfAmmo END=Salir\n";
    } else {
        std::cout << "[-] Fallo inyeccion. COD: " << GetLastError() << "\n";
    }
    VirtualFreeEx(hProc, mem, 0, MEM_RELEASE);
    CloseHandle(hProc);
    system("pause");
    return 0;
}
