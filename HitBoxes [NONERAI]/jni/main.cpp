#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <fstream>
#include <sys/mman.h>
#include "Utils.h" // Твой заголовочный файл с утилитами

// Целевая библиотека Black Russia
const char* libName = "libblackrussia-client.so"; 

// Путь к конфигурационному файлу
const char* configPath = "/sdcard/Download/hitbox_config.txt"; 

// --- ХИТБОКСЫ (ARM64-v8a) ---
uintptr_t HEAD        = 0x14247F8;
uintptr_t TORSO_1     = HEAD + 0x20;
uintptr_t TORSO_2     = TORSO_1 + 0x20;
uintptr_t MID         = TORSO_2 + 0x20;
uintptr_t LEFTARM     = MID + 0x20;
uintptr_t RIGHTARM    = LEFTARM + 0x20;
uintptr_t LEFTLEG_1   = RIGHTARM + 0x20;
uintptr_t RIGHTLEG_1  = LEFTLEG_1 + 0x20;
uintptr_t LEFTLEG_2   = RIGHTLEG_1 + 0x20;
uintptr_t RIGHTLEG_2  = LEFTLEG_2 + 0x20;

// Функция безопасной записи во Float-память с обходом защиты
void WriteFloat(uintptr_t address, float value) {
    size_t pageSize = sysconf(_SC_PAGESIZE);
    uintptr_t pageStart = address & ~(pageSize - 1);

    // Снимаем защиту страницы на чтение/запись/выполнение
    mprotect((void*)pageStart, pageSize, PROT_READ | PROT_WRITE | PROT_EXEC);

    // Записываем новое значение
    *(float*)address = value;
}

// Фоновый поток для мониторинга текстового файла
void *monitor_thread(void *) {
    // Ждем загрузки либы в память (вызов глобальной функции без Utils::)
    while (!isLibraryLoaded(libName)) {
        sleep(1);
    }

    // Получаем базовый адрес (вызов глобальной функции без Utils::)
    // ПРИМЕЧАНИЕ: Если компилятор выдаст ошибку "undeclared identifier" на get_base_address,
    // замени её на: uintptr_t libBase = KittyMemory::getMemoryBase(libName);
    uintptr_t libBase = get_base_address(libName);
    
    float last_value = 1.0f; // Начальное дефолтное значение

    // Бесконечный цикл проверки файла раз в секунду
    while (true) {
        std::ifstream configFile(configPath);
        
        if (configFile.is_open()) {
            float new_value;
            if (configFile >> new_value) {
                // Если значение в файле изменилось, применяем ко всем хитбоксам
                if (new_value != last_value) {
                    last_value = new_value;
                    
                    // Обновляем всю структуру хитбоксов «на лету»
                    WriteFloat(libBase + HEAD,       new_value);
                    WriteFloat(libBase + TORSO_1,   new_value);
                    WriteFloat(libBase + TORSO_2,   new_value);
                    WriteFloat(libBase + MID,       new_value);
                    WriteFloat(libBase + LEFTARM,   new_value);
                    WriteFloat(libBase + RIGHTARM,  new_value);
                    WriteFloat(libBase + LEFTLEG_1, new_value);
                    WriteFloat(libBase + RIGHTLEG_1,new_value);
                    WriteFloat(libBase + LEFTLEG_2, new_value);
                    WriteFloat(libBase + RIGHTLEG_2,new_value);
                }
            }
            configFile.close();
        }
        
        // Отдыхаем 1 секунду, чтобы не нагружать процессор
        sleep(1); 
    }

    return nullptr;
}

// Инициализатор, который срабатывает сразу при инжекте нашей .so в игру
__attribute__((constructor)) void init() {
    pthread_t thread;
    pthread_create(&thread, nullptr, monitor_thread, nullptr);
}
