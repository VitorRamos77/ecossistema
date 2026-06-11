#ifndef UI_HPP
#define UI_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <sstream>
#include <iomanip>

// ─────────────────────────────────────────────────────────────────────────────
// Botao — widget simples com hover e estado toggle
// ─────────────────────────────────────────────────────────────────────────────
struct Botao {
    sf::RectangleShape shape;
    sf::Text           label;
    sf::Color          corNormal = sf::Color(50,  50,  65);
    sf::Color          corHover  = sf::Color(70,  70,  90);
    sf::Color          corAtivo  = sf::Color(80, 120, 180);
    bool               ativo     = false;

    void init(const sf::Font& f, const std::string& txt,
              float x, float y, float w, float h) {
        shape.setSize(sf::Vector2f(w, h));
        shape.setOutlineThickness(1.f);
        shape.setOutlineColor(sf::Color(80, 80, 100));

        label.setFont(f);
        label.setString(txt);
        label.setCharacterSize(12);
        label.setFillColor(sf::Color(220, 220, 220));

        setPosition(x, y);
    }

    void setPosition(float x, float y) {
        shape.setPosition(x, y);
        centralizarLabel();
    }

    void centralizarLabel() {
        auto b = label.getLocalBounds();
        auto p = shape.getPosition();
        auto s = shape.getSize();
        label.setPosition(
            p.x + (s.x - b.width)  / 2.f - b.left,
            p.y + (s.y - b.height) / 2.f - b.top
        );
    }

    bool contemPonto(sf::Vector2f p) const {
        return shape.getGlobalBounds().contains(p);
    }

    void setLabel(const std::string& txt) {
        label.setString(txt);
        centralizarLabel();
    }

    void desenhar(sf::RenderWindow& w, sf::Vector2f mousePos) {
        if (ativo)
            shape.setFillColor(corAtivo);
        else if (contemPonto(mousePos))
            shape.setFillColor(corHover);
        else
            shape.setFillColor(corNormal);
        w.draw(shape);
        w.draw(label);
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// Slider — trilho horizontal com handle arrastável
// ─────────────────────────────────────────────────────────────────────────────
struct Slider {
    sf::RectangleShape trilho;
    sf::CircleShape    handle;
    sf::Text           labelTxt;
    sf::Text           valorTxt;

    float minVal     = 0.f;
    float maxVal     = 1.f;
    float valor      = 1.f;
    bool  arrastando = false;

    void init(const sf::Font& f, const std::string& lbl,
              float x, float y, float largura,
              float vmin, float vmax, float vinicial) {
        minVal = vmin;
        maxVal = vmax;
        valor  = vinicial;

        trilho.setSize(sf::Vector2f(largura, 4.f));
        trilho.setPosition(x, y + 16.f);
        trilho.setFillColor(sf::Color(60, 60, 80));

        handle.setRadius(7.f);
        handle.setOrigin(7.f, 7.f);
        handle.setFillColor(sf::Color(100, 150, 220));
        handle.setOutlineThickness(1.f);
        handle.setOutlineColor(sf::Color(140, 180, 255));

        labelTxt.setFont(f);
        labelTxt.setCharacterSize(11);
        labelTxt.setFillColor(sf::Color(180, 180, 200));
        labelTxt.setString(lbl);
        labelTxt.setPosition(x, y);

        valorTxt.setFont(f);
        valorTxt.setCharacterSize(11);
        valorTxt.setFillColor(sf::Color(220, 220, 255));

        atualizarHandlePos();
        atualizarValorTxt();
    }

    void atualizarHandlePos() {
        float trilhoX = trilho.getPosition().x;
        float trilhoW = trilho.getSize().x;
        float t = (valor - minVal) / (maxVal - minVal);
        handle.setPosition(trilhoX + t * trilhoW,
                           trilho.getPosition().y + 2.f);
    }

    void atualizarValorTxt() {
        float trilhoX = trilho.getPosition().x;
        float trilhoW = trilho.getSize().x;
        float y       = labelTxt.getPosition().y;
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << valor;
        valorTxt.setString(oss.str());
        valorTxt.setPosition(trilhoX + trilhoW + 6.f, y);
    }

    // Retorna true se o valor mudou
    bool processarMouse(const sf::Event& ev) {
        float trilhoX = trilho.getPosition().x;
        float trilhoW = trilho.getSize().x;

        if (ev.type == sf::Event::MouseButtonPressed &&
            ev.mouseButton.button == sf::Mouse::Left) {
            float mx = static_cast<float>(ev.mouseButton.x);
            float my = static_cast<float>(ev.mouseButton.y);
            // Área clicável: handle ou o trilho inteiro (com folga vertical)
            sf::FloatRect areaTrilho(trilhoX - 8.f, trilho.getPosition().y - 10.f,
                                     trilhoW + 16.f, 24.f);
            if (handle.getGlobalBounds().contains(mx, my)) {
                arrastando = true;
            } else if (areaTrilho.contains(mx, my)) {
                // Clique direto no trilho: salta o handle para a posição
                arrastando = true;
                float t = (mx - trilhoX) / trilhoW;
                if (t < 0.f) t = 0.f;
                if (t > 1.f) t = 1.f;
                float novoValor = minVal + t * (maxVal - minVal);
                if (novoValor != valor) {
                    valor = novoValor;
                    atualizarHandlePos();
                    atualizarValorTxt();
                    return true;
                }
            }
        }

        if (ev.type == sf::Event::MouseButtonReleased &&
            ev.mouseButton.button == sf::Mouse::Left)
            arrastando = false;

        if (arrastando && ev.type == sf::Event::MouseMoved) {
            float t = (static_cast<float>(ev.mouseMove.x) - trilhoX) / trilhoW;
            if (t < 0.f) t = 0.f;
            if (t > 1.f) t = 1.f;
            float novoValor = minVal + t * (maxVal - minVal);
            if (novoValor != valor) {
                valor = novoValor;
                atualizarHandlePos();
                atualizarValorTxt();
                return true;
            }
        }
        return false;
    }

    void resetar() {
        valor = 1.0f;
        atualizarHandlePos();
        atualizarValorTxt();
    }

    void desenhar(sf::RenderWindow& w) {
        w.draw(trilho);
        w.draw(handle);
        w.draw(labelTxt);
        w.draw(valorTxt);
    }
};

#endif // UI_HPP
