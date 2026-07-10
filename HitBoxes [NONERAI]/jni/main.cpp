#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <pthread.h>

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

// Пути к конфигурационным файлам
const string cmd_path = "/storage/emulated/0/Nonerai/Modules/control_signals/hitbox_cmd.txt";
const string cfg_path = "/storage/emulated/0/Nonerai/Modules/control_signals/cfg.txt";

// Функция для чтения команды (START/STOP)
string readCommand(const string& path) {
    ifstream file(path);
    if (!file.is_open()) return "STOP"; // Если файл не найден, по умолчанию выключаем
    string cmd;
    if (file >> cmd) {
        return cmd;
    }
    return "STOP";
}

// Функция для чтения множителя (1 - 5)
float readMultiplier(const string& path, float default_val) {
    ifstream file(path);
    if (!file.is_open()) return default_val;
    float val;
    if (file >> val) {
        return val;
    }
    return default_val;
}

void *main_thread(void *) {
    do { sleep(1); } while (!isLibraryLoaded(libName));

    // Получаем абсолютные адреса один раз при запуске для экономии ресурсов
    uintptr_t addr_HEAD = getAbsoluteAddress(libName, HEAD);
    uintptr_t addr_TORSO_1 = getAbsoluteAddress(libName, TORSO_1);
    uintptr_t addr_TORSO_2 = getAbsoluteAddress(libName, TORSO_2);
    uintptr_t addr_MID = getAbsoluteAddress(libName, MID);
    uintptr_t addr_LEFTARM = getAbsoluteAddress(libName, LEFTARM);
    uintptr_t addr_RIGHTARM = getAbsoluteAddress(libName, RIGHTARM);
    uintptr_t addr_LEFTLEG_1 = getAbsoluteAddress(libName, LEFTLEG_1);
    uintptr_t addr_RIGHTLEG_1 = getAbsoluteAddress(libName, RIGHTLEG_1);
    uintptr_t addr_LEFTLEG_2 = getAbsoluteAddress(libName, LEFTLEG_2);
    uintptr_t addr_RIGHTLEG_2 = getAbsoluteAddress(libName, RIGHTLEG_2);

    float last_applied_multiplier = -1.0f; // Хранит предыдущее примененное значение

    while (true) {
        // Задержка ~8 кадров при 60 FPS (133 000 микросекунд)
        usleep(133000);

        string cmd = readCommand(cmd_path);
        float target_multiplier = 1.0f; // По умолчанию стандартные хитбоксы (множитель 1)

        if (cmd == "START") {
            // Читаем значение из cfg.txt, если файла нет или ошибка — ставим дефолтное 4.0f
            target_multiplier = readMultiplier(cfg_path, 4.0f);
        } else {
            // Если STOP или файл недоступен — возвращаем стандартный размер (1.0f)
            target_multiplier = 1.0f;
        }

        // Записываем в память только если значение изменилось
        if (target_multiplier != last_applied_multiplier) {
            Utils::WriteMemory<float>(addr_HEAD, 0.15f * target_multiplier);
            Utils::WriteMemory<float>(addr_TORSO_1, 0.2f * target_multiplier);
            Utils::WriteMemory<float>(addr_TORSO_2, 0.25f * target_multiplier);
            Utils::WriteMemory<float>(addr_MID, 0.25f * target_multiplier);
            Utils::WriteMemory<float>(addr_LEFTARM, 0.25f * target_multiplier);
            Utils::WriteMemory<float>(addr_RIGHTARM, 0.16f * target_multiplier);
            Utils::WriteMemory<float>(addr_LEFTLEG_1, 0.15f * target_multiplier);
            Utils::WriteMemory<float>(addr_RIGHTLEG_1, 0.15f * target_multiplier);
            Utils::WriteMemory<float>(addr_LEFTLEG_2, 0.15f * target_multiplier);
            Utils::WriteMemory<float>(addr_RIGHTLEG_2, 0.15f * target_multiplier);

            last_applied_multiplier = target_multiplier;
        }
    }

    pthread_exit(nullptr);
    return nullptr;
}

__attribute__((constructor)) void _init(){
    pthread_t ptid;
    pthread_create(&ptid, NULL, main_thread, NULL);
}
