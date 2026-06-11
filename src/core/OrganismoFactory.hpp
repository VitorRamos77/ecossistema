#ifndef ORGANISMOFACTORY_HPP
#define ORGANISMOFACTORY_HPP

#include "Organismo.hpp"
#include <memory>

class OrganismoFactory {
public:
    static std::unique_ptr<Organismo> criar(TipoOrganismo tipo, float x, float y);
    static std::unique_ptr<Organismo> criarAleatorio(TipoOrganismo tipo, int largura, int altura, WorldManager& world);
};

#endif // ORGANISMOFACTORY_HPP