#ifndef WORLDMANAGER_HPP
#define WORLDMANAGER_HPP

#include "Organismo.hpp"
#include "SimParams.hpp"
#include <vector>
#include <deque>
#include <memory>
#include <random>
#include <unordered_set>

// Snapshot de populações gravado a cada tick para o gráfico
struct SnapshotPopulacao {
    int tick;
    int nPlantas;
    int nPresas;
    int nPredadores;
};

class WorldManager {
public:
    // Parâmetros ajustáveis em tempo real pelos sliders
    SimParams params;

private:
    int largura, altura;
    std::vector<std::unique_ptr<Organismo>> organismos;
    // Grade espacial flat [y*largura + x] para lookups O(1) e eating O(n)
    std::vector<std::vector<Organismo*>> gradeEspacial;
    // Conjunto de ponteiros válidos — torna organismoExisteEVivo O(1)
    std::unordered_set<const Organismo*> vivosSet;
    std::mt19937 rng;
    int tickCounter;
    int spawnPlantaCounter;

    static constexpr int MAX_HISTORICO = 500;
    std::deque<SnapshotPopulacao> historicoPopulacao;

    void registrarSnapshot();
    void reconstruirGrade();

public:
    WorldManager(int seed = std::random_device{}());
    void inicializar(int largura, int altura, int nPlantas, int nPresas, int nPredadores);
    void reiniciar(int nPlantas, int nPresas, int nPredadores);
    void tick();
    void renderizar() const;
    void adicionarOrganismo(std::unique_ptr<Organismo> org);
    Organismo* obterOrganismoEm(int x, int y) const;
    bool celulaTaOcupada(int x, int y) const;
    bool podeEntrarNaCelula(int x, int y, TipoOrganismo tipoEntrante) const;
    std::mt19937& getRng();
    int getLargura() const;
    int getAltura()  const;
    int getTickAtual() const { return tickCounter; }

    const std::vector<std::unique_ptr<Organismo>>& getOrganismos() const { return organismos; }
    const std::deque<SnapshotPopulacao>& getHistorico() const { return historicoPopulacao; }

    // Vizinhança por raio euclidiano (usa grade espacial)
    std::vector<Organismo*> obterVizinhos(float cx, float cy, float raio,
                                          const Organismo* self = nullptr) const;
    std::vector<Organismo*> obterVizinhosDeTipo(float cx, float cy, float raio,
                                                 TipoOrganismo tipo,
                                                 const Organismo* self = nullptr) const;
    // Busca combinada em dois tipos numa única varredura da grade
    void obterVizinhos2Tipos(float cx, float cy, float raio,
                              TipoOrganismo t1, TipoOrganismo t2,
                              const Organismo* self,
                              std::vector<Organismo*>& out1,
                              std::vector<Organismo*>& out2) const;

    bool organismoExisteEVivo(const Organismo* ptr) const;

    // Contagens instantâneas a partir do último snapshot (O(1))
    void getCounts(int& nPlantas, int& nPresas, int& nPredadores) const;
};

#endif // WORLDMANAGER_HPP
