#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

// Defines several possible options for camera movement
enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Camera class
class Camera
{
public:
    // Camera attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // Euler angles
    float Yaw;
    float Pitch;

    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f, float pitch = 0.0f)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(2.5f),
          MouseSensitivity(0.1f), Zoom(45.0f)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Processes input received from any keyboard-like input system
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }

    // Processes input received from a mouse input system
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // Update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // Processes input received from a mouse scroll-wheel event
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

private:
    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // Calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // Also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

// Global variables
Camera camera(glm::vec3(50.0f, 20.0f, 50.0f));
float lastX = 400.0f;
float lastY = 300.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Texture loading function
unsigned int loadTexture(const char *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    // For now, we'll create a simple procedural texture
    // In a real implementation, you'd load from image files
    const int width = 256;
    const int height = 256;
    unsigned char *data = new unsigned char[width * height * 3];

    // Generate a simple procedural texture
    for (int i = 0; i < width * height; i++)
    {
        int x = i % width;
        int y = i / width;

        // Create different patterns based on texture type
        if (strstr(path, "grass"))
        {
            // Grass texture - green with some variation
            data[i * 3] = 34 + (rand() % 50);      // R
            data[i * 3 + 1] = 139 + (rand() % 50); // G
            data[i * 3 + 2] = 34 + (rand() % 50);  // B
        }
        else if (strstr(path, "rock"))
        {
            // Rock texture - gray with variation
            int gray = 100 + (rand() % 80);
            data[i * 3] = gray;     // R
            data[i * 3 + 1] = gray; // G
            data[i * 3 + 2] = gray; // B
        }
        else if (strstr(path, "sand"))
        {
            // Sand texture - beige
            data[i * 3] = 194 + (rand() % 40);     // R
            data[i * 3 + 1] = 178 + (rand() % 40); // G
            data[i * 3 + 2] = 128 + (rand() % 40); // B
        }
        else
        {
            // Default - brown earth
            data[i * 3] = 139 + (rand() % 40);    // R
            data[i * 3 + 1] = 69 + (rand() % 40); // G
            data[i * 3 + 2] = 19 + (rand() % 40); // B
        }
    }

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    delete[] data;
    return textureID;
}

// Terrain class
class Terrain
{
public:
    unsigned int VAO, VBO, EBO;
    int width, height;
    std::vector<float> heights;
    std::vector<unsigned int> indices;
    std::vector<float> vertices;
    unsigned int grassTexture, rockTexture, sandTexture, earthTexture;

    Terrain(int w, int h) : width(w), height(h)
    {
        // Load textures
        grassTexture = loadTexture("grass");
        rockTexture = loadTexture("rock");
        sandTexture = loadTexture("sand");
        earthTexture = loadTexture("earth");

        generateTerrain();
        setupMesh();
    }

    void generateTerrain()
    {
        heights.resize(width * height);

        // Generate heightmap using simple noise
        for (int z = 0; z < height; z++)
        {
            for (int x = 0; x < width; x++)
            {
                float heightValue = generateHeight(x, z);
                heights[z * width + x] = heightValue;
            }
        }

        // Generate vertices and indices
        generateMesh();
    }

    float generateHeight(int x, int z)
    {
        // Simple noise function for terrain generation
        float scale = 0.1f;
        float amplitude = 5.0f;

        float height = 0.0f;
        height += sin(x * scale) * cos(z * scale) * amplitude;
        height += sin(x * scale * 0.5f) * cos(z * scale * 0.5f) * amplitude * 0.5f;
        height += sin(x * scale * 0.25f) * cos(z * scale * 0.25f) * amplitude * 0.25f;

        return height;
    }

