// Gera apresentacao_POO.pptx — Simulador de Ecossistema, foco em POO
const pptxgen = require("pptxgenjs");

const pptx = new pptxgen();
pptx.defineLayout({ name: "W16x9", width: 10, height: 5.625 });
pptx.layout = "W16x9";

// ── Paleta (Forest & Moss) ───────────────────────────────────────────────────
const DARK   = "16291A"; // fundo capa/conclusão
const FOREST = "2C5F2D"; // primária
const MOSS   = "97BC62"; // secundária
const CREAM  = "F7F7F2"; // fundo slides de conteúdo
const INK    = "22301F"; // texto principal
const MUTE   = "6B7A66"; // texto apagado
const CODEBG = "1E241E"; // fundo de código
const CODE   = "E8EFE4"; // texto de código
const CMT    = "7FAF7A"; // comentários
const KW     = "A9D18E"; // destaque em código
const PLANTA = "2EB82E";
const PRESA  = "64B4FF";
const PRED   = "FF5A5A";

const HFONT = "Trebuchet MS";
const BFONT = "Calibri";
const MONO  = "Consolas";

const IMG = (f) => `${__dirname}/slides_img/${f}`;

// ── Helpers ──────────────────────────────────────────────────────────────────
function fundo(s, cor) {
  s.background = { color: cor };
}

function titulo(s, txt, opts = {}) {
  s.addText(txt, {
    x: 0.55, y: 0.32, w: 8.9, h: 0.7,
    fontFace: HFONT, fontSize: 30, bold: true,
    color: opts.cor || FOREST, align: "left",
  });
}

// três pontinhos (planta/presa/predador) — motivo visual recorrente
function motivo(s, x, y, r = 0.09) {
  const cores = [PLANTA, PRESA, PRED];
  cores.forEach((c, i) => {
    s.addShape("ellipse", {
      x: x + i * (r * 2 + 0.07), y, w: r * 2, h: r * 2, fill: { color: c }, line: { type: "none" },
    });
  });
}

// bloco de código com barra de título
function codigo(s, x, y, w, h, arquivo, linhas, fontSize = 12) {
  s.addShape("roundRect", {
    x, y, w, h, rectRadius: 0.06,
    fill: { color: CODEBG }, line: { type: "none" },
  });
  s.addText(arquivo, {
    x: x + 0.12, y: y + 0.04, w: w - 0.3, h: 0.28,
    fontFace: MONO, fontSize: 10, color: MOSS, align: "left",
  });
  const runs = [];
  linhas.forEach((l, i) => {
    runs.push({
      text: l.t,
      options: { color: l.c || CODE, breakLine: true },
    });
  });
  s.addText(runs, {
    x: x + 0.12, y: y + 0.36, w: w - 0.24, h: h - 0.48,
    fontFace: MONO, fontSize, align: "left", valign: "top",
    lineSpacing: fontSize * 1.18,
  });
}

// cartão com círculo numerado/ícone
function cartao(s, x, y, w, h, corCirc, letra, cab, corpo, corpoSize = 12) {
  s.addShape("roundRect", {
    x, y, w, h, rectRadius: 0.06,
    fill: { color: "FFFFFF" }, line: { color: "DDE3D8", width: 1 },
  });
  s.addShape("ellipse", {
    x: x + 0.18, y: y + 0.18, w: 0.42, h: 0.42,
    fill: { color: corCirc }, line: { type: "none" },
  });
  s.addText(letra, {
    x: x + 0.18, y: y + 0.18, w: 0.42, h: 0.42,
    fontFace: HFONT, fontSize: 16, bold: true, color: "FFFFFF", align: "center", valign: "middle",
  });
  s.addText(cab, {
    x: x + 0.72, y: y + 0.18, w: w - 0.9, h: 0.42,
    fontFace: HFONT, fontSize: 15, bold: true, color: INK, align: "left", valign: "middle",
  });
  s.addText(corpo, {
    x: x + 0.22, y: y + 0.68, w: w - 0.44, h: h - 0.85,
    fontFace: BFONT, fontSize: corpoSize, color: INK, align: "left", valign: "top",
    lineSpacing: corpoSize * 1.25,
  });
}

// caixa de diagrama
function caixa(s, x, y, w, h, txt, fill, corTxt = "FFFFFF", sub = null, bold = true) {
  s.addShape("roundRect", {
    x, y, w, h, rectRadius: 0.05, fill: { color: fill }, line: { type: "none" },
  });
  if (sub) {
    s.addText([
      { text: txt, options: { bold, fontSize: 13, breakLine: true } },
      { text: sub, options: { fontSize: 9.5, color: corTxt === "FFFFFF" ? "DDEEDD" : MUTE } },
    ], {
      x, y, w, h, fontFace: HFONT, color: corTxt, align: "center", valign: "middle",
    });
  } else {
    s.addText(txt, {
      x, y, w, h, fontFace: HFONT, fontSize: 13, bold, color: corTxt, align: "center", valign: "middle",
    });
  }
}

