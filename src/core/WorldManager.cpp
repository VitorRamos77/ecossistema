#include "WorldManager.hpp"
#include "OrganismoFactory.hpp"
#include "../entities/Planta.hpp"
#include "../entities/Presa.hpp"
#include "../entities/Predador.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

WorldManager::WorldManager(int seed)
    : largura(0), altura(0), rng(seed), tickCounter(0), spawnPlantaCounter(0) {}

// ─── Grade espacial ──────────────────────────────────────────────────────────

void WorldManager::reconstruirGrade() {
    for (auto& celula : gradeEspacial) celula.clear();
    for (const auto& org : organismos) {
        if (!org->estaVivo()) continue;
        int ix = static_cast<int>(org->getX());
        int iy = static_cast<int>(org->getY());
        if (ix >= 0 && ix < largura && iy >= 0 && iy < altura)
            gradeEspacial[iy * largura + ix].push_back(org.get());
    }
}

// ─── Inicialização ───────────────────────────────────────────────────────────

void WorldManager::inicializar(int w, int h, int nPlantas, int nPresas, int nPredadores) {
    largura = w;
    altura  = h;
    organismos.clear();
    vivosSet.clear();
    gradeEspacial.assign(static_cast<std::size_t>(largura * altura), {});

    for (int i = 0; i < nPlantas; ++i) {
        auto org = OrganismoFactory::criarAleatorio(TipoOrganismo::Planta, largura, altura, *this);
        if (org) adicionarOrganismo(std::move(org));
    }
    for (int i = 0; i < nPresas; ++i) {
        auto org = OrganismoFactory::criarAleatorio(TipoOrganismo::Presa, largura, altura, *this);
        if (org) adicionarOrganismo(std::move(org));
    }
    for (int i = 0; i < nPredadores; ++i) {
        auto org = OrganismoFactory::criarAleatorio(TipoOrganismo::Predador, largura, altura, *this);
        if (org) adicionarOrganismo(std::move(org));
    }
}

void WorldManager::reiniciar(int nPlantas, int nPresas, int nPredadores) {
    organismos.clear();
    historicoPopulacao.clear();
    tickCounter        = 0;
    spawnPlantaCounter = 0;
    // params NÃO é resetado — sliders mantêm seus valores entre reinicializações
    inicializar(largura, altura, nPlantas, nPresas, nPredadores);
}

// ─── Tick principal ───────────────────────────────────────────────────────────

