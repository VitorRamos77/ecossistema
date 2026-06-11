#include "Organismo.hpp"

Organismo::Organismo(float x, float y, float energiaInicial, int cooldownReproducao)
    : x(x), y(y), energia(energiaInicial), idade(0), tempoParaReproducao(cooldownReproducao), vivo(true) {}

bool Organismo::estaVivo() const {
    return vivo;
}

float Organismo::getX() const {
    return x;
}

float Organismo::getY() const {
    return y;
}

float Organismo::getEnergia() const {
    return energia;
}

int Organismo::getTempoParaReproducao() const {
    return tempoParaReproducao;
}

void Organismo::envelhecer() {
    idade++;
    if (tempoParaReproducao > 0) {
        tempoParaReproducao--;
    }
}

void Organismo::consumirEnergia(float quantidade) {
    energia -= quantidade;
    if (energia <= 0) {
        energia = 0;
        vivo = false;
    }
}

void Organismo::resetCooldown(int value) {
    tempoParaReproducao = value;
}