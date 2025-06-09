// SFML Breakout Final Version with Main Menu
// Features: Lives, Pause, Volume Control, Power-ups, High Score, Win Detection, Start Menu

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

// Enum for game state
enum class GameState { Menu, Playing, Paused, GameOver, Win };

// Enum for power-up types
enum class PowerUpType { ExpandPaddle, SlowBall, ExtraLife };

struct PowerUp {
    sf::CircleShape shape;
    PowerUpType type;
    sf::Vector2f velocity = {0.f, 150.f};

    PowerUp(sf::Vector2f pos, PowerUpType type) : type(type) {
        shape.setRadius(10.f);
        shape.setPosition(pos);
        if (type == PowerUpType::ExpandPaddle) shape.setFillColor(sf::Color::Green);
        else if (type == PowerUpType::SlowBall) shape.setFillColor(sf::Color::Cyan);
        else if (type == PowerUpType::ExtraLife) shape.setFillColor(sf::Color::Magenta);
    }

    void update(float dt) { shape.move(velocity * dt); }
};

std::vector<sf::RectangleShape> makeBricks() {
    std::vector<sf::RectangleShape> bricks;
    for (int i = 0; i < 5; ++i)
        for (int j = 0; j < 10; ++j) {
            sf::RectangleShape brick(sf::Vector2f(60.f - 4, 20.f - 4));
            brick.setFillColor(sf::Color::Red);
            brick.setPosition(60.f + j * 60, 40.f + i * 20);
            bricks.push_back(brick);
        }
    return bricks;
}

int loadHighScore() {
    std::ifstream in("highscore.txt");
    int hs = 0;
    in >> hs;
    return hs;
}

