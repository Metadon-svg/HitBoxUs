#include <jni.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <android/log.h>

#define LOG_TAG "HBHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#define LIB_NAME "libblackrussia-client.so"

// Оригинальные байты пролога (16 байт)
unsigned char orig_bytes[16];

// Адреса
uintptr_t sendChatMessage_addr = 0x00ce4fe4;
uintptr_t cave_addr = 0x01326c00;

// Хитбокс оффсеты от базы lib
uintptr_t HEAD_OFF = 0x14247F8;
uintptr_t TORSO_1_OFF = HEAD_OFF + 0x20;
uintptr_t TORSO_2_OFF = TORSO_1_OFF + 0x20;
uintptr_t MID_OFF = TORSO_2_OFF + 0x20;
uintptr_t LEFTARM_OFF = MID_OFF + 0x20;
uintptr_t RIGHTARM_OFF = LEFTARM_OFF + 0x20;
uintptr_t LEFTLEG_1_OFF = RIGHTARM_OFF + 0x20;
uintptr_t RIGHTLEG_1_OFF = LEFTLEG_1_OFF + 0x20;
uintptr_t LEFTLEG_2_OFF = RIGHTLEG_1_OFF + 0x20;
uintptr_t RIGHTLEG_2_OFF = LEFTLEG_2_OFF + 0x20;

// База библиотеки в памяти
uintptr_t lib_base = 0;

// Оригинальная функция
void (*orig_sendChatMessage)(JNIEnv*, jobject, jstring);

