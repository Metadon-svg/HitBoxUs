#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <fstream>
#include <sys/mman.h>
#include "Utils.h" // Твои утилиты для поиска базового адреса

// Указываем имя целевого модуля (например, "libblackrussia-client.so")
const char* libName = "libblackrussia-client.so"; 

// Путь к файлу настроек на внутренней памяти
const char* configPath = "/sdcard/Download/hitbox_config.txt"; 

// Твой целевой оффсет (убедись, что он актуален для новой библиотеки)
uintptr_t HEAD_OFFSET = 0x14247F8;

// Функция безопасной записи в защищенную память
void WriteFloat(uintptr_t address, float value) {
    size_t pageSize = sysconf(_SC_PAGESIZE);
    uintptr_t pageStart = address & ~(pageSize - 1);
    
    // Снимаем защиту страницы памяти (PROT_WRITE)
    mprotect((void*)pageStart, pageSize, PROT_READ | PROT_WRITE | PROT_EXEC);
    
    // Перезаписываем значение
    *(float*)address = value;
}

// Фоновый поток для отслеживания изменений в файле
void *monitor_thread(void *) {
    // Ждем, пока целевой модуль загрузится в память процесса
    while (!Utils::isLibraryLoaded(libName)) {
        sleep(1);
    }

    // Получаем базовый адрес загруженного модуля
    uintptr_t libBase = Utils::get_base_address(libName);
    float last_value = 1.0f; // Начальное значение для сравнения

    // Цикл мониторинга файла
    while (true) {
        std::ifstream configFile(configPath);
        
        if (configFile.is_open()) {
            float new_value;
            if (configFile >> new_value) {
                // Если значение в текстовом файле изменилось
                if (new_value != last_value) {
                    last_value = new_value;
                    
                    // Вычисляем точный адрес и пишем туда float
                    WriteFloat(libBase + HEAD_OFFSET, new_value);
                }
            }
            configFile.close();
        }
        
        // Интервал проверки файла (1 секунда), чтобы не перегружать CPU
        sleep(1); 
    }

    return nullptr;
}

// Инициализатор, срабатывающий при загрузке этой .so в процесс
__attribute__((constructor)) void init() {
    pthread_t thread;
    pthread_create(&thread, nullptr, monitor_thread, nullptr);
}
