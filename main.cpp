#define TINYOBJLOADER_IMPLEMENTATION
#define M_PI 3.14159265358979323846
#include "tiny_obj_loader.h"
#include "common/GLShader.h"
#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include <math.h>
#include <ostream>
#include <iostream>
#include <vector>


const int WINDOWS_WIDTH = 640;
const int WINDOWS_HEIGHT = 480;
const float camSpeed = 1.0f;
double lastX = 0, lastY = 0; // Position initiale (centre de la fenêtre)
bool firstMouse = true;

struct vec2 { float x, y; };
struct vec3 { float x, y, z; 
    vec3 operator-(const vec3& other) const {
        return {x - other.x, y - other.y, z - other.z};
    }

    vec3 operator*(float scalar) const {
        return {x * scalar, y * scalar, z * scalar};
    }

    vec3 operator+(const vec3& other) const {
        return {x + other.x, y + other.y, z + other.z};
    }

    float dot(const vec3& other) {
        return x * other.x + y * other.y + z * other.z;
    }

    vec3 cross(const vec3& other) const {
        return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        };
    }

    vec3 cross(vec3& other) {
        return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        };
    }

    vec3 normalize() const {
        float length = std::sqrt(x * x + y * y + z * z);
        return {x / length, y / length, z / length};
    }
};

struct vec4 { 
    float x, y, z , w; 

    float& operator[](int index) {
        switch (index) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            case 3: return w;
            default: throw std::out_of_range("Index out of range");
        }
    }

    const float& operator[](int index) const {
        switch (index) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            case 3: return w;
            default: throw std::out_of_range("Index out of range");
        }
    }    

    
};

struct Vertex{
    vec2 position;
    vec3 color;
    vec2 texcoords;
};


struct VertexTiny{
    vec3 position;
    vec3 normal;
    vec2 texcoords;
};

struct Mesh {
    VertexTiny* vertices;
    uint32_t vertexCount;
};


struct mat4
{
    vec4 matrix[4];

    vec4& operator[](int index) {
        if (index < 0 || index >= 4)
            throw std::out_of_range("Index out of range");
        return matrix[index];
    }

    const vec4& operator[](int index) const {
        if (index < 0 || index >= 4)
            throw std::out_of_range("Index out of range");
        return matrix[index];
    }

    mat4 operator*(const mat4& other) const {
        mat4 result;

        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result[i][j] = 0;
                for (int k = 0; k < 4; ++k) {
                    result[i][j] += matrix[k][i] * other[j][k];
                }
            }
        }

        return result;
    }
};

class Object
{
    private:   
        
        GLuint sizeVertices = 0;
        GLuint VAOObject = 0;
        GLuint VBOObject = 0;

    public: 
        GLShader BasicShader;
        tinyobj::material_t material;
        Object(const std::string& path, const char* VertexShader, const char* FragmentShader)
        {
            BasicShader.LoadVertexShader(VertexShader);
            BasicShader.LoadFragmentShader(FragmentShader);
            BasicShader.Create();
            int size = 0;
            tinyobj::attrib_t attributes;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warnings;
            std::string errors;

            tinyobj::LoadObj(&attributes, &shapes, &materials, &warnings, &errors, path.c_str());
            std::vector<VertexTiny> vertices;
            for (int i = 0; i < shapes.size(); i ++) {
                tinyobj::shape_t &shape = shapes[i];
                tinyobj::mesh_t &mesh = shape.mesh;
                size += mesh.indices.size();
                for (int j = 0; j < mesh.indices.size(); j++) {
                    tinyobj::index_t i = mesh.indices[j];
                    vec3 normal = {0.0f, 0.0f, 0.0f};
                    vec2 texCoord = {0.0f, 0.0f};
                    vec3 position = {
                        attributes.vertices[i.vertex_index * 3],
                        attributes.vertices[i.vertex_index * 3 + 1],
                        attributes.vertices[i.vertex_index * 3 + 2]
                    };
                    if(i.normal_index >= 0)
                        vec3 normal = {
                            attributes.normals[i.normal_index * 3],
                            attributes.normals[i.normal_index * 3 + 1],
                            attributes.normals[i.normal_index * 3 + 2]
                        };
                    if(i.texcoord_index >= 0)
                        vec2 texCoord = {
                            attributes.texcoords[i.texcoord_index * 2],
                            attributes.texcoords[i.texcoord_index * 2 + 1],
                        };
                    VertexTiny vert = { position, normal, texCoord };
                    vertices.push_back(vert);
                }
            }
            glGenVertexArrays(1, &VAOObject);
            glBindVertexArray(VAOObject);
            glGenBuffers(1, &VBOObject);
            glBindBuffer(GL_ARRAY_BUFFER, VBOObject);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, nullptr);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void *) (sizeof(float) * 3));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void *) (sizeof(float) * 6));
            sizeVertices = size*3;
        }

        Object()
        {
        }

        void Render()
        {
            glEnable(GL_CULL_FACE);
            glBindVertexArray(VAOObject);
	        glDrawArrays(GL_TRIANGLES, 0, sizeVertices);
        } 
    
};


