#include "Game.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

static Game* g_gameInstance = nullptr;

Game::Game()
    : m_window(nullptr), m_width(1600), m_height(900), m_cameraPos(0.0f, 1.5f, 8.0f), m_cameraFront(0.0f, 0.0f, -1.0f), m_cameraUp(0.0f, 1.0f, 0.0f), m_yaw(-90.0f), m_pitch(0.0f), m_lastX(800.0f), m_lastY(450.0f), m_firstMouse(true), m_fov(45.0f), m_lightPos(5.0f, 10.0f, 5.0f), m_lightColor(1.0f, 0.95f, 0.85f), m_floorTexture(0), m_floorY(1.5f), m_shootCooldown(0.0f), m_maxCooldown(0.5f), m_wasLMBPressed(false) {
    g_gameInstance = this;
}

Game::~Game() { cleanup(); }

// Инициализация всего: камеры, окна, всех объектов
bool Game::init(int width, int height, const char* title, const std::string& serverIP) {
    m_width = width;
    m_height = height;
    m_lastX = width / 2.0f;
    m_lastY = height / 2.0f;

    m_serverIP = serverIP;

    m_window = new Window(width, height, title);
    GLFWwindow* w = m_window->getGLFWwindow();

    std::cout << "OpenGL: " << glGetString(GL_VERSION) << std::endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.35f, 0.55f, 0.75f, 1.0f);

    m_window->setCursorDisabled(true);
    m_window->setCursorPosCallback(mouseCallback);
    m_window->setScrollCallback(scrollCallback);
    glfwSetWindowUserPointer(w, this);

    // Деревья!
    // Загружаем шейдеры для деревьев и сами модели, всего их 3 разных
    m_shader.load("shaders/tree_vertex.glsl", "shaders/tree_fragment.glsl");

    m_treeModels.resize(3);
    m_treeModels[0].load("assets/Tree1.glb", false);
    m_treeModels[1].load("assets/Tree2.glb", false);
    m_treeModels[2].load("assets/Tree3.glb", false);

    // Генерируем деревья: позиции, модели и размеры определяются случайно
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

    // Кусты!
    // Шейдеры те же, грузим модельку
    m_bushModel.load("assets/Bush.glb", false);

    // Кусты аналогично генерируем
    for (int i = 0; i < 50; ++i) {
        float bx = (rand() % 10000 / 100.0f) - 50.0f;
        float bz = (rand() % 10000 / 100.0f) - 50.0f;
        float by = m_terrain.getHeight(bx, bz);

        BushInstance b;
        b.pos = glm::vec3(bx, m_terrain.getHeight(bx, bz), bz);
        b.scale = (0.6f + (rand() % 81) / 100.0f) * 4;
        m_bushes.push_back(b);
    }

    // Грузим лося, землю (подробнее в отдельном классе)
    m_mooseModel.load("assets/Moose.glb", true);
    m_terrain.init("assets/Grass.png");

    // Ставим лося в центр карты и даем начальное направление
    // m_moosePos = glm::vec3(0.0f, m_terrain.getHeight(0.0f, 0.0f), 0.0f);
    // m_mooseDir = glm::normalize(glm::vec2(1.0f, 0.5f));

    // Забор!
    // Грузим забор и расставляем его вдоль границы карты
    m_fenceModel.load("assets/fence.glb", false);
    const float MAP_LIMIT = 50.0f;
    const float FENCE_SPACING = 1.8f;

    for (float x = -MAP_LIMIT; x <= MAP_LIMIT + FENCE_SPACING / 2; x += FENCE_SPACING) {
        m_fences.push_back({glm::vec3(x, 0.0f, -MAP_LIMIT), 0.0f, 1.0f});
        m_fences.push_back({glm::vec3(x, 0.0f, MAP_LIMIT), 180.0f, 1.0f});
    }

    for (float z = -MAP_LIMIT; z <= MAP_LIMIT; z += FENCE_SPACING) {
        m_fences.push_back({glm::vec3(-MAP_LIMIT, 0.0f, z), 90.0f, 1.0f});
        m_fences.push_back({glm::vec3(MAP_LIMIT, 0.0f, z), -90.0f, 1.0f});
    }

    // 2д картинки: спрайты и анимация перезарядки, прицел и тд
    m_uiShader.load("shaders/ui_vertex.glsl", "shaders/ui_fragment.glsl");
    setupUI();
    m_gunShader.load("shaders/gun_vertex.glsl", "shaders/gun_fragment.glsl");
    setupGunUI();

    for (int i = 1; i <= 8; i++) {
        std::string path = "assets/reloading/reload_" + std::to_string(i) + ".png";
        m_gunFrames.push_back(loadTexture(path.c_str()));
    }

    // Стартовая высота камеры
    m_cameraPos.y = m_terrain.getHeight(m_cameraPos.x, m_cameraPos.z) + m_cameraHeight;

    // НАСТРОЙКА СЕТИ
    if (enet_initialize() != 0) return false;
    m_clientHost = enet_host_create(NULL, 1, 2, 0, 0);

    if (!initFont("assets/fonts/Arial.ttf", 48)) {
        std::cerr << "Failed to load font!" << std::endl;
    }
    // Инициализация звука
    if (ma_engine_init(NULL, &m_audioEngine) != MA_SUCCESS) {
        std::cerr << "Failed to init audio engine!" << std::endl;
    } else {
        ma_engine_play_sound(&m_audioEngine, "assets/sounds/forest.mp3", NULL);
    }

    std::cout << "Init OK!" << std::endl;
    return true;
}

