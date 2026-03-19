#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <memory>
#include <functional>   // для std::function

// Структура узла бинарного дерева
struct Node {
    int value;
    Node* left;
    Node* right;
    float x, y;

    Node(int val) : value(val), left(nullptr), right(nullptr), x(0), y(0) {}
};

// Пример построения дерева
Node* createSampleTree() {
    Node* root = new Node(5);
    root->left = new Node(3);
    root->right = new Node(8);
    root->left->left = new Node(2);
    root->left->right = new Node(4);
    root->right->right = new Node(9);
    root->left->left->left = new Node(1);
    root->right->right->left = new Node(7);
    return root;
}

void deleteTree(Node* node) {
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

// Рекурсивное назначение координат (in-order)
void computePositions(Node* node, int& index, float levelY, float vertSpacing, float horSpacing) {
    if (!node) return;
    computePositions(node->left, index, levelY + vertSpacing, vertSpacing, horSpacing);
    node->x = index * horSpacing;
    node->y = levelY;
    index++;
    computePositions(node->right, index, levelY + vertSpacing, vertSpacing, horSpacing);
}

// Рисование рёбер (SFML 3: используем sf::PrimitiveType::Lines)
void drawEdges(sf::RenderWindow& window, Node* node, const sf::Color& color) {
    if (!node) return;
    if (node->left) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(node->x, node->y), color),
            sf::Vertex(sf::Vector2f(node->left->x, node->left->y), color)
        };
        window.draw(line, 2, sf::PrimitiveType::Lines);
    }
    if (node->right) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(node->x, node->y), color),
            sf::Vertex(sf::Vector2f(node->right->x, node->right->y), color)
        };
        window.draw(line, 2, sf::PrimitiveType::Lines);
    }
    drawEdges(window, node->left, color);
    drawEdges(window, node->right, color);
}

// Рисование узлов (SFML 3: получение границ текста через getLocalBounds(), позиционирование)
void drawNodes(sf::RenderWindow& window, Node* node, const sf::Font& font) {
    if (!node) return;

    sf::CircleShape circle(25);
    circle.setFillColor(sf::Color(50, 150, 250, 220));
    circle.setOutlineColor(sf::Color::White);
    circle.setOutlineThickness(2);
    circle.setPosition(node->x - circle.getRadius(), node->y - circle.getRadius());
    window.draw(circle);

    sf::Text text;
    text.setFont(font);
    text.setString(std::to_string(node->value));
    text.setCharacterSize(20);
    text.setFillColor(sf::Color::Black);
    text.setStyle(sf::Text::Bold);

    // Центрирование текста: используем getLocalBounds() (в SFML 3 он возвращает sf::FloatRect с полями position и size)
    sf::FloatRect textBounds = text.getLocalBounds();
    // В SFML 3 у sf::FloatRect есть методы .position.x, .size.x и т.д.
    text.setOrigin(textBounds.position.x + textBounds.size.x / 2.0f,
        textBounds.position.y + textBounds.size.y / 2.0f);
    text.setPosition(node->x, node->y);
    window.draw(text);

    drawNodes(window, node->left, font);
    drawNodes(window, node->right, font);
}

int main() {
    // Настройки окна (SFML 3: antialiasingLevel -> antiAliasingLevel)
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;      // изменилось имя
    // SFML 3: конструктор sf::VideoMode принимает sf::Vector2u или два числа в фигурных скобках
    sf::RenderWindow window(sf::VideoMode({ 1200, 800 }), "Binary Tree Visualization",
        sf::Style::Default, sf::State::Windowed, settings);
    window.setFramerateLimit(60);

    // Загрузка шрифта (путь может отличаться; лучше положить Arial.ttf в папку с проектом)
    sf::Font font;
    if (!font.loadFromFile("Arial.ttf")) {
        std::cerr << "Error loading font. Make sure Arial.ttf is in the working directory.\n";
        return -1;
    }

    Node* root = createSampleTree();

    // Вычисление позиций узлов
    int index = 0;
    float vertSpacing = 100.0f;
    float horSpacing = 80.0f;
    float startY = 100.0f;
    computePositions(root, index, startY, vertSpacing, horSpacing);

    // Сбор всех узлов для центрирования
    std::vector<Node*> nodes;
    // Явно объявляем тип функции для лямбды
    std::function<void(Node*)> collect = [&](Node* n) {
        if (!n) return;
        nodes.push_back(n);
        collect(n->left);
        collect(n->right);
        };
    collect(root);

    // Вычисление минимальных/максимальных координат
    float minX = nodes[0]->x, maxX = nodes[0]->x;
    float minY = nodes[0]->y, maxY = nodes[0]->y;
    for (Node* n : nodes) {
        if (n->x < minX) minX = n->x;
        if (n->x > maxX) maxX = n->x;
        if (n->y < minY) minY = n->y;
        if (n->y > maxY) maxY = n->y;
    }

    float offsetX = (window.getSize().x - (maxX - minX)) / 2.0f - minX;
    float offsetY = (window.getSize().y - (maxY - minY)) / 2.0f - minY + 20;
    for (Node* n : nodes) {
        n->x += offsetX;
        n->y += offsetY;
    }

    // Главный цикл (SFML 3: обработка событий через getSize() и т.д.)
    while (window.isOpen()) {
        // В SFML 3 цикл событий можно организовать так же
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        window.clear(sf::Color(30, 30, 30));
        drawEdges(window, root, sf::Color(200, 200, 200, 150));
        drawNodes(window, root, font);
        window.display();
    }

    deleteTree(root);
    return 0;
}