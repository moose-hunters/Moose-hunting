#include "Game.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
static Game* g_gameInstance = nullptr;

Game::Game()
    : m_window(nullptr), m_width(1024), m_height(768), m_cameraPos(0.0f, 1.5f, 8.0f), m_cameraFront(0.0f, 0.0f, -1.0f), m_cameraUp(0.0f, 1.0f, 0.0f), m_yaw(-90.0f), m_pitch(0.0f), m_lastX(512.0f), m_lastY(384.0f), m_firstMouse(true), m_fov(45.0f), m_lightPos(5.0f, 10.0f, 5.0f), m_lightColor(1.0f, 0.95f, 0.85f), m_floorTexture(0), m_floorY(1.5f), m_shootCooldown(0.0f), m_maxCooldown(0.5f), m_wasLMBPressed(false) {
    g_gameInstance = this;
}

Game::~Game() { cleanup(); }

void Game::updateMoose(float dt) {
    m_mooseTimer -= dt;

    // Если время вышло — меняем направление на случайное
    if (m_mooseTimer <= 0.0f) {
        float randomAngle = (rand() % 360) * 3.14159265f / 180.0f; // Угол в радианах
        m_mooseDir.x = cosf(randomAngle);
        m_mooseDir.y = sinf(randomAngle);

        // Логика движения слона - лось будет идти в эту сторону от 2 до 5 секунд
        m_mooseTimer = 2.0f + (rand() % 301) / 100.0f;
    }

    // Двигаем лося
    m_moosePos.x += m_mooseDir.x * m_mooseSpeed * dt;
    m_moosePos.z += m_mooseDir.y * m_mooseSpeed * dt;

    // Привязываем лося к высоте рельефа, чтобы он не летал и не проваливался
    m_moosePos.y = m_terrain.getHeight(m_moosePos.x, m_moosePos.z);
}


// ------- collisions ---------
bool Game::checkTreeCollision(glm::vec3 nextPos) {
    const float playerRadius = 0.3f;
    const float treeRadius = 0.5f;
    const float minDist = playerRadius + treeRadius;

    // Пробегаемся по деревьям
    for (const auto& tree : m_trees) {
        float dx = nextPos.x - tree.pos.x;
        float dz = nextPos.z - tree.pos.z;
        if (dx * dx + dz * dz < minDist * minDist) {
            return true;
        }
    }

    return false;
}