function seta(s, x1, y1, x2, y2, cor = MUTE) {
  s.addShape("line", {
    x: x1, y: y1, w: x2 - x1, h: y2 - y1,
    line: { color: cor, width: 1.75, endArrowType: "triangle" },
  });
}

function rodape(s, n) {
  s.addText(`Simulador de Ecossistema — POO em C++  ·  ${n}`, {
    x: 0.55, y: 5.28, w: 9.0, h: 0.3, fontFace: BFONT, fontSize: 9, color: MUTE, align: "left",
  });
}

// ═════════════════════════════════════════════════════════════════════════════
// SLIDE 1 — Capa
// ═════════════════════════════════════════════════════════════════════════════
{
  const s = pptx.addSlide();
  fundo(s, DARK);
  // faixa de "grade" decorativa à direita (lembra o grid da simulação)
  for (let i = 0; i < 5; i++) {
    for (let j = 0; j < 7; j++) {
      const acende = (i * 7 + j) % 4 === 0;
      s.addShape("rect", {
        x: 7.6 + i * 0.46, y: 1.35 + j * 0.46, w: 0.38, h: 0.38,
        fill: { color: acende ? FOREST : "1D331F" }, line: { type: "none" },
      });
    }
  }
  s.addShape("ellipse", { x: 8.06, y: 2.27, w: 0.38, h: 0.38, fill: { color: PRESA }, line: { type: "none" } });
  s.addShape("ellipse", { x: 8.52, y: 3.19, w: 0.38, h: 0.38, fill: { color: PRED }, line: { type: "none" } });
  s.addShape("ellipse", { x: 7.6,  y: 3.65, w: 0.38, h: 0.38, fill: { color: PLANTA }, line: { type: "none" } });

  motivo(s, 0.62, 1.18, 0.1);
  s.addText("Simulador de Ecossistema", {
    x: 0.55, y: 1.5, w: 6.9, h: 1.5,
    fontFace: HFONT, fontSize: 44, bold: true, color: "FFFFFF", align: "left",
  });
  s.addText("Programação Orientada a Objetos em C++17 com SFML", {
    x: 0.58, y: 2.95, w: 6.6, h: 0.5,
    fontFace: BFONT, fontSize: 18, color: MOSS, align: "left",
  });
  s.addText("Plantas, presas e predadores interagindo em tempo real —\nherança, polimorfismo e comportamento emergente.", {
    x: 0.58, y: 3.5, w: 6.4, h: 0.8,
    fontFace: BFONT, fontSize: 13, italic: true, color: "B9C7B4", align: "left",
  });
  s.addText("[ Seu nome  ·  Disciplina  ·  Data ]", {
    x: 0.58, y: 4.75, w: 5.5, h: 0.4,
    fontFace: BFONT, fontSize: 13, color: "8FA38A", align: "left",
  });
}

// ═════════════════════════════════════════════════════════════════════════════
// SLIDE 2 — O Projeto
// ═════════════════════════════════════════════════════════════════════════════
{
  const s = pptx.addSlide();
  fundo(s, CREAM);
  titulo(s, "O projeto em uma tela");

  s.addText("Uma cadeia alimentar simulada: plantas crescem e se espalham, presas forrageiam em bando e fogem, predadores caçam com perseguição preditiva. Tudo emerge da interação entre objetos autônomos.", {
    x: 0.55, y: 1.0, w: 4.35, h: 1.15,
    fontFace: BFONT, fontSize: 13.5, color: INK, align: "left", lineSpacing: 18,
  });

  // três espécies com cor/forma do app
  const esp = [
    { c: PLANTA, forma: "rect",     n: "Planta",   d: "quadrado verde — cresce em clusters" },
    { c: PRESA,  forma: "ellipse",  n: "Presa",    d: "círculo — forrageia e foge em bando" },
    { c: PRED,   forma: "triangle", n: "Predador", d: "triângulo — caça a presa mais vantajosa" },
  ];
  esp.forEach((e, i) => {
    const y = 2.32 + i * 0.62;
    s.addShape(e.forma, { x: 0.62, y, w: 0.34, h: 0.34, fill: { color: e.c }, line: { type: "none" } });
    s.addText([
      { text: e.n + "  ", options: { bold: true, fontSize: 13.5 } },
      { text: e.d, options: { fontSize: 12, color: MUTE } },
    ], { x: 1.1, y: y - 0.06, w: 3.8, h: 0.5, fontFace: BFONT, color: INK, align: "left", valign: "middle" });
  });

  // stats
  const stats = [
    { v: "60×60", l: "células na grade espacial" },
    { v: "10/s",  l: "ticks de simulação (até 8x)" },
    { v: "7",     l: "parâmetros ajustáveis ao vivo" },
  ];
  stats.forEach((st, i) => {
    s.addText(st.v, {
      x: 0.5 + i * 1.55, y: 4.35, w: 1.5, h: 0.5,
      fontFace: HFONT, fontSize: 26, bold: true, color: FOREST, align: "center",
    });
    s.addText(st.l, {
      x: 0.5 + i * 1.55, y: 4.85, w: 1.5, h: 0.5,
      fontFace: BFONT, fontSize: 9.5, color: MUTE, align: "center",
    });
  });

  // screenshot real
  s.addImage({ path: IMG("simulacao.png"), x: 5.15, y: 1.0, w: 4.35, h: 3.26 });
  s.addText("A simulação em execução: grid, painel de estatísticas, gráfico de populações e sliders.", {
    x: 5.15, y: 4.34, w: 4.35, h: 0.5, fontFace: BFONT, fontSize: 10, italic: true, color: MUTE, align: "center",
  });
  rodape(s, 2);
}