// Сам игровой цикл
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
            updateGunAnimation(dt);
            if (m_enemy.active) {
                m_roarTimer -= dt;
                if (m_roarTimer <= 0.0f) {
                    ma_sound* pRoarSound = new ma_sound();
                    //  Инициализируем звук из файла
                    ma_result result = ma_sound_init_from_file(&m_audioEngine, "assets/sounds/roar.wav", 0, NULL, NULL, pRoarSound);

                    if (result == MA_SUCCESS) {
                        ma_sound_set_spatialization_enabled(pRoarSound, MA_TRUE);
                        ma_sound_set_position(pRoarSound, m_enemy.pos.x, m_enemy.pos.y, m_enemy.pos.z);
                        ma_sound_set_looping(pRoarSound, MA_FALSE);
                        ma_sound_set_volume(pRoarSound, 3.0f);
                        ma_sound_set_min_distance(pRoarSound, 20.0f);
                        ma_sound_start(pRoarSound);
                    } else {
                        delete pRoarSound;
                    }
                    m_roarTimer = 20.0f + (rand() % 11);
                }
            }
        }
        render();
        m_window->swapBuffers();
        m_window->pollEvents();
    }
}

// Обработка нажатий
void Game::processInput(float dt) {
    GLFWwindow* w = m_window->getGLFWwindow();

    if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(w, true);

    if (m_state == GameState::SPLASH) {
        m_stateTimer += dt;
        if (m_stateTimer > 4.0f) {
            m_selectedRole = EntityType::MOOSE;
            m_state = GameState::CONNECTING;

            std::cout << "[CLIENT] Connecting to " << m_serverIP << "..." << std::endl;
            ENetAddress address;
            enet_address_set_host(&address, m_serverIP.c_str());
            address.port = 12345;
            m_serverPeer = enet_host_connect(m_clientHost, &address, 2, 0);
        }
    } else if (m_state == GameState::PLAYING) {
        // Ускорение
        bool isMoving = false;
        bool isRunning = glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
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

        // Воспроизведение шагов
        if (isMoving && m_isGrounded) {
            m_stepTimer -= dt;
            if (m_stepTimer <= 0.0f) {
                ma_engine_play_sound(&m_audioEngine, "assets/sounds/step.wav", NULL);
                m_stepTimer = isRunning ? 0.3f : 0.5f;
            }
        } else if (!isMoving) {
            m_stepTimer = 0.0f;
        }

        const float MAP_LIMIT = 49.9f;

        // Обработка коллизий с краем карты
        m_cameraPos.x = glm::clamp(m_cameraPos.x, -MAP_LIMIT, MAP_LIMIT);
        m_cameraPos.z = glm::clamp(m_cameraPos.z, -MAP_LIMIT, MAP_LIMIT);

        // Прыжок и гравитация
        if (glfwGetKey(w, GLFW_KEY_SPACE) == GLFW_PRESS && m_isGrounded) {
            m_velocityY = m_jumpForce;
            m_isGrounded = false;
        }
        m_velocityY += m_gravity * dt;
        m_cameraPos.y += m_velocityY * dt;

        float groundY = m_terrain.getHeight(m_cameraPos.x, m_cameraPos.z) + m_cameraHeight;
        if (m_cameraPos.y <= groundY) {
            m_cameraPos.y = groundY;
            m_velocityY = 0.0f;
            m_isGrounded = true;
        }

        // Механика стрельбы
        if (m_shootCooldown > 0.0f) {
            m_shootCooldown -= dt;
        }

        bool isLMBPressed = glfwGetMouseButton(m_window->getGLFWwindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

        if (isLMBPressed && !m_wasLMBPressed) {
            if (m_shootCooldown <= 0.0f) {
                ma_engine_play_sound(&m_audioEngine, "assets/sounds/shoot.wav", NULL);

                if (checkMooseHit()) {
                    std::cout << "HIT! Moose down!" << std::endl;
                    // Телепортируем лося в случайное место после попадания
                    m_moosePos = glm::vec3((rand() % 80) - 40, 1.5f, (rand() % 80) - 40);
                } else {
                    std::cout << "MISS!" << std::endl;
                }
                if (checkPlayerHit()) {
                    std::cout << "PLAYER HIT!" << std::endl;
                    // Отправляем серверу информацию о попадании
                    PacketHit hit;
                    hit.victimId = 0;
                    ENetPacket* p = enet_packet_create(&hit, sizeof(PacketHit), ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(m_serverPeer, 0, p);
                }
                m_shootCooldown = m_maxCooldown;
            }
        }
        m_wasLMBPressed = isLMBPressed;
    }
}

// Загрузчик текстур
GLuint Game::loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);

    GLenum format = (nrComponents == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return textureID;
}

// Камера
void Game::updateCamera() {
    glm::vec3 front;
    front.x = cosf(glm::radians(m_yaw)) * cosf(glm::radians(m_pitch));
    front.y = sinf(glm::radians(m_pitch));
    front.z = sinf(glm::radians(m_yaw)) * cosf(glm::radians(m_pitch));
    m_cameraFront = glm::normalize(front);
}

// Рендер
void Game::render() {
    m_window->clear();
    if (m_state == GameState::SPLASH) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        m_window->clear();

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        renderText("Moose Hunters", m_width / 2.0f - 220.0f, m_height / 2.0f, 1.3f, glm::vec3(1.0f, 0.0f, 0.0f));

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    } else if (m_state == GameState::MENU || m_state == GameState::CONNECTING) {
        glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
    } else if (m_state == GameState::PLAYING) {
        glClearColor(0.35f, 0.55f, 0.75f, 1.0f);
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

        // Рисуем забор
        for (const auto& fence : m_fences) {
            m_fenceModel.render(m_shader, fence.pos, fence.scale, fence.yaw);
        }

        if (m_enemy.active) {
            glm::vec3 modelPos = m_enemy.pos;
            modelPos.y -= m_cameraHeight;
            if (m_enemy.role == EntityType::MOOSE) {
                // Рисуем модель лося по вражеским координатам
                m_mooseModel.render(m_shader, modelPos, 0.4f, m_enemy.yaw);
            } else {
                // Второй игрок тоже лось
                m_mooseModel.render(m_shader, modelPos, 0.4f, m_enemy.yaw);
            }
        }

        ma_engine_listener_set_position(&m_audioEngine, 0, m_cameraPos.x, m_cameraPos.y, m_cameraPos.z);
        ma_engine_listener_set_direction(&m_audioEngine, 0, m_cameraFront.x, m_cameraFront.y, m_cameraFront.z);
        ma_engine_listener_set_position(&m_audioEngine, 0, m_cameraPos.x, m_cameraPos.y, m_cameraPos.z);
        ma_engine_listener_set_direction(&m_audioEngine, 0, m_cameraFront.x, m_cameraFront.y, m_cameraFront.z);

        // Поверх всего рисуем спрайты оружия и интерфейс
        renderUI();
        renderGun();
    }
}