// ---- Init ----
bool Game::init(int width, int height, const char* title) {
    m_width = width;
    m_height = height;
    m_lastX = width / 2.0f;
    m_lastY = height / 2.0f;

    m_window = new Window(width, height, title);
    GLFWwindow* w = m_window->getGLFWwindow();

    std::cout << "OpenGL: " << glGetString(GL_VERSION) << std::endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.35f, 0.55f, 0.75f, 1.0f);  // небо

    m_window->setCursorDisabled(true);
    m_window->setCursorPosCallback(mouseCallback);
    m_window->setScrollCallback(scrollCallback);

    // Шейдеры
    if (!m_shader.load("shaders/tree_vertex.glsl", "shaders/tree_fragment.glsl")) {
        std::cerr << "Failed to load shaders!" << std::endl;
        return false;
    }

    // Подгружаем деревья
    m_treeModels.resize(3);
    m_treeModels[0].load("assets/Tree1.glb", false);
    m_treeModels[1].load("assets/Tree2.glb", false);
    m_treeModels[2].load("assets/Tree3.glb", false);

    // Загружаю куст
    m_bushModel.load("assets/Bush.glb", false);

    // Загружаю лося
    m_mooseModel.load("assets/Moose.glb", true);

    // Загружаю землю
    m_terrain.init("assets/Grass.png");

    m_fenceModel.load("assets/fence.glb", false);
    const float MAP_LIMIT = 50.0f;
    const float FENCE_SPACING = 1.8f; // ПОДБЕРИ ЭТО ЗНАЧЕНИЕ под длину забора

    // 1. Верхняя и нижняя границы (идем по оси X)
    for (float x = -MAP_LIMIT; x <= MAP_LIMIT + FENCE_SPACING / 2; x += FENCE_SPACING) {
        // Верх (z = -20)
        m_fences.push_back({ glm::vec3(x, 0.0f, -MAP_LIMIT), 0.0f, 1.0f });
        // Низ (z = 20)
        m_fences.push_back({ glm::vec3(x, 0.0f, MAP_LIMIT), 180.0f, 1.0f });
    }

    // 2. Левая и правая границы (идем по оси Z)
    // Начинаем с отступом, чтобы секции на углах не накладывались друг на друга
    for (float z = -MAP_LIMIT; z <= MAP_LIMIT; z += FENCE_SPACING) {
        // Лево (x = -20)
        m_fences.push_back({ glm::vec3(-MAP_LIMIT, 0.0f, z), 90.0f, 1.0f });
        // Право (x = 20)
        m_fences.push_back({ glm::vec3(MAP_LIMIT, 0.0f, z), -90.0f, 1.0f });
    }

    std::cout << "Init OK. WASD = move, Mouse = look, ESC = exit" << std::endl;



    for (int i = 0; i < 50; ++i) {
        float tx = (rand() % 10000 / 100.0f) - 50.0f;
        float tz = (rand() % 10000 / 100.0f) - 50.0f;
        float ty = m_terrain.getHeight(tx, tz);

        TreeInstance t;
        t.pos = glm::vec3(tx, ty, tz);
        t.type = rand() % 3;
        t.scale = 0.8f + (rand() % 41) / 100.0f;
        m_trees.push_back(t);
    }

    for (int i = 0; i < 50; ++i) {
        float bx = (rand() % 10000 / 100.0f) - 50.0f;
        float bz = (rand() % 10000 / 100.0f) - 50.0f;
        float by = m_terrain.getHeight(bx, bz);

        BushInstance b;
        b.pos = glm::vec3(bx, m_terrain.getHeight(bx, bz), bz);
        b.scale = (0.6f + (rand() % 81) / 100.0f) * 4;
        m_bushes.push_back(b);
    }

    // Ставим лося в центр карты и даем начальное направление
    m_moosePos = glm::vec3(0.0f, m_terrain.getHeight(0.0f, 0.0f), 0.0f);
    m_mooseDir = glm::normalize(glm::vec2(1.0f, 0.5f));

    // Стартовая позиция камеры на новой поверхности
    m_cameraPos.y = m_terrain.getHeight(m_cameraPos.x, m_cameraPos.z) + m_cameraHeight;
    
    // шейдеры для 2д картинки
    m_uiShader.load("shaders/ui_vertex.glsl", "shaders/ui_fragment.glsl");
    setupUI();

    // спрайты перезарядки
    for (int i = 1; i <= 8; i++) {
        std::string path = "assets/reloading/reload_" + std::to_string(i) + ".png";
        m_gunFrames.push_back(loadTexture(path.c_str()));
    }

    m_gunShader.load("shaders/gun_vertex.glsl", "shaders/gun_fragment.glsl");
    setupGunUI();

    // --- НАСТРОЙКА СЕТИ ---
    // Инициализация сети (без подключения, ждем выбора в меню)
    m_serverIP = "26.189.211.204";
    if (enet_initialize() != 0) return false;
    m_clientHost = enet_host_create(NULL, 1, 2, 0, 0);
    return true;
}

// ---- Loop ----
void Game::run() {
    float last = (float)glfwGetTime();
    while (!m_window->shouldClose()) {
        float now = (float)glfwGetTime();
        float dt = now - last;
        last = now;

        processInput(dt);

        if (m_state == GameState::CONNECTING || m_state == GameState::PLAYING) {
            processNetwork();
        }

        if (m_state == GameState::PLAYING) {
            // updateMoose(dt);
            updateGunAnimation(dt);
        }
        render();
        m_window->swapBuffers();
        m_window->pollEvents();
    }
}

