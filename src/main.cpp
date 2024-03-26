#include <iostream>

#include <glad/glad.h>

#include "AppWindow.h"
#include "Model.h"
#include "Timer.h"
#include "Scene.h"
#include "Global.h"
#include <assert.h>
#include <filesystem>
namespace fs = std::filesystem;


// Helper functions to use sin in 0-1 normalized range
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

int main(int , char** argv)  {
    

    //
    // Initialization
    //

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

    TextureBindContext::Init();
    FileManager::Init(argv[0]);

    // Load shaders
    Shader blinnPhongShader(FileManager::FromRoot("assets/shaders/blinn-phong.vert"), FileManager::FromRoot("assets/shaders/blinn-phong.frag"));
    Shader skyboxShader(FileManager::FromRoot("assets/shaders/skybox.vert"), FileManager::FromRoot("assets/shaders/skybox.frag"));
    Shader depthMapShader(FileManager::FromRoot("assets/shaders/depth-map.vert"), FileManager::FromRoot("assets/shaders/depth-map.frag"));
    Shader depthMapShader3D(FileManager::FromRoot("assets/shaders/depth-map3D.vert"), FileManager::FromRoot("assets/shaders/depth-map3D.frag"), FileManager::FromRoot("assets/shaders/depth-map3D.geo"));

    // Add 3D models to scene and set transform matrices
    Scene scene;
    scene.AddModel(new Model(FileManager::FromRoot("assets/models/cottage/cottage.obj")))
        ->GetTransform().SetTranslation({ 0, -5, -100 });
    scene.AddModel(new Model(FileManager::FromRoot("assets/models/container/container.obj")))
        ->GetTransform().SetTranslation({ 30, -5, -90 })
            .SetScale({ 0.05f, 0.05f, 0.05f });


    MaterialLibrary::Get().GetMaterial("Container")->reflectiveness = 0.1f;
    MaterialLibrary::Get().GetMaterial("Klimatizacia")->reflectiveness = 0.3f;

    

    //
    // Set up lights
    //
    PointLight pointLight;
    pointLight.position = {-20, 15, -95};
    pointLight.innerRadius = 15.f;
    pointLight.outerRadius = 50.f;
    pointLight.diffuse = {1.0f, 0.2f, 0.8f};
    pointLight.intensity = 1.0f;
    pointLight.specular = pointLight.diffuse.Multiply(0.4f);
    pointLight.depthMapInfo.cast = true;
    size_t pointLightIndex = scene.AddPointLight(pointLight);
    Vec2 pointLightXBounds{-30, 20};
    Vec2 pointLightZBounds{-50, -80};
    Vec3 pointLightTargetPos{0, 0, 0};
    Vec3 pointLightLastPos{0, 0, 0};

    DirectionalLight dirLight;
    dirLight.direction = { -.5f, -0.5f, 1.f };
    dirLight.intensity = 0.8f;
    dirLight.diffuse = { 1.0f, 0.8f, 0.3f };
    size_t sunIndex = scene.AddDirLight(dirLight);

    size_t flashLightIndex = scene.AddSpotLight();
    scene.GetSpotLightAt(flashLightIndex).cutOff = PI32 * 0.1f;
    scene.GetSpotLightAt(flashLightIndex).outerCutOff = PI32 * 0.1f * 1.5f;

    SpotLight spotLight;
    spotLight.position = { 5, 4.f, -80 };
    spotLight.cutOff = PI32 * 0.2f;
    spotLight.outerCutOff = spotLight.cutOff * 1.5f;
    size_t spinningLightindex = scene.AddSpotLight(spotLight);

    bool flashLightOn = false;
    bool spinningLightOn = false;
    bool pointLightOn = true;

    Texture grassTexture = loadTexture(FileManager::FromRoot("assets/textures/grass.png"));
    Texture grassNormalMap = loadTexture(FileManager::FromRoot("assets/textures/grass_normals.png"));

    //
    // Generate grass quads 
    //

    // Generate more grasses in Release mode because Debug cant handle it
#ifdef _DEBUG
    int numGrasses = 1500;
#else
    int numGrasses = 7000;
#endif

    float grassY = -5.f;
    float radius = 120.f;

    // Randomly place grass within radius
    for (int i = 0; i < numGrasses; ++i) {
        float xOffset = ((rand() % 100) / 100.0f) * radius - radius / 2.0f;
        float zOffset = ((rand() % 100) / 100.0f) * radius - radius / 2.0f;

        float xPosition = 1.0f + xOffset;
        float zPosition = -60.0f + zOffset;

        float yPosition = grassY;

        float rotation = (float)(rand() % (int)((PI32 * 2.0f) * 100.f)) / 100.f;

        scene.AddGrass({xPosition, yPosition, zPosition}, rotation, { 3.f, 5.f });
    }

    // MaterialLibrary::Get().GetMaterial("Container")->shouldCastShadow = false;

    // scene.GetPointLightAt(pointLightIndex).depthMapInfo.cast = false;    

    // Movement constants
    const float camMoveSpeed = 10.f;
    const float camRotateSpeed = 1.f;

    glEnable(GL_MULTISAMPLE); 

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

    // Set resource references for the rendering
    DrawContext drawContext;
    drawContext.objShader = &blinnPhongShader;
    drawContext.skyboxShader = &skyboxShader;
    drawContext.depthMapShader = &depthMapShader;
    drawContext.depthMapShader3D = &depthMapShader3D;
    drawContext.grassTexture = grassTexture;
    drawContext.grassNormalMap = grassNormalMap;

    Timer animationTimer; // Used for animating with std::sin
    Timer frameTimer; // Used for deltaTime
    Timer pointLightTimer; // Used for changing pointLight position
    while (window.IsRunning()) {

        Duration frameTime = frameTimer.Record();
        frameTimer.Reset();

        float deltaTime = frameTime.GetSecondsF();

        // Clear buffer
        glClearColor(.6f, .3f, .6f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        Vec3 rotate{0 , 0, 0};
        Vec3 move{0 , 0, 0};

        // Set camera movement based on input

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

        // R: Hot Reload shaders
        static bool wasRDown = false;
        if (window.IsKeyDown(GLFW_KEY_R) && !wasRDown) {
            wasRDown = true;

            blinnPhongShader.HotReload();
            depthMapShader.HotReload();
            depthMapShader3D.HotReload();
            skyboxShader.HotReload();
        }
        if (!window.IsKeyDown(GLFW_KEY_R)) wasRDown = false;

        
        if (window.IsKeyDown(GLFW_KEY_LEFT_CONTROL)) {

            static bool wasDDown = false;
            // CTRL + D: Draw depth maps (for debugging) 
            if (window.IsKeyDown(GLFW_KEY_D) && !wasDDown) {
                wasDDown = true;

                g_ShouldDrawDepthMaps = !g_ShouldDrawDepthMaps;
            }
            if (!window.IsKeyDown(GLFW_KEY_D)) wasDDown = false;

            // CTRL + L: Next depth map
            // CTRL + K: Previous depth map
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
            if (g_DrawDepthMapIndex < 0) g_DrawDepthMapIndex = 0;


            // CTRL + F: Toggle flashlight
            static bool wasFDown = false;
            if (window.IsKeyDown(GLFW_KEY_F) && !wasFDown) {
                wasFDown = true;
                
                flashLightOn = !flashLightOn;
            }
            if (!window.IsKeyDown(GLFW_KEY_F)) wasFDown = false;

            // CTRL + G: Toggle spinning spotlight
            static bool wasGDown = false;
            if (window.IsKeyDown(GLFW_KEY_G) && !wasGDown) {
                wasGDown = true;
                
                spinningLightOn = !spinningLightOn;
            }
            if (!window.IsKeyDown(GLFW_KEY_G)) wasGDown = false;

            // CTRL + H: Toggle point light
            static bool wasHDown = false;
            if (window.IsKeyDown(GLFW_KEY_H) && !wasHDown) {
                wasHDown = true;
                
                pointLightOn = !pointLightOn;
            }
            if (!window.IsKeyDown(GLFW_KEY_H)) wasHDown = false;

        }

        // Animation "seed" or time to use for animating stuff
        float animationSeed = animationTimer.Record().GetSecondsF();

        //
        // Flashlight animation
        //

        if (flashLightOn) {
            // "Flicker" animation with a noisy sin function to make it look
            // like the flashlight is broken/low battery
            auto& flashLight = scene.GetSpotLightAt(flashLightIndex);
            float flickerSpeed = 20.f;
            float flickerStrength = 0.1f;
            flashLight.intensity = 1.0f - (noisySin(animationSeed * flickerSpeed) * flickerStrength);
            flashLight.position = scene.GetViewMatrix().GetTranslation();
            flashLight.direction = scene.GetViewMatrix().TransformDirection({ 0, 0, -1 });
        } else {
            scene.GetSpotLightAt(flashLightIndex).intensity = 0.0f;
        }

        //
        // Spinning light animation
        //

        auto& spinningLight = scene.GetSpotLightAt(spinningLightindex);
        if (spinningLightOn) {
            // Spinning and blinking like a siren
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

        //
        // Point light animation
        //

        // Every pointLightIntervall, select new position to move to
        auto& pointLight = scene.GetPointLightAt(pointLightIndex);
        const float pointLightIntervall = 3.0f;
        if (pointLightTimer.Record().GetSecondsF() >= pointLightIntervall) {
            pointLightTimer.Reset();

            pointLightTargetPos.x = pointLightXBounds.x + (static_cast<float>(rand()) / RAND_MAX) * (pointLightXBounds.y - pointLightXBounds.x);
            pointLightTargetPos.z = pointLightZBounds.x + (static_cast<float>(rand()) / RAND_MAX) * (pointLightZBounds.y - pointLightZBounds.x);

            pointLightLastPos = pointLight.position;
        }
        float progress = pointLightTimer.Record().GetSecondsF() / pointLightIntervall;
        pointLight.position.x = pointLightLastPos.x + progress * (pointLightTargetPos.x - pointLightLastPos.x);
        pointLight.position.z = pointLightLastPos.z + progress * (pointLightTargetPos.z - pointLightLastPos.z);

        if (pointLightOn) {
            // Pulse animation
            float pulseSpeed = 5.0f;
            float pulseStrength = 0.7f;
            pointLight.intensity = 1.0f - (pulseStrength * normSin(std::sin(animationSeed * pulseSpeed)));
        } else {
            pointLight.intensity = 0.0f;    
        }
        

        //
        // Day/Night cycle
        //

        // Use a normalized sin curve to determine time of day in
        // 0-1 range, and use that for sun intensity as well.
        float intensity = std::max(normalizeNDC(std::sin(animationSeed * dayCycleSpeed)), 0.1f);
        sunLight.intensity = intensity;

        // The different colors to lerp between in the day/night cycle
        Vec3 minIntensityColor = Vec3(0.1f, 0.1f, 0.2f);
        Vec3 lowIntensityColor = Vec3(1.0f, 0.3f, 0.0f);
        Vec3 midIntensityColor = Vec3(1.0f, 0.5f, 0.0f);
        Vec3 highIntensityColor = Vec3(1.0f, 1.0f, 1.0f);

        Vec3 color;

        // Interpolate between the different colors depending in intensity
        // (AKA time of day in 0-1 range)
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
        sunLight.specular = color;

        // Set ambient color to same as the sun light but relatively darker
        scene.SetAmbientColor(sunLight.diffuse.Multiply(0.2f * intensity));

        // Apply rotation and movement to the view transform
        scene.GetViewMatrix().Rotate(rotate.x, {1, 0, 0})
            .Rotate(rotate.y, {0, 1, 0})
            .Rotate(rotate.z, {0, 0, 1});

        // Move on local axes for camera
        Vec3 localMove = scene.GetViewMatrix().TransformDirection(move);
        scene.GetViewMatrix().Translate(localMove);

        scene.Draw(drawContext);

        // Poll events and swap window buffer
        window.PollEvents();
        window.SwapBuffers();
    }
}