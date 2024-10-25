#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <iomanip>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int BLOCK_WIDTH = 60;
const int BLOCK_HEIGHT = 20;
const int PADDLE_WIDTH = 100;
const int PADDLE_HEIGHT = 20;
const float BALL_RADIUS = 10.0f;
const float BALL_VELOCITY = 5.0f;
const float PADDLE_VELOCITY = 8.0f;
const int INITIAL_LIVES = 3;
const int ITEM_WIDTH = 20;
const int ITEM_HEIGHT = 20;

enum class ItemType {
    ExtraLife,
    PaddleIncrease,
    PaddleDecrease,
    ExtraPoints,
    ScoreMultiplier
};

struct SpecialItem {
    sf::RectangleShape shape;
    ItemType type;
};

struct Block {
    sf::RectangleShape shape;
    int hitPoints;
};

void updateBlockColor(Block &block) {
    if (block.hitPoints == 3) {
        block.shape.setFillColor(sf::Color::Red); // 3 acertos = Vermelho
    } else if (block.hitPoints == 2) {
        block.shape.setFillColor(sf::Color::Yellow); // 2 acertos = Amarelo
    } else if (block.hitPoints == 1) {
        block.shape.setFillColor(sf::Color::Green); // 1 acerto = Verde
    }
}

SpecialItem createSpecialItem(float x, float y) {
    SpecialItem item;
    item.shape.setSize(sf::Vector2f(ITEM_WIDTH, ITEM_HEIGHT));
    item.shape.setPosition(x, y);

    // Define aleatoriamente o tipo do item especial
    int randomType = std::rand() % 5;
    switch (randomType) {
        case 0:
            item.type = ItemType::ExtraLife;
            item.shape.setFillColor(sf::Color::Green); // Item VIDA EXTRA = VERDE
            break;
        case 1:
            item.type = ItemType::PaddleIncrease;
            item.shape.setFillColor(sf::Color::Blue); // Item AUMENTA PADDLE = AZUL
            break;
        case 2:
            item.type = ItemType::PaddleDecrease;
            item.shape.setFillColor(sf::Color::Red);// Item DIMINUI PADDLE = VERMELHO
            break;
        case 3:
            item.type = ItemType::ExtraPoints;
            item.shape.setFillColor(sf::Color::Yellow); // Item PONTOS EXTRAS = AMARELO
            break;
        case 4:
            item.type = ItemType::ScoreMultiplier;
            item.shape.setFillColor(sf::Color::Magenta); // Item MULTIPLICADOR DE PONTOS = MAGENTA
            break;
    }
    return item;
}

// Função recursiva para calcular pontuação adicional baseada nas fases completadas
int calculateBonus(int phase) {
    if (phase <= 0) {
        return 0;
    }
    return phase * 100 + calculateBonus(phase - 1);
}

// Template para salvar qualquer tipo de dado no arquivo de pontuações
template <typename T>
void saveScore(T score, T lives) {
    std::ofstream outFile("scores.txt", std::ios::app);
    if (outFile.is_open()) {
        outFile << "Pontuacao: " << score << " Vidas: " << lives << "\n";
        outFile.close();
    }
}

// Sobrecarga da função saveScore para salvar também o tempo e o nome do jogador
void saveScore(const std::string& playerName, int score, int lives, float timeElapsed) {
    std::ofstream outFile("scores.txt", std::ios::app);
    if (outFile.is_open()) {
        outFile << "Jogador: " << playerName << " Pontuacao: " << score << " Vidas: " << lives << " Tempo Total: " << std::fixed << std::setprecision(2) << timeElapsed << "s\n";
        outFile.close();
    }
}

// Função para salvar pontuações organizadas
void saveSortedScores() {
    std::ifstream inFile("scores.txt");
    std::vector<std::string> lines;
    std::string line;

    // Lê todas as linhas do arquivo
    while (std::getline(inFile, line)) {
        lines.push_back(line);
    }
    inFile.close();

    // Ordena as linhas pela pontuação
    std::sort(lines.begin(), lines.end(), [](const std::string& a, const std::string& b) {
        int scoreA = std::stoi(a.substr(a.find("Pontuacao: ") + 10));
        int scoreB = std::stoi(b.substr(b.find("Pontuacao: ") + 10));
        return scoreA > scoreB;
    });

    // Reescreve o arquivo com as linhas ordenadas
    std::ofstream outFile("scores.txt");
    for (const auto& sortedLine : lines) {
        outFile << sortedLine << "\n";
    }
}

int score = 0;
int scoreMultiplier = 1;
int currentPhase = 1;
int maxPhases = 5;
std::string playerName;
bool scoreSaved = false;