// ═════════════════════════════════════════════════════════════════════════════
// SLIDE 3 — Arquitetura
// ═════════════════════════════════════════════════════════════════════════════
{
  const s = pptx.addSlide();
  fundo(s, CREAM);
  titulo(s, "Arquitetura em camadas");

  // camadas
  caixa(s, 3.7, 1.05, 2.6, 0.62, "main.cpp", FOREST, "FFFFFF", "loop principal");
  seta(s, 3.9, 1.67, 2.9, 1.95);   // main -> Renderer
  seta(s, 6.1, 1.67, 7.1, 1.95);   // main -> WorldManager

  caixa(s, 1.0, 1.95, 3.0, 0.78, "Renderer + UI", MOSS, INK, "janela SFML, painel, sliders");
  caixa(s, 6.0, 1.95, 3.0, 0.78, "WorldManager", MOSS, INK, "tick, grade espacial, reprodução");

  seta(s, 7.5, 2.73, 7.5, 3.1);
  caixa(s, 5.4, 3.1, 4.2, 0.78, "Organismos (polimórficos)", FOREST, "FFFFFF", "vector<unique_ptr<Organismo>>");

  seta(s, 6.2, 3.88, 5.6, 4.25);
  seta(s, 7.5, 3.88, 7.5, 4.25);
  seta(s, 8.8, 3.88, 9.0, 4.25);
  caixa(s, 4.7, 4.25, 1.7, 0.6, "Planta", "FFFFFF", PLANTA);
  caixa(s, 6.6, 4.25, 1.7, 0.6, "Presa", "FFFFFF", "2B7BC4");
  caixa(s, 8.5, 4.25, 1.4, 0.6, "Predador", "FFFFFF", PRED);

  // texto à esquerda
  s.addText([
    { text: "Visual separado da lógica\n", options: { bold: true, fontSize: 13.5, breakLine: true } },
    { text: "O mundo simula sem saber que existe tela: o modo terminal usa o mesmo WorldManager.\n\n", options: { fontSize: 11.5, breakLine: true } },
    { text: "Baixo acoplamento\n", options: { bold: true, fontSize: 13.5, breakLine: true } },
    { text: "O Renderer só conhece a interface pública do mundo; os organismos não conhecem a UI.", options: { fontSize: 11.5 } },
  ], {
    x: 0.55, y: 2.95, w: 3.6, h: 2.2, fontFace: BFONT, color: INK, align: "left", valign: "top", lineSpacing: 14,
  });
  rodape(s, 3);
}

// ═════════════════════════════════════════════════════════════════════════════
// SLIDE 4 — Os 4 pilares (visão geral)
// ═════════════════════════════════════════════════════════════════════════════
{
  const s = pptx.addSlide();
  fundo(s, CREAM);
  titulo(s, "Os quatro pilares da POO no projeto");

  cartao(s, 0.55, 1.1, 4.4, 1.95, FOREST, "A", "Abstração",
    "Organismo é uma classe abstrata: define O QUE todo ser vivo faz (agir, mover, desenhar), sem dizer COMO. Cada espécie preenche o contrato.");
  cartao(s, 5.05, 1.1, 4.4, 1.95, MOSS, "H", "Herança",
    "Hierarquia em dois níveis: Organismo → Planta e Organismo → Animal → Presa / Predador. Animal concentra o que é comum a quem se move.");
  cartao(s, 0.55, 3.2, 4.4, 1.95, "2B7BC4", "P", "Polimorfismo",
    "Um único loop trata todos: org->agir(world) despacha dinamicamente para o comportamento da espécie. Sem if (tipo == ...) na lógica central.");
  cartao(s, 5.05, 3.2, 4.4, 1.95, "B85042", "E", "Encapsulamento",
    "Estado interno protegido com invariantes: ninguém altera energia diretamente — consumirEnergia() garante que energia ≤ 0 ⇒ morte.");
  rodape(s, 4);
}