    void generateMesh()
    {
        vertices.clear();
        indices.clear();

        // Generate vertices
        for (int z = 0; z < height; z++)
        {
            for (int x = 0; x < width; x++)
            {
                float y = heights[z * width + x];

                // Position
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                // Normal (simplified - will calculate proper normals later)
                vertices.push_back(0.0f);
                vertices.push_back(1.0f);
                vertices.push_back(0.0f);

                // Texture coordinates
                vertices.push_back(x * 0.1f); // Scale for texture tiling
                vertices.push_back(z * 0.1f);
            }
        }

        // Generate indices
        for (int z = 0; z < height - 1; z++)
        {
            for (int x = 0; x < width - 1; x++)
            {
                unsigned int topLeft = z * width + x;
                unsigned int topRight = topLeft + 1;
                unsigned int bottomLeft = (z + 1) * width + x;
                unsigned int bottomRight = bottomLeft + 1;

                // First triangle
                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                // Second triangle
                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }
    }

    void setupMesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Texture coordinates
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void render()
    {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    float getHeight(float x, float z)
    {
        // Convert world coordinates to grid coordinates
        int gridX = (int)x;
        int gridZ = (int)z;

        if (gridX < 0 || gridX >= width - 1 || gridZ < 0 || gridZ >= height - 1)
        {
            return 0.0f;
        }

        // Bilinear interpolation for smooth height
        float xCoord = x - gridX;
        float zCoord = z - gridZ;

        float h00 = heights[gridZ * width + gridX];
        float h10 = heights[gridZ * width + gridX + 1];
        float h01 = heights[(gridZ + 1) * width + gridX];
        float h11 = heights[(gridZ + 1) * width + gridX + 1];

        float h0 = h00 * (1 - xCoord) + h10 * xCoord;
        float h1 = h01 * (1 - xCoord) + h11 * xCoord;

        return h0 * (1 - zCoord) + h1 * zCoord;
    }
};

// Vehicle class
class Vehicle
{
public:
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 rotation;
    float width, height, length;
    unsigned int VAO, VBO, EBO;

    Vehicle(float w = 2.0f, float h = 1.0f, float l = 4.0f)
        : width(w), height(h), length(l)
    {
        position = glm::vec3(50.0f, 10.0f, 50.0f);
        velocity = glm::vec3(0.0f);
        rotation = glm::vec3(0.0f);
        createMesh();
    }

    void createMesh()
    {
        // Create a cuboid mesh
        float w2 = width * 0.5f;
        float h2 = height * 0.5f;
        float l2 = length * 0.5f;

        float vertices[] = {
            // Front face
            -w2, -h2, l2, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            w2, -h2, l2, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
            w2, h2, l2, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
            -w2, h2, l2, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,

            // Back face
            -w2, -h2, -l2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
            w2, -h2, -l2, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
            w2, h2, -l2, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
            -w2, h2, -l2, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,

            // Left face
            -w2, -h2, -l2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -w2, -h2, l2, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            -w2, h2, l2, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
            -w2, h2, -l2, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,

            // Right face
            w2, -h2, -l2, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            w2, -h2, l2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            w2, h2, l2, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            w2, h2, -l2, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,

            // Top face
            -w2, h2, -l2, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            w2, h2, -l2, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            w2, h2, l2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
            -w2, h2, l2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,

            // Bottom face
            -w2, -h2, -l2, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
            w2, -h2, -l2, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
            w2, -h2, l2, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            -w2, -h2, l2, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f};

        unsigned int indices[] = {
            0, 1, 2, 2, 3, 0,       // Front
            4, 5, 6, 6, 7, 4,       // Back
            8, 9, 10, 10, 11, 8,    // Left
            12, 13, 14, 14, 15, 12, // Right
            16, 17, 18, 18, 19, 16, // Top
            20, 21, 22, 22, 23, 20  // Bottom
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Texture coordinates
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void update(float deltaTime, Terrain &terrain)
    {
        // Update position first
        position += velocity * deltaTime;

        // Get terrain height at current position
        float terrainHeight = terrain.getHeight(position.x, position.z);

        // Always adjust Y position to terrain height (with vehicle height offset)
        if (position.y > terrainHeight + height * 0.5f)
        {
            // Vehicle is above terrain - apply gravity
            velocity.y -= 9.8f * deltaTime;
        }
        else
        {
            // Vehicle is at or below terrain - snap to terrain surface
            position.y = terrainHeight + height * 0.5f;
            velocity.y = 0.0f;
        }

        // Damping for horizontal movement
        velocity.x *= 0.95f;
        velocity.z *= 0.95f;
    }

    void render()
    {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    glm::mat4 getModelMatrix()
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        return model;
    }
};

// Global vehicle pointer for input handling
Vehicle *globalVehicle = nullptr;

// Vertex Shader source code
const char *vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    layout (location = 2) in vec2 aTexCoord;
    
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    
    out vec3 FragPos;
    out vec3 Normal;
    out vec2 TexCoord;
    
    void main()
    {
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;
        TexCoord = aTexCoord;
        
        gl_Position = projection * view * vec4(FragPos, 1.0);
    }
)";

// Fragment Shader source code
const char *fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    
    in vec3 FragPos;
    in vec3 Normal;
    in vec2 TexCoord;
    
    uniform sampler2D grassTexture;
    uniform sampler2D rockTexture;
    uniform sampler2D sandTexture;
    uniform sampler2D earthTexture;
    uniform vec3 lightPos;
    uniform vec3 viewPos;
    uniform vec3 lightColor;
    uniform vec3 objectColor;
    
    void main()
    {
        // Check if we're rendering terrain or vehicle
        vec4 finalColor;
        
        // If objectColor is set (vehicle), use it directly
        if (objectColor.x > 0.0 || objectColor.y > 0.0 || objectColor.z > 0.0) {
            finalColor = vec4(objectColor, 1.0);
        } else {
            // Calculate height and slope for texture blending (terrain)
            float height = FragPos.y;
            float slope = 1.0 - dot(normalize(Normal), vec3(0.0, 1.0, 0.0));
            
            // Sample all textures
            vec4 grass = texture(grassTexture, TexCoord);
            vec4 rock = texture(rockTexture, TexCoord);
            vec4 sand = texture(sandTexture, TexCoord);
            vec4 earth = texture(earthTexture, TexCoord);
            
            // Blend textures based on height and slope
            // Low areas get sand
            if (height < 1.0) {
                finalColor = mix(sand, earth, smoothstep(0.0, 1.0, height));
            }
            // Medium areas get grass
            else if (height < 3.0) {
                finalColor = mix(earth, grass, smoothstep(1.0, 3.0, height));
            }
            // High areas get rock
            else {
                finalColor = mix(grass, rock, smoothstep(3.0, 5.0, height));
            }
            
            // Steep slopes get more rock
            if (slope > 0.3) {
                finalColor = mix(finalColor, rock, smoothstep(0.3, 0.7, slope));
            }
        }
        
        // Lighting calculations
        // Ambient
        float ambientStrength = 0.2;
        vec3 ambient = ambientStrength * lightColor;
        
        // Diffuse
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;
        
        // Specular
        float specularStrength = 0.3;
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
        vec3 specular = specularStrength * spec * lightColor;
        
        vec3 result = (ambient + diffuse + specular) * finalColor.rgb;
        FragColor = vec4(result, 1.0);
    }
)";

// Error callback for GLFW
void errorCallback(int error, const char *description)
{
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// Window resize callback
void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Mouse callback
void mouseCallback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// Scroll callback
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// Process input
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    // Vehicle controls
    if (globalVehicle)
    {
        float speed = 10.0f;
        float rotationSpeed = 90.0f; // degrees per second

        // Forward/Backward movement
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            float angle = glm::radians(globalVehicle->rotation.y);
            globalVehicle->velocity.x = -sin(angle) * speed;
            globalVehicle->velocity.z = -cos(angle) * speed;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            float angle = glm::radians(globalVehicle->rotation.y);
            globalVehicle->velocity.x = sin(angle) * speed;
            globalVehicle->velocity.z = cos(angle) * speed;
        }

        // Rotation
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            globalVehicle->rotation.y += rotationSpeed * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            globalVehicle->rotation.y -= rotationSpeed * deltaTime;
        }

        // Stop movement when no keys pressed
        if (glfwGetKey(window, GLFW_KEY_UP) != GLFW_PRESS &&
            glfwGetKey(window, GLFW_KEY_DOWN) != GLFW_PRESS)
        {
            globalVehicle->velocity.x *= 0.8f;
            globalVehicle->velocity.z *= 0.8f;
        }
    }
}