void saveHighScore(int score) {
    std::ofstream out("highscore.txt");
    out << score;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Breakout Game Final");
    GameState gameState = GameState::Menu;
    int score = 0, highScore = loadHighScore(), lives = 3, volume = 50;

    // Font
    sf::Font font;
    font.loadFromFile("Arapey-Regular.ttf");

    sf::Text scoreText("", font, 20);
    scoreText.setPosition(10, 10);

    sf::Text lifeText("", font, 20);
    lifeText.setPosition(10, 40);

    sf::Text volumeText("", font, 20);
    volumeText.setPosition(600, 10);

    sf::Text highScoreText("", font, 20);
    highScoreText.setPosition(600, 40);

    sf::Text pauseText("Paused - Press P to Resume", font, 24);
    pauseText.setFillColor(sf::Color::Yellow);
    pauseText.setPosition(220, 300);

    sf::Text gameOverText("Game Over - Press Enter", font, 30);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setPosition(220, 300);

    sf::Text winText("You Win! Press Enter to Continue", font, 30);
    winText.setFillColor(sf::Color::Green);
    winText.setPosition(180, 300);

    sf::Text menuText("Press Enter to Play", font, 28);
    menuText.setFillColor(sf::Color::White);
    menuText.setPosition(250, 280);

    // Shapes
    sf::RectangleShape paddle(sf::Vector2f(100.f, 20.f));
    paddle.setFillColor(sf::Color::Green);
    paddle.setPosition(350.f, 550.f);

    sf::CircleShape ball(10.f);
    ball.setFillColor(sf::Color::White);
    ball.setPosition(400.f, 300.f);
    sf::Vector2f ballVelocity(300.f, -300.f);

    std::vector<sf::RectangleShape> bricks = makeBricks();
    std::vector<PowerUp> powerUps;

    // Audio
    sf::SoundBuffer bounceBuffer;
    bounceBuffer.loadFromFile("bounce1.wav");
    sf::Sound bounceSound;
    bounceSound.setBuffer(bounceBuffer);

    sf::Music music;
    music.openFromFile("menu_music.ogg");
    music.setLoop(true);
    music.setVolume(volume);
    music.play();

    sf::Clock clock;
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (gameState == GameState::Menu && event.key.code == sf::Keyboard::Enter) {
                    gameState = GameState::Playing;
                    lives = 3;
                    score = 0;
                    bricks = makeBricks();
                    paddle.setSize(sf::Vector2f(100.f, 20.f));
                    ball.setPosition(400.f, 300.f);
                    ballVelocity = {300.f, -300.f};
                }
                if (event.key.code == sf::Keyboard::P && gameState == GameState::Playing)
                    gameState = GameState::Paused;
                else if (event.key.code == sf::Keyboard::P && gameState == GameState::Paused)
                    gameState = GameState::Playing;
                if (event.key.code == sf::Keyboard::Enter && (gameState == GameState::GameOver || gameState == GameState::Win)) {
                    gameState = GameState::Menu;
                }
                if (event.key.code == sf::Keyboard::Add || event.key.code == sf::Keyboard::Equal) {
                    volume = std::min(100, volume + 10);
                    music.setVolume(volume);
                }
                if (event.key.code == sf::Keyboard::Subtract || event.key.code == sf::Keyboard::Dash) {
                    volume = std::max(0, volume - 10);
                    music.setVolume(volume);
                }
            }
        }

        if (gameState == GameState::Playing) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && paddle.getPosition().x > 0)
                paddle.move(-400.f * dt, 0);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && paddle.getPosition().x + paddle.getSize().x < 800)
                paddle.move(400.f * dt, 0);

            ball.move(ballVelocity * dt);

            if (ball.getPosition().x <= 0 || ball.getPosition().x + 20 >= 800)
                ballVelocity.x *= -1;
            if (ball.getPosition().y <= 0)
                ballVelocity.y *= -1;

            if (ball.getGlobalBounds().intersects(paddle.getGlobalBounds())) {
                ballVelocity.y *= -1;
                bounceSound.play();
            }

            for (auto it = bricks.begin(); it != bricks.end();) {
                if (ball.getGlobalBounds().intersects(it->getGlobalBounds())) {
                    bounceSound.play();
                    if (rand() % 5 == 0)
                        powerUps.emplace_back(it->getPosition(), static_cast<PowerUpType>(rand() % 3));
                    it = bricks.erase(it);
                    score += 10;
                    ballVelocity.y *= -1;
                    break;
                } else ++it;
            }

            if (bricks.empty()) {
                gameState = GameState::Win;
                if (score > highScore) {
                    highScore = score;
                    saveHighScore(score);
                }
            }

            for (auto it = powerUps.begin(); it != powerUps.end();) {
                it->update(dt);
                if (it->shape.getGlobalBounds().intersects(paddle.getGlobalBounds())) {
                    if (it->type == PowerUpType::ExpandPaddle) paddle.setSize(sf::Vector2f(150.f, 20.f));
                    if (it->type == PowerUpType::SlowBall) ballVelocity *= 0.8f;
                    if (it->type == PowerUpType::ExtraLife) ++lives;
                    it = powerUps.erase(it);
                } else if (it->shape.getPosition().y > 600) {
                    it = powerUps.erase(it);
                } else ++it;
            }

            if (ball.getPosition().y > 600) {
                --lives;
                if (lives == 0) {
                    gameState = GameState::GameOver;
                    if (score > highScore) {
                        highScore = score;
                        saveHighScore(score);
                    }
                }
                ball.setPosition(400.f, 300.f);
                paddle.setSize(sf::Vector2f(100.f, 20.f));
                ballVelocity = {300.f, -300.f};
            }
        }

        scoreText.setString("Score: " + std::to_string(score));
        lifeText.setString("Lives: " + std::to_string(lives));
        volumeText.setString("Volume: " + std::to_string(volume));
        highScoreText.setString("High: " + std::to_string(highScore));

        window.clear();

        if (gameState == GameState::Menu) {
            window.draw(menuText);
        } else {
            window.draw(paddle);
            window.draw(ball);
            for (auto& b : bricks) window.draw(b);
            for (auto& p : powerUps) window.draw(p.shape);
            window.draw(scoreText);
            window.draw(lifeText);
            window.draw(volumeText);
            window.draw(highScoreText);
            if (gameState == GameState::Paused) window.draw(pauseText);
            if (gameState == GameState::GameOver) window.draw(gameOverText);
            if (gameState == GameState::Win) window.draw(winText);
        }

        window.display();
    }

    return 0;
}