// ---- Input ----
void Game::processInput(float dt) {
    GLFWwindow* w = m_window->getGLFWwindow();

    if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(w, true);

    if (m_state == GameState::SPLASH) {
        m_stateTimer += dt;
        if (m_stateTimer > 1.0f) {
            m_state = GameState::MENU;
            std::cout << "[MENU] Press '1' to play as Moose, '2' to play as Hunter." << std::endl;
        }
    } else if (m_state == GameState::MENU) {
        if (glfwGetKey(w, GLFW_KEY_1) == GLFW_PRESS || glfwGetKey(w, GLFW_KEY_2) == GLFW_PRESS) {
            m_selectedRole = (glfwGetKey(w, GLFW_KEY_1) == GLFW_PRESS) ? EntityType::MOOSE : EntityType::HUNTER;
            m_state = GameState::CONNECTING;

            std::cout << "[CLIENT] Connecting to " << m_serverIP << "..." << std::endl;
            ENetAddress address;
            enet_address_set_host(&address, m_serverIP.c_str());
            address.port = 12345;
            m_serverPeer = enet_host_connect(m_clientHost, &address, 2, 0);
        }
    } else if (m_state == GameState::PLAYING) {
        // Ускорение
        float speed = (glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? 10.0f : 5.0f;
        speed *= dt;

        glm::vec3 flatFront = glm::normalize(glm::vec3(m_cameraFront.x, 0.0f, m_cameraFront.z));
        glm::vec3 right = glm::normalize(glm::cross(flatFront, m_cameraUp));

        auto moveWithCollision = [&](glm::vec3 direction) {
            glm::vec3 nextPos = m_cameraPos + direction;
            if (!checkTreeCollision(nextPos)) {
                m_cameraPos = nextPos;
            }
        };

        if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS) moveWithCollision(flatFront * speed);
        if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS) moveWithCollision(-flatFront * speed);
        if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS) moveWithCollision(-right * speed);
        if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS) moveWithCollision(right * speed);

        const float MAP_LIMIT = 49.9f;

        // Ограничиваем X и Z, чтобы не уйти за края
        m_cameraPos.x = glm::clamp(m_cameraPos.x, -MAP_LIMIT, MAP_LIMIT);
        m_cameraPos.z = glm::clamp(m_cameraPos.z, -MAP_LIMIT, MAP_LIMIT);

        // Прыжок и гравитация
        if (glfwGetKey(w, GLFW_KEY_SPACE) == GLFW_PRESS && m_isGrounded) {
            m_velocityY = m_jumpForce;
            m_isGrounded = false;
        }
        m_velocityY += m_gravity * dt;
        m_cameraPos.y += m_velocityY * dt;

        // Высота берется из Terrain
        float groundY = m_terrain.getHeight(m_cameraPos.x, m_cameraPos.z) + m_cameraHeight;
        if (m_cameraPos.y <= groundY) {
            m_cameraPos.y = groundY;
            m_velocityY = 0.0f;
            m_isGrounded = true;
        }
        // --- Механика стрельбы ---
        if (m_shootCooldown > 0.0f) {
            m_shootCooldown -= dt;
        }

        bool isLMBPressed = glfwGetMouseButton(m_window->getGLFWwindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

        // Обрабатываем только одиночный клик (чтобы нельзя было зажать кнопку)
        if (isLMBPressed && !m_wasLMBPressed) {
            if (m_shootCooldown <= 0.0f) {  // Если перезарядились
                if (checkMooseHit()) {
                    std::cout << "HIT! Moose down!" << std::endl;
                    // Телепортируем лося в случайное место после попадания
                    m_moosePos = glm::vec3((rand() % 40) - 20, 1.5f, (rand() % 40) - 20);
                } else {
                    std::cout << "MISS!" << std::endl;
                }
                if (checkPlayerHit()) {
                    std::cout << "PLAYER HIT!" << std::endl;

                    // Отправляем серверу информацию о попадании
                    PacketHit hit;
                    hit.victimId = 0;  // В простом PvP с 2 игроками ID можно не уточнять
                    ENetPacket* p = enet_packet_create(&hit, sizeof(PacketHit), ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(m_serverPeer, 0, p);
                }
                m_shootCooldown = m_maxCooldown;  // Уходим на перезарядку
            }
        }
        m_wasLMBPressed = isLMBPressed;
    }
}

