#include <iostream>
#include <fstream>
#include <unistd.h>

using namespace std;

#include "Utils.h"

// HIT BOXES (find string - PedModelInfo.cpp line: 85)
#if defined(__aarch64__)
    uintptr_t HEAD = 0x14247F8 ;
    uintptr_t TORSO_1 = HEAD + 0x20;
    uintptr_t TORSO_2 = TORSO_1 + 0x20;
    uintptr_t MID = TORSO_2 + 0x20;
    uintptr_t LEFTARM = MID + 0x20;
    uintptr_t RIGHTARM = LEFTARM + 0x20;
    uintptr_t LEFTLEG_1 = RIGHTARM + 0x20;
    uintptr_t RIGHTLEG_1 = LEFTLEG_1 + 0x20;
    uintptr_t LEFTLEG_2 = RIGHTLEG_1 + 0x20;
    uintptr_t RIGHTLEG_2 = LEFTLEG_2 + 0x20;
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
#endif

#define libName "libblackrussia-client.so"

// Функция чисто читает файл. Никаких четверок.
float GetHitboxSize() {
    const char* filePath = "/storage/emulated/0/CONFIGHITBOX/Hitbox_Size.txt";
    float size = 1.0f; // Если файл не прочитается, будет дефолтный хитбокс (умножение на 1)
    
    ifstream fileIn(filePath);
    if (fileIn.is_open()) {
        fileIn >> size;
        fileIn.close();
    }
    
    return size;
}

void *main_thread(void *) {
    // Ждем загрузки либки игры
    do { sleep(1); } while (!isLibraryLoaded(libName));

    // Бесконечный цикл для обновления каждую секунду
    while (true) {
        // Читаем значение из файла (актуальное на текущую секунду)
        float MultiplyValue = GetHitboxSize();

        // Записываем обновленное значение в память
        Utils::WriteMemory<float>(getAbsoluteAddress(libName, HEAD), 0.15f * MultiplyValue);
        Utils::WriteMemory<float>(getAbsoluteAddress(libName, TORSO_1), 0.2f * MultiplyValue);
        Utils::WriteMemory<float>(getAbsoluteAddress(libName, TORSO_2), 0.25f * MultiplyValue);
        Utils::WriteMemory<float>(getAbsoluteAddress(libName, MID), 0.25f * MultiplyValue);
        Utils::WriteMemory<float>(getAbsoluteAddress(libName, LEFTARM), 0.25f * MultiplyValue);
        Utils::WriteMemory<float>(getAbsoluteAddress(libName, RIGHTARM), 0.16f * MultiplyValue);
        Utils::WriteMemory<float>(getAbsoluteAddress(libName, LEFTLEG_1), 0.15f * MultiplyValue);
        Utils::WriteMemory<float>(getAbsoluteAddress(libName, RIGHTLEG_1), 0.15f * MultiplyValue);
        Utils::WriteMemory<float>(getAbsoluteAddress(libName, LEFTLEG_2), 0.15f * MultiplyValue);
        Utils::WriteMemory<float>(getAbsoluteAddress(libName, RIGHTLEG_2), 0.15f * MultiplyValue);

        // Спим 1 секунду перед следующим чтением файла
        sleep(1);
    }

    return nullptr;
}

__attribute__((constructor)) void _init(){
    pthread_t ptid;
    pthread_create(&ptid, NULL, main_thread, NULL);
}
