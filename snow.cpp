#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

// ===========================
// Struttura per un semplice vettore 2D
// ===========================
struct Vector2 {
    float x, y;

    Vector2 operator+(const Vector2& other) const { return {x + other.x, y + other.y}; }
    Vector2 operator*(float scalar) const { return {x * scalar, y * scalar}; }
    Vector2& operator+=(const Vector2& other) { x += other.x; y += other.y; return *this; }
};

const float defaultWidth = 1600;
const float defaultHeight = 1200;

// ===========================
// Funzione per ottenere la risoluzione dello schermo
// ===========================
bool getDisplayResolution(int& screenWidth, int& screenHeight) {
    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0, &displayMode) != 0) {
        SDL_Log("Impossibile ottenere la modalità di visualizzazione: %s", SDL_GetError());
        return false;
    }
    screenWidth = displayMode.w;
    screenHeight = displayMode.h;
    return true;
}

// ===========================
// Struttura che rappresenta un fiocco di neve
// ===========================
struct Snowflake {
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    float size;
    float oscillationOffset;
    float depth;

    Snowflake(float x, float y) {
        position = {x, y};
        velocity = {float(rand() % 2 - 1), float(rand() % 2 + 1)};
        acceleration = {0, 0};
        size = float(rand() % 3 + 6);
        oscillationOffset = float(rand() % 360);
        depth = (rand() % 100 + 1) / 10.0f;
    }

    void applyForce(Vector2 force) {
        acceleration += force;
    }

    void update(float deltaTime, float windForce) {
        float oscillation = sin((position.y + oscillationOffset) * 0.02f) * 1.5f;
        velocity.x = windForce + oscillation;
        velocity.y += acceleration.y * deltaTime * (1.0f / depth);
        position += velocity * deltaTime;
        acceleration = {0, 0};

        int screenWidth, screenHeight;
        if (!getDisplayResolution(screenWidth, screenHeight)) {
            screenWidth = defaultWidth;
            screenHeight = defaultHeight;
        }

        if (position.y > screenHeight) {
            position.y = 0;
            position.x = rand() % screenWidth;
            velocity = {float(rand() % 3 - 1), float(rand() % 3 + 1)};
        }

        const int horizontalOffset = 50;
        if (position.x < -horizontalOffset) {
            position.x = screenWidth + horizontalOffset;
        } else if (position.x > screenWidth + horizontalOffset) {
            position.x = -horizontalOffset;
        }
    }
};

// ===========================
// Funzione principale: Inizializza SDL e gestisce il loop principale
// ===========================
int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Errore durante l'inizializzazione di SDL: %s", SDL_GetError());
        return -1;
    }

    int screenWidth, screenHeight;
    if (!getDisplayResolution(screenWidth, screenHeight)) {
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Snow", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    if (!window || !renderer) {
        SDL_Log("Errore nella creazione della finestra o del renderer: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    IMG_Init(IMG_INIT_PNG);
    SDL_Texture* snowflakeTexture = IMG_LoadTexture(renderer, "snowflake.png");
    if (!snowflakeTexture) {
        SDL_Log("Errore nel caricare l'immagine: %s", IMG_GetError());
        return -1;
    }

    std::srand(std::time(nullptr));
    int extendedWidth = screenWidth + 800;
    std::vector<Snowflake> snowflakes;
    for (int i = 0; i < 500; ++i) {
        snowflakes.emplace_back(rand() % extendedWidth - 400, rand() % screenHeight);
    }

    bool running = true;
    SDL_Event event;
    float windForce = 0.0f;
    int windChangeTimer = 0;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        if (++windChangeTimer > 180) {
            windForce = (rand() % 200 - 100) / 200.0f;
            windChangeTimer = 0;
            if (rand() % 5 == 0) windForce *= 4;
        }

        float gravity = 0.005f;
        for (auto& flake : snowflakes) {
            float windSpeed = windForce * (1.0f / flake.depth);
            flake.applyForce({0, gravity});
            flake.update(1.0f, windForce);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 30, 255);
        SDL_RenderClear(renderer);
        
        for (const auto& flake : snowflakes) {
            float scale = (1.0f / flake.depth);
            SDL_Rect destRect = {int(flake.position.x), int(flake.position.y), int(flake.size*scale), int(flake.size*scale)};
            SDL_RenderCopy(renderer, snowflakeTexture, nullptr, &destRect);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(snowflakeTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