GLuint Game::loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    int width, height, nrComponents;
    // Для UI текстур лучше не переворачивать картинку по вертикали
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);

    if (data) {
        GLenum format = (nrComponents == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cerr << "Failed to load gun texture: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}

// ---- Camera ----
void Game::updateCamera() {
    glm::vec3 front;
    front.x = cosf(glm::radians(m_yaw)) * cosf(glm::radians(m_pitch));
    front.y = sinf(glm::radians(m_pitch));
    front.z = sinf(glm::radians(m_yaw)) * cosf(glm::radians(m_pitch));
    m_cameraFront = glm::normalize(front);
}

// ---- Render ----
void Game::render() {
    m_window->clear();
    if (m_state == GameState::SPLASH) {
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);  // Темная заставка
    } else if (m_state == GameState::MENU || m_state == GameState::CONNECTING) {
        glClearColor(0.2f, 0.2f, 0.3f, 1.0f);  // Серо-индиго фон меню
    } else if (m_state == GameState::PLAYING) {
        glClearColor(0.35f, 0.55f, 0.75f, 1.0f);  // Игровое небо
        updateCamera();

        glm::mat4 view = glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
        glm::mat4 proj = glm::perspective(glm::radians(m_fov),
            (float)m_width / (float)m_height,
            0.1f, 200.0f);

        m_shader.use();
        m_shader.setMat4("view", view);
        m_shader.setMat4("projection", proj);
        m_shader.setVec3("lightPos", m_lightPos);
        m_shader.setVec3("lightColor", m_lightColor);
        m_shader.setVec3("viewPos", m_cameraPos);
        m_shader.setFloat("ambientStrength", 0.35f);

        m_terrain.render(m_shader);

        // Рисуем кусты
        for (const auto& bush : m_bushes) {
            m_bushModel.render(m_shader, bush.pos, bush.scale);
        }

        // Рисуем деревья
        for (const auto& tree : m_trees) {
            m_treeModels[tree.type].render(m_shader, tree.pos, tree.scale);
        }

        // рисуем лося
        // float angle = glm::degrees(atan2(m_mooseDir.x, m_mooseDir.y));
        // m_mooseModel.render(m_shader, m_moosePos, 0.5f, angle);

        // рисуем забор
        for (const auto& fence : m_fences) {
            m_fenceModel.render(m_shader, fence.pos, fence.scale, fence.yaw);
        }

        if (m_enemy.active) {
            glm::vec3 modelPos = m_enemy.pos;
            modelPos.y -= m_cameraHeight;
            if (m_enemy.role == EntityType::MOOSE) {
                // Рисуем модель лося по вражеским координатам
                m_mooseModel.render(m_shader, modelPos, 0.5f, m_enemy.yaw);
            } else {
                // второй игрок тоже лось
                m_mooseModel.render(m_shader, modelPos, 0.4f, m_enemy.yaw);
            }
        }

        // В САМОМ КОНЦЕ рисуем UI поверх всего:
        renderUI();
        renderGun();
    }
}

bool Game::checkMooseHit() {
    const float MOOSE_HIT_RADIUS = 1.2f; // Немного уменьшим радиус, раз точка точнее

    // ИСПРАВЛЕНИЕ: Поднимаем точку еще выше (на 2.2 метра)
    glm::vec3 mooseCenter = m_moosePos + glm::vec3(0.0f, 3.0f, 0.0f);

    glm::vec3 toMoose = mooseCenter - m_cameraPos;

    if (glm::dot(m_cameraFront, glm::normalize(toMoose)) < 0.0f) return false;

    glm::vec3 crossProd = glm::cross(toMoose, m_cameraFront);
    float distToRay = glm::length(crossProd);

    return distToRay <= MOOSE_HIT_RADIUS;
}

bool Game::checkPlayerHit() {
    if (!m_enemy.active) return false;

    const float HIT_RADIUS = 1.2f;
    // Проверяем попадание во врага (учитываем, что центр модели ниже камеры)
    glm::vec3 enemyCenter = m_enemy.pos - glm::vec3(0.0f, 0.5f, 0.0f);

    glm::vec3 toEnemy = enemyCenter - m_cameraPos;
    if (glm::dot(m_cameraFront, glm::normalize(toEnemy)) < 0.0f) return false;

    glm::vec3 crossProd = glm::cross(toEnemy, m_cameraFront);
    return glm::length(crossProd) <= HIT_RADIUS;
}

void Game::setupUI() {
    glGenVertexArrays(1, &m_uiVAO);
    glGenBuffers(1, &m_uiVBO);
    glBindVertexArray(m_uiVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_uiVBO);
    // Память под 4 вершины (x, y)
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 2, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}


void Game::renderUI() {
    glDisable(GL_DEPTH_TEST); // UI рисуется поверх всего мира
    m_uiShader.use();

    // 2D Матрица экрана
    glm::mat4 projection = glm::ortho(0.0f, (float)m_width, 0.0f, (float)m_height);
    m_uiShader.setMat4("projection", projection);

    glBindVertexArray(m_uiVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_uiVBO);

    // 1. Крестик по центру экрана
    float cx = m_width / 2.0f;
    float cy = m_height / 2.0f;
    float size = 15.0f; // Размер крестика
    float crosshairVerts[] = {
        cx - size, cy, cx + size, cy, // Горизонтальная линия
        cx, cy - size, cx, cy + size  // Вертикальная линия
    };
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(crosshairVerts), crosshairVerts);

    // Цвет: зеленый если готовы стрелять, красный если перезарядка
    glm::vec3 crossColor = (m_shootCooldown <= 0.0f) ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
    m_uiShader.setVec3("color", crossColor);

    glLineWidth(2.0f); // Делаем крестик потолще
    glDrawArrays(GL_LINES, 0, 4);
    glLineWidth(1.0f); // Возвращаем как было

    // --- 2. СЧЕТЧИК УБИЙСТВ (Сверху по центру) ---
    float boxSize = 15.0f;
    float spacing = 5.0f;
    float totalW = m_kills * (boxSize + spacing);
    float startX = (m_width - totalW) / 2.0f;  // Центрируем
    float topY = m_height - 30.0f;             // Отступ сверху

    m_uiShader.setVec3("color", glm::vec3(1.0f, 0.2f, 0.2f));  // Красные "зарубки"

    for (int i = 0; i < m_kills; ++i) {
        float x1 = startX + i * (boxSize + spacing);
        float y1 = topY;
        float x2 = x1 + boxSize;
        float y2 = y1 + boxSize;

        float killVerts[] = {x1, y1, x2, y1, x1, y2, x2, y2};
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(killVerts), killVerts);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // 2. Полоска перезарядки в правом нижнем углу
    float barW = 150.0f, barH = 20.0f, pad = 20.0f;
    float bx1 = m_width - barW - pad;
    float by1 = pad;
    float bx2 = m_width - pad;
    float by2 = pad + barH;

    // Рисуем темный фон полоски
    float bgVerts[] = { bx1, by1,  bx2, by1,  bx1, by2,  bx2, by2 };
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(bgVerts), bgVerts);
    m_uiShader.setVec3("color", glm::vec3(0.2f, 0.2f, 0.2f));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Рисуем заполнение полоски
    if (m_shootCooldown > 0.0f) {
        float progress = 1.0f - (m_shootCooldown / m_maxCooldown);
        float curW = barW * progress;
        float fillVerts[] = { bx1, by1,  bx1 + curW, by1,  bx1, by2,  bx1 + curW, by2 };
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fillVerts), fillVerts);
        m_uiShader.setVec3("color", glm::vec3(1.0f, 0.8f, 0.0f)); // Желтая полоска заполняется
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    else {
        m_uiShader.setVec3("color", glm::vec3(0.0f, 1.0f, 0.0f)); // Зеленая, когда готова
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    glEnable(GL_DEPTH_TEST); // Включаем глубину обратно для 3D
}

