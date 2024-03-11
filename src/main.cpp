#include <iostream>
#include <assert.h>
#include <filesystem>
namespace fs = std::filesystem;

#include "AppWindow.h"
#include "Model.h"
#include "Timer.h"
#include "Scene.h"
#include "Global.h"

class FileManager {
private:
    static std::string s_Root;

public:
    static void Init(const std::string& exePath);
    static std::string DirectoryOf(const std::string& path);
    static std::string ToAbsolute(const std::string& path);

    static std::string FromRoot(const std::string& relPath);
    static std::string GetRoot() { return s_Root; }
};
std::string FileManager::s_Root = "";

void FileManager::Init(const std::string& exePath) {

    // Search for root project directory

    std::string dir = DirectoryOf(ToAbsolute(exePath));

    while (!fs::exists(dir + "/include") && !fs::exists(dir + "/src") && !dir.empty()) {
        dir = DirectoryOf(dir);
    }

    s_Root = dir;

    assert(!dir.empty() && "Failed finding root project directory (include/ & src/)");

    std::cout << "Root directory is '" << s_Root << "'\n";
}

std::string FileManager::DirectoryOf(const std::string& path) {
    return fs::path(path).parent_path().generic_u8string();
}
std::string FileManager::ToAbsolute(const std::string& path) {
    fs::path fsPath = fs::path(path);
    if (fsPath.is_absolute()) return path;
    return fs::absolute(fsPath).generic_u8string();
}

std::string FileManager::FromRoot(const std::string& relPath) {
    return s_Root + "/" + relPath;
}

float normalizeNDC(float x) {
    return (x + 1.0f) / 2.0f;
}
float normSin(float seed) {
    return normalizeNDC(std::sin(seed));
}

// Noisy sine waves, sporadius big ones up and down and sometimes moments
// of less noisy but still uneven waves
// https://www.desmos.com/calculator
float noisySin(float seed) {
    return (normSin(seed*.3f)+1.f) * (normSin(seed*.6f)+.1f) * (normSin(seed*2.5f)+2.f) * (normSin(seed*.1f)+.9f);
}