// ═════════════════════════════════════════════════════════════════════════════
// SLIDE 5 — Abstração
// ═════════════════════════════════════════════════════════════════════════════
{
  const s = pptx.addSlide();
  fundo(s, CREAM);
  titulo(s, "Abstração — o contrato de todo ser vivo");

  codigo(s, 0.55, 1.05, 5.3, 4.0, "src/core/Organismo.hpp", [
    { t: "class Organismo {" },
    { t: "protected:" },
    { t: "    float x, y, energia;", c: KW },
    { t: "    int idade;  bool vivo;", c: KW },
    { t: "public:" },
    { t: "    // metodos virtuais puros: cada", c: CMT },
    { t: "    // especie DEVE implementar", c: CMT },
    { t: "    virtual void agir(WorldManager&)  = 0;", c: KW },
    { t: "    virtual void mover(WorldManager&) = 0;", c: KW },
    { t: "    virtual void desenhar(...) const  = 0;", c: KW },
    { t: "    virtual TipoOrganismo getTipo() const = 0;" },
    { t: "    virtual float getEnergiaParaReproducao() const = 0;" },
    { t: "" },
    { t: "    // destrutor virtual: essencial!", c: CMT },
    { t: "    virtual ~Organismo() = default;" },
    { t: "};" },
  ], 11.5);

  const pontos = [
    ["Interface, não implementação", "Quem usa um Organismo não precisa saber se é planta ou predador — só que ele sabe agir, mover e se desenhar."],
    ["= 0 obriga as filhas", "Métodos virtuais puros tornam a classe abstrata: é impossível instanciar um “ser vivo genérico”."],
    ["Destrutor virtual", "Essencial: os objetos são destruídos via ponteiro para a base — sem ele, seria comportamento indefinido."],
  ];
  pontos.forEach((p, i) => {
    const y = 1.15 + i * 1.32;
    s.addShape("ellipse", { x: 6.1, y: y + 0.03, w: 0.3, h: 0.3, fill: { color: FOREST }, line: { type: "none" } });
    s.addText(`${i + 1}`, { x: 6.1, y: y + 0.03, w: 0.3, h: 0.3, fontFace: HFONT, fontSize: 12, bold: true, color: "FFFFFF", align: "center", valign: "middle" });
    s.addText([
      { text: p[0] + "\n", options: { bold: true, fontSize: 13.5, breakLine: true } },
      { text: p[1], options: { fontSize: 11.5, color: "39462F" } },
    ], { x: 6.5, y: y - 0.05, w: 3.0, h: 1.25, fontFace: BFONT, color: INK, align: "left", valign: "top", lineSpacing: 14 });
  });
  rodape(s, 5);
}

// ═════════════════════════════════════════════════════════════════════════════
// SLIDE 6 — Herança
// ═════════════════════════════════════════════════════════════════════════════
{
  const s = pptx.addSlide();
  fundo(s, CREAM);
  titulo(s, "Herança — especializar sem repetir");

  // diagrama de hierarquia
  caixa(s, 1.45, 1.05, 2.4, 0.62, "Organismo", FOREST, "FFFFFF", "abstrata");
  seta(s, 2.1, 1.67, 1.6, 2.25);
  seta(s, 3.2, 1.67, 3.7, 2.25);
  caixa(s, 0.6, 2.25, 2.0, 0.62, "Planta", PLANTA, "FFFFFF", "não se move");
  caixa(s, 2.85, 2.25, 2.0, 0.62, "Animal", MOSS, INK, "velocidade, visão, vx/vy");
  seta(s, 3.35, 2.87, 2.9, 3.45);
  seta(s, 4.35, 2.87, 4.8, 3.45);
  caixa(s, 1.95, 3.45, 1.9, 0.62, "Presa", "2B7BC4", "FFFFFF", "forrageia, foge");
  caixa(s, 3.95, 3.45, 1.9, 0.62, "Predador", "B8312F", "FFFFFF", "caça");

  s.addText("Animal é uma base intermediária: concentra física de movimento (aplicarForca), visão e velocidade. Planta herda direto de Organismo — não paga pelo que não usa.", {
    x: 0.6, y: 4.35, w: 5.3, h: 1.0, fontFace: BFONT, fontSize: 11.5, color: INK, align: "left", lineSpacing: 14,
  });

  codigo(s, 6.15, 1.05, 3.45, 4.0, "src/entities/Presa.cpp", [
    { t: "// construtor encadeia a base:", c: CMT },
    { t: "Presa::Presa(float x, float y)" },
    { t: "  : Animal(x, y,", c: KW },
    { t: "      ENERGIA_INICIAL,", c: KW },
    { t: "      COOLDOWN_REPRODUCAO,", c: KW },
    { t: "      VELOCIDADE, RAIO_VISAO) {}", c: KW },
    { t: "" },
    { t: "// e Animal encadeia Organismo:", c: CMT },
    { t: "Animal::Animal(...)" },
    { t: "  : Organismo(x, y, energia,", c: KW },
    { t: "              cooldown),", c: KW },
    { t: "    velocidade(velocidade)," },
    { t: "    raioVisao(raioVisao) {}" },
  ], 11);
  rodape(s, 6);
}

