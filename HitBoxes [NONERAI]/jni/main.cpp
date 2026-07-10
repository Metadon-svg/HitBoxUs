#include <iostream>
#include <string>
#include <pthread.h>
#include "Utils.h"

// HIT BOXES 
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

float currentMultiplier = 4.0f;

// Функция применения хитбоксов
void ApplyHitboxes(float mult) {
    currentMultiplier = mult;
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, HEAD), 0.15f * mult);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, TORSO_1), 0.2f * mult);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, TORSO_2), 0.25f * mult);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, MID), 0.25f * mult);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, LEFTARM), 0.25f * mult);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, RIGHTARM), 0.16f * mult);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, LEFTLEG_1), 0.15f * mult);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, RIGHTLEG_1), 0.15f * mult);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, LEFTLEG_2), 0.15f * mult);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, RIGHTLEG_2), 0.15f * mult);
}

// JNI sendChatMessage
typedef void (*SendChatFn)(JNIEnv* env, jobject thiz, jstring msg);
SendChatFn sendChat = nullptr;

// Отправка сообщения в чат
void SendChatMessage(JNIEnv* env, jobject thiz, const char* text) {
    if (sendChat) {
        jstring jmsg = env->NewStringUTF(text);
        sendChat(env, thiz, jmsg);
        env->DeleteLocalRef(jmsg);
    }
}

void *main_thread(void *) {
    do { sleep(1); } while (!isLibraryLoaded(libName));

    void* libBase = getAbsoluteAddress(libName, 0);
    sendChat = (SendChatFn)(libBase + 0x00ce4fe4);  // оффсет sendChatMessage

    ApplyHitboxes(currentMultiplier);  // начальное

    // TODO: Добавь hook на ввод чата здесь (Frida или native hook)
    // Пример: в хуке sendChatMessage проверяй ProcessChatCommand

    pthread_exit(nullptr);
    return nullptr;
}

// Обработчик команды
void ProcessChatCommand(JNIEnv* env, jobject thiz, const char* message) {
    std::string msg(message);
    if (msg.rfind("/sethb ", 0) == 0) {
        try {
            float newVal = std::stof(msg.substr(7));
            if (newVal > 0 && newVal <= 20) {  // защита
                ApplyHitboxes(newVal);
                char buf[128];
                snprintf(buf, sizeof(buf), "Хитбокс увеличен в %.0fx", newVal);
                SendChatMessage(env, thiz, buf);  // зелёное сообщение (цвет зависит от игры)
            }
        } catch (...) {}
    }
}

__attribute__((constructor)) void _init(){
    pthread_t ptid;
    pthread_create(&ptid, NULL, main_thread, NULL);
}