void WorldManager::tick() {
    tickCounter++;

    // Envelhecer todos
    for (auto& org : organismos) {
        if (org->estaVivo()) org->envelhecer();
    }

    // Agir e mover (interleaved por organismo — grade da iteração anterior é usada)
    for (auto& org : organismos) {
        if (org->estaVivo()) {
            org->agir(*this);
            org->mover(*this);
        }
    }

    // Rebuild grade com posições atualizadas — base para o eating O(n)
    reconstruirGrade();

    // Resolver interações (comer) — O(n): para cada organismo, checar vizinhança 3×3
    for (auto& org : organismos) {
        if (!org->estaVivo()) continue;
        TipoOrganismo tipoOrg = org->getTipo();
        if (tipoOrg == TipoOrganismo::Planta) continue; // plantas não comem

        int ix = static_cast<int>(org->getX());
        int iy = static_cast<int>(org->getY());

        for (int ndy = -1; ndy <= 1 && org->estaVivo(); ++ndy) {
            int cy = iy + ndy;
            if (cy < 0 || cy >= altura) continue;
            for (int ndx = -1; ndx <= 1 && org->estaVivo(); ++ndx) {
                int cx = ix + ndx;
                if (cx < 0 || cx >= largura) continue;
                for (Organismo* other : gradeEspacial[cy * largura + cx]) {
                    if (!other->estaVivo() || other == org.get()) continue;
                    float ddx = org->getX() - other->getX();
                    float ddy = org->getY() - other->getY();
                    if (ddx*ddx + ddy*ddy > 1.0f) continue;
                    TipoOrganismo tipoOther = other->getTipo();
                    if (tipoOrg == TipoOrganismo::Presa && tipoOther == TipoOrganismo::Planta) {
                        org->consumirEnergia(-other->getEnergiaFornecidaAoSerComido());
                        other->consumirEnergia(other->getEnergia());
                    } else if (tipoOrg == TipoOrganismo::Predador && tipoOther == TipoOrganismo::Presa) {
                        org->consumirEnergia(-other->getEnergiaFornecidaAoSerComido());
                        other->consumirEnergia(other->getEnergia());
                    }
                }
            }
        }
    }

    // Remover mortos (retirando do vivosSet antes de liberar a memória)
    for (const auto& org : organismos)
        if (!org->estaVivo()) vivosSet.erase(org.get());
    organismos.erase(
        std::remove_if(organismos.begin(), organismos.end(),
            [](const std::unique_ptr<Organismo>& o) { return !o->estaVivo(); }),
        organismos.end());

    // Rebuild grade após remoção — grade válida para reprodução/spreading
    reconstruirGrade();

    // Reprodução com exigência de proximidade
    // Aplicando params.taxaReproducaoPresa / taxaReproducaoPredador no cooldown
    std::vector<std::unique_ptr<Organismo>> novos;
    for (auto& org : organismos) {
        if (!org->estaVivo()) continue;
        if (org->getTipo() == TipoOrganismo::Planta) continue;
        if (org->getEnergia() < org->getEnergiaParaReproducao()) continue;
        if (org->getTempoParaReproducao() > 0) continue;

        float raio = org->getRaioReproducao();
        if (raio > 0.f) {
            auto vizinhos = obterVizinhosDeTipo(org->getX(), org->getY(), raio, org->getTipo(), org.get());
            if (vizinhos.empty()) continue;
        }

        int ix = static_cast<int>(org->getX());
        int iy = static_cast<int>(org->getY());
        std::vector<std::pair<int,int>> adj = {{ix-1,iy},{ix+1,iy},{ix,iy-1},{ix,iy+1}};
        for (auto& p : adj) {
            if (p.first < 0 || p.first >= largura || p.second < 0 || p.second >= altura) continue;
            if (celulaTaOcupada(p.first, p.second)) continue;

            novos.push_back(OrganismoFactory::criar(org->getTipo(), p.first, p.second));
            // Registra já na grade para que outro pai não escolha a mesma célula neste tick
            gradeEspacial[p.second * largura + p.first].push_back(novos.back().get());
            org->consumirEnergia(org->getCustoReproducao());

            // Aplicar multiplicador de reprodução ao cooldown
            int baseCooldown = org->getCooldownReproducao();
            float mult = (org->getTipo() == TipoOrganismo::Presa)
                         ? params.taxaReproducaoPresa
                         : params.taxaReproducaoPredador;
            int cooldownEfetivo = static_cast<int>(baseCooldown / mult);
            if (cooldownEfetivo < 1) cooldownEfetivo = 1;
            org->resetCooldown(cooldownEfetivo);
            break;
        }
    }
    // Já estão na grade; só transferir a posse (adicionarOrganismo duplicaria a entrada)
    for (auto& novo : novos) {
        vivosSet.insert(novo.get());
        organismos.push_back(std::move(novo));
    }

    // Contar plantas para impor limite global
    static constexpr int LIMITE_PLANTAS = 2000;
    int nPlantasAtual = 0;
    for (const auto& org : organismos)
        if (org->estaVivo() && org->getTipo() == TipoOrganismo::Planta) nPlantasAtual++;

    // Spawn de plantas — aplicando params.taxaSpawnPlanta
    spawnPlantaCounter++;
    int intervaloSpawn = static_cast<int>(5.f / params.taxaSpawnPlanta);
    if (intervaloSpawn < 1) intervaloSpawn = 1;
    if (spawnPlantaCounter >= intervaloSpawn && nPlantasAtual < LIMITE_PLANTAS) {
        spawnPlantaCounter = 0;
        std::uniform_int_distribution<int> distX(0, largura - 1);
        std::uniform_int_distribution<int> distY(0, altura  - 1);
        int attempts = 0;
        while (attempts < 100) {
            int nx = distX(rng);
            int ny = distY(rng);
            if (!celulaTaOcupada(nx, ny)) {
                adicionarOrganismo(OrganismoFactory::criar(TipoOrganismo::Planta, nx, ny));
                nPlantasAtual++;
                break;
            }
            attempts++;
        }
    }

    // Espalhamento orgânico de plantas: cria clusters naturais
    {
        static const int dx4[4] = {-1, 1, 0, 0};
        static const int dy4[4] = { 0, 0,-1, 1};
        std::uniform_int_distribution<int> dir4(0, 3);
        std::vector<std::unique_ptr<Organismo>> novasPlantas;

        for (auto& org : organismos) {
            if (nPlantasAtual + static_cast<int>(novasPlantas.size()) >= LIMITE_PLANTAS) break;
            if (!org->estaVivo() || org->getTipo() != TipoOrganismo::Planta) continue;
            auto* planta = static_cast<Planta*>(org.get());
            if (!planta->deveEspalhar()) continue;
            planta->resetarEspalhamento();

            int ix     = static_cast<int>(org->getX());
            int iy     = static_cast<int>(org->getY());
            int offset = dir4(rng);

            for (int t = 0; t < 4; ++t) {
                int d  = (offset + t) % 4;
                int nx = ix + dx4[d];
                int ny = iy + dy4[d];
                if (nx < 0 || nx >= largura || ny < 0 || ny >= altura) continue;
                if (celulaTaOcupada(nx, ny)) continue;

                // Controle de densidade: não cresce onde já é muito denso
                int vizPlanta = 0;
                for (int k = 0; k < 4; ++k) {
                    Organismo* v = obterOrganismoEm(nx + dx4[k], ny + dy4[k]);
                    if (v && v->getTipo() == TipoOrganismo::Planta) vizPlanta++;
                }
                if (vizPlanta >= 3) continue;

                novasPlantas.push_back(OrganismoFactory::criar(TipoOrganismo::Planta, nx, ny));
                gradeEspacial[ny * largura + nx].push_back(novasPlantas.back().get());
                break;
            }
        }
        for (auto& p : novasPlantas) {
            vivosSet.insert(p.get());
            organismos.push_back(std::move(p));
        }
    }

    registrarSnapshot();
}