// ═════════════════════════════════════════════════════════════════════════════
// SLIDE 7 — Polimorfismo
// ═════════════════════════════════════════════════════════════════════════════
{
  const s = pptx.addSlide();
  fundo(s, CREAM);
  titulo(s, "Polimorfismo — um loop para todos");

  codigo(s, 0.55, 1.05, 4.7, 2.1, "src/core/WorldManager.cpp", [
    { t: "// o mundo nao sabe quem e quem:", c: CMT },
    { t: "for (auto& org : organismos) {" },
    { t: "    if (org->estaVivo()) {" },
    { t: "        org->agir(*this);   // despacho", c: KW },
    { t: "        org->mover(*this);  // dinamico", c: KW },
    { t: "    }" },
    { t: "}" },
  ], 12);

  s.addText("A mesma chamada org->desenhar(...) produz três formas diferentes — é o que aparece na tela:", {
    x: 0.55, y: 3.35, w: 4.7, h: 0.65, fontFace: BFONT, fontSize: 12.5, color: INK, align: "left", lineSpacing: 15,
  });
  const formas = [
    { f: "rect", c: PLANTA, t: "Planta::desenhar → RectangleShape" },
    { f: "ellipse", c: PRESA, t: "Presa::desenhar → CircleShape" },
    { f: "triangle", c: PRED, t: "Predador::desenhar → ConvexShape rotacionado" },
  ];
  formas.forEach((fo, i) => {
    const y = 4.05 + i * 0.45;
    s.addShape(fo.f, { x: 0.62, y, w: 0.3, h: 0.3, fill: { color: fo.c }, line: { type: "none" } });
    s.addText(fo.t, { x: 1.05, y: y - 0.05, w: 4.3, h: 0.42, fontFace: MONO, fontSize: 10.5, color: INK, align: "left", valign: "middle" });
  });

  codigo(s, 5.55, 1.05, 4.0, 3.05, "identificação sem dynamic_cast", [
    { t: "// cada classe responde quem e:", c: CMT },
    { t: "TipoOrganismo Presa::getTipo()" },
    { t: "        const override {" },
    { t: "    return TipoOrganismo::Presa;", c: KW },
    { t: "}" },
    { t: "" },
    { t: "// e parametros por especie:", c: CMT },
    { t: "float getEnergiaParaReproducao()" },
    { t: "        const override {" },
    { t: "    return 40.0f;  // Presa", c: KW },
    { t: "}   // Predador retorna 55.0f", c: CMT },
  ], 11.5);
  s.addText("override pega erros em compilação; a reprodução no WorldManager usa esses getters polimórficos sem conhecer as espécies.", {
    x: 5.55, y: 4.25, w: 4.0, h: 0.9, fontFace: BFONT, fontSize: 11.5, color: INK, align: "left", lineSpacing: 14,
  });
  rodape(s, 7);
}

// ═════════════════════════════════════════════════════════════════════════════
// SLIDE 8 — Encapsulamento
// ═════════════════════════════════════════════════════════════════════════════
{
  const s = pptx.addSlide();
  fundo(s, CREAM);
  titulo(s, "Encapsulamento — invariantes protegidas");

  codigo(s, 0.55, 1.05, 4.55, 2.55, "src/core/Organismo.cpp", [
    { t: "void Organismo::consumirEnergia(" },
    { t: "        float quantidade) {" },
    { t: "    energia -= quantidade;" },
    { t: "    if (energia <= 0) {", c: KW },
    { t: "        energia = 0;", c: KW },
    { t: "        vivo = false;  // invariante", c: KW },
    { t: "    }" },
    { t: "}" },
  ], 12);

  codigo(s, 0.55, 3.75, 4.55, 1.55, "src/core/WorldManager.hpp", [
    { t: "private:" },
    { t: "    // ninguem fora ve a grade", c: CMT },
    { t: "    std::vector<std::vector<" },
    { t: "      Organismo*>> gradeEspacial;", c: KW },
  ], 11.5);

  const itens = [
    ["Estado morre junto com a regra", "energia e vivo são protected. A ÚNICA porta de entrada é consumirEnergia() — é impossível um organismo ficar com energia negativa e “vivo”."],
    ["Estrutura interna trocável", "A grade espacial é private: o resto do código usa obterVizinhos(). A otimização de O(n) para O(1) foi feita sem tocar nas espécies."],
    ["Leitura sem escrita", "Getters const (getX, getEnergia, getOrganismos) expõem o estado para o Renderer sem permitir modificação."],
  ];
  itens.forEach((p, i) => {
    const y = 1.15 + i * 1.4;
    s.addShape("roundRect", { x: 5.45, y: y - 0.08, w: 4.1, h: 1.28, rectRadius: 0.05, fill: { color: "FFFFFF" }, line: { color: "DDE3D8", width: 1 } });
    s.addText([
      { text: p[0] + "\n", options: { bold: true, fontSize: 13, breakLine: true } },
      { text: p[1], options: { fontSize: 11, color: "39462F" } },
    ], { x: 5.62, y, w: 3.8, h: 1.2, fontFace: BFONT, color: INK, align: "left", valign: "top", lineSpacing: 13.5 });
  });
  rodape(s, 8);
}

