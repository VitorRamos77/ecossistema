#include "OrganismoFactory.hpp"
#include "../entities/Planta.hpp"
#include "../entities/Presa.hpp"
#include "../entities/Predador.hpp"
#include "WorldManager.hpp"
#include <random>

std::unique_ptr<Organismo> OrganismoFactory::criar(TipoOrganismo tipo, float x, float y) {
    switch (tipo) {
        case TipoOrganismo::Planta:
            return std::make_unique<Planta>(x, y);
        case TipoOrganismo::Presa:
            return std::make_unique<Presa>(x, y);
        case TipoOrganismo::Predador:
            return std::make_unique<Predador>(x, y);
        default:
            return nullptr;
    }
}

std::unique_ptr<Organismo> OrganismoFactory::criarAleatorio(TipoOrganismo tipo, int largura, int altura, WorldManager& world) {
    std::uniform_int_distribution<int> distX(0, largura - 1);
    std::uniform_int_distribution<int> distY(0, altura - 1);
    int attempts = 0;
    while (attempts < 100) {
        int x = distX(world.getRng());
        int y = distY(world.getRng());
        if (!world.celulaTaOcupada(x, y)) {
            return criar(tipo, x, y);
        }
        attempts++;
    }
    return nullptr;
}