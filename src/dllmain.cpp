// dllmain.cpp — AssaultCube 1.3.0.2 Internal Cheat
// SDL2 SwapWindow hook + GL menu + ESP + Aimbot + InfAmmo + NoRecoil
#include <Windows.h>
#include <gl/GL.h>
#include <cmath>
#include <string>
#include <cstdio>
#include "offsets.hpp"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl2.h"

// Forward declare WndProc handler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ===== CONFIG =====
static bool g_esp      = true;
static bool g_aimbot   = false;
static bool g_noRecoil = false;
static bool g_infAmmo  = false;
static bool g_menu     = false;
static bool g_teamCheck = true;

static const int KEY_MENU    = VK_INSERT;
static const int KEY_AIMBOT  = VK_LBUTTON;

// ===== TIPOS =====
struct Vec3 { float x, y, z; };

struct Entity {
    // Para no lidiar con paddings incorrectos, leemos directo usando Offsets::
    Vec3& pos() { return *(Vec3*)((uintptr_t)this + Offsets::o_posX); }
    float& pitch() { return *(float*)((uintptr_t)this + Offsets::o_pitch); }
    float& yaw() { return *(float*)((uintptr_t)this + Offsets::o_yaw); }
    int& health() { return *(int*)((uintptr_t)this + Offsets::o_health); }
    int& armour() { return *(int*)((uintptr_t)this + Offsets::o_armour); }
    int& team() { return *(int*)((uintptr_t)this + Offsets::o_team); }
    char* name() { return (char*)((uintptr_t)this + Offsets::o_name); }
    int& ammoMag() { return *(int*)((uintptr_t)this + Offsets::o_ammo); }
    int& ammoRes() { return *(int*)((uintptr_t)this + Offsets::o_ammoRes); }
    uint8_t& state() { return *(uint8_t*)((uintptr_t)this + 0x82); } // 0=Alive, 1=Dead, 5=Spectator
};

// ===== GL FUNCTIONS =====
typedef void (APIENTRY* tglBegin)(GLenum);
typedef void (APIENTRY* tglEnd)();
typedef void (APIENTRY* tglVertex2f)(GLfloat, GLfloat);
typedef void (APIENTRY* tglColor4f)(GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY* tglEnable)(GLenum);
typedef void (APIENTRY* tglDisable)(GLenum);
typedef void (APIENTRY* tglBlendFunc)(GLenum, GLenum);
typedef void (APIENTRY* tglMatrixMode)(GLenum);
typedef void (APIENTRY* tglLoadIdentity)();
typedef void (APIENTRY* tglOrtho)(double, double, double, double, double, double);
typedef void (APIENTRY* tglPushAttrib)(GLbitfield);
typedef void (APIENTRY* tglPopAttrib)();
typedef void (APIENTRY* tglPushMatrix)();
typedef void (APIENTRY* tglPopMatrix)();
typedef void (APIENTRY* tglLineWidth)(GLfloat);
typedef void (APIENTRY* tglGetIntegerv)(GLenum, GLint*);
typedef void (APIENTRY* tglRasterPos2f)(GLfloat, GLfloat);
typedef void (APIENTRY* tglListBase)(GLuint);
typedef void (APIENTRY* tglCallLists)(GLsizei, GLenum, const GLvoid*);
typedef GLuint (APIENTRY* tglGenLists)(GLsizei);

static tglBegin         p_glBegin;
static tglEnd           p_glEnd;
static tglVertex2f      p_glVertex2f;
static tglColor4f       p_glColor4f;
static tglEnable        p_glEnable;
static tglDisable       p_glDisable;
static tglBlendFunc     p_glBlendFunc;
static tglMatrixMode    p_glMatrixMode;
static tglLoadIdentity  p_glLoadIdentity;
static tglOrtho         p_glOrtho;
static tglPushAttrib    p_glPushAttrib;
static tglPopAttrib     p_glPopAttrib;
static tglPushMatrix    p_glPushMatrix;
static tglPopMatrix     p_glPopMatrix;
static tglLineWidth     p_glLineWidth;
static tglGetIntegerv   p_glGetIntegerv;
static tglRasterPos2f   p_glRasterPos2f;
static tglListBase      p_glListBase;
static tglCallLists     p_glCallLists;
static tglGenLists      p_glGenLists;