// ═════════════════════════════════════════════════════════════════════════════
// SLIDE 9 — RAII + Factory
// ═════════════════════════════════════════════════════════════════════════════
{
  const s = pptx.addSlide();
  fundo(s, CREAM);
  titulo(s, "Composição, RAII e o padrão Factory");

  codigo(s, 0.55, 1.05, 5.1, 2.5, "src/core/OrganismoFactory.cpp", [
    { t: "std::unique_ptr<Organismo>" },
    { t: "OrganismoFactory::criar(TipoOrganismo t," },
    { t: "                        float x, float y) {" },
    { t: "  switch (t) {" },
    { t: "    case TipoOrganismo::Planta:" },
    { t: "      return std::make_unique<Planta>(x, y);", c: KW },
    { t: "    case TipoOrganismo::Presa:" },
    { t: "      return std::make_unique<Presa>(x, y);", c: KW },
    { t: "    case TipoOrganismo::Predador:" },
    { t: "      return std::make_unique<Predador>(x, y);", c: KW },
    { t: "  }" },
    { t: "}" },
  ], 10.5);

  codigo(s, 0.55, 3.7, 5.1, 1.55, "posse exclusiva — sem new/delete manuais", [
    { t: "// WorldManager COMPOE os organismos:", c: CMT },
    { t: "std::vector<std::unique_ptr<Organismo>>" },
    { t: "    organismos;  // destruicao automatica", c: KW },
  ], 11);

  const itens = [
    ["Factory Method", "Um único ponto cria qualquer espécie. A reprodução chama criar(org->getTipo(), x, y) — funciona para uma espécie nova sem mudar o WorldManager."],
    ["RAII com unique_ptr", "O mundo é dono dos organismos: quando um morre e sai do vector, a memória é liberada — zero vazamentos, zero delete."],
    ["Composição vs. herança", "WorldManager TEM organismos (composição); Presa É UM Animal (herança). Cada relação no mecanismo certo."],
  ];
  itens.forEach((p, i) => {
    const y = 1.15 + i * 1.4;
    s.addText([
      { text: p[0] + "\n", options: { bold: true, fontSize: 13.5, color: FOREST, breakLine: true } },
      { text: p[1], options: { fontSize: 11.5, color: "39462F" } },
    ], { x: 6.0, y, w: 3.55, h: 1.35, fontFace: BFONT, color: INK, align: "left", valign: "top", lineSpacing: 14 });
  });
  rodape(s, 9);
}

// ═════════════════════════════════════════════════════════════════════════════
// SLIDE 10 — Máquina de estados
// ═════════════════════════════════════════════════════════════════════════════
{
  const s = pptx.addSlide();
  fundo(s, CREAM);
  titulo(s, "Comportamento por máquina de estados");

  codigo(s, 0.55, 1.05, 3.6, 1.7, "src/core/Animal.hpp", [
    { t: "enum class EstadoAgente {" },
    { t: "    WANDERING,", c: KW },
    { t: "    HUNTING,   // predador", c: CMT },
    { t: "    FORAGING,  // presa", c: CMT },
    { t: "    FLEEING    // presa", c: CMT },
    { t: "};" },
  ], 11.5);

  s.addText("Cada animal decide em agir() e executa em mover(). A transição depende do que ele vê na vizinhança — e fugir custa 1,5x mais energia.", {
    x: 0.55, y: 2.95, w: 3.6, h: 1.3, fontFace: BFONT, fontSize: 12, color: INK, align: "left", lineSpacing: 15,
  });
  s.addText("[ Espaço para imagem/zoom de presas fugindo em bando — opcional ]", {
    x: 0.55, y: 4.25, w: 3.6, h: 0.85, fontFace: BFONT, fontSize: 10, italic: true, color: MUTE, align: "center", valign: "middle",
  });
  s.addShape("roundRect", { x: 0.55, y: 4.25, w: 3.6, h: 0.85, rectRadius: 0.05, fill: { type: "none" }, line: { color: MUTE, width: 1, dashType: "dash" } });

  // FSM da presa
  s.addText("FSM da Presa", { x: 4.6, y: 1.0, w: 4.9, h: 0.35, fontFace: HFONT, fontSize: 13, bold: true, color: FOREST, align: "center" });
  caixa(s, 5.0, 1.45, 1.8, 0.6, "WANDERING", "FFFFFF", INK, "vagar (flocking)");
  caixa(s, 7.6, 1.45, 1.8, 0.6, "FORAGING", PLANTA, "FFFFFF", "buscar planta");
  caixa(s, 6.3, 2.55, 1.8, 0.6, "FLEEING", PRED, "FFFFFF", "fugir (evade)");
  seta(s, 6.8, 1.65, 7.6, 1.65, FOREST);
  seta(s, 7.6, 1.85, 6.8, 1.85, MUTE);
  seta(s, 5.9, 2.05, 6.7, 2.55, PRED);
  seta(s, 8.5, 2.05, 7.7, 2.55, PRED);
  s.addText("viu planta → / ← sem plantas", {
    x: 6.0, y: 1.18, w: 2.4, h: 0.26, fontFace: BFONT, fontSize: 8.5, color: MUTE, align: "center",
  });
  s.addText("predador à vista ↓", {
    x: 6.4, y: 3.17, w: 1.6, h: 0.26, fontFace: BFONT, fontSize: 8.5, color: MUTE, align: "center",
  });

  // FSM do predador
  s.addText("FSM do Predador", { x: 4.6, y: 3.45, w: 4.9, h: 0.35, fontFace: HFONT, fontSize: 13, bold: true, color: FOREST, align: "center" });
  caixa(s, 5.0, 3.9, 1.8, 0.6, "WANDERING", "FFFFFF", INK, "patrulhar");
  caixa(s, 7.6, 3.9, 1.8, 0.6, "HUNTING", "B8312F", "FFFFFF", "pursue preditivo");
  seta(s, 6.8, 4.12, 7.6, 4.12, "B8312F");
  seta(s, 7.6, 4.32, 6.8, 4.32, MUTE);
  s.addText("presa no raio de visão →   /   ← alvo perdido ou comido", {
    x: 4.85, y: 4.55, w: 4.8, h: 0.3, fontFace: BFONT, fontSize: 9, color: MUTE, align: "center",
  });
  rodape(s, 10);
}

