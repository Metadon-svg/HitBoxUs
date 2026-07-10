#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <pthread.h>

// Подключаем хукер (например, Dobby)
#include "dobby.h" 
#include "Utils.h"

#define libName "libblackrussia-client.so"
const char* filePath = "/storage/emulated/0/Android/data/com.br.top/files/Hitbox_Size.txt";

// Переменная, где ВСЕГДА лежит актуальный размер из файла
float globalHitboxSize = 1.0f;

// Смещение функции обновления кастомных калибров/костей педа (примерное, зависит от версии)
// Тебе нужно будет найти точный оффсет функции типа SetupBones или СPed::Update в IDA/Gidra
uintptr_t fnUpdateScaleOffset = 0x3A4B5C; 

// Указатель на оригинальную функцию игры
void (*orig_UpdateScale)(void* instance, float x, float y, float z);

// Наша кастомная функция, которая подменяет размеры
void hook_UpdateScale(void* instance, float x, float y, float z) {
    // Вместо оригинальных x, y, z подставляем умноженные на наш globalHitboxSize
    float newX = x * globalHitboxSize;
    float newY = y * globalHitboxSize;
    float newZ = z * globalHitboxSize;

    // Вызываем оригинальную функцию уже с нашими «раздутыми» размерами
    orig_UpdateScale(instance, newX, newY, newZ);
}

// Поток, который просто спокойно читает файл раз в полсекунды
void *file_reader_thread(void *) {
    while (true) {
        float size = 1.0f;
        FILE* file = fopen(filePath, "r");
        if (file) {
            fscanf(file, "%f", &size);
            fclose(file);
        }
        // Обновляем глобальную переменную — хук сразу подхватит её в следующем кадре
        globalHitboxSize = size; 
        
        usleep(500000); // 0.5 сек задержка
    }
    return nullptr;
}

void *main_thread(void *) {
    do { sleep(1); } while (!isLibraryLoaded(libName));

    uintptr_t libBase = getAbsoluteAddress(libName, 0);

    // Ставим хук на функцию отрисовки/обновления масштаба костей
    DobbyHook((void*)(libBase + fnUpdateScaleOffset), (void*)hook_UpdateScale, (void**)&orig_UpdateScale);

    // Запускаем фоновый поток для чтения текстового файла
    pthread_t threadId;
    pthread_create(&threadId, NULL, file_reader_thread, NULL);

    return nullptr;
}

__attribute__((constructor)) void _init(){
    pthread_t ptid;
    pthread_create(&ptid, NULL, main_thread, NULL);
}