static bool g_glInit = false;
static int  g_W = 0, g_H = 0;

static HWND g_hwnd = nullptr;
static WNDPROC oWndProc = nullptr;

typedef int(__cdecl* tSDL_SetRelativeMouseMode)(int);
static tSDL_SetRelativeMouseMode pSetRelativeMouseMode = nullptr;

LRESULT __stdcall hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;
    
    // Si el menu esta abierto, bloqueamos los clicks para que el juego no dispare de fondo
    if (g_menu && uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)
        return 1;
        
    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

static bool InitGL() {
    HMODULE h = GetModuleHandleA("OPENGL32.dll");
    if (!h) h = GetModuleHandleA("opengl32.dll");
    if (!h) return false;
    #define RES(fn) p_##fn = (t##fn)GetProcAddress(h, #fn); if(!p_##fn) return false
    RES(glBegin); RES(glEnd); RES(glVertex2f); RES(glColor4f);
    RES(glEnable); RES(glDisable); RES(glBlendFunc);
    RES(glMatrixMode); RES(glLoadIdentity); RES(glOrtho);
    RES(glPushAttrib); RES(glPopAttrib); RES(glPushMatrix); RES(glPopMatrix);
    RES(glLineWidth); RES(glGetIntegerv);
    RES(glRasterPos2f); RES(glListBase); RES(glCallLists); RES(glGenLists);
    #undef RES
    return g_glInit = true;
}

// ===== GL DRAW HELPERS =====
static void GL2D_Begin(int w, int h) {
    p_glPushAttrib(0x000FFFFF);
    p_glMatrixMode(0x1701); p_glPushMatrix(); p_glLoadIdentity();
    p_glOrtho(0, w, h, 0, -1, 1);
    p_glMatrixMode(0x1700); p_glPushMatrix(); p_glLoadIdentity();
    p_glDisable(0x0B71); // GL_DEPTH_TEST
    p_glEnable(0x0BE2);  // GL_BLEND
    p_glBlendFunc(0x0302, 0x0303); // SRC_ALPHA, ONE_MINUS_SRC_ALPHA
}
static void GL2D_End() {
    p_glMatrixMode(0x1700); p_glPopMatrix();
    p_glMatrixMode(0x1701); p_glPopMatrix();
    p_glPopAttrib();
}
static void FilledRect(float x, float y, float w, float h, float r, float g, float b, float a=0.8f) {
    p_glColor4f(r,g,b,a);
    p_glBegin(0x0007); // GL_QUADS
    p_glVertex2f(x,y); p_glVertex2f(x+w,y);
    p_glVertex2f(x+w,y+h); p_glVertex2f(x,y+h);
    p_glEnd();
}
static void OutlineRect(float x, float y, float w, float h, float r, float g, float b, float a=1.f) {
    p_glColor4f(r,g,b,a);
    p_glLineWidth(1.f);
    p_glBegin(0x0002); // GL_LINES
    p_glVertex2f(x,y);   p_glVertex2f(x+w,y);
    p_glVertex2f(x+w,y); p_glVertex2f(x+w,y+h);
    p_glVertex2f(x+w,y+h); p_glVertex2f(x,y+h);
    p_glVertex2f(x,y+h); p_glVertex2f(x,y);
    p_glEnd();
}
static void DrawLine(float x1, float y1, float x2, float y2, float r, float g, float b) {
    p_glColor4f(r,g,b,1.f);
    p_glLineWidth(1.f);
    p_glBegin(0x0001); // GL_LINES
    p_glVertex2f(x1,y1); p_glVertex2f(x2,y2);
    p_glEnd();
}

