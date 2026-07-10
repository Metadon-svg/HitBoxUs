#include <iostream>
#include <unistd.h>
using namespace std;

#include "Utils.h"

// HIT BOXES (find string - PedModelInfo.cpp line: 85)
#if defined(__aarch64__)
    uintptr_t HEAD = 0x14247F8;
    uintptr_t TORSO_1 = HEAD + 0x20;
    uintptr_t TORSO_2 = TORSO_1 + 0x20;
    uintptr_t MID = TORSO_2 + 0x20;
    uintptr_t LEFTARM = MID + 0x20;
    uintptr_t RIGHTARM = LEFTARM + 0x20;
    uintptr_t LEFTLEG_1 = RIGHTARM + 0x20;
    uintptr_t RIGHTLEG_1 = LEFTLEG_1 + 0x20;
    uintptr_t LEFTLEG_2 = RIGHTLEG_1 + 0x20;
    uintptr_t RIGHTLEG_2 = LEFTLEG_2 + 0x20;

    // РЕЗЕРВНЫЙ АДРЕС для множителя (куда Lua будет писать)
    // Берём адрес за пределами хитбоксов, но рядом
    uintptr_t MULTIPLIER_ADDR = 0x1424A00;
#else
    uintptr_t HEAD = 0xD31C88;
    uintptr_t TORSO_1 = HEAD + 0x18;
    uintptr_t TORSO_2 = TORSO_1 + 0x18;
    uintptr_t MID = TORSO_2 + 0x18;
    uintptr_t LEFTARM = MID + 0x18;
    uintptr_t RIGHTARM = LEFTARM + 0x18;
    uintptr_t LEFTLEG_1 = RIGHTARM + 0x18;
    uintptr_t RIGHTLEG_1 = LEFTLEG_1 + 0x18;
    uintptr_t LEFTLEG_2 = RIGHTLEG_1 + 0x18;
    uintptr_t RIGHTLEG_2 = LEFTLEG_2 + 0x18;

    uintptr_t MULTIPLIER_ADDR = 0xD32000;
#endif

#define libName "libblackrussia-client.so"

// Применяем хитбоксы с текущим множителем
void ApplyHitboxes(float mult) {
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, HEAD), 0.15f * mult);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, TORSO_1), 0.20f * mult);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, TORSO_2), 0.25f * mult);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, MID), 0.25f * mult);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, LEFTARM), 0.16f * mult);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, RIGHTARM), 0.16f * mult);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, LEFTLEG_1), 0.20f * mult);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, RIGHTLEG_1), 0.20f * mult);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, LEFTLEG_2), 0.15f * mult);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, RIGHTLEG_2), 0.15f * mult);
}

// Читаем множитель из резервного адреса
float ReadMultiplier() {
    return Utils::ReadMemory<float>(getAbsoluteAddress(libName, MULTIPLIER_ADDR));
}

// Пишем дефолтный множитель в резервный адрес
void InitMultiplier(float val) {
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, MULTIPLIER_ADDR), val);
}

void *main_thread(void *) {
    do { sleep(1); } while (!isLibraryLoaded(libName));

    // Инициализируем множитель x4
    InitMultiplier(4.0f);
    ApplyHitboxes(4.0f);

    float lastMultiplier = 4.0f;

    // Бесконечный цикл — проверяем множитель каждые 100мс
    while (true) {
        float currentMult = ReadMultiplier();

        // Если множитель изменился (Lua записал новое значение)
        if (currentMult != lastMultiplier && currentMult >= 0.1f && currentMult <= 50.0f) {
            ApplyHitboxes(currentMult);
            lastMultiplier = currentMult;
        }

        usleep(100000); // 100ms
    }

    pthread_exit(nullptr);
    return nullptr;
}

__attribute__((constructor)) void _init(){
    pthread_t ptid;
    pthread_create(&ptid, NULL, main_thread, NULL);
}