class Camera
{
private:
    float ratio;
public:
    unsigned int width,height;
    vec3 c_position;
    vec3 t_position;
    Camera();
    Camera(int width, int height)
    {
        ratio = width / (float)height;
        c_position = vec3{0.0f, 0.0f, 0.0f};
    }
    mat4 LookAt() {
        vec3 up = {0.0f, 1.0f, 0.0f};
        vec3 forward = (t_position - c_position).normalize() * -1.0f;
        vec3 right = up.cross(forward).normalize();
        vec3 correctedUp = forward.cross(right).normalize();

        float dotRight = -right.dot(c_position);
        float dotUp = -correctedUp.dot(c_position);
        float dotForward = -forward.dot(c_position);

        mat4 viewMatrix = {vec4{right.x, correctedUp.x, forward.x, 0.0f},
                           vec4{right.y, correctedUp.y, forward.y, 0.0f},
                           vec4{right.z, correctedUp.z, forward.z, 0.0f},
                           vec4{dotRight, dotUp, dotForward, 1.0f}};

        return viewMatrix;
    }
};
Camera camera(WINDOWS_WIDTH, WINDOWS_HEIGHT);

struct Application
{
    GLShader g_BasicShader;
    GLuint VBO;
    GLuint IBO;
    GLuint VAO;
    Object suzanne;
    bool Initialise()
    {
        static const Vertex triangle[] = {
            {{0.5f, 0.5f},{1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f},{0.0f, 1.0f, 0.0f}},
            {{-0.5f, -0.5f},{0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f},{1.0f, 0.0f, 1.0f}}
        };

        unsigned int indices[]{
            0, 1, 3,
            1, 2, 3
        };
        suzanne = Object("./objects/suzanne.obj","suzanne.vs","suzanne.fs");

        g_BasicShader.LoadVertexShader("Basic.vs");
        g_BasicShader.LoadFragmentShader("Basic.fs");
        g_BasicShader.Create();
        auto basicProgram = g_BasicShader.GetProgram();
        glUseProgram(basicProgram);
        
        
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*4, triangle, GL_STATIC_DRAW);


        int loc_position = glGetAttribLocation(basicProgram, "a_position");
        glEnableVertexAttribArray(loc_position);
        glVertexAttribPointer(loc_position, 2, GL_FLOAT, false, sizeof(Vertex), 0);