void WorldManager::registrarSnapshot() {
    SnapshotPopulacao snap{tickCounter, 0, 0, 0};
    for (const auto& org : organismos) {
        if (!org->estaVivo()) continue;
        switch (org->getTipo()) {
            case TipoOrganismo::Planta:   ++snap.nPlantas;    break;
            case TipoOrganismo::Presa:    ++snap.nPresas;     break;
            case TipoOrganismo::Predador: ++snap.nPredadores; break;
        }
    }
    historicoPopulacao.push_back(snap);
    if (historicoPopulacao.size() > static_cast<std::size_t>(MAX_HISTORICO))
        historicoPopulacao.pop_front();
}

void WorldManager::renderizar() const {
    std::vector<std::string> grid(altura, std::string(largura, '.'));
    for (const auto& org : organismos) {
        if (org->estaVivo()) {
            int ix = static_cast<int>(org->getX());
            int iy = static_cast<int>(org->getY());
            if (ix >= 0 && ix < largura && iy >= 0 && iy < altura)
                grid[iy][ix] = org->getSimboloDesenho();
        }
    }
    for (const auto& row : grid) std::cout << row << '\n';
}

// ─── Gestão de organismos ─────────────────────────────────────────────────────

void WorldManager::adicionarOrganismo(std::unique_ptr<Organismo> org) {
    int ix = static_cast<int>(org->getX());
    int iy = static_cast<int>(org->getY());
    if (ix >= 0 && ix < largura && iy >= 0 && iy < altura)
        gradeEspacial[iy * largura + ix].push_back(org.get());
    vivosSet.insert(org.get());
    organismos.push_back(std::move(org));
}

// O(1): lê direto da grade
Organismo* WorldManager::obterOrganismoEm(int x, int y) const {
    if (x < 0 || x >= largura || y < 0 || y >= altura) return nullptr;
    for (auto* org : gradeEspacial[y * largura + x])
        if (org->estaVivo()) return org;
    return nullptr;
}

bool WorldManager::celulaTaOcupada(int x, int y) const {
    return obterOrganismoEm(x, y) != nullptr;
}

bool WorldManager::podeEntrarNaCelula(int x, int y, TipoOrganismo tipoEntrante) const {
    Organismo* existente = obterOrganismoEm(x, y);
    if (!existente) return true;
    if (tipoEntrante == TipoOrganismo::Presa   && existente->getTipo() == TipoOrganismo::Planta) return true;
    if (tipoEntrante == TipoOrganismo::Predador && existente->getTipo() == TipoOrganismo::Presa)  return true;
    return false;
}