// ═════════════════════════════════════════════════════════════════════════════
// SLIDE 11 — Comportamento emergente
// ═════════════════════════════════════════════════════════════════════════════
{
  const s = pptx.addSlide();
  fundo(s, CREAM);
  titulo(s, "Steering → comportamento emergente");

  const beh = [
    ["seek / flee", "ir até um alvo / afastar-se dele"],
    ["pursue / evade", "antecipa a posição futura do alvo"],
    ["wander", "vagueio suave com jitter aleatório"],
    ["flocking", "separação + alinhamento + coesão"],
  ];
  beh.forEach((b, i) => {
    const y = 1.1 + i * 0.78;
    s.addShape("roundRect", { x: 0.55, y, w: 4.3, h: 0.66, rectRadius: 0.05, fill: { color: "FFFFFF" }, line: { color: "DDE3D8", width: 1 } });
    s.addText([
      { text: b[0] + "   ", options: { bold: true, fontSize: 12.5, fontFace: MONO, color: FOREST } },
      { text: b[1], options: { fontSize: 11, color: "39462F" } },
    ], { x: 0.72, y, w: 4.05, h: 0.66, fontFace: BFONT, color: INK, align: "left", valign: "middle" });
  });

  s.addText("Nenhuma linha de código diz “formem bandos” ou “cresçam em moitas” — isso emerge das regras locais de cada objeto.", {
    x: 0.55, y: 4.45, w: 4.3, h: 0.9, fontFace: BFONT, fontSize: 12, italic: true, color: FOREST, align: "left", lineSpacing: 15,
  });

  s.addImage({ path: IMG("simulacao_tarde.png"), x: 5.15, y: 1.05, w: 4.35, h: 3.26 });
  s.addText("Clusters de plantas e o gráfico predador–presa (ciclo de Lotka-Volterra) — tudo emergente.", {
    x: 5.15, y: 4.39, w: 4.35, h: 0.5, fontFace: BFONT, fontSize: 10, italic: true, color: MUTE, align: "center",
  });
  rodape(s, 11);
}

// ═════════════════════════════════════════════════════════════════════════════
// SLIDE 12 — Desempenho
// ═════════════════════════════════════════════════════════════════════════════
{
  const s = pptx.addSlide();
  fundo(s, CREAM);
  titulo(s, "Engenharia além da POO: desempenho");

  const stats = [
    { v: "0,07 ms", l: "por tick com ~700 organismos\n(medido com -O2)" },
    { v: "O(1)", l: "busca de vizinhos via grade espacial\n(antes: varredura O(n))" },
    { v: "~2100 → 3", l: "draw calls por frame com batching\nem sf::VertexArray" },
  ];
  stats.forEach((st, i) => {
    const x = 0.55 + i * 3.05;
    s.addShape("roundRect", { x, y: 1.1, w: 2.85, h: 1.6, rectRadius: 0.06, fill: { color: "FFFFFF" }, line: { color: "DDE3D8", width: 1 } });
    s.addText(st.v, { x, y: 1.22, w: 2.85, h: 0.65, fontFace: HFONT, fontSize: 30, bold: true, color: FOREST, align: "center" });
    s.addText(st.l, { x: x + 0.1, y: 1.9, w: 2.65, h: 0.75, fontFace: BFONT, fontSize: 10.5, color: MUTE, align: "center" });
  });

  codigo(s, 0.55, 3.0, 5.7, 2.15, "validação de ponteiros O(1) — encapsulada", [
    { t: "// alvos guardados entre ticks podem morrer;", c: CMT },
    { t: "// conjunto de vivos valida em O(1):", c: CMT },
    { t: "std::unordered_set<const Organismo*> vivosSet;" },
    { t: "" },
    { t: "bool organismoExisteEVivo(const Organismo* p) {" },
    { t: "    return vivosSet.count(p) && p->estaVivo();", c: KW },
    { t: "}" },
  ], 11);

  s.addText([
    { text: "Encapsulamento pagou aqui: ", options: { bold: true, breakLine: false } },
    { text: "a troca de O(n) para O(1) aconteceu dentro do WorldManager — nenhuma espécie precisou mudar.", options: {} },
  ], {
    x: 6.5, y: 3.1, w: 3.1, h: 1.9, fontFace: BFONT, fontSize: 12.5, color: INK, align: "left", valign: "top", lineSpacing: 16,
  });
  rodape(s, 12);
}