// ===== ACCESO A MEMORIA =====
static uintptr_t BASE = 0;

template<typename T>
static T Mem(uintptr_t addr) {
    return *reinterpret_cast<T*>(addr);
}

static Entity* GetLocalPlayer() {
    uintptr_t ptr = BASE + Offsets::dwLocalPlayer;
    if (IsBadReadPtr((void*)ptr, 4)) return nullptr;
    Entity* lp = Mem<Entity*>(ptr);
    if (IsBadReadPtr(lp, 0x310)) return nullptr;
    return lp;
}
static Entity* GetEntity(int i) {
    uintptr_t list = Mem<uintptr_t>(BASE + Offsets::dwEntityList);
    if (!list) return nullptr;
    if (IsBadReadPtr((void*)(list + i * 4), 4)) return nullptr;
    Entity* e = Mem<Entity*>(list + i * 4);
    if (!e || IsBadReadPtr(e, 0x310)) return nullptr;
    return e;
}
static int GetNumPlayers() {
    return Mem<int>(BASE + Offsets::dwNumPlayers);
}

// ===== MATH UTILS =====
static float Dist3D(Vec3 a, Vec3 b) {
    float dx=a.x-b.x, dy=a.y-b.y, dz=a.z-b.z;
    return sqrtf(dx*dx+dy*dy+dz*dz);
}
// WorldToScreen usando la ViewMatrix de OpenGL (perfecto para cualquier angulo)
static bool W2S(Vec3 pos, int sw, int sh, float& sx, float& sy) {
    float* matrix = (float*)(BASE + Offsets::dwViewMatrix);
    if (IsBadReadPtr(matrix, 16 * 4)) return false;
    
    float clipX = pos.x * matrix[0] + pos.y * matrix[4] + pos.z * matrix[8] + matrix[12];
    float clipY = pos.x * matrix[1] + pos.y * matrix[5] + pos.z * matrix[9] + matrix[13];
    float clipW = pos.x * matrix[3] + pos.y * matrix[7] + pos.z * matrix[11] + matrix[15];

    if (clipW < 0.2f) return false;

    float NDCx = clipX / clipW;
    float NDCy = clipY / clipW;

    sx = (sw / 2.0f) * (1.0f + NDCx);
    sy = (sh / 2.0f) * (1.0f - NDCy); // Y invertida en pantalla
    return true;
}

// ===== ESP =====
static void DrawESP(int sw, int sh) {
    Entity* lp = GetLocalPlayer();
    if (!lp || lp->health() <= 0) return;
    int count = GetNumPlayers();
    if (count <= 0 || count > 32) return;

    for (int i = 0; i < count; i++) {
        Entity* e = GetEntity(i);
        if (!e) continue;
        bool sameTeam = (e->team() == lp->team());
        if (g_teamCheck && sameTeam && e != lp) {
            // Si teamcheck esta activo y es de mi equipo
            if (e->health() <= 0 || e->health() > 200 || e->state() != 0) continue;
        } else {
            if (e == lp || e->health() <= 0 || e->health() > 200 || e->state() != 0) continue;
        }

        // Colors (si teamcheck esta apagado, todos son rojos = enemigos)
        bool enemy = !g_teamCheck || !sameTeam;
        float r = enemy ? 1.f : 0.f;
        float g = enemy ? 0.f : 1.f;
        float b = enemy ? 0.f : 1.f;

        // Posicion en pantalla
        float hx, hy;
        Vec3 headPos = e->pos(); headPos.z += 0.8f;
        if (!W2S(headPos, sw, sh, hx, hy)) continue;

        float bx, by;
        Vec3 botPos = e->pos(); botPos.z -= 2.4f;
        if (!W2S(botPos, sw, sh, bx, by)) continue;

        float boxH = by - hy;
        if (boxH < 2.f) continue;
        float boxW = boxH / 2.f;
        
        // Box outline
        OutlineRect(bx - boxW/2, hy, boxW, boxH, r, g, b, 0.8f);
        OutlineRect(bx - boxW/2 - 1, hy - 1, boxW + 2, boxH + 2, 0.f, 0.f, 0.f, 0.8f);
        OutlineRect(bx - boxW/2 + 1, hy + 1, boxW - 2, boxH - 2, 0.f, 0.f, 0.f, 0.8f);

        // Barra de vida (Health)
        float hpRatio = fminf(fmaxf((float)e->health() / 100.f, 0.f), 1.f);
        FilledRect(bx - boxW/2 - 6, hy, 4, boxH, 0.1f, 0.1f, 0.1f, 0.8f);
        FilledRect(bx - boxW/2 - 5, by - (boxH * hpRatio), 2, boxH * hpRatio, 1.f - hpRatio, hpRatio, 0.f);

        // Distancia como mini barra horizontal encima de la caja
        float dist = Dist3D(e->pos(), lp->pos());
        float distRatio = 1.f - fminf(dist / 200.f, 1.f); // 0-200m -> 1-0
        FilledRect(hx - boxW/2, hy - 5, boxW * distRatio, 3, r, g, b, 0.9f);
        OutlineRect(hx - boxW/2, hy - 5, boxW, 3, 0.f, 0.f, 0.f, 0.5f);
    }
}

