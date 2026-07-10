#include <iostream>
#include <cstdio>       // Для fopen, fscanf, fclose (работает без кэша)
#include <unistd.h>     // Для usleep

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

// Надежное чтение напрямую с диска
float GetHitboxSize() {
    const char* filePath = "/storage/emulated/0/CONFIGHITBOX/Hitbox_Size.txt";
    float size = 1.0f; 
    
    FILE* file = fopen(filePath, "r");
    if (file) {
        fscanf(file, "%f", &size);
        fclose(file);
    }
    
    return size;
}

void *main_thread(void *) {
    // Ждем загрузки либки игры
    do { sleep(1); } while (!isLibraryLoaded(libName));

    // ИСПРАВЛЕНИЕ: Получаем базовый адрес ОДИН раз при старте
    uintptr_t libBase = getAbsoluteAddress(libName, 0);
    
    // Переменная для отслеживания изменений в файле
    float lastValue = -1.0f; 

    while (true) {
        // Читаем актуальное значение из txt файла
        float MultiplyValue = GetHitboxSize();

        // Если значение изменилось — только тогда обновляем память игры
        if (MultiplyValue != lastValue) {
            Utils::WriteMemory<float>(libBase + HEAD, 0.15f * MultiplyValue);
            Utils::WriteMemory<float>(libBase + TORSO_1, 0.2f * MultiplyValue);
            Utils::WriteMemory<float>(libBase + TORSO_2, 0.25f * MultiplyValue);
            Utils::WriteMemory<float>(libBase + MID, 0.25f * MultiplyValue);
            Utils::WriteMemory<float>(libBase + LEFTARM, 0.25f * MultiplyValue);
            Utils::WriteMemory<float>(libBase + RIGHTARM, 0.16f * MultiplyValue);
            Utils::WriteMemory<float>(libBase + LEFTLEG_1, 0.15f * MultiplyValue);
            Utils::WriteMemory<float>(libBase + RIGHTLEG_1, 0.15f * MultiplyValue);
            Utils::WriteMemory<float>(libBase + LEFTLEG_2, 0.15f * MultiplyValue);
            Utils::WriteMemory<float>(libBase + RIGHTLEG_2, 0.15f * MultiplyValue);
            
            // Запоминаем новое значение
            lastValue = MultiplyValue; 
        }

        // Проверяем файл каждые 500 миллисекунд (полсекунды)
        usleep(500000); 
    }

    return nullptr;
}

__attribute__((constructor)) void _init(){
    pthread_t ptid;
    pthread_create(&ptid, NULL, main_thread, NULL);
}
