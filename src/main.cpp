#include <iostream>

#include "AppWindow.h"
#include "Model.h"
#include "Timer.h"
#include "Scene.h"

#include <assert.h>
#include <filesystem>
namespace fs = std::filesystem;

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

int main(int argc, char** argv)  {
    
    // Create Window
    AppWindow window("OpenGL", 800, 600);
    if (!window.IsValid()) {
        // Error message happens in window code
        return 1;
    }
    
    // Initialize GLAD (Load OpenGL)
    std::cout << "Initializing GLAD...\n";
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    std::cout << "OK!\n";

    FileManager::Init(argv[0]);

    // Load a blinn-phong Shader
    Shader blinnPhongShader(FileManager::FromRoot("assets/shaders/blinn-phong.vs"), FileManager::FromRoot("assets/shaders/blinn-phong.fs"));

    Scene scene;
    scene.AddModel(new Model(FileManager::FromRoot("assets/models/cottage/cottage.obj")))
        ->GetTransform().SetTranslation({ 0, -5, -100 });
    scene.AddModel(new Model(FileManager::FromRoot("assets/models/container/container.obj")))
        ->GetTransform().SetTranslation({ 30, -5, -90 })
            .SetScale({ 0.05f, 0.05f, 0.05f });

    
    scene.AddQuad(GrassQuad({ -30, -5.f + 2.5f, -100 }, { 5.f, 5.f }, { 0, 0.f, 0 }));
    scene.AddQuad(GrassQuad({ -30, -5.f + 2.5f, -100 + 0.1f }, { 5.f, 5.f }, { 0, PI32, 0 }));
    scene.AddQuad(GrassQuad({ -30, -5.f + 2.5f, -100 }, { 5.f, 5.f }, { 0, PI32 / 2.f, 0 }));
    scene.AddQuad(GrassQuad({ -30 + 0.1f, -5.f + 2.5f, -100 }, { 5.f, 5.f }, { 0, PI32 / 2.f + PI32, 0 }));

    scene.AddPointLight();
    scene.AddDirLight();
    scene.AddSpotLight();
    size_t flashLightIndex = scene.AddSpotLight();

    Texture grassTexture = loadTexture(FileManager::FromRoot("assets/textures/grass.png"));
    Texture grassNormalMap = loadTexture(FileManager::FromRoot("assets/textures/grass_normals.png"));

    // Movement constants
    const float camMoveSpeed = 10.f;
    const float camRotateSpeed = 1.f;

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

    // Timer to keep track of frametime for delta time
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

        static bool was_r_down = false;
        if (window.IsKeyDown(GLFW_KEY_R) && !was_r_down) {
            was_r_down = true;

            blinnPhongShader.HotReload();
        }
        if (!window.IsKeyDown(GLFW_KEY_R)) was_r_down = false;

        // Apply rotation and movement to the view transform
        scene.GetViewMatrix().Rotate(rotate.x, {1, 0, 0})
            .Rotate(rotate.y, {0, 1, 0})
            .Rotate(rotate.z, {0, 0, 1});

        // Move on local axes for camera
        Vec3 localMove = scene.GetViewMatrix().TransformDirection(move);
        scene.GetViewMatrix().Translate(localMove);

        SpotLight& flashLight = scene.GetSpotLightAt(flashLightIndex);

        flashLight.position = scene.GetViewMatrix().GetTranslation();
        flashLight.direction = scene.GetViewMatrix().TransformDirection({ 0, 0, -1 });

        scene.Draw(blinnPhongShader, grassTexture, grassNormalMap);

        // Poll events and swap window buffer
        window.PollEvents();
        window.SwapBuffers();
    }
}