// ===== AIMBOT =====
static void DoAimbot(decltype(&GetAsyncKeyState) pGAS) {
    if (!pGAS) return;
    Entity* lp = GetLocalPlayer();
    if (!lp || lp->health() <= 0) return;
    int count = GetNumPlayers();

    float bestFov = 999.f;
    int best = -1;
    float bestYaw = 0.f, bestPitch = 0.f;

    for (int i = 0; i < count; i++) {
        Entity* e = GetEntity(i);
        if (!e || e == lp || e->state() != 0) continue;
        if (g_teamCheck && e->team() == lp->team()) continue; // Team check
        if (e->health() <= 0 || e->health() > 200) continue;

        float dx = e->pos().x - lp->pos().x;
        float dy = e->pos().y - lp->pos().y;
        float dz = (e->pos().z + 1.0f) - (lp->pos().z + 1.0f);
        float dist = sqrtf(dx*dx + dy*dy + dz*dz);
        
        float yaw   = -(atan2f(dx, dy) * 180.f / 3.14159f) + 180.f; 
        float pitch = asinf(dz / dist) * 180.f / 3.14159f;

        float dyaw   = yaw - lp->yaw();
        float dpitch = pitch - lp->pitch();
        while (dyaw > 180.f) dyaw -= 360.f;
        while (dyaw < -180.f) dyaw += 360.f;

        float fov = sqrtf(dyaw*dyaw + dpitch*dpitch);
        if (fov < bestFov && fov < 25.f) { // Limitar el Aimbot a 25 grados (evita saltos a la pared)
            bestFov = fov;
            bestYaw = yaw;
            bestPitch = pitch;
            best = i;
        }
    }

    if (best != -1 && pGAS && (pGAS(VK_LBUTTON) & 0x8000)) {
        // SMOOTH AIMBOT (Calidad AAA)
        float smooth = 4.0f; // Mayor = mas suave, Menor = mas rapido
        float diffYaw = bestYaw - lp->yaw();
        float diffPitch = bestPitch - lp->pitch();
        
        // Normalizar diferencia para evitar giros largos
        while (diffYaw > 180.f) diffYaw -= 360.f;
        while (diffYaw < -180.f) diffYaw += 360.f;
        
        lp->yaw() += diffYaw / smooth;
        lp->pitch() += diffPitch / smooth;
    }
}

// ===== INF AMMO =====
static void DoInfAmmo() {
    // Offset 0x140 en AC 1.3.0.2 interfiere con el estado de la granada!
    // Para hacer InfAmmo real en 1.3.0.2 hay que inyectar codigo (NOP) en la funcion de disparo.
    // Desactivado temporalmente para evitar granadas fantasma.
}