int main(int , char** argv)  {
    
    // Create Window
    AppWindow window("OpenGL", SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!window.IsValid()) {
        // Error message happens in window code
        return 1;
    }
    SetMainWindow(&window);

    
    // Initialize GLAD (Load OpenGL)
    std::cout << "Initializing GLAD...\n";
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    std::cout << "OK!\n";

    FileManager::Init(argv[0]);

    Shader blinnPhongShader(FileManager::FromRoot("assets/shaders/blinn-phong.vs"), FileManager::FromRoot("assets/shaders/blinn-phong.fs"));
    Shader skyboxShader(FileManager::FromRoot("assets/shaders/skybox.vs"), FileManager::FromRoot("assets/shaders/skybox.fs"));
    Shader depthMapShader(FileManager::FromRoot("assets/shaders/depth-map.vs"), FileManager::FromRoot("assets/shaders/depth-map.fs"));

    Scene scene;
    scene.AddModel(new Model(FileManager::FromRoot("assets/models/cottage/cottage.obj")))
        ->GetTransform().SetTranslation({ 0, -5, -100 });
    scene.AddModel(new Model(FileManager::FromRoot("assets/models/container/container.obj")))
        ->GetTransform().SetTranslation({ 30, -5, -90 })
            .SetScale({ 0.05f, 0.05f, 0.05f });

    // These  materials are loaded from the container files
    MaterialLibrary::Get().GetMaterial("Container")->reflectiveness = 0.1f;
    MaterialLibrary::Get().GetMaterial("Klimatizacia")->reflectiveness = 0.3f;

    PointLight pointLight;
    pointLight.position = {-20, -2, -95};
    scene.AddPointLight(pointLight);

    DirectionalLight dirLight;
    dirLight.direction = { -.5f, -0.5f, 1.f };
    dirLight.intensity = 0.8f;
    dirLight.diffuse = { 1.0f, 0.8f, 0.3f };
    size_t sunIndex = scene.AddDirLight(dirLight);

    size_t flashLightIndex = scene.AddSpotLight();
    SpotLight spotLight;
    spotLight.position = { 0, 4.f, -80 };
    spotLight.cutOff = PI32 * 0.2f;
    spotLight.outerCutOff = spotLight.cutOff * 1.5f;
    size_t spinningLightindex = scene.AddSpotLight(spotLight);

    bool flashLightOn = false;

    bool spinningLightOn = true;

    Texture grassTexture = loadTexture(FileManager::FromRoot("assets/textures/grass.png"));
    Texture grassNormalMap = loadTexture(FileManager::FromRoot("assets/textures/grass_normals.png"));

    float grassY = -5.f;

    int numGrasses = 1500;
    float radius = 40.f;

    for (int i = 0; i < numGrasses; ++i) {
        float xOffset = ((rand() % 100) / 100.0f) * radius - radius / 2.0f;
        float zOffset = ((rand() % 100) / 100.0f) * radius - radius / 2.0f;

        float xPosition = 1.0f + xOffset;
        float zPosition = -60.0f + zOffset;

        float yPosition = grassY;

        float rotation = (float)(rand() % (int)((PI32 * 2.0f) * 100.f)) / 100.f;

        scene.AddGrass({xPosition, yPosition, zPosition}, rotation, { 3.f, 5.f });
    }

    // Movement constants
    const float camMoveSpeed = 10.f;
    const float camRotateSpeed = 1.f;

    glEnable(GL_MULTISAMPLE); 

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

    // Timer to keep track of frametime for delta time
    Timer animationTimer;
    Timer frameTimer;
    while (window.IsRunning()) {

        Duration frameTime = frameTimer.Record();
        frameTimer.Reset();

        float deltaTime = frameTime.GetSecondsF();

        // Clear buffer
        glClearColor(.6f, .3f, .6f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        Vec3 rotate{0 , 0, 0};
        Vec3 move{0 , 0, 0};

        // Set accumulative rotation and movement for 
        // current frame depending on user input
        if (window.IsKeyDown(GLFW_KEY_W)) {
            move.z -= camMoveSpeed * deltaTime;
        }
        if (window.IsKeyDown(GLFW_KEY_A)) {
            move.x -= camMoveSpeed * deltaTime;
        }
        if (window.IsKeyDown(GLFW_KEY_S)) {
            move.z += camMoveSpeed * deltaTime;
        }
        if (window.IsKeyDown(GLFW_KEY_D)) {
            move.x += camMoveSpeed * deltaTime;
        }
        if (window.IsKeyDown(GLFW_KEY_SPACE)) {
            move.y += camMoveSpeed * deltaTime;
        }
        if (window.IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
            move.y -= camMoveSpeed * deltaTime;
        }

        if (window.IsKeyDown(GLFW_KEY_UP)) {
            rotate.x += camRotateSpeed * deltaTime;
        }
        if (window.IsKeyDown(GLFW_KEY_LEFT)) {
            rotate.y += camRotateSpeed * deltaTime;
        }
        if (window.IsKeyDown(GLFW_KEY_DOWN)) {
            rotate.x -= camRotateSpeed * deltaTime;
        }
        if (window.IsKeyDown(GLFW_KEY_RIGHT)) {
            rotate.y -= camRotateSpeed * deltaTime;
        }

        if (window.IsKeyDown(GLFW_KEY_Q)) {
            rotate.z += camRotateSpeed * deltaTime;
        }
        if (window.IsKeyDown(GLFW_KEY_E)) {
            rotate.z -= camRotateSpeed * deltaTime;
        }

        static bool wasRDown = false;
        if (window.IsKeyDown(GLFW_KEY_R) && !wasRDown) {
            wasRDown = true;

            blinnPhongShader.HotReload();
            depthMapShader.HotReload();
        }
        if (!window.IsKeyDown(GLFW_KEY_R)) wasRDown = false;

        static bool wasDDown = false;
        
        if (window.IsKeyDown(GLFW_KEY_LEFT_CONTROL)) {
            if (window.IsKeyDown(GLFW_KEY_D) && !wasDDown) {
                wasDDown = true;

                g_ShouldDrawDepthMaps = !g_ShouldDrawDepthMaps;
            }

            static bool wasLDown = false;
            static bool wasKDown = false;
            if (window.IsKeyDown(GLFW_KEY_L) && !wasLDown) {
                wasLDown = true;
                g_DrawDepthMapIndex++;
            }
            if (window.IsKeyDown(GLFW_KEY_K) && ! wasKDown) {
                wasKDown = true;
                g_DrawDepthMapIndex--;
            }

            if (!window.IsKeyDown(GLFW_KEY_L)) wasLDown = false;
            if (!window.IsKeyDown(GLFW_KEY_K)) wasKDown = false;

            static bool wasFDown = false;
            if (window.IsKeyDown(GLFW_KEY_F) && !wasFDown) {
                wasFDown = true;
                
                flashLightOn = !flashLightOn;
            }
            if (!window.IsKeyDown(GLFW_KEY_F)) wasFDown = false;

            static bool wasGDown = false;
            if (window.IsKeyDown(GLFW_KEY_G) && !wasGDown) {
                wasGDown = true;
                
                spinningLightOn = !spinningLightOn;
            }
            if (!window.IsKeyDown(GLFW_KEY_G)) wasGDown = false;

            if (g_DrawDepthMapIndex < 0) g_DrawDepthMapIndex = 0;
        }
        if (!window.IsKeyDown(GLFW_KEY_D)) wasDDown = false;

        float animationSeed = animationTimer.Record().GetSecondsF();

        if (flashLightOn) {
            auto& flashLight = scene.GetSpotLightAt(flashLightIndex);
            float flickerSpeed = 20.f;
            float flickerStrength = 0.1f;
            flashLight.intensity = 1.0f - (noisySin(animationSeed * flickerSpeed) * flickerStrength);
            flashLight.position = scene.GetViewMatrix().GetTranslation();
            flashLight.direction = scene.GetViewMatrix().TransformDirection({ 0, 0, -1 });
        } else {
            scene.GetSpotLightAt(flashLightIndex).intensity = 0.0f;
        }

        auto& spinningLight = scene.GetSpotLightAt(spinningLightindex);
        if (spinningLightOn) {
            float base = 0.2f;
            float blinkSpeed = 20.0f;
            spinningLight.diffuse = { std::max(std::round(normalizeNDC(std::cos(animationSeed * blinkSpeed + 1.5f))), base), base, std::max(std::round(normalizeNDC(std::sin(animationSeed * blinkSpeed))), base) };
            spinningLight.specular = spinningLight.diffuse;
            spinningLight.intensity = 2.0f;

            float spinSpeed = 8.f;
            spinningLight.direction = { 0, 0, 0 };
            float angle = animationSeed * spinSpeed;
            spinningLight.direction.x = std::cos(angle);
            spinningLight.direction.z = std::sin(angle);
        } else {
            spinningLight.intensity = 0.0f;
        }

        auto& sunLight = scene.GetDirLightAt(sunIndex);

        float dayCycleSpeed = 0.1f;

        float intensity = std::max(normalizeNDC(std::sin(animationSeed * dayCycleSpeed)), 0.1f);
        sunLight.intensity = intensity;

        Vec3 minIntensityColor = Vec3(0.1f, 0.1f, 0.2f);
        Vec3 lowIntensityColor = Vec3(1.0f, 0.3f, 0.0f);
        Vec3 midIntensityColor = Vec3(1.0f, 0.5f, 0.0f);
        Vec3 highIntensityColor = Vec3(1.0f, 1.0f, 1.0f);

        Vec3 color;
        if (intensity < 0.3f) {
            float factor = intensity / 0.3f; 
            color = Vec3::Mix(minIntensityColor, lowIntensityColor, factor);
        } else if (intensity < 0.5f) {
            float factor = (intensity - 0.3f) / (0.5f - 0.3f);
            color = Vec3::Mix(lowIntensityColor, midIntensityColor, factor);
        } else {
            float factor = (intensity - 0.5f) / (1.0f - 0.5f);
            color = Vec3::Mix(midIntensityColor, highIntensityColor, factor);
        }

        sunLight.diffuse = color;
        sunLight.specular = color; // Assuming specular color is the same as diffuse

        scene.SetAmbientColor(sunLight.diffuse.Multiply(0.2f * intensity));

        // Apply rotation and movement to the view transform
        scene.GetViewMatrix().Rotate(rotate.x, {1, 0, 0})
            .Rotate(rotate.y, {0, 1, 0})
            .Rotate(rotate.z, {0, 0, 1});

        // Move on local axes for camera
        Vec3 localMove = scene.GetViewMatrix().TransformDirection(move);
        scene.GetViewMatrix().Translate(localMove);

        scene.Draw(blinnPhongShader, skyboxShader, depthMapShader, grassTexture, grassNormalMap);

        // Poll events and swap window buffer
        window.PollEvents();
        window.SwapBuffers();
    }
}