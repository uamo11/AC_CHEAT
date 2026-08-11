#include <windows.h>
#include <iostream>
#include <iomanip>

int main() {
    HWND hwnd = FindWindowA(NULL, "AssaultCube");
    if (!hwnd) { std::cout << "No AC found\n"; return 1; }
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    HANDLE hProc = OpenProcess(PROCESS_VM_READ, FALSE, pid);
    
    uintptr_t base = 0x540000;
    uintptr_t lpPtr = 0x00916550; // The user provided this!
    
    std::cout << "LocalPlayer: 0x" << std::hex << lpPtr << "\n";
    
    for (int i = 0x360; i <= 0x390; i += 4) {
        uintptr_t val = 0;
        ReadProcessMemory(hProc, (LPCVOID)(lpPtr + i), &val, 4, NULL);
        std::cout << "Offset 0x" << std::hex << i << ": 0x" << val << "\n";
    }
    return 0;
}
