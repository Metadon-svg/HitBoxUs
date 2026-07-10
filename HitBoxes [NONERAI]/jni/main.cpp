#include <iostream>
#include <cstdio>
#include <unistd.h> // Для usleep

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

// Путь во внутреннюю папку игры для стабильного чтения
const char* filePath = "/storage/emulated/0/Android/data/com.br.top/files/Hitbox_Size.txt";

float GetHitboxSize() {
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

    // Получаем базовый адрес ОДИН раз, чтобы не грузить проц в цикле
    uintptr_t libBase = getAbsoluteAddress(libName, 0);
    
    float lastValue = -1.0f; 
    int frameCounter = 0;

    while (true) {
        // Проверяем файл строго каждые 8 кадров
        if (frameCounter >= 8) {
            float MultiplyValue = GetHitboxSize();

            // Если значение в файле изменилось — обновляем адреса игры
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
                
                lastValue = MultiplyValue; 
            }
            
            frameCounter = 0; // Сбрасываем счетчик
        }

        frameCounter++;

        // Задержка ~16.6 миллисекунд (симуляция 60 кадров в секунду)
        usleep(16666); 
    }

    return nullptr;
}

__attribute__((constructor)) void _init(){
    pthread_t ptid;
    pthread_create(&ptid, NULL, main_thread, NULL);
}
