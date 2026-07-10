#include <iostream>
#include <fstream>      // И для чтения (ifstream), и для записи (ofstream)
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

float GetHitboxSize() {
    const char* filePath = "/storage/emulated/0/CONFIGHITBOX/Hitbox_Size.txt";
    
    // 1. Сначала принудительно открываем файл НА ЗАПИСЬ и фигачим туда 1
    ofstream fileOut(filePath);
    if (fileOut.is_open()) {
        fileOut << 1; // Записали единицу
        fileOut.close();
    }

    // 2. Теперь объявляем переменную без дефолтного значения 4.0f
    float size; 
    
    // 3. Открываем этот же файл НА ЧТЕНИЕ
    ifstream fileIn(filePath);
    if (fileIn.is_open()) {
        fileIn >> size; // Считали ту самую единицу
        fileIn.close();
    }
    
    return size;
}

void *main_thread(void *) {
    do { sleep(1); } while (!isLibraryLoaded(libName));

    // Сюда прилетит 1 из файла
    float MultiplyValue = GetHitboxSize();

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

    pthread_exit(nullptr);
    return nullptr;
}

__attribute__((constructor)) void _init(){
    pthread_t ptid;
    pthread_create(&ptid, NULL, main_thread, NULL);
}