void resetGame(int& lives, sf::CircleShape& ball, sf::Vector2f& ballVelocity, sf::RectangleShape& paddle, sf::Clock& clock) {
    lives = INITIAL_LIVES;
    score = 0;
    currentPhase = 1;
    paddle.setSize(sf::Vector2f(PADDLE_WIDTH, PADDLE_HEIGHT));
    ball.setPosition(paddle.getPosition().x + PADDLE_WIDTH / 2 - BALL_RADIUS, paddle.getPosition().y - BALL_RADIUS * 2);
    ballVelocity = sf::Vector2f(0, 0);
    clock.restart();
    scoreSaved = false;
}

void loadNextPhase(std::vector<Block>& blocks, sf::RectangleShape& paddle, int difficulty, int currentPhase) {
    blocks.clear();
    paddle.setSize(sf::Vector2f(PADDLE_WIDTH, PADDLE_HEIGHT));
    int rows = 6 + currentPhase; // Aumenta o número de linhas conforme a fase
    int cols = 10 + difficulty; // Aumenta o número de colunas conforme a dificuldade
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (std::rand() % 2 == 0) { // Padrão aleatório
                Block block;
                block.shape.setSize(sf::Vector2f(BLOCK_WIDTH, BLOCK_HEIGHT));
                block.shape.setPosition(col * (BLOCK_WIDTH + 5) + 20, row * (BLOCK_HEIGHT + 5) + 20);
                int randomValue = std::rand() % 3;
                if (randomValue == 0) {
                    block.hitPoints = 3; // Vermelho = 3 acertos
                    block.shape.setFillColor(sf::Color::Red);
                } else if (randomValue == 1) {
                    block.hitPoints = 2; // Amarelo = 2 acertos
                    block.shape.setFillColor(sf::Color::Yellow);
                } else {
                    block.hitPoints = 1; // Verde = 1 acerto
                    block.shape.setFillColor(sf::Color::Green);
                }
                updateBlockColor(block);
                blocks.push_back(block);
            }
        }
    }
}