// Функции для NPC-лося (использовался для дебага)
// Логика движения крайне простая: случайно выбираем время, на протяжении которого лось будет двигаться по прямой
// потом - поворот на случайный угол
void Game::updateMoose(float dt) {
    m_mooseTimer -= dt;

    if (m_mooseTimer <= 0.0f) {
        float randomAngle = (rand() % 360) * 3.14159265f / 180.0f;
        m_mooseDir.x = cosf(randomAngle);
        m_mooseDir.y = sinf(randomAngle);

        m_mooseTimer = 2.0f + (rand() % 301) / 100.0f;
    }

    m_moosePos.x += m_mooseDir.x * m_mooseSpeed * dt;
    m_moosePos.z += m_mooseDir.y * m_mooseSpeed * dt;
    m_moosePos.y = m_terrain.getHeight(m_moosePos.x, m_moosePos.z);
}

// Обработка коллизии с деревьями - простой расчет расстояний между игроком и центром дерева
bool Game::checkTreeCollision(glm::vec3 nextPos) {
    const float playerRadius = 0.3f;
    const float treeRadius = 0.5f;
    const float minDist = playerRadius + treeRadius;

    for (const auto& tree : m_trees) {
        float dx = nextPos.x - tree.pos.x;
        float dz = nextPos.z - tree.pos.z;
        if (dx * dx + dz * dz < minDist * minDist) {
            return true;
        }
    }

    return false;
}

