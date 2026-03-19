#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <functional>
#include <optional>

// Структура узла дерева
struct Node {
    int value;
    Node* left;
    Node* right;
    float x, y;

    Node(int val) : value(val), left(nullptr), right(nullptr), x(0), y(0) {}
};

// Создание примера дерева
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

// Рекурсивное удаление дерева
void deleteTree(Node* node) {
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

// Вычисление координат узлов (in-order обход)
void computePositions(Node* node, int& index, float levelY, float vertSpacing, float horSpacing) {
    if (!node) return;
    computePositions(node->left, index, levelY + vertSpacing, vertSpacing, horSpacing);
    node->x = static_cast<float>(index) * horSpacing;
    node->y = levelY;
    index++;
    computePositions(node->right, index, levelY + vertSpacing, vertSpacing, horSpacing);
}

// Рисование рёбер (исправлено для SFML 3)
void drawEdges(sf::RenderWindow& window, Node* node, const sf::Color& color) {
    if (!node) return;
    if (node->left) {
        sf::Vertex line[] = {
            sf::Vertex{ {node->x, node->y}, color },
            sf::Vertex{ {node->left->x, node->left->y}, color }
        };
        window.draw(line, 2, sf::PrimitiveType::Lines);
    }
    if (node->right) {
        sf::Vertex line[] = {
            sf::Vertex{ {node->x, node->y}, color },
            sf::Vertex{ {node->right->x, node->right->y}, color }
        };
        window.draw(line, 2, sf::PrimitiveType::Lines);
    }
    drawEdges(window, node->left, color);
    drawEdges(window, node->right, color);
}

// Рисование узлов (исправлено для SFML 3)
void drawNodes(sf::RenderWindow& window, Node* node, const sf::Font& font) {
    if (!node) return;

    // Круг
    sf::CircleShape circle(25);
    circle.setFillColor(sf::Color(50, 150, 250, 220));
    circle.setOutlineColor(sf::Color::White);
    circle.setOutlineThickness(2);
    circle.setPosition({ node->x - circle.getRadius(), node->y - circle.getRadius() });
    window.draw(circle);

    // Текст
    sf::Text text(font, std::to_string(node->value), 20);
    text.setFillColor(sf::Color::Black);
    text.setStyle(sf::Text::Bold);

    // Центрирование текста (SFML 3: FloatRect имеет .position и .size)
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin({ bounds.position.x + bounds.size.x / 2.0f,
                    bounds.position.y + bounds.size.y / 2.0f });
    text.setPosition({ node->x, node->y });
    window.draw(text);

    drawNodes(window, node->left, font);
    drawNodes(window, node->right, font);
}

int main() {
    // Включение сглаживания
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;

    // Создание окна
    sf::RenderWindow window(sf::VideoMode({ 1200, 800 }), "Binary Tree Visualization",
        sf::Style::Default, sf::State::Windowed, settings);
    window.setFramerateLimit(60);

    // Загрузка шрифта (SFML 3 использует openFromFile)
    sf::Font font;
    if (!font.openFromFile("Arial.ttf")) {
        std::cerr << "Error: cannot load Arial.ttf. Please place it in the executable directory.\n";
        return -1;
    }

    Node* root = createSampleTree();

    // Вычисление позиций узлов
    int index = 0;
    const float vertSpacing = 100.0f;
    const float horSpacing = 80.0f;
    const float startY = 100.0f;
    computePositions(root, index, startY, vertSpacing, horSpacing);

    // Сбор всех узлов для центрирования
    std::vector<Node*> nodes;
    std::function<void(Node*)> collect = [&](Node* n) {
        if (!n) return;
        nodes.push_back(n);
        collect(n->left);
        collect(n->right);
        };
    collect(root);

    // Определение границ дерева
    float minX = nodes[0]->x, maxX = nodes[0]->x;
    float minY = nodes[0]->y, maxY = nodes[0]->y;
    for (Node* n : nodes) {
        if (n->x < minX) minX = n->x;
        if (n->x > maxX) maxX = n->x;
        if (n->y < minY) minY = n->y;
        if (n->y > maxY) maxY = n->y;
    }

    // Центрирование дерева в окне
    sf::Vector2u winSize = window.getSize();
    float offsetX = (winSize.x - (maxX - minX)) / 2.0f - minX;
    float offsetY = (winSize.y - (maxY - minY)) / 2.0f - minY + 20.0f;
    for (Node* n : nodes) {
        n->x += offsetX;
        n->y += offsetY;
    }

    // Главный цикл
    while (window.isOpen()) {
        // Обработка событий (SFML 3)
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
