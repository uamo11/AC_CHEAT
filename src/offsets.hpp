// offsets.hpp — AssaultCube 1.3.0.2 Win32
// Verificados con pattern scan en runtime
#pragma once
#include <cstdint>

namespace Offsets {
    // ===== PUNTEROS GLOBALES =====
    constexpr uintptr_t dwLocalPlayer  = 0x18AC00; // *(entity**)  — jugador local
    constexpr uintptr_t dwEntityList   = 0x18AC04;  // *(entity**)[] — array de punteros
    constexpr uintptr_t dwNumPlayers   = 0x18AC0C;  // *(int*)       — jugadores en servidor
    constexpr uintptr_t dwViewMatrix   = 0x17DFD0;  // float[16]     — proyeccion OpenGL

    // ===== STRUCT entity (jugador) =====
    // AC 1.3.0.2 offsets
    constexpr uintptr_t o_posX     = 0x04;   // float X
    constexpr uintptr_t o_posY     = 0x08;   // float Y  
    constexpr uintptr_t o_posZ     = 0x0C;   // float Z
    constexpr uintptr_t o_yaw      = 0x34;   // float yaw (izquierda/derecha) en grados
    constexpr uintptr_t o_pitch    = 0x38;   // float pitch (arriba/abajo) en grados
    constexpr uintptr_t o_health   = 0xEC;   // int health (100) -> 1.3.0.2 verdadero
    constexpr uintptr_t o_armour   = 0xF0;   // int armadura
    constexpr uintptr_t o_name     = 0x205;  // char[16] nombre del jugador (1.3.0.2)
    constexpr uintptr_t o_team     = 0x30C;  // int team (0=CLA, 1=RVSF)
    constexpr uintptr_t o_ammo     = 0x140;  // int bala en cargador
    constexpr uintptr_t o_ammoRes  = 0x144;  // int reserva
}