// Проверка попаданий в лося
bool Game::checkMooseHit() {
    const float MOOSE_HIT_RADIUS = 3.2f;

    glm::vec3 mooseCenter = m_moosePos + glm::vec3(0.0f, 3.0f, 0.0f);
    glm::vec3 toMoose = mooseCenter - m_cameraPos;

    if (glm::dot(m_cameraFront, glm::normalize(toMoose)) < 0.0f) return false;
    glm::vec3 crossProd = glm::cross(toMoose, m_cameraFront);
    float distToRay = glm::length(crossProd);

    return distToRay <= MOOSE_HIT_RADIUS;
}

// Аналогичная проверка для игрока
bool Game::checkPlayerHit() {
    if (!m_enemy.active) return false;
    const float HIT_RADIUS = 2.1f;

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 2, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void Game::renderUI() {
    glDisable(GL_DEPTH_TEST);
    m_uiShader.use();

    glm::mat4 projection = glm::ortho(0.0f, (float)m_width, 0.0f, (float)m_height);
    m_uiShader.setMat4("projection", projection);

    glBindVertexArray(m_uiVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_uiVBO);

    // Создаем крестик для прицела
    float cx = m_width / 2.0f;
    float cy = m_height / 2.0f;
    float size = 15.0f;
    float crosshairVerts[] = {
        cx - size, cy, cx + size, cy,
        cx, cy - size, cx, cy + size};
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(crosshairVerts), crosshairVerts);

    // Цвет: зеленый если готовы стрелять, красный если перезарядка
    glm::vec3 crossColor = (m_shootCooldown <= 0.0f) ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
    m_uiShader.setVec3("color", crossColor);

    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, 4);
    glLineWidth(1.0f);

    // СЧЕТЧИК УБИЙСТВ
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(m_uiVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_uiVBO);

    float boxW = 280.0f;
    float boxH = 45.0f;
    float centerX = m_width / 2.0f;
    float topY = m_height - 60.0f;

    float x1 = centerX - boxW / 2.0f;
    float x2 = centerX + boxW / 2.0f;
    float y1 = topY;
    float y2 = topY + boxH;

    float bgVerts_2[] = {x1, y1, x2, y1, x1, y2, x2, y2};
    glBufferData(GL_ARRAY_BUFFER, sizeof(bgVerts_2), bgVerts_2, GL_STREAM_DRAW);
    m_uiShader.setVec3("color", glm::vec3(0.08f, 0.08f, 0.08f));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    std::vector<float> frameLines = {
        x1, y1, x2, y1,
        x2, y1, x2, y2,
        x2, y2, x1, y2,
        x1, y2, x1, y1,
        centerX, y1, centerX, y2};
    glLineWidth(2.0f);
    glBufferData(GL_ARRAY_BUFFER, frameLines.size() * sizeof(float), frameLines.data(), GL_STREAM_DRAW);
    m_uiShader.setVec3("color", glm::vec3(0.3f, 0.3f, 0.3f));
    glDrawArrays(GL_LINES, 0, frameLines.size() / 2);

    float textScale = 0.45f;
    float textY = y1 + 15.0f;

    std::string killStr = "KILLS: " + std::to_string(m_kills);
    renderText(killStr, x1 + 15.0f, textY, textScale, glm::vec3(0.2f, 1.0f, 0.2f));
    std::string deathStr = "DEATHS: " + std::to_string(m_deaths);
    renderText(deathStr, centerX + 15.0f, textY, textScale, glm::vec3(1.0f, 0.2f, 0.2f));

    glLineWidth(1.0f);
    glDisable(GL_BLEND);

    m_uiShader.use();
    glBindVertexArray(m_uiVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_uiVBO);

    // Полоска перезарядки в правом нижнем углу
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
        m_uiShader.setVec3("color", glm::vec3(1.0f, 0.8f, 0.0f));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    else {
        m_uiShader.setVec3("color", glm::vec3(0.0f, 1.0f, 0.0f));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    glEnable(GL_DEPTH_TEST);
}

