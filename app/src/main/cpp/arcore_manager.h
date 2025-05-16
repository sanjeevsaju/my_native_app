//
// Created by sanjeev on 02/05/25.
//

#ifndef MY_NATIVE_APP_ARCORE_MANAGER_H
#define MY_NATIVE_APP_ARCORE_MANAGER_H

#include "arcore_c_api.h"
#include <android/asset_manager.h>
#include <jni.h>
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <android/log.h>

#include <sys/syscall.h>
#include <unistd.h>
#include <pthread.h>
#define LOG_TAG "ARCore Manager"
#define LOG_TID(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "[TID:%ld] " __VA_ARGS__, syscall(SYS_gettid))
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

class ARCoreManager {
public:
    bool Initialize(void* env, jobject context, jobject activity);
    void Resume();
    void Pause();
    void OnSurfaceCreated();
    void OnDrawFrame(int width, int height, int displayRotation);
    void OnTouch(float x, float y);
    void RotateCube(float degrees);
    void ScaleCube(float scale);
    void TranslateCube(float x, float y, float z);
    void DrawVector(glm::vec3 start, glm::vec3 end);

private:

    float test_x_s = 0.0f;
    float test_y_s = 0.0f;
    float test_z_s = 0.0f;

    float test_x_e = 0.0f;
    float test_y_e = 0.0f;
    float test_z_e = 0.0f;

    ArSession* ar_session = nullptr;
    ArFrame* ar_frame = nullptr;

    int32_t screen_width = 0;
    int32_t screen_height = 0;

    GLfloat scaling_factor = 0.5f;

    GLuint plane_shader_program;
    GLuint camera_shader_program;
    GLuint object_shader_program;
    GLuint axis_shader_program;

    GLuint axis_vao;
    GLuint axis_vbo;

    GLuint cube_vao;
    GLuint cube_vbo;
    GLuint cube_index_buffer;

    GLuint plane_vao;
    GLuint plane_vbo;

    GLuint camera_vao;
    GLuint camera_vbo;

    GLuint test_vao;
    GLuint test_vbo;

    GLuint indexBuffer;
    GLuint cameraTextureId;

    bool object_place = false;
    glm::mat4 hit_pose_matrix = glm::mat4(0.0f);
    glm::vec3 hit_point_world_pos = glm::vec3(0);

    float cube_rotation_angle = 0.0f;
    glm::vec3 cube_rotation_axis = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cube_world_pos = glm::vec3(0.0f);

    glm::vec3 cube_translation_vector = glm::vec3(0.0f);
    glm::vec3 plane_normal = glm::vec3(0.0f);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 proj = glm::mat4(1.0f);
    glm::mat4 mvp = glm::mat4(1.0f);

};
#endif //MY_NATIVE_APP_ARCORE_MANAGER_H