// ===== NO RECOIL =====
static void DoNoRecoil(decltype(&GetAsyncKeyState) pGAS) {
    static float lastPitch = 0.f;
    Entity* lp = GetLocalPlayer();
    if (!lp) return;
    
    if (pGAS && (pGAS(VK_LBUTTON) & 0x8000)) {
        if (fabsf(lp->pitch() - lastPitch) > 0.1f && fabsf(lp->pitch() - lastPitch) < 20.f)
            lp->pitch() = lastPitch;
    } else {
        lastPitch = lp->pitch();
    }
}

// ===== MENU =====
static float g_smoothAim = 4.0f;
static float g_aimFov = 25.0f;

static void DrawMenu(int sw, int sh) {
    if (!g_menu) return;

    ImGui::SetNextWindowSize(ImVec2(350, 220), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("ENI AC CHEAT (Calidad AAA)", &g_menu, ImGuiWindowFlags_NoCollapse)) {
        ImGui::Checkbox("ESP Wallhack", &g_esp);
        ImGui::Checkbox("Aimbot (Click Izq)", &g_aimbot);
        if (g_aimbot) {
            ImGui::SliderFloat("Suavizado (Smooth)", &g_smoothAim, 1.0f, 10.0f);
            ImGui::SliderFloat("FOV (Grados)", &g_aimFov, 5.0f, 180.0f);
        }
        ImGui::Checkbox("No Recoil", &g_noRecoil);
        ImGui::Checkbox("Ignorar Compañeros (Team Check)", &g_teamCheck);
        
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Desactivado:");
        ImGui::Checkbox("Municion Infinita", &g_infAmmo); // Desactivado logicamente
        ImGui::End();
    }
}

// ===== HOOK SDL_GL_SwapWindow =====
typedef void(__cdecl* tSwap)(void*);
static tSwap oSwap = nullptr;