void Game::setupGunUI() {
    // Устанавливаем положения для спрайтов ружья
    float vertices[] = {
        -0.4f, -1.0f,         0.0f, 0.0f,
         0.4f, -1.0f,         1.0f, 0.0f,
        -0.4f, -0.2f,         0.0f, 1.0f,
         0.4f, -0.2f,         1.0f, 1.0f
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
    // Проверяем нажатие на левую кнопку мыши, если мы еще не стреляем
    if (glfwGetMouseButton(m_window->getGLFWwindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !m_isShooting) {
        m_isShooting = true;
        m_currentGunFrame = 1;
        m_animationTimer = 0.0f;
    }

    // Если процесс стрельбы идет — крутим таймер
    if (m_isShooting) {
        m_animationTimer += dt;
        if (m_animationTimer >= m_frameDuration) {
            m_animationTimer = 0.0f;
            m_currentGunFrame++;

            // Если кадры закончились, возвращаемся в режим покоя
            if (m_currentGunFrame >= m_gunFrames.size()) {
                m_currentGunFrame = 0;
                m_isShooting = false;
            }
        }
    }
}

void Game::renderGun() {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_gunShader.use();

    float aspect = (float)m_width / (float)m_height;
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(+1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.0f / aspect, 1.0f, 1.0f));
    m_gunShader.setMat4("model", model);

    glBindVertexArray(m_gunVAO);
    glBindTexture(GL_TEXTURE_2D, m_gunFrames[m_currentGunFrame]);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

// Очистка
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
    ma_engine_uninit(&m_audioEngine);
}

// Callbacks
void Game::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    Game* g = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (!g) return;

    if (g->m_firstMouse) {
        g->m_lastX = (float)xpos;
        g->m_lastY = (float)ypos;
        g->m_firstMouse = false;
    }

    float dx = static_cast<float>(xpos) - g->m_lastX;
    float dy = g->m_lastY - static_cast<float>(ypos);
    g->m_lastX = (float)xpos;
    g->m_lastY = (float)ypos;

    const float sens = 0.1f;
    g->m_yaw += dx * sens;
    g->m_pitch += dy * sens;
    if (g->m_pitch > 89.0f) g->m_pitch = 89.0f;
    if (g->m_pitch < -89.0f) g->m_pitch = -89.0f;
}

