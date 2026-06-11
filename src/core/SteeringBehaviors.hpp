#ifndef STEERINGBEHAVIORS_HPP
#define STEERINGBEHAVIORS_HPP

#include <cmath>
#include <random>
#include <vector>

struct Vec2 {
    float x = 0.f, y = 0.f;

    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s)       const { return {x * s,   y * s};   }

    float length() const { return std::sqrt(x * x + y * y); }
    Vec2 normalized() const {
        float l = length();
        return l > 0.0001f ? Vec2{x / l, y / l} : Vec2{0.f, 0.f};
    }
};

namespace Steering {

inline Vec2 seek(Vec2 pos, Vec2 target, float maxSpeed) {
    return (target - pos).normalized() * maxSpeed;
}

inline Vec2 flee(Vec2 pos, Vec2 danger, float maxSpeed) {
    return (pos - danger).normalized() * maxSpeed;
}

inline Vec2 pursue(Vec2 pos, Vec2 preyPos, Vec2 preyVel, float maxSpeed) {
    float dist = (preyPos - pos).length();
    float fatorTempo = dist / maxSpeed;
    Vec2 futurePos = preyPos + preyVel * fatorTempo;
    return seek(pos, futurePos, maxSpeed);
}

inline Vec2 evade(Vec2 pos, Vec2 predPos, Vec2 predVel, float maxSpeed) {
    float dist = (predPos - pos).length();
    float fatorTempo = dist / maxSpeed;
    Vec2 futurePos = predPos + predVel * fatorTempo;
    return flee(pos, futurePos, maxSpeed);
}

inline Vec2 wander(Vec2 /*pos*/, Vec2 vel, float& wanderAngle,
                   float wanderRadius, float wanderDistance,
                   float wanderJitter, std::mt19937& rng) {
    std::uniform_real_distribution<float> jitterDist(-wanderJitter, wanderJitter);
    wanderAngle += jitterDist(rng);

    Vec2 circleCenter = vel.normalized() * wanderDistance;
    Vec2 displacement{
        wanderRadius * std::cos(wanderAngle),
        wanderRadius * std::sin(wanderAngle)
    };
    return (circleCenter + displacement).normalized();
}

struct FlockingResult {
    Vec2 separacao;
    Vec2 alinhamento;
    Vec2 coesao;
};

inline FlockingResult calcularFlocking(Vec2 pos, Vec2 /*vel*/,
                                        const std::vector<Vec2>& posVizinhos,
                                        const std::vector<Vec2>& velVizinhos,
                                        float raioSeparacao) {
    FlockingResult result;
    if (posVizinhos.empty()) return result;

    Vec2 sepForce{0.f, 0.f};
    int  sepCount = 0;
    Vec2 velSum{0.f, 0.f};
    Vec2 posSum{0.f, 0.f};

    for (std::size_t i = 0; i < posVizinhos.size(); ++i) {
        float dist = (posVizinhos[i] - pos).length();
        if (dist < raioSeparacao && dist > 0.0001f) {
            sepForce = sepForce + (pos - posVizinhos[i]).normalized();
            sepCount++;
        }
        velSum = velSum + velVizinhos[i];
        posSum = posSum + posVizinhos[i];
    }

    if (sepCount > 0)
        result.separacao = sepForce.normalized();

    result.alinhamento = velSum.normalized();

    Vec2 centroMassa = posSum * (1.f / static_cast<float>(posVizinhos.size()));
    result.coesao = seek(pos, centroMassa, 1.0f);

    return result;
}

} // namespace Steering

#endif // STEERINGBEHAVIORS_HPP
