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