int main() {
    // Solicita o nome do jogador
    std::cout << "Digite seu nome: ";
    std::getline(std::cin, playerName);

    // Inicializa a semente do gerador de números aleatórios
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Cria a janela do jogo
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Arkanoid Game");
    window.setFramerateLimit(60);

    // Variáveis para o menu e o estado do jogo
    bool inMenu = true;
    bool inGame = false;
    bool showInstructions = false;
    bool showAbout = false;
    bool gameWon = false;
    bool gameOver = false;
    bool autoPlay = false;
    int difficulty = 1;

    // Fonte para o texto
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        return -1; // Erro ao carregar a fonte
    }

    // Textos do menu
    sf::Text titleText;
    titleText.setFont(font);
    titleText.setCharacterSize(48);
    titleText.setFillColor(sf::Color::White);
    titleText.setString("Arkanoid Game");
    titleText.setPosition(WINDOW_WIDTH / 4, 50);

    sf::Text startText;
    startText.setFont(font);
    startText.setCharacterSize(36);
    startText.setFillColor(sf::Color::White);
    startText.setString("1. Iniciar Jogo");
    startText.setPosition(WINDOW_WIDTH / 4, 150);

    sf::Text instructionsText;
    instructionsText.setFont(font);
    instructionsText.setCharacterSize(36);
    instructionsText.setFillColor(sf::Color::White);
    instructionsText.setString("2. Como Jogar");
    instructionsText.setPosition(WINDOW_WIDTH / 4, 220);

    sf::Text aboutText;
    aboutText.setFont(font);
    aboutText.setCharacterSize(36);
    aboutText.setFillColor(sf::Color::White);
    aboutText.setString("3. Sobre o Jogo");
    aboutText.setPosition(WINDOW_WIDTH / 4, 290);

    sf::Text difficultyText;
    difficultyText.setFont(font);
    difficultyText.setCharacterSize(36);
    difficultyText.setFillColor(sf::Color::White);
    difficultyText.setString("4. Escolher Dificuldade");
    difficultyText.setPosition(WINDOW_WIDTH / 4, 360);

    sf::Text autoPlayText;
    autoPlayText.setFont(font);
    autoPlayText.setCharacterSize(36);
    autoPlayText.setFillColor(sf::Color::White);
    autoPlayText.setString("5. Jogar Sozinho");
    autoPlayText.setPosition(WINDOW_WIDTH / 4, 430);

    sf::Text backText;
    backText.setFont(font);
    backText.setCharacterSize(30);
    backText.setFillColor(sf::Color::White);
    backText.setString("Pressione B para voltar ao menu");
    backText.setPosition(WINDOW_WIDTH / 4, WINDOW_HEIGHT - 100);

    sf::Text instructionsDetails;
    instructionsDetails.setFont(font);
    instructionsDetails.setCharacterSize(24);
    instructionsDetails.setFillColor(sf::Color::White);
    instructionsDetails.setString("Use as setas para mover o paddle;\nPressione espaco para lancar a bola;\nQuebre todos os blocos para vencer;\nItens Especiais:\n1. VIDA EXTRA = VERDE\n2. AUMENTO DE PADDLE = AZUL\n3. DIMINUICAO DE PADDLE = VERMELHO\n4. PONTOS EXTRAS = AMARELO\n5. MULTIPLICADOR DE PONTOS = MAGENTA");
    instructionsDetails.setPosition(WINDOW_WIDTH / 4, 150);

    sf::Text aboutDetails;
    aboutDetails.setFont(font);
    aboutDetails.setCharacterSize(24);
    aboutDetails.setFillColor(sf::Color::White);
    aboutDetails.setString("Arkanoid é um jogo eletrônico para arcade\ndesenvolvido pela Taito em 1986.\nO jogo é baseado em Breakout\njogo da Atari também para arcade lançado em 1976.\nDesenvolvedores:\nGustavo Fonseca, Nicoly Colutti, Maria Eduarda Lima\nTecnologias utilizadas:\nSFML e C++");

    // Calcule a posição centralizada
    sf::FloatRect textBounds = aboutDetails.getGlobalBounds();
    aboutDetails.setPosition((WINDOW_WIDTH - textBounds.width) / 2, 150);


    sf::Text difficultyLevelText;
    difficultyLevelText.setFont(font);
    difficultyLevelText.setCharacterSize(24);
    difficultyLevelText.setFillColor(sf::Color::White);
    difficultyLevelText.setPosition(WINDOW_WIDTH / 4, 500);

    sf::Text victoryText;
    victoryText.setFont(font);
    victoryText.setCharacterSize(36);
    victoryText.setFillColor(sf::Color::White);
    victoryText.setString("VITORIA! Parabens, voce ganhou!\nClique 1 para ir para a proxima fase\nClique 2 para voltar ao menu principal");
    victoryText.setPosition(WINDOW_WIDTH / 6, WINDOW_HEIGHT / 2 - 50);

    // Cria o paddle
    sf::RectangleShape paddle(sf::Vector2f(PADDLE_WIDTH, PADDLE_HEIGHT));
    paddle.setFillColor(sf::Color::Blue);
    paddle.setPosition(WINDOW_WIDTH / 2 - PADDLE_WIDTH / 2, WINDOW_HEIGHT - PADDLE_HEIGHT - 50);

    // Criação do texto de pontuação e vidas
    int lives = INITIAL_LIVES;
    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(WINDOW_WIDTH - 200, WINDOW_HEIGHT - 40);
    sf::Text livesText;
    livesText.setFont(font);
    livesText.setCharacterSize(24);
    livesText.setFillColor(sf::Color::White);
    livesText.setPosition(10, WINDOW_HEIGHT - 40);

    sf::Text timeText;
    timeText.setFont(font);
    timeText.setCharacterSize(24);
    timeText.setFillColor(sf::Color::White);
    float centerX = (livesText.getPosition().x + scoreText.getPosition().x) / 2;
    timeText.setPosition(centerX - timeText.getLocalBounds().width / 2, WINDOW_HEIGHT - 40);

    // Cria a bola, inicialmente posicionada no paddle
    sf::CircleShape ball(BALL_RADIUS);
    ball.setFillColor(sf::Color::Red);
    ball.setPosition(paddle.getPosition().x + PADDLE_WIDTH / 2 - BALL_RADIUS, paddle.getPosition().y - BALL_RADIUS * 2);
    sf::Vector2f ballVelocity(0, 0); // A bola começa parada

    // Cria os blocos
    std::vector<Block> blocks;
    loadNextPhase(blocks, paddle, difficulty, currentPhase);

    std::vector<SpecialItem> specialItems;
    sf::Clock clock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (inMenu) {
                if (event.type == sf::Event::KeyPressed) {
                    switch (event.key.code) {
                        case sf::Keyboard::Num1:
                            inMenu = false;
                            inGame = true;
                            lives = INITIAL_LIVES;
                            score = 0;
                            currentPhase = 1;
                            scoreSaved = false;
                            loadNextPhase(blocks, paddle, difficulty, currentPhase);
                            clock.restart(); // Reinicia o relógio ao iniciar o jogo
                            break;
                        case sf::Keyboard::Num2:
                            inMenu = false;
                            showInstructions = true;
                            break;
                        case sf::Keyboard::Num3:
                            inMenu = false;
                            showAbout = true;
                            break;
                        case sf::Keyboard::Num4:
                            difficulty = (difficulty % 3) + 1; // Alterna entre 1, 2, 3
                            break;
                        case sf::Keyboard::Num5:
                            inMenu = false;
                            inGame = true;
                            autoPlay = true;
                            lives = INITIAL_LIVES;
                            score = 0;
                            currentPhase = 1;
                            scoreSaved = false;
                            loadNextPhase(blocks, paddle, difficulty, currentPhase);
                            clock.restart(); // Reinicia o relógio ao iniciar o jogo
                            break;
                        default:
                            break;
                    }
                }
            } else if (showInstructions || showAbout) {
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::B) {
                    inMenu = true;
                    showInstructions = false;
                    showAbout = false;
                }
            }

            if (inGame) {
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space && ballVelocity == sf::Vector2f(0, 0)) {
                    float velocity = BALL_VELOCITY * (0.75f + 0.25f * difficulty);
                    ballVelocity = sf::Vector2f(velocity, -velocity);
                }
            }

            if (gameWon || gameOver) {
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Num1) {
                        if (currentPhase < maxPhases) {
                            currentPhase++;
                            inGame = true;
                            gameWon = false;
                            ballVelocity = sf::Vector2f(0, 0);
                            loadNextPhase(blocks, paddle, difficulty, currentPhase);
                            clock.restart(); // Reinicia o relógio para a nova fase
                            scoreSaved = false;
                        }
                    } else if (event.key.code == sf::Keyboard::Num2) {
                        inMenu = true;
                        gameWon = false;
                        gameOver = false;
                        saveSortedScores();
                    }
                }
            }
        }

        // Menu inicial
        if (inMenu) {
            window.clear(sf::Color::Black);
            window.draw(titleText);
            window.draw(startText);
            window.draw(instructionsText);
            window.draw(aboutText);
            window.draw(difficultyText);
            window.draw(autoPlayText);

            difficultyLevelText.setString("Pressione 4 para mudar a dificuldade: " + std::to_string(difficulty));
            window.draw(difficultyLevelText);

            window.display();
            continue;
        }

        // Instruções
        if (showInstructions) {
            window.clear(sf::Color::Black);
            window.draw(instructionsDetails);
            window.draw(backText);
            window.display();
            continue;
        }

        // Sobre o jogo
        if (showAbout) {
            window.clear(sf::Color::Black);
            window.draw(aboutDetails);
            window.draw(backText);
            window.display();
            continue;
        }

        if (inGame) {
            float elapsedTime = clock.getElapsedTime().asSeconds();

            // Movimento do paddle
            if (!autoPlay) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && paddle.getPosition().x > 0) {
                    paddle.move(-PADDLE_VELOCITY, 0);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && paddle.getPosition().x + paddle.getSize().x < WINDOW_WIDTH) {
                    paddle.move(PADDLE_VELOCITY, 0);
                }
            } else {
                // Movimento automático do paddle para vencer
                float ballCenterX = ball.getPosition().x + BALL_RADIUS;
                float paddleCenterX = paddle.getPosition().x + paddle.getSize().x / 2;
                if (ballCenterX < paddleCenterX) {
                    paddle.move(-PADDLE_VELOCITY, 0);
                } else if (ballCenterX > paddleCenterX) {
                    paddle.move(PADDLE_VELOCITY, 0);
                }
            }

            // Movimento da bola
            if (ballVelocity != sf::Vector2f(0, 0)) {
                // Verifica se a bola passou do limite inferior (perde uma vida)
                if (ball.getPosition().y > WINDOW_HEIGHT) {
                    lives--;
                    if (lives <= 0) {
                        inGame = false;
                        gameOver = true;
                        if (!scoreSaved) {
                            saveScore(playerName, score, lives, elapsedTime);
                            scoreSaved = true;
                        }
                    } else {
                        ball.setPosition(paddle.getPosition().x + PADDLE_WIDTH / 2 - BALL_RADIUS, paddle.getPosition().y - BALL_RADIUS * 2);
                        ballVelocity = sf::Vector2f(0, 0); // Reseta a bola
                    }
                }
                ball.move(ballVelocity);
            } else {
                // Mantém a bola no paddle até ser lançada
                ball.setPosition(paddle.getPosition().x + PADDLE_WIDTH / 2 - BALL_RADIUS, paddle.getPosition().y - BALL_RADIUS * 2);
            }

            // Colisão da bola com as bordas da janela
            if (ball.getPosition().x <= 0 || ball.getPosition().x + BALL_RADIUS * 2 >= WINDOW_WIDTH) {
                ballVelocity.x = -ballVelocity.x;
            }
            if (ball.getPosition().y <= 0) {
                ballVelocity.y = -ballVelocity.y;
            }

            // Colisão da bola com o paddle
            if (ball.getGlobalBounds().intersects(paddle.getGlobalBounds())) {
                ballVelocity.y = -ballVelocity.y;
                ball.setPosition(ball.getPosition().x, paddle.getPosition().y - BALL_RADIUS * 2);
            }

            // Colisão da bola com os blocos
            for (auto it = blocks.begin(); it != blocks.end();) {
                if (ball.getGlobalBounds().intersects(it->shape.getGlobalBounds())) {
                    ballVelocity.y = -ballVelocity.y;
                    it->hitPoints--;

                    // Verifica se o bloco foi destruído
                    if (it->hitPoints <= 0) {
                        // Chance de 25% de gerar um item especial
                        if (std::rand() % 4 == 0) {
                            SpecialItem newItem = createSpecialItem(it->shape.getPosition().x, it->shape.getPosition().y);
                            specialItems.push_back(newItem);
                        }
                        it = blocks.erase(it);
                        score += 100 * scoreMultiplier; // Pontos com o multiplicador
                    } else {
                        updateBlockColor(*it);
                        ++it;
                    }
                } else {
                    ++it;
                }
            }
            // Movimentação dos itens especiais
            for (auto it = specialItems.begin(); it != specialItems.end();) {
                it->shape.move(0, 2.0f); // Velocidade de queda dos itens
                if (it->shape.getGlobalBounds().intersects(paddle.getGlobalBounds())) {
                    // Aplica o efeito do item
                    switch (it->type) {
                        case ItemType::ExtraLife:
                            lives++;
                            break;
                        case ItemType::PaddleIncrease:
                            paddle.setSize(sf::Vector2f(paddle.getSize().x + 20, PADDLE_HEIGHT));
                            break;
                        case ItemType::PaddleDecrease:
                            paddle.setSize(sf::Vector2f(std::max(40.0f, paddle.getSize().x - 20), PADDLE_HEIGHT));
                            break;
                        case ItemType::ExtraPoints:
                            score += 500;
                            break;
                        case ItemType::ScoreMultiplier:
                            scoreMultiplier *= 2; // Dobra o multiplicador de pontos
                            break;
                    }
                    it = specialItems.erase(it); // Remove o item após ser pego
                } else if (it->shape.getPosition().y > WINDOW_HEIGHT) {
                    it = specialItems.erase(it); // Remove o item se sair da tela
                } else {
                    ++it;
                }
            }

            // Atualiza o texto da pontuação e das vidas
            std::stringstream ss;
            ss << "Pontuação: " << score;
            scoreText.setString(ss.str());
            ss.str("");
            ss << "Vidas: " << lives;
            livesText.setString(ss.str());
            ss.str("");
            ss << "Tempo: " << std::fixed << std::setprecision(2) << elapsedTime << "s";
            timeText.setString(ss.str());

            // Renderização
            window.clear(sf::Color::Black);
            window.draw(paddle);
            window.draw(ball);
            for (const auto& block : blocks) {
                window.draw(block.shape);
            }
            for (const auto &item : specialItems) {
                window.draw(item.shape);
            }
            window.draw(scoreText);
            window.draw(livesText);
            window.draw(timeText);
            window.display();
        }

        // Verificação de vitória
        bool allBlocksDestroyed = blocks.empty();
        if (allBlocksDestroyed) {
            inGame = false;
            gameWon = true;
            float elapsedTime = clock.getElapsedTime().asSeconds();
            if (!scoreSaved) {
                saveScore(playerName, score, lives, elapsedTime);
                scoreSaved = true;
            }
        }

        // Game over
        if (gameOver) {
            window.clear(sf::Color::Black);
            sf::Text gameOverText;
            gameOverText.setFont(font);
            gameOverText.setCharacterSize(36);
            gameOverText.setFillColor(sf::Color::Red);
            gameOverText.setString("GAME OVER\nClique 2 para voltar ao menu principal");
            gameOverText.setPosition(WINDOW_WIDTH / 6, WINDOW_HEIGHT / 2 - 50);
            window.draw(gameOverText);
            window.display();
        }

        // Vitória
        if (gameWon) {
            window.clear(sf::Color::Black);
            window.draw(victoryText);
            window.display();
        }
    }

    return 0;
}
