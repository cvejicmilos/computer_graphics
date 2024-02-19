#include <iostream>

#include "AppWindow.h"
#include "Model.h"
#include "Timer.h"
#include <filesystem>
#include <assert.h>
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


int main(int , char** argv)  {
    
    // Create Window
    AppWindow window("OpenGL", 800, 600);
    if (!window.IsValid()) {
        std::cerr << "Window init failed\n";
        return 1;
    }
    
    // Initialize GLAD (Load OpenGL)
    std::cout << "Initializing GLAD...\n";
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    std::cout << "OK!\n";

    // First command line argument is the executable
    FileManager::Init(argv[0]);

    // Load cottage Model
    Model cottageModel(FileManager::FromRoot("assets/models/cottage/cottage.obj"));

    // Load a basic Shader
    Shader basicShader(FileManager::FromRoot("assets/shaders/basic.vs"), FileManager::FromRoot("assets/shaders/basic.fs"));


    // Camera transformation
    Matrix4 projMatrix = Matrix4::Perspective(3.1415f / 4.f, 1280.0f / 720.0f, 0.1f, 1000.0f);
    Matrix4 viewMatrix = Matrix4::Identity();

    // Model transformation
    Matrix4 modelMatrix = Matrix4::CreateTranslation({ 0, -5, -100 });

    // Movement constants
    const float camMoveSpeed = 10.f;
    const float camRotateSpeed = 1.f;

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

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

        // Apply rotation and movement to the view transform
        viewMatrix.Rotate(rotate.x, {1, 0, 0})
            .Rotate(rotate.y, {0, 1, 0})
            .Rotate(rotate.z, {0, 0, 1});

        // Move on local axes for camera
        Vec3 localMove = viewMatrix.TransformDirection(move);
        viewMatrix.Translate(localMove);
        
        // Inverse the view matrix before passing it
        Matrix4 viewMatrxInverse = viewMatrix;
        viewMatrxInverse.Invert();

        // Draw the model with the shader and the transformations
        cottageModel.Draw(basicShader, modelMatrix, viewMatrxInverse, projMatrix);

        // Poll events and swap window buffer
        window.PollEvents();
        window.SwapBuffers();
    }
}