struct TrampolineHook {
    BYTE* src; BYTE* gw; size_t len;
    bool Install(BYTE* s, BYTE* d, size_t l) {
        src=s; len=l;
        gw=(BYTE*)VirtualAlloc(nullptr,l+5,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
        if(!gw) return false;
        memcpy(gw,s,l);
        uintptr_t back=(uintptr_t)(s+l)-(uintptr_t)(gw+l)-5;
        gw[l]=0xE9; *(uintptr_t*)(gw+l+1)=back;
        DWORD op; VirtualProtect(s,l,PAGE_EXECUTE_READWRITE,&op);
        memset(s,0x90,l);
        *(BYTE*)s=0xE9; *(uintptr_t*)(s+1)=(uintptr_t)d-(uintptr_t)s-5;
        VirtualProtect(s,l,op,&op);
        return true;
    }
    void Remove() {
        if(!src||!gw) return;
        DWORD op; VirtualProtect(src,len,PAGE_EXECUTE_READWRITE,&op);
        memcpy(src,gw,len); VirtualProtect(src,len,op,&op);
        VirtualFree(gw,0,MEM_RELEASE); gw=nullptr;
    }
} g_hook;

static decltype(&GetAsyncKeyState) pGAS = nullptr;
static volatile bool g_unloading = false;
static volatile bool g_unloaded = false;

void __cdecl hkSwap(void* win) {
    if (g_unloading) {
        if (!g_unloaded) {
            if (oWndProc && g_hwnd) {
                SetWindowLongPtr(g_hwnd, GWL_WNDPROC, (LONG_PTR)oWndProc);
                oWndProc = nullptr;
            }
            if (pSetRelativeMouseMode) pSetRelativeMouseMode(1);
            if (g_glInit) {
                ImGui_ImplOpenGL2_Shutdown();
                ImGui_ImplWin32_Shutdown();
                ImGui::DestroyContext();
                g_glInit = false;
            }
            g_unloaded = true;
        }
        if (oSwap) oSwap(win);
        return;
    }

    if (!g_glInit) {
        if (!InitGL()) return;

        typedef void(__cdecl* tGetWinSize)(void*, int*, int*);
        static tGetWinSize pGWS = nullptr;
        HMODULE sdl = GetModuleHandleA("SDL2.dll");
        if (sdl) {
            pGWS = (tGetWinSize)GetProcAddress(sdl, "SDL_GetWindowSize");
            pSetRelativeMouseMode = (tSDL_SetRelativeMouseMode)GetProcAddress(sdl, "SDL_SetRelativeMouseMode");
        }
        
        if (pGWS && win) pGWS(win, &g_W, &g_H);
        if (g_W <= 0) g_W = 800; if (g_H <= 0) g_H = 600;

        // Init ImGui
        g_hwnd = FindWindowA("SDL_app", nullptr);
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr; // no guardar config
        ImGui_ImplWin32_Init(g_hwnd);
        ImGui_ImplOpenGL2_Init();
        oWndProc = (WNDPROC)SetWindowLongPtr(g_hwnd, GWL_WNDPROC, (LONG_PTR)hkWndProc);
    }

    if (pGAS) {
        static bool menuKeyWasDown = false;
        bool menuKeyDown = (pGAS(KEY_MENU) & 0x8000) != 0;
        if (menuKeyDown && !menuKeyWasDown) {
            g_menu = !g_menu;
        }
        menuKeyWasDown = menuKeyDown;
        
        static bool last_menu = false;
        if (g_menu != last_menu) {
            if (pSetRelativeMouseMode) {
                pSetRelativeMouseMode(g_menu ? 0 : 1);
            }
            last_menu = g_menu;
        }
        
        static void* pShowCursor = nullptr;
        if (!pShowCursor) {
            HMODULE hSDL = GetModuleHandleA("SDL2.dll");
            if (hSDL) pShowCursor = (void*)GetProcAddress(hSDL, "SDL_ShowCursor");
        }
        
        if (pShowCursor) {
            auto ShowCursor = (int(__cdecl*)(int))pShowCursor;
            ShowCursor(g_menu ? 1 : 0);
        }

        ImGui::GetIO().MouseDrawCursor = g_menu;
        if (g_menu) {
            ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
        } else {
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        }

        if (g_aimbot) DoAimbot(pGAS);
        if (g_infAmmo) DoInfAmmo();
        if (g_noRecoil) DoNoRecoil(pGAS);
    }
    
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    
    DrawMenu(g_W, g_H);
    
    ImGui::Render();

    GL2D_Begin(g_W, g_H);
    if (g_esp) DrawESP(g_W, g_H);
    GL2D_End();
    
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    // Swap al final — presenta todo junto
    if (oSwap) oSwap(win);
}

// ===== MAIN THREAD =====
DWORD WINAPI MainThread(HMODULE hMod) {
    FILE* f; fopen_s(&f, "C:\\Users\\Usuario\\Documents\\C\\AC_CHEAT\\cheat_log.txt", "w");
    if(f){fprintf(f,"[+] AC Cheat cargando...\n");fflush(f);}

    BASE = (uintptr_t)GetModuleHandleA("ac_client.exe");
    if(f){fprintf(f,"[+] Base: %p\n",(void*)BASE);fflush(f);}

    HMODULE hSDL = nullptr;
    while(!(hSDL=GetModuleHandleA("SDL2.dll"))) Sleep(100);
    if(f){fprintf(f,"[+] SDL2 OK\n");fflush(f);}

    HMODULE hU = GetModuleHandleA("user32.dll");
    if(hU) pGAS = (decltype(pGAS))GetProcAddress(hU,"GetAsyncKeyState");

    uintptr_t fnSwap = (uintptr_t)GetProcAddress(hSDL, "SDL_GL_SwapWindow");
    if(!fnSwap){if(f){fprintf(f,"[-] SwapWindow no encontrado\n");fclose(f);}FreeLibraryAndExitThread(hMod,0);return 0;}
    if(f){fprintf(f,"[+] SwapWindow @ %p\n",(void*)fnSwap);fflush(f);}

    g_hook.Install((BYTE*)fnSwap,(BYTE*)hkSwap, 9);
    oSwap = (tSwap)g_hook.gw;
    if(f){fprintf(f,"[+] Hook OK | INSERT=Menu F1=ESP F2=Aimbot F3=NoRecoil F4=InfAmmo END=Salir\n");fclose(f);}

    // ===== DEBUG DUMP DETALLADO =====
    Sleep(3000); // esperar a que el juego cargue jugadores
    FILE* f2; fopen_s(&f2,"C:\\Users\\Usuario\\Documents\\C\\AC_CHEAT\\cheat_log_v2.txt","w");
    if(f2){
        uintptr_t listBase = Mem<uintptr_t>(BASE + Offsets::dwEntityList);
        fprintf(f2,"[DBG] dwEntityList VA    = %p\n", (void*)(BASE + Offsets::dwEntityList));
        fprintf(f2,"[DBG] EntityList data ptr = %p\n", (void*)listBase);
        int np = Mem<int>(BASE + Offsets::dwNumPlayers);
        fprintf(f2,"[DBG] dwNumPlayers = %d\n", np);

        Entity* lp = GetLocalPlayer();
        fprintf(f2,"[DBG] LocalPlayer (entity[0]) = %p\n", lp);
        if(lp){
            fprintf(f2,"[DBG] LP health=%d armour=%d\n", lp->health(), lp->armour());
            fprintf(f2,"[DBG] LP pos=%.2f,%.2f,%.2f pitch=%.2f yaw=%.2f\n",
                lp->pos().x, lp->pos().y, lp->pos().z, lp->pitch(), lp->yaw());
            fprintf(f2,"[DBG] LP name='%s' team=%d\n", lp->name(), lp->team());
            fprintf(f2,"[DBG] LP ammo=%d ammoRes=%d\n", lp->ammoMag(), lp->ammoRes());
        }
        // Dump raw bytes en offsets conocidos para verificar
        if(lp){
            fprintf(f2,"[DBG] LP raw bytes:\n");
            BYTE* raw = (BYTE*)lp;
            for(int o = 0; o <= 0x114; o+=4){
                int v = *(int*)(raw+o);
                if(v != 0) fprintf(f2,"  [%03X] = %d (0x%08X)\n", o, v, v);
            }
            fprintf(f2,"[DBG] Scanning LP for weapon pointers:\n");
            for (int o = 0x350; o <= 0x390; o += 4) {
                uintptr_t ptr = *(uintptr_t*)(raw + o);
                if (ptr > 0x400000 && ptr < 0x20000000) {
                    fprintf(f2, "  [%03X] = %p\n", o, (void*)ptr);
                }
            }
        }
        // Dump todos los jugadores
        if(listBase && np > 0 && np <= 32){
            for(int i=0;i<np;i++){
                Entity* e = Mem<Entity*>(listBase + i*4);
                if(e) fprintf(f2,"[DBG] entity[%d]=%p hp=%d pos=%.1f,%.1f team=%d name='%s'\n",
                    i, e, e->health(), e->pos().x, e->pos().y, e->team(), e->name());
                else  fprintf(f2,"[DBG] entity[%d]=NULL\n", i);
            }
        }
        fclose(f2);
    }


    while(!(pGAS && (pGAS(VK_END)&0x8000))) Sleep(50);
    g_unloading = true;
    while (!g_unloaded) Sleep(10);
    
    g_hook.Remove();
    Sleep(50);
    
    FreeLibraryAndExitThread(hMod, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hMod, DWORD reason, LPVOID) {
    if(reason==DLL_PROCESS_ATTACH){
        DisableThreadLibraryCalls(hMod);
        HANDLE h=CreateThread(nullptr,0,(LPTHREAD_START_ROUTINE)MainThread,hMod,0,nullptr);
        if(h) CloseHandle(h);
    }
    return TRUE;
}
