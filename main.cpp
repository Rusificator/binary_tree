#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <functional>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <clocale>
using namespace std;

// Структура узла дерева
struct Node {
    int value;
    Node* left;
    Node* right;
    float x, y;

    Node(int val) : value(val), left(nullptr), right(nullptr), x(0), y(0) {}
};

// --- Функции для работы с бинарным деревом поиска ---

void insert(Node*& root, int value) {
    if (root == nullptr) {
        root = new Node(value);
        return;
    }
    if (value < root->value)
        insert(root->left, value);
    else if (value > root->value)
        insert(root->right, value);
}

void deleteTree(Node* node) {
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

// --- Функции для вывода обходов ---

void printPreOrder(Node* node) {
    if (!node) return;
    cout << node->value << " ";
    printPreOrder(node->left);
    printPreOrder(node->right);
}

void printPostOrder(Node* node) {
    if (!node) return;
    printPostOrder(node->left);
    printPostOrder(node->right);
    cout << node->value << " ";
}

// --- Функции для графики ---

void computePositions(Node* node, int& index, float levelY, float vertSpacing, float horSpacing) {
    if (!node) return;
    computePositions(node->left, index, levelY + vertSpacing, vertSpacing, horSpacing);
    node->x = static_cast<float>(index) * horSpacing;
    node->y = levelY;
    index++;
    computePositions(node->right, index, levelY + vertSpacing, vertSpacing, horSpacing);
}

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

void drawNodes(sf::RenderWindow& window, Node* node, const sf::Font& font) {
    if (!node) return;

    sf::CircleShape circle(25);
    circle.setFillColor(sf::Color(50, 150, 250, 220));
    circle.setOutlineColor(sf::Color::White);
    circle.setOutlineThickness(2);
    circle.setPosition({ node->x - circle.getRadius(), node->y - circle.getRadius() });
    window.draw(circle);

    sf::Text text(font, to_string(node->value), 20);
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
    setlocale(LC_ALL, "Russian");

    // ----- Интерактивный ввод данных -----
    int nodeCount = 0;
    cout << "Введите количество узлов в бинарном дереве: ";
    cin >> nodeCount;
    if (nodeCount <= 0) {
        cerr << "Неверное количество. Выход.\n";
        return 1;
    }

    cout << "Выберите способ ввода:\n";
    cout << "1. Ввести значения вручную\n";
    cout << "2. Сгенерировать случайные значения\n";
    int choice;
    cin >> choice;

    vector<int> values;
    if (choice == 1) {
        cout << "Введите " << nodeCount << " целых чисел (через пробел): ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string line;
        getline(cin, line);
        istringstream iss(line);
        int val;
        while (iss >> val) {
            values.push_back(val);
        }
        if (values.size() != nodeCount) {
            cerr << "Предупреждение: вы ввели " << values.size() << " чисел, ожидалось " << nodeCount << ". Будут использованы все введённые.\n";
            nodeCount = values.size();
        }
    }
    else if (choice == 2) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dist(1, 99);

        set<int> uniqueVals;
        while (uniqueVals.size() < nodeCount) {
            uniqueVals.insert(dist(gen));
        }
        values.assign(uniqueVals.begin(), uniqueVals.end());
        shuffle(values.begin(), values.end(), gen);
        cout << "Сгенерированные значения: ";
        for (int v : values) cout << v << " ";
        cout << endl;
    }
    else {
        cerr << "Неверный выбор. Выход.\n";
        return 1;
    }

    // Строим бинарное дерево поиска
    Node* root = nullptr;
    for (int v : values) {
        insert(root, v);
    }

    cout << "\nПрямой обход (pre-order):  ";
    printPreOrder(root);
    cout << endl;

    cout << "Обратный обход (post-order): ";
    printPostOrder(root);
    cout << endl << endl;

    // ----- Подготовка графики -----
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode({ 1200, 800 }), "Визуализация бинарного дерева",
        sf::Style::Default, sf::State::Windowed, settings);
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("Arial.ttf")) {
        cerr << "Ошибка: не удалось загрузить шрифт Arial.ttf. Поместите его в папку с программой.\n";
        return -1;
    }

    // Вычисляем позиции узлов в логических координатах
    int index = 0;
    const float vertSpacing = 100.0f;
    const float horSpacing = 80.0f;
    const float startY = 100.0f;
    computePositions(root, index, startY, vertSpacing, horSpacing);

    // Собираем все узлы для определения границ
    vector<Node*> nodes;
    function<void(Node*)> collect = [&](Node* n) {
        if (!n) return;
        nodes.push_back(n);
        collect(n->left);
        collect(n->right);
        };
    collect(root);

    // Определяем минимальные и максимальные координаты узлов (bounding box дерева)
    float minX = nodes[0]->x, maxX = nodes[0]->x;
    float minY = nodes[0]->y, maxY = nodes[0]->y;
    for (Node* n : nodes) {
        if (n->x < minX) minX = n->x;
        if (n->x > maxX) maxX = n->x;
        if (n->y < minY) minY = n->y;
        if (n->y > maxY) maxY = n->y;
    }

    // Добавляем отступы вокруг дерева (например, по 100 пикселей)
    float padding = 100.0f;
    float treeWidth = maxX - minX + 2 * padding;
    float treeHeight = maxY - minY + 2 * padding;
    sf::Vector2f treeCenter((minX + maxX) / 2.0f, (minY + maxY) / 2.0f);

    // Создаём вид, который будет показывать всё дерево с отступами
    sf::View view(treeCenter, sf::Vector2f(treeWidth, treeHeight));
    window.setView(view);

    // Переменные для интерактивности
    bool isDragging = false;
    sf::Vector2f lastMouseWorld; // последняя позиция мыши в мировых координатах
    sf::Vector2f initialViewCenter = view.getCenter();
    sf::Vector2f initialViewSize = view.getSize();

    // Главный цикл
    while (window.isOpen()) {
        // Обработка событий
        while (const optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            else if (auto* mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
                // Если зажата левая кнопка, перемещаем вид (панорамирование)
                if (isDragging) {
                    // Текущая позиция мыши в мировых координатах
                    sf::Vector2f currentMouseWorld = window.mapPixelToCoords({ mouseMoved->position.x, mouseMoved->position.y }, view);
                    // Разница между предыдущей и текущей позицией в мире
                    sf::Vector2f delta = lastMouseWorld - currentMouseWorld;
                    // Сдвигаем центр вида
                    view.move(delta);
                    lastMouseWorld = currentMouseWorld; // обновляем для следующего шага
                }
            }
            else if (auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mousePressed->button == sf::Mouse::Button::Left) {
                    isDragging = true;
                    // Запоминаем позицию мыши в мировых координатах в момент нажатия
                    lastMouseWorld = window.mapPixelToCoords({ mousePressed->position.x, mousePressed->position.y }, view);
                }
            }
            else if (auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouseReleased->button == sf::Mouse::Button::Left) {
                    isDragging = false;
                }
            }
            else if (auto* mouseWheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (mouseWheel->wheel == sf::Mouse::Wheel::Vertical) {
                    // Коэффициент масштабирования
                    float zoomFactor = (mouseWheel->delta > 0) ? 0.9f : 1.1f; // колёсико вверх (положительное) уменьшаем вид (приближаем), вниз - отдаляем
                    // Позиция мыши в мировых координатах до зумирования
                    sf::Vector2f mouseWorld = window.mapPixelToCoords({ mouseWheel->position.x, mouseWheel->position.y }, view);
                    // Масштабируем вид: новый размер = старый размер * zoomFactor
                    sf::Vector2f newSize = view.getSize() * zoomFactor;
                    view.setSize(newSize);
                    // Чтобы точка под мышью осталась на месте, корректируем центр
                    sf::Vector2f newCenter = mouseWorld + (view.getCenter() - mouseWorld) * zoomFactor;
                    view.setCenter(newCenter);
                }
            }
            else if (auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                // Сброс вида на начальный по клавише R или Home
                if (keyPressed->code == sf::Keyboard::Key::R || keyPressed->code == sf::Keyboard::Key::Home) {
                    view.setCenter(initialViewCenter);
                    view.setSize(initialViewSize);
                }
            }
        }

        // Применяем вид к окну
        window.setView(view);

        // Отрисовка
        window.clear(sf::Color(30, 30, 30));
        drawEdges(window, root, sf::Color(200, 200, 200, 150));
        drawNodes(window, root, font);
        window.display();
    }

    deleteTree(root);
    return 0;
}