void Game::scrollCallback(GLFWwindow* window, double, double yoff) {
    Game* g = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (!g) return;

    g->m_fov -= (float)yoff * 2.0f;
    if (g->m_fov < 10.0f) g->m_fov = 10.0f;
    if (g->m_fov > 90.0f) g->m_fov = 90.0f;
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
                ma_engine_play_sound(&m_audioEngine, "assets/sounds/player_death.wav", NULL);
                m_deaths++;
                float rx = (rand() % 40) - 20.0f;
                float rz = (rand() % 40) - 20.0f;
                m_cameraPos = glm::vec3(rx, m_terrain.getHeight(rx, rz) + m_cameraHeight, rz);
                m_velocityY = 0;
            } else if (header->type == PacketType::KILL_CONFIRM) {
                // ТЫ УБИЛ
                m_kills++;

                ma_sound* pMooseDeathSound = new ma_sound();
                ma_result result = ma_sound_init_from_file(&m_audioEngine, "assets/sounds/moose_death.wav", 0, NULL, NULL, pMooseDeathSound);

                if (result == MA_SUCCESS) {
                    ma_sound_set_spatialization_enabled(pMooseDeathSound, MA_TRUE);
                    ma_sound_set_position(pMooseDeathSound, m_enemy.pos.x, m_enemy.pos.y, m_enemy.pos.z);
                    ma_sound_set_looping(pMooseDeathSound, MA_FALSE);
                    ma_sound_start(pMooseDeathSound);
                } else {
                    delete pMooseDeathSound;
                }
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

bool Game::initFont(const char* fontPath, unsigned int fontSize) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return false;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath, 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font: " << fontPath << std::endl;
        FT_Done_FreeType(ft);
        return false;
    }

    FT_Set_Pixel_Sizes(face, 0, fontSize);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
            continue;
        }

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RED,
            face->glyph->bitmap.width, face->glyph->bitmap.rows,
            0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)};
        m_characters.insert(std::pair<char, Character>(c, character));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glGenVertexArrays(1, &m_textVAO);
    glGenBuffers(1, &m_textVBO);
    glBindVertexArray(m_textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_textShader.load("shaders/text_vertex.glsl", "shaders/text_fragment.glsl");

    return true;
}

void Game::renderText(std::string text, float x, float y, float scale, glm::vec3 color) {
    m_textShader.use();
    m_textShader.setVec3("textColor", color);

    glm::mat4 projection = glm::ortho(0.0f, (float)m_width, 0.0f, (float)m_height);
    m_textShader.setMat4("projection", projection);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_textVAO);

    for (std::string::const_iterator c = text.begin(); c != text.end(); ++c) {
        Character ch = m_characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        float vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos, ypos, 0.0f, 1.0f},
            {xpos + w, ypos, 1.0f, 1.0f},

            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos + w, ypos + h, 1.0f, 0.0f}};

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, m_textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.Advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
