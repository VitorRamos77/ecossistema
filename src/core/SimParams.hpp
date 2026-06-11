#ifndef SIMPARAMS_HPP
#define SIMPARAMS_HPP

// Parâmetros ajustáveis em tempo de execução via sliders.
// WorldManager mantém uma instância desta struct.
// Os valores são multiplicadores — 1.0 = comportamento padrão.
struct SimParams {
    float taxaReproducaoPresa    = 1.0f;  // multiplicador cooldown (0.1 – 3.0)
    float taxaReproducaoPredador = 1.0f;  // multiplicador cooldown (0.1 – 3.0)
    float velocidadePresa        = 1.0f;  // multiplicador maxSpeed (0.1 – 3.0)
    float velocidadePredador     = 1.0f;  // multiplicador maxSpeed (0.1 – 3.0)
    float taxaSpawnPlanta        = 1.0f;  // multiplicador frequência spawn (0.1 – 5.0)
    float raioVisaoPresa         = 1.0f;  // multiplicador raioVisao (0.25 – 4.0)
    float raioVisaoPredador      = 1.0f;  // multiplicador raioVisao (0.25 – 4.0)
};

#endif // SIMPARAMS_HPP