// ═════════════════════════════════════════════════════════════════════════════
// SLIDE 13 — Demonstração
// ═════════════════════════════════════════════════════════════════════════════
{
  const s = pptx.addSlide();
  fundo(s, CREAM);
  titulo(s, "Demonstração");

  s.addImage({ path: IMG("tela_inicial.png"), x: 0.55, y: 1.05, w: 4.6, h: 3.45 });
  s.addText("Tela inicial: o usuário escolhe as populações antes de começar.", {
    x: 0.55, y: 4.55, w: 4.6, h: 0.4, fontFace: BFONT, fontSize: 10, italic: true, color: MUTE, align: "center",
  });

  s.addText("Controles ao vivo", { x: 5.55, y: 1.05, w: 4.0, h: 0.4, fontFace: HFONT, fontSize: 16, bold: true, color: FOREST, align: "left" });
  const ctl = [
    ["Espaço", "pausa / retoma"],
    ["F", "velocidade 1x → 2x → 4x → 8x"],
    ["Scroll", "zoom no grid"],
    ["WASD", "mover a câmera"],
    ["Sliders", "7 parâmetros em tempo real"],
    ["Esc", "sair"],
  ];
  ctl.forEach((c, i) => {
    const y = 1.55 + i * 0.5;
    s.addShape("roundRect", { x: 5.55, y, w: 1.1, h: 0.4, rectRadius: 0.05, fill: { color: FOREST }, line: { type: "none" } });
    s.addText(c[0], { x: 5.55, y, w: 1.1, h: 0.4, fontFace: MONO, fontSize: 11, bold: true, color: "FFFFFF", align: "center", valign: "middle" });
    s.addText(c[1], { x: 6.8, y, w: 2.8, h: 0.4, fontFace: BFONT, fontSize: 12, color: INK, align: "left", valign: "middle" });
  });
  s.addText("Sugestão: subir “Reprod. Presa” e ver a população de predadores responder no gráfico.", {
    x: 5.55, y: 4.65, w: 4.0, h: 0.7, fontFace: BFONT, fontSize: 11, italic: true, color: MUTE, align: "left", lineSpacing: 13,
  });
  rodape(s, 13);
}

// ═════════════════════════════════════════════════════════════════════════════
// SLIDE 14 — Conclusão
// ═════════════════════════════════════════════════════════════════════════════
{
  const s = pptx.addSlide();
  fundo(s, DARK);
  motivo(s, 0.62, 0.55, 0.1);
  s.addText("O que a POO comprou neste projeto", {
    x: 0.55, y: 0.85, w: 8.9, h: 0.7, fontFace: HFONT, fontSize: 30, bold: true, color: "FFFFFF", align: "left",
  });

  const itens = [
    ["Extensibilidade", "Uma espécie nova (ex.: necrófago) = uma subclasse nova. O loop do mundo, a fábrica e o renderer já estão prontos para ela."],
    ["Manutenção localizada", "Mudar a IA da presa não toca o predador; otimizar a grade não toca as espécies. Cada mudança tem um endereço."],
    ["Complexidade emergente", "Objetos simples + regras locais + polimorfismo = ciclos populacionais que ninguém programou explicitamente."],
  ];
  itens.forEach((p, i) => {
    const x = 0.55 + i * 3.08;
    s.addShape("roundRect", { x, y: 1.85, w: 2.88, h: 2.3, rectRadius: 0.06, fill: { color: "1D331F" }, line: { color: FOREST, width: 1 } });
    s.addText([
      { text: p[0] + "\n", options: { bold: true, fontSize: 15, color: MOSS, breakLine: true } },
      { text: p[1], options: { fontSize: 11.5, color: "D5DFD0" } },
    ], { x: x + 0.18, y: 2.05, w: 2.52, h: 2.0, fontFace: BFONT, align: "left", valign: "top", lineSpacing: 15 });
  });

  s.addText("Obrigado!", {
    x: 0.55, y: 4.45, w: 4.0, h: 0.6, fontFace: HFONT, fontSize: 24, bold: true, color: "FFFFFF", align: "left",
  });
  s.addText("[ Seu nome  ·  contato / repositório GitHub ]", {
    x: 0.58, y: 5.05, w: 5.0, h: 0.35, fontFace: BFONT, fontSize: 11, color: "8FA38A", align: "left",
  });
}

pptx.writeFile({ fileName: `${__dirname}/apresentacao_POO.pptx` })
  .then(() => console.log("OK: apresentacao_POO.pptx gerada"));
