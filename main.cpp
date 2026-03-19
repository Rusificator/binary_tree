#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <functional>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <string>

// Структура узла дерева
struct Node {
    int value;
    Node* left;
    Node* right;
    float x, y;

    Node(int val) : value(val), left(nullptr), right(nullptr), x(0), y(0) {}
};

// --- Функции для работы с бинарным деревом поиска ---

// Вставка значения в бинарное дерево поиска (без повторов)
void insert(Node*& root, int value) {
    if (root == nullptr) {
        root = new Node(value);
        return;
    }
    if (value < root->value)
        insert(root->left, value);
    else if (value > root->value)
        insert(root->right, value);
    // Если равно, ничего не делаем (игнорируем дубликаты)
}

// Рекурсивное удаление дерева
void deleteTree(Node* node) {
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

// --- Функции для вывода обходов ---

// Прямой обход (Pre-order): корень -> левое -> правое
void printPreOrder(Node* node) {
    if (!node) return;
    std::cout << node->value << " ";
    printPreOrder(node->left);
    printPreOrder(node->right);
}

// Обратный обход (Post-order): левое -> правое -> корень
void printPostOrder(Node* node) {
    if (!node) return;
    printPostOrder(node->left);
    printPostOrder(node->right);
    std::cout << node->value << " ";
}

// --- Функции для графики ---

// Вычисление координат узлов (in-order обход)
void computePositions(Node* node, int& index, float levelY, float vertSpacing, float horSpacing) {
    if (!node) return;
    computePositions(node->left, index, levelY + vertSpacing, vertSpacing, horSpacing);
    node->x = static_cast<float>(index) * horSpacing;
    node->y = levelY;
    index++;
    computePositions(node->right, index, levelY + vertSpacing, vertSpacing, horSpacing);
}

// Рисование рёбер
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

// Рисование узлов
void drawNodes(sf::RenderWindow& window, Node* node, const sf::Font& font) {
    if (!node) return;

    sf::CircleShape circle(25);
    circle.setFillColor(sf::Color(50, 150, 250, 220));
    circle.setOutlineColor(sf::Color::White);
    circle.setOutlineThickness(2);
    circle.setPosition({ node->x - circle.getRadius(), node->y - circle.getRadius() });
    window.draw(circle);

    sf::Text text(font, std::to_string(node->value), 20);
    text.setFillColor(sf::Color::Black);
    text.setStyle(sf::Text::Bold);

    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin({ bounds.position.x + bounds.size.x / 2.0f,
                     bounds.position.y + bounds.size.y / 2.0f });
    text.setPosition({ node->x, node->y });
    window.draw(text);

    drawNodes(window, node->left, font);
    drawNodes(window, node->right, font);
}

// --- Главная функция ---
int main() {
    // ----- Интерактивный ввод данных -----
    int nodeCount = 0;
    std::cout << "Enter the number of nodes in the binary tree: ";
    std::cin >> nodeCount;
    if (nodeCount <= 0) {
        std::cerr << "Invalid number. Exiting.\n";
        return 1;
    }

    std::cout << "Choose input method:\n";
    std::cout << "1. Enter values manually\n";
    std::cout << "2. Generate random values\n";
    int choice;
    std::cin >> choice;

    std::vector<int> values;
    if (choice == 1) {
        // Ручной ввод
        std::cout << "Enter " << nodeCount << " integer values (separated by spaces): ";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // очистка буфера
        std::string line;
        std::getline(std::cin, line);
        std::istringstream iss(line);
        int val;
        while (iss >> val) {
            values.push_back(val);
        }
        if (values.size() != nodeCount) {
            std::cerr << "Warning: you entered " << values.size() << " numbers, expected " << nodeCount << ". Will use all entered.\n";
            nodeCount = values.size(); // подстраиваемся под реальное количество
        }
    }
    else if (choice == 2) {
        // Генерация случайных уникальных чисел
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(1, 99); // числа от 1 до 99

        std::set<int> uniqueVals;
        while (uniqueVals.size() < nodeCount) {
            uniqueVals.insert(dist(gen));
        }
        values.assign(uniqueVals.begin(), uniqueVals.end());
        // Перемешаем, чтобы порядок вставки не был отсортирован (иначе дерево выродится)
        std::shuffle(values.begin(), values.end(), gen);
        std::cout << "Generated values: ";
        for (int v : values) std::cout << v << " ";
        std::cout << std::endl;
    }
    else {
        std::cerr << "Invalid choice. Exiting.\n";
        return 1;
    }

    // Строим бинарное дерево поиска
    Node* root = nullptr;
    for (int v : values) {
        insert(root, v);
    }

    // Выводим обходы для проверки
    std::cout << "\nPre-order traversal (прямой обход):  ";
    printPreOrder(root);
    std::cout << std::endl;

    std::cout << "Post-order traversal (обратный обход): ";
    printPostOrder(root);
    std::cout << std::endl << std::endl;

    // ----- Графическая часть -----
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode({ 1200, 800 }), "Binary Tree Visualization",
        sf::Style::Default, sf::State::Windowed, settings);
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("Arial.ttf")) {
        std::cerr << "Error: cannot load Arial.ttf. Please place it in the executable directory.\n";
        return -1;
    }

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