std::mt19937& WorldManager::getRng() { return rng; }
int WorldManager::getLargura() const  { return largura; }
int WorldManager::getAltura()  const  { return altura;  }

// ─── Busca de vizinhos (grade espacial — O((2r+1)²) vs O(n)) ─────────────────

std::vector<Organismo*> WorldManager::obterVizinhos(float cx, float cy, float raio,
                                                     const Organismo* self) const {
    std::vector<Organismo*> result;
    float raio2 = raio * raio;
    int x0 = std::max(0, static_cast<int>(cx - raio));
    int x1 = std::min(largura - 1, static_cast<int>(cx + raio) + 1);
    int y0 = std::max(0, static_cast<int>(cy - raio));
    int y1 = std::min(altura  - 1, static_cast<int>(cy + raio) + 1);
    for (int iy = y0; iy <= y1; ++iy) {
        for (int ix = x0; ix <= x1; ++ix) {
            for (auto* org : gradeEspacial[iy * largura + ix]) {
                if (!org->estaVivo() || org == self) continue;
                float dx = org->getX() - cx;
                float dy = org->getY() - cy;
                if (dx*dx + dy*dy <= raio2) result.push_back(org);
            }
        }
    }
    return result;
}

std::vector<Organismo*> WorldManager::obterVizinhosDeTipo(float cx, float cy, float raio,
                                                           TipoOrganismo tipo,
                                                           const Organismo* self) const {
    std::vector<Organismo*> result;
    float raio2 = raio * raio;
    int x0 = std::max(0, static_cast<int>(cx - raio));
    int x1 = std::min(largura - 1, static_cast<int>(cx + raio) + 1);
    int y0 = std::max(0, static_cast<int>(cy - raio));
    int y1 = std::min(altura  - 1, static_cast<int>(cy + raio) + 1);
    for (int iy = y0; iy <= y1; ++iy) {
        for (int ix = x0; ix <= x1; ++ix) {
            for (auto* org : gradeEspacial[iy * largura + ix]) {
                if (!org->estaVivo() || org == self || org->getTipo() != tipo) continue;
                float dx = org->getX() - cx;
                float dy = org->getY() - cy;
                if (dx*dx + dy*dy <= raio2) result.push_back(org);
            }
        }
    }
    return result;
}

// Busca em dois tipos simultaneamente — economiza uma varredura da grade
void WorldManager::obterVizinhos2Tipos(float cx, float cy, float raio,
                                        TipoOrganismo t1, TipoOrganismo t2,
                                        const Organismo* self,
                                        std::vector<Organismo*>& out1,
                                        std::vector<Organismo*>& out2) const {
    float raio2 = raio * raio;
    int x0 = std::max(0, static_cast<int>(cx - raio));
    int x1 = std::min(largura - 1, static_cast<int>(cx + raio) + 1);
    int y0 = std::max(0, static_cast<int>(cy - raio));
    int y1 = std::min(altura  - 1, static_cast<int>(cy + raio) + 1);
    for (int iy = y0; iy <= y1; ++iy) {
        for (int ix = x0; ix <= x1; ++ix) {
            for (auto* org : gradeEspacial[iy * largura + ix]) {
                if (!org->estaVivo() || org == self) continue;
                float dx = org->getX() - cx;
                float dy = org->getY() - cy;
                if (dx*dx + dy*dy > raio2) continue;
                TipoOrganismo t = org->getTipo();
                if      (t == t1) out1.push_back(org);
                else if (t == t2) out2.push_back(org);
            }
        }
    }
}

bool WorldManager::organismoExisteEVivo(const Organismo* ptr) const {
    // O(1) — antes era varredura linear chamada por cada animal a cada tick
    return vivosSet.count(ptr) != 0 && ptr->estaVivo();
}

void WorldManager::getCounts(int& nPlantas, int& nPresas, int& nPredadores) const {
    if (historicoPopulacao.empty()) {
        nPlantas = nPresas = nPredadores = 0;
        return;
    }
    const auto& snap = historicoPopulacao.back();
    nPlantas    = snap.nPlantas;
    nPresas     = snap.nPresas;
    nPredadores = snap.nPredadores;
}