        loc_position = glGetAttribLocation(basicProgram, "a_color");
        glEnableVertexAttribArray(loc_position);
        glVertexAttribPointer(loc_position, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, color));

        glGenBuffers(1, &IBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*6, indices, GL_STATIC_DRAW);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        
        
        return true;
    }

    void Terminate() {
        g_BasicShader.Destroy();
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &IBO);
    }

    void Render()
    {
        glViewport(0, 0, WINDOWS_WIDTH, WINDOWS_HEIGHT);
        glClearColor(0.5f, 0.5f, 0.5f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
        
        float currentTime = (float)glfwGetTime();

        auto basicProgram2 = suzanne.BasicShader.GetProgram();
        glUseProgram(basicProgram2);
        //Rotation
        mat4 matrix_rotation;
        Rotate_mx(&matrix_rotation,1.0f);
        Identity_m(&matrix_rotation);
        int timeLocation = glGetUniformLocation(basicProgram2, "m_rotation"); 
        glUniformMatrix4fv(timeLocation,1,GL_FALSE,(float*)&matrix_rotation);

        mat4 matrix_translation;
        Trans_m(&matrix_translation,0,0,-5.0f);
        timeLocation = glGetUniformLocation(basicProgram2, "m_translation"); 
        glUniformMatrix4fv(timeLocation,1,GL_FALSE,(float*)&matrix_translation);
        //Scaling
        float scale = 1.0f;
        mat4 matrix_scale = {vec4{scale, 0.0f, 0.0f,0.0f},vec4{0.0f,scale,0.0f,0.0f},vec4{0.0f,0.0f,scale,0.0f},vec4{0.0f,0.0f,0.0f,1.0f}};
        timeLocation = glGetUniformLocation(basicProgram2, "m_scale"); 
        glUniformMatrix4fv(timeLocation,1,GL_FALSE,(float*)&matrix_scale);

        //mat4 matrix_projection = {vec4{1.0f/WINDOWS_WIDTH, 0.0f, 0.0f,0.0f},vec4{0.0f,1.0f/WINDOWS_HEIGHT,0.0f,0.0f},vec4{0.0f,0.0f,1.0f,0.0f},vec4{0.0f,0.0f,0.0f,1.0f}};
        mat4 matrix_projection;
        //Ortho_m(&matrix_projection, -WINDOWS_WIDTH, WINDOWS_WIDTH, -WINDOWS_HEIGHT, WINDOWS_HEIGHT, 0.0f, 100.0f);
        Projo3D_m(&matrix_projection, 3.14f/4.0f,((float)WINDOWS_WIDTH/(float)WINDOWS_HEIGHT),0.1f,1000.0f);
        timeLocation = glGetUniformLocation(basicProgram2, "m_proj"); 
        glUniformMatrix4fv(timeLocation,1,GL_FALSE,(float*)&matrix_projection);

        mat4 matrix_world = CreateWorldMAtrix(matrix_translation,matrix_rotation,matrix_scale);
        timeLocation = glGetUniformLocation(basicProgram2, "m_worldMatrix"); 
        glUniformMatrix4fv(timeLocation,1,GL_FALSE,(float*)&matrix_world);

        
        mat4 matrix_view = camera.LookAt();
        timeLocation = glGetUniformLocation(basicProgram2, "m_viewMatrix"); 
        glUniformMatrix4fv(timeLocation,1,GL_FALSE,(float*)&matrix_view);
        
        suzanne.Render();

        /*
        auto basicProgram = g_BasicShader.GetProgram();
        glUseProgram(basicProgram);        

        //Rotation
        matrix_rotation = {vec4{1.0f * sin(currentTime), 0.0f, 0.0f,0.0f},vec4{0.0f,1.0f,0.0f,0.0f},vec4{0.0f,0.0f,1.0f,0.0f},vec4{1.0f,0.0f,-5.0f,1.0f}};
        timeLocation = glGetUniformLocation(basicProgram, "m_rotation"); 
        glUniformMatrix4fv(timeLocation,1,GL_FALSE,(float*)&matrix_rotation);

        //Scaling
        float scale2 = 1.0f;
        matrix_scale = {vec4{scale2, 0.0f, 0.0f,0.0f},vec4{0.0f,scale2,0.0f,0.0f},vec4{0.0f,0.0f,scale2,0.0f},vec4{0.0f,0.0f,0.0f,1.0f}};
        timeLocation = glGetUniformLocation(basicProgram, "m_scale"); 
        glUniformMatrix4fv(timeLocation,1,GL_FALSE,(float*)&matrix_scale);

        //mat4 matrix_projection = {vec4{1.0f/WINDOWS_WIDTH, 0.0f, 0.0f,0.0f},vec4{0.0f,1.0f/WINDOWS_HEIGHT,0.0f,0.0f},vec4{0.0f,0.0f,1.0f,0.0f},vec4{0.0f,0.0f,0.0f,1.0f}};
        matrix_projection;
        //Ortho_m(&matrix_projection, -WINDOWS_WIDTH, WINDOWS_WIDTH, -WINDOWS_HEIGHT, WINDOWS_HEIGHT, 0.0f, 100.0f);
        Projo3D_m(&matrix_projection, 3.14f/4.0f,((float)WINDOWS_WIDTH/(float)WINDOWS_HEIGHT),0.1f,1000.0f);
        timeLocation = glGetUniformLocation(basicProgram, "m_proj"); 
        glUniformMatrix4fv(timeLocation,1,GL_FALSE,(float*)&matrix_projection);
        
        
        //Face Culling
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        
        glBindVertexArray(VAO);        
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,0);
        */
        
        
    }

    void Identity_m(mat4* m){
        *m = {vec4{1.0f, 0.0f, 0.0f,0.0f},vec4{0.0f,1.0f,0.0f,0.0f},vec4{0.0f,0.0f,1.0f,0.0f},vec4{0.0f,0.0f,0.0f,1.0f}};
    }

    void Ortho_m(mat4* m, float left, float right, float bottom, float top, float znear, float zfar){
        *m  = {vec4{2/(right-left), 0.0f, 0.0f,0.0f},
               vec4{0.0f,2/(top-bottom),0.0f,0.0f},
               vec4{0.0f,0.0f,-2/(zfar-znear),0.0f},
               vec4{-(right+left)/(right-left),-(top+bottom)/(top-bottom),-(zfar+znear)/(zfar-znear),1.0f}};
    }

    void Projo3D_m(mat4* m, float fov,float aspect, float znear, float zfar){
        *m  = {vec4{(1/tanf(fov/2.0f))/aspect, 0.0f, 0.0f,0.0f},
               vec4{0.0f,1/tanf(fov/2.0f),0.0f,0.0f},
               vec4{0.0f,0.0f,-(zfar+znear)/(zfar-znear),-1.0f},
               vec4{0.0f,0.0f,(-2.0f*(zfar*znear))/(zfar-znear),0.0f}};
    }

    void Trans_m(mat4* m,float tx,float ty,float tz){
        *m = {vec4{1.0f, 0.0f, 0.0f,0.0f},vec4{0.0f,1.0f,0.0f,0.0f},vec4{0.0f,0.0f,1.0f,0.0f},vec4{tx,ty,tz,1.0f}};
    }

    void Scale_m(mat4* m,float sx,float sy,float sz){
        *m = {vec4{sx, 0.0f, 0.0f,0.0f},vec4{0.0f,sy,0.0f,0.0f},vec4{0.0f,0.0f,sz,0.0f},vec4{0.0f,0.0f,0.0f,1.0f}};
    }

    void Rotate_mx(mat4* m,float d){
        *m = {vec4{1.0f, 0.0f, 0.0f,0.0f},vec4{0.0f,cos(d),sin(d),0.0f},vec4{0.0f,-sin(d),cos(d),0.0f},vec4{0.0f,0.0f,0.0f,1.0f}};
    }

    void Rotate_my(mat4* m,float d){
        *m = {vec4{cos(d), 0.0f, -sin(d),0.0f},vec4{0.0f,1.0f,0.0f,0.0f},vec4{sin(d),0.0f,cos(d),0.0f},vec4{0.0f,0.0f,0.0f,1.0f}};
    }

    void Rotate_mz(mat4* m,float d){
        *m = {vec4{cos(d), sin(d), 0.0f,0.0f},vec4{-sin(d),cos(d),0.0f,0.0f},vec4{0.0f,0.0f,1.0f,0.0f},vec4{0.0f,0.0f,0.0f,1.0f}};
    }

    mat4 CreateWorldMAtrix(const mat4 m_trans,const mat4 m_roation,const mat4 m_scale){
        return m_trans*m_roation*m_scale;
    }

    void printVertex(VertexTiny vert){
        fprintf(stderr,"Vertice = x=%f  y=%f  z=%f\n",vert.position.x, vert.position.y, vert.position.z);
    }

};

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos; // Inverser car les coordonnées y vont de bas en haut

    lastX = xpos;
    lastY = ypos;
    camera.t_position.x += xoffset/200.0f;
    camera.t_position.y += yoffset/200.0f;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                break;
            case GLFW_KEY_UP:
                camera.t_position.y += camSpeed;
                camera.c_position.y += camSpeed;
                std::cout << "Flèche haut pressée" << std::endl;
                break;
            case GLFW_KEY_DOWN:
                camera.t_position.y -= camSpeed;
                camera.c_position.y -= camSpeed;
                std::cout << "Flèche bas pressée" << std::endl;
                break;
            case GLFW_KEY_LEFT:
                camera.t_position.x -= camSpeed;
                camera.c_position.x -= camSpeed;
                std::cout << "Flèche gauche pressée" << std::endl;
                break;
            case GLFW_KEY_RIGHT:
                camera.t_position.x += camSpeed;
                camera.c_position.x += camSpeed;
                std::cout << "Flèche droite pressée" << std::endl;
                break;
            default:
                break;
        }
    }
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(WINDOWS_WIDTH, WINDOWS_HEIGHT, "Projet OpenGl", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    
    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // ICI !
    GLenum error = glewInit();
    if (error != GLEW_OK) {
        std::cout << "Erreur d'initialisation de GLEW" << std::endl;
    }
    camera.t_position = {0.0f, 0.0f, -5.0f};
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, key_callback);
    Application app;
    app.Initialise();    
    
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {        
        glEnable(GL_DEPTH_TEST); 
        glDepthFunc(GL_LESS); 
        /* Render here */
        app.Render();
    
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
        
    }

    app.Terminate();

    glfwTerminate();
    return 0;
}