void Game::setupGunUI() {
    // Координаты прямоугольника (X, Y, U, V)
    // Размещаем в нижней части экрана, по центру
    float vertices[] = {
        // Позиция (x, y)      // UV (u, v)
        -0.4f, -1.0f,         0.0f, 0.0f, // Лево низ
         0.4f, -1.0f,         1.0f, 0.0f, // Право низ
        -0.4f, -0.2f,         0.0f, 1.0f, // Лево верх
         0.4f, -0.2f,         1.0f, 1.0f  // Право верх
    };

    glGenVertexArrays(1, &m_gunVAO);
    glGenBuffers(1, &m_gunVBO);

    glBindVertexArray(m_gunVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_gunVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void Game::updateGunAnimation(float dt) {
    // 1. Проверяем нажатие на левую кнопку мыши, если мы еще не стреляем
    if (glfwGetMouseButton(m_window->getGLFWwindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !m_isShooting) {
        m_isShooting = true;
        m_currentGunFrame = 1; // Переходим к первому кадру вспышки
        m_animationTimer = 0.0f;
    }

    // 2. Если процесс стрельбы идет — крутим таймер
    if (m_isShooting) {
        m_animationTimer += dt;
        if (m_animationTimer >= m_frameDuration) {
            m_animationTimer = 0.0f;
            m_currentGunFrame++;

            // 3. Если кадры закончились, возвращаемся в режим покоя (кадр 0)
            if (m_currentGunFrame >= m_gunFrames.size()) {
                m_currentGunFrame = 0;
                m_isShooting = false;
            }
        }
    }
}

void Game::renderGun() {
    glDisable(GL_DEPTH_TEST); // Чтобы ружье не "тонуло" в текстурах земли
    glEnable(GL_BLEND);       // Включаем прозрачность для PNG
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_gunShader.use(); // Отдельный простой шейдер (или основной, но с identity матрицами)

    float aspect = (float)m_width / (float)m_height;
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(1.0f / aspect, 1.0f, 1.0f));
    m_gunShader.setMat4("model", model);

    glBindVertexArray(m_gunVAO);
    glBindTexture(GL_TEXTURE_2D, m_gunFrames[m_currentGunFrame]);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

// ---- Cleanup ----
void Game::cleanup() {
    for (auto& tree : m_treeModels) {
        tree.cleanup();
    }
    m_bushModel.cleanup();

    if (m_floorTexture) {
        glDeleteTextures(1, &m_floorTexture);
        m_floorTexture = 0;
    }

    m_mooseModel.cleanup();
    delete m_window;
    m_window = nullptr;

    if (m_uiVAO) glDeleteVertexArrays(1, &m_uiVAO);
    if (m_uiVBO) glDeleteBuffers(1, &m_uiVBO);
}

// ---- Callbacks ----
void Game::mouseCallback(GLFWwindow*, double xpos, double ypos) {
    if (!g_gameInstance) return;
    Game& g = *g_gameInstance;

    if (g.m_firstMouse) {
        g.m_lastX = (float)xpos;
        g.m_lastY = (float)ypos;
        g.m_firstMouse = false;
    }

    float dx = (float)xpos - g.m_lastX;
    float dy = g.m_lastY - (float)ypos;
    g.m_lastX = (float)xpos;
    g.m_lastY = (float)ypos;

    const float sens = 0.1f;
    g.m_yaw += dx * sens;
    g.m_pitch += dy * sens;
    if (g.m_pitch > 89.0f) g.m_pitch = 89.0f;
    if (g.m_pitch < -89.0f) g.m_pitch = -89.0f;
}

void Game::scrollCallback(GLFWwindow*, double, double yoff) {
    if (!g_gameInstance) return;
    g_gameInstance->m_fov -= (float)yoff * 2.0f;
    if (g_gameInstance->m_fov < 10.0f) g_gameInstance->m_fov = 10.0f;
    if (g_gameInstance->m_fov > 90.0f) g_gameInstance->m_fov = 90.0f;
}

void Game::processNetwork() {
    if (!m_clientHost) return;

    ENetEvent event;
    while (enet_host_service(m_clientHost, &event, 0) > 0) {
        if (event.type == ENET_EVENT_TYPE_CONNECT) {
            std::cout << "[CLIENT] Connected! Requesting role..." << std::endl;
            PacketJoinRequest req;
            req.requestedRole = m_selectedRole;
            ENetPacket* packet = enet_packet_create(&req, sizeof(PacketJoinRequest), ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(m_serverPeer, 0, packet);
        } else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            PacketHeader* header = reinterpret_cast<PacketHeader*>(event.packet->data);

            if (header->type == PacketType::INIT) {
                PacketInit* initData = reinterpret_cast<PacketInit*>(event.packet->data);
                m_myId = initData->myId;
                m_myRole = initData->myRole;
                m_state = GameState::PLAYING;
                std::cout << "[CLIENT] Server confirmed Role: " << (int)m_myRole << ". GAME START!" << std::endl;
            } else if (header->type == PacketType::UPDATE && m_state == GameState::PLAYING) {
                PacketUpdate* updateData = reinterpret_cast<PacketUpdate*>(event.packet->data);
                m_enemy.active = true;
                m_enemy.role = updateData->role;
                m_enemy.pos = glm::vec3(updateData->x, updateData->y, updateData->z);
                m_enemy.yaw = updateData->yaw;
            } else if (header->type == PacketType::RESPAWN) {
                // ТЕБЯ УБИЛИ
                std::cout << "YOU DIED! Respawning..." << std::endl;
                float rx = (rand() % 40) - 20.0f;
                float rz = (rand() % 40) - 20.0f;
                m_cameraPos = glm::vec3(rx, m_terrain.getHeight(rx, rz) + m_cameraHeight, rz);
                m_velocityY = 0;
            } else if (header->type == PacketType::KILL_CONFIRM) {
                // ТЫ УБИЛ
                m_kills++;  // Добавляем фраг в локальную переменную
            }
            enet_packet_destroy(event.packet);
        }
    }

    if (m_state == GameState::PLAYING && m_myId != 0 && m_serverPeer && m_serverPeer->state == ENET_PEER_STATE_CONNECTED) {
        PacketUpdate myUpdate;
        myUpdate.playerId = m_myId;
        myUpdate.role = m_myRole;
        myUpdate.x = m_cameraPos.x;
        myUpdate.y = m_cameraPos.y;
        myUpdate.z = m_cameraPos.z;
        myUpdate.yaw = m_yaw;

        ENetPacket* packet = enet_packet_create(&myUpdate, sizeof(PacketUpdate), ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
        enet_peer_send(m_serverPeer, 1, packet);
    }
}