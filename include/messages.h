#ifndef MESSAGES_H
#define MESSAGES_H

#include <Arduino.h>

// Good morning messages - one for each day
// Add or modify messages as you like!
const char *const MORNING_MESSAGES[] = {
    "Bom dia meu docinho de coco!", "Bodia pudizinho sem lactose!",
    "Bodia bananinha amassada!",    "Bodia meu rodinho de pia!",
    "Bodia trem mais lindo!",       "Bom dia flor do dia!",
    "Bodia meu pao de queijo!",     "Bodia minha estrelinha!",
    "Bodia raio de sol!",           "Bodia meu churrasquinho!",
    "Bodia minha vitamina!",        "Bodia brigadeiro gourmet!",
    "Bodia meu cafezinho!",         "Bodia bolinho de chuva!",
    "Bodia meu bem mais precioso!", "Bodia coxinha do meu coracao",
    "Bodia minha pipoquinha!",      "Bodia acucar do meu cafe!",
    "Bodia meu solzinho!",          "Bodia minha princesa!",
    "Bodia amor da minha vida!",    "Bodia meu sorrisinho!",
    "Bodia meu pedacinho de céu!",  "Bodia minha felicidade!",
    "Bodia meu suspirinho!",        "Bodia minha jabuticaba!",
    "Bodia meu amendoim doce!",     "Bodia minha tapioquinha!",
    "Bodia meu coracaozinho!"};

const int NUM_MORNING_MESSAGES =
    sizeof(MORNING_MESSAGES) / sizeof(MORNING_MESSAGES[0]);

inline const char *getMorningMessage(int dayOfYear) {
  return MORNING_MESSAGES[dayOfYear % NUM_MORNING_MESSAGES];
}

#endif // MESSAGES_H