// Проверка префикса
bool starts_with(const char* str, const char* prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

// Запись хитбоксов
void write_hitboxes(float mult) {
    if (!lib_base) return;
    
    float* head = (float*)(lib_base + HEAD_OFF);
    float* torso1 = (float*)(lib_base + TORSO_1_OFF);
    float* torso2 = (float*)(lib_base + TORSO_2_OFF);
    float* mid = (float*)(lib_base + MID_OFF);
    float* leftarm = (float*)(lib_base + LEFTARM_OFF);
    float* rightarm = (float*)(lib_base + RIGHTARM_OFF);
    float* leftleg1 = (float*)(lib_base + LEFTLEG_1_OFF);
    float* rightleg1 = (float*)(lib_base + RIGHTLEG_1_OFF);
    float* leftleg2 = (float*)(lib_base + LEFTLEG_2_OFF);
    float* rightleg2 = (float*)(lib_base + RIGHTLEG_2_OFF);
    
    *head = 0.15f * mult;
    *torso1 = 0.2f * mult;
    *torso2 = 0.25f * mult;
    *mid = 0.25f * mult;
    *leftarm = 0.25f * mult;
    *rightarm = 0.16f * mult;
    *leftleg1 = 0.15f * mult;
    *rightleg1 = 0.15f * mult;
    *leftleg2 = 0.15f * mult;
    *rightleg2 = 0.15f * mult;
    
    LOGI("Hitboxes set to multiplier: %.2f", mult);
}

// Наш хук
void hooked_sendChatMessage(JNIEnv* env, jobject thiz, jstring msg) {
    if (msg) {
        const char* cstr = env->GetStringUTFChars(msg, nullptr);
        if (cstr) {
            if (starts_with(cstr, "/sethb ")) {
                float mult = strtof(cstr + 7, nullptr);
                if (mult > 0) {
                    write_hitboxes(mult);
                }
                env->ReleaseStringUTFChars(msg, cstr);
                // Блокируем отправку — возвращаемся без вызова оригинала
                return;
            }
            env->ReleaseStringUTFChars(msg, cstr);
        }
    }
    // Вызываем оригинал
    orig_sendChatMessage(env, thiz, msg);
}

// Утилита: сделать память RWX
void set_rwx(void* addr, size_t len) {
    uintptr_t page_start = (uintptr_t)addr & ~0xFFF;
    size_t page_len = ((uintptr_t)addr + len - page_start + 0xFFF) & ~0xFFF;
    mprotect((void*)page_start, page_len, PROT_READ | PROT_WRITE | PROT_EXEC);
}

// Код в cave (ARM64 asm)
// Сохраняем регистры, вызываем hooked_sendChatMessage, восстанавливаем, возвращаемся
// Размер: ~80 байт
__attribute__((naked)) void cave_trampoline() {
    __asm__ volatile (
        // Сохраняем x0-x3, x30 (нужны для вызова C-функции)
        "sub sp, sp, #0x40\n"
        "stp x0, x1, [sp, #0x00]\n"
        "stp x2, x3, [sp, #0x10]\n"
        "stp x29, x30, [sp, #0x20]\n"
        "str x4, [sp, #0x30]\n"
        
        // Вызываем hooked_sendChatMessage(x0=env, x1=thiz, x2=msg)
        // Аргументы уже в x0, x1, x2
        "bl hooked_sendChatMessage\n"
        
        // Восстанавливаем
        "ldr x4, [sp, #0x30]\n"
        "ldp x29, x30, [sp, #0x20]\n"
        "ldp x2, x3, [sp, #0x10]\n"
        "ldp x0, x1, [sp, #0x00]\n"
        "add sp, sp, #0x40\n"
        
        // Выполняем оригинальные 16 байт пролога
        "sub sp, sp, #0x60\n"
        "stp x29, x30, [sp, #0x20]\n"
        "stp x24, x23, [sp, #0x30]\n"
        "stp x22, x21, [sp, #0x40]\n"
        
        // Возврат в оригинал + 16
        "ldr x16, =0x00ce4ff4\n"
        "br x16\n"
    );
}

// Установка hook'а
void install_hook() {
    // Находим базу библиотеки
    Dl_info info;
    if (dladdr((void*)sendChatMessage_addr, &info) == 0) {
        LOGI("Failed to get base address");
        return;
    }
    lib_base = (uintptr_t)info.dli_fbase;
    LOGI("Library base: 0x%lx", lib_base);
    
    // Абсолютные адреса
    void* target = (void*)(lib_base + sendChatMessage_addr);
    void* cave = (void*)(lib_base + cave_addr);
    
    LOGI("Target: %p, Cave: %p", target, cave);
    
    // Делаем RWX
    set_rwx(target, 16);
    set_rwx(cave, 256);
    
    // Копируем оригинальные байты
    memcpy(orig_bytes, target, 16);
    
    // Пишем trampoline в cave
    // Копируем код cave_trampoline в cave
    // НО: naked функция не даёт взять адрес напрямую, поэтому пишем байты вручную
    
    // Проще: скомпилируем asm отдельно или используем готовые байты
    // Вместо naked — пишем байты напрямую
    
    // Патч: adrp x16, page; add x16, x16, offset; br x16
    // Вычисляем относительный адрес cave от target
    
    intptr_t delta = (intptr_t)cave - (intptr_t)target;
    LOGI("Delta: 0x%lx", delta);
    
    // adrp x16, imm21 (delta >> 12)
    // add x16, x16, (delta & 0xFFF)
    // br x16
    
    // Пишем байты вручную через uint32_t
    uint32_t* patch = (uint32_t*)target;
    
    // adrp x16, delta >> 12
    int64_t imm = delta >> 12;
    uint32_t adrp = 0x90000010 | ((imm & 3) << 29) | (((imm >> 2) & 0x7FFFF) << 5);
    patch[0] = adrp;
    
    // add x16, x16, delta & 0xFFF
    uint32_t add = 0x91000010 | ((delta & 0xFFF) << 10);
    patch[1] = add;
    
    // br x16
    patch[2] = 0xD61F0200;
    
    // nop (4 байта заполнения)
    patch[3] = 0xD503201F;
    
    LOGI("Hook installed");
}

// Конструктор — вызывается при загрузке .so
__attribute__((constructor)) void _init() {
    LOGI("Module loaded, installing hook...");
    
    // Ждём загрузку целевой библиотеки
    // Наша .so должна загружаться ПОСЛЕ libblackrussia-client.so
    // Или использовать dlopen
    
    void* handle = dlopen(LIB_NAME, RTLD_NOW);
    if (!handle) {
        LOGI("Failed to open %s", LIB_NAME);
        return;
    }
    
    install_hook();
}