int main()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set GLFW error callback
    glfwSetErrorCallback(errorCallback);

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    GLFWwindow *window = glfwCreateWindow(800, 600, "3D Camera System", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Set framebuffer resize callback
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // Set mouse callbacks
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // Tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Set initial viewport
    glViewport(0, 0, 800, 600);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Build and compile our shader program
    // Vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Check for vertex shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    // Fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Check for fragment shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    // Link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }

    // Delete shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Set up vertex data (and buffer(s)) and configure vertex attributes
    float vertices[] = {
        // Positions (x, y, z)
        -0.5f, -0.5f, 0.0f, // Bottom left
        0.5f, -0.5f, 0.0f,  // Bottom right
        0.0f, 0.5f, 0.0f    // Top
    };

    // Vertex Buffer Object (VBO) and Vertex Array Object (VAO)
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Bind VAO first, then bind and set VBO, and then configure vertex attributes
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Note that this is allowed, the call to glVertexAttribPointer registered VBO
    // as the vertex attribute's bound vertex buffer object so we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterward to avoid other VAO calls
    // accidentally modifying this VAO, but this rarely happens.
    glBindVertexArray(0);

    // Create terrain
    Terrain terrain(100, 100);

    // Create vehicle
    Vehicle vehicle;

    // Global vehicle pointer
    globalVehicle = &vehicle;

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput(window);

        // Update vehicle
        vehicle.update(deltaTime, terrain);

        // Render
        // Set clear color (dark blue background)
        glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activate shader
        glUseProgram(shaderProgram);

        // Create transformations
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);

        // Retrieve the matrix uniform locations
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");

        // Pass them to the shaders
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Set lighting uniforms
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 50.0f, 20.0f, 50.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);

        // Bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, terrain.grassTexture);
        glUniform1i(glGetUniformLocation(shaderProgram, "grassTexture"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, terrain.rockTexture);
        glUniform1i(glGetUniformLocation(shaderProgram, "rockTexture"), 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, terrain.sandTexture);
        glUniform1i(glGetUniformLocation(shaderProgram, "sandTexture"), 2);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, terrain.earthTexture);
        glUniform1i(glGetUniformLocation(shaderProgram, "earthTexture"), 3);

        // Render terrain
        terrain.render();

        // Render vehicle with its own model matrix
        glm::mat4 vehicleModel = vehicle.getModelMatrix();
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(vehicleModel));

        // Use a simple color for the vehicle (red)
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.8f, 0.2f, 0.2f);

        vehicle.render();

        // Reset objectColor to zero for terrain rendering
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.0f, 0.0f, 0.0f);

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Optional: De-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // Clean up
    glfwTerminate();
    return 0;
}