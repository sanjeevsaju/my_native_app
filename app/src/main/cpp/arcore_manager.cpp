#include "arcore_manager.h"

void TransformPoint(const float model_matrix[16], const float local_point[3], float world_point[3]) {
    /* Convert the local point to homogeneous coordinates */
    float local_point_homogeneous[4] = {local_point[0], local_point[1], local_point[2], 1.0f};
    float result[4] = {0};

    /* Perform matrix multiplication : result = model_matrix * local_point_homogeneous */
    /* The model_matrix is in column major order */
    /* I think this can be optimized using SIMD */
    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {
            result[i] += model_matrix[j * 4 + i] * local_point_homogeneous[j];
        }
    }

    world_point[0] = result[0];
    world_point[1] = result[1];
    world_point[2] = result[2];
}

/* Runs on the main thread */
bool ARCoreManager::Initialize(void *env, jobject context, jobject activity) {
//    LOG_TID("SANJU : ARCoreManager::Initialize - ");

    if(ar_session) return true;

    ArStatus status = ArSession_create((JNIEnv*)env, context, &ar_session);
    if(status != AR_SUCCESS) {
        LOGI("SANJU : ArSession_create error");
        return false;
    } else {
        LOGI("SANJU : ArSession_create success");
    }

    ArFrame_create(ar_session, &ar_frame);

    return true;
}

void ARCoreManager::Resume() {
//    LOG_TID("SANJU : ARCoreManager::Resume - ");
    if(ar_session) ArSession_resume(ar_session);
    LOGI("SANJU : ArSession_resume");
}

void ARCoreManager::Pause() {
//    LOG_TID("SANJU : ARCoreManager::Pause - ");
    if(ar_session) ArSession_pause(ar_session);
    LOGI("SANJU : ArSession_pause");
}

/* Runs on the GL thread */
void ARCoreManager::OnSurfaceCreated() {
//    LOG_TID("SANJU : ARCoreManager::OnSurfaceCreated - ");

    float test_vector_vertices[] = {
           test_x_s, test_y_s, test_z_s, 1.0f, 0.0f, 0.0f,
           test_x_e, test_y_e, test_z_e, 1.0f, 0.0f, 0.0f
    };

    glGenVertexArrays(1, &test_vao);
    glBindVertexArray(test_vao);

    glGenBuffers(1, &test_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, test_vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(test_vector_vertices), test_vector_vertices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /************ Axis Rendering Starts *************************/

    // Axis shaders
    const char* axisVertexShaderSource = R"(
    #version 300 es
    layout(location = 0) in vec3 vPosition;
    layout(location = 1) in vec3 vColor;
    out vec3 fragColor;
    uniform mat4 mvp;
    void main() {
        gl_Position = mvp * vec4(vPosition, 1.0);
        fragColor = vColor;
    }
)";

    const char* axisFragmentShaderSource = R"(
    #version 300 es
    precision mediump float;
    in vec3 fragColor;
    out vec4 outColor;
    void main() {
        outColor = vec4(fragColor, 1.0);
    }
)";

// Compile and link axis shaders
    GLuint axisVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(axisVertexShader, 1, &axisVertexShaderSource, nullptr);
    glCompileShader(axisVertexShader);

    GLuint axisFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(axisFragmentShader, 1, &axisFragmentShaderSource, nullptr);
    glCompileShader(axisFragmentShader);

    axis_shader_program = glCreateProgram();
    glAttachShader(axis_shader_program, axisVertexShader);
    glAttachShader(axis_shader_program, axisFragmentShader);
    glLinkProgram(axis_shader_program);

    // Axis data (3 lines: X, Y, Z)
    float axis_vertices[] = {
            // Positions         // Colors (RGB)
            // X-axis (red)
            0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,

            // Y-axis (green)
            0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,

            // Z-axis (blue)
            0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f
    };

// Create VAO/VBO for axes
    glGenVertexArrays(1, &axis_vao);
    glBindVertexArray(axis_vao);

    glGenBuffers(1, &axis_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, axis_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axis_vertices), axis_vertices, GL_STATIC_DRAW);

// Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

// Color attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);

    /*********** Axis Rendering Stops **************************/

    /* Object vertex shader source code */
    const char* planeVertexShaderSource = R"(
        #version 300 es
        layout(location = 0) in vec3 vPosition;
        uniform mat4 mvp;
        void main() {
            gl_Position = mvp * vec4(vPosition, 1.0);
        }
    )";

    /* Object fragment shader source code with ES3 syntax */
    const char* planeFragmentShaderSource = R"(
        #version 300 es
        precision mediump float;
        out vec4 fragColor;
        void main() {
            fragColor = vec4(0.0, 1.0, 0.0, 0.1);
        }
    )";

    /* Compile and link object rendering shaders */
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &planeVertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &planeFragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    plane_shader_program = glCreateProgram();
    glAttachShader(plane_shader_program, vertexShader);
    glAttachShader(plane_shader_program, fragmentShader);
    glLinkProgram(plane_shader_program);

    /* Camera vertex shader source code */
    const char* cameraVertexShaderSource = R"(
        #version 300 es
        layout(location = 0) in vec4 a_Position;
        layout(location = 1) in vec2 a_TexCoord;
        out vec2 v_TexCoord;
        uniform int u_Rotation;

        void main() {
            gl_Position = a_Position;
            switch(u_Rotation) {
                case 1: // 90° counter-clockwise (landscape)
                    v_TexCoord = vec2(a_TexCoord.x, a_TexCoord.y);
                    break;
                case 2: // 180° (upside down portrait)
                    v_TexCoord = vec2(1.0 - a_TexCoord.y, a_TexCoord.x);
                    break;
                case 3: // 270° counter-clockwise (reverse landscape)
                    v_TexCoord = vec2(1.0 - a_TexCoord.x, 1.0 - a_TexCoord.y);
                    break;
                default: // 0° (portrait)
                    // Rotate 90° clockwise from landscape to portrait
                    v_TexCoord = vec2(a_TexCoord.y, 1.0 - a_TexCoord.x);
            }
        }
    )";

    /* Camera fragment shader source code */
    const char* cameraFragmentShaderSource = R"(
        #version 300 es
        #extension GL_OES_EGL_image_external_essl3 : require
        precision mediump float;
        in vec2 v_TexCoord;
        layout(location = 0) out vec4 outColor;
        uniform samplerExternalOES u_Texture;

        void main() {
            outColor = texture(u_Texture, v_TexCoord);
        }
    )";

    /* Compile and link object rendering shaders */
    GLuint cameraVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(cameraVertexShader, 1, &cameraVertexShaderSource, nullptr);
    glCompileShader(cameraVertexShader);

    GLuint cameraFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(cameraFragmentShader, 1, &cameraFragmentShaderSource, nullptr);
    glCompileShader(cameraFragmentShader);

    camera_shader_program = glCreateProgram();
    glAttachShader(camera_shader_program, cameraVertexShader);
    glAttachShader(camera_shader_program, cameraFragmentShader);
    glLinkProgram(camera_shader_program);

    const char* objectVertexShaderSource = R"(
        #version 300 es
        layout(location = 0) in vec3 vPosition;
        layout(location = 1) in vec4 vColor;
        out vec4 frag_vColor;
        uniform mat4 mvp;
        void main() {
            gl_Position = mvp * vec4(vPosition, 1.0);
            frag_vColor = vColor;
        }
    )";

    // Fragment shader source with ES3 syntax
    const char* objectFragmentShaderSource = R"(
        #version 300 es
        precision mediump float;
        in vec4 frag_vColor;
        out vec4 fragColor;
        void main() {
            fragColor = frag_vColor;
        }
    )";

    // Compile and link shaders
    GLuint objectVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(objectVertexShader, 1, &objectVertexShaderSource, nullptr);
    glCompileShader(objectVertexShader);

    GLuint objectFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(objectFragmentShader, 1, &objectFragmentShaderSource, nullptr);
    glCompileShader(objectFragmentShader);

    object_shader_program = glCreateProgram();
    glAttachShader(object_shader_program, objectVertexShader);
    glAttachShader(object_shader_program, objectFragmentShader);
    glLinkProgram(object_shader_program);

    /* ------------------------------------------------------------------------------------------ */

    glGenTextures(1, &cameraTextureId);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, cameraTextureId);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* Set texture for ARCore */
    ArSession_setCameraTextureName(ar_session, cameraTextureId);

    float quad_vertices[] = {
            // positions                //tex coords
            -1.0f, -1.0f,   0.0f, 1.0f,
            1.0f, -1.0f,   1.0f, 1.0f,
            -1.0f,  1.0f,   0.0f, 0.0f,
            1.0f,  1.0f,   1.0f, 0.0f
    };

    glGenVertexArrays(1, &camera_vao);
    glBindVertexArray(camera_vao);

    glGenBuffers(1, &camera_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, camera_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);   // Position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);   // Texture
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    /* ------------------------------------------------------------------------------------------ */


    /************************************************************************************************/
    const GLfloat cube_vertices[] = {
            // Positions        // Colors (R, G, B, A)
            // Front face
            -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, // Bottom-left (red)
            0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, // Bottom-right (red)
            0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, // Top-right (red)
            -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, // Top-left (red)

            // Back face
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, // Bottom-left (green)
            0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, // Bottom-right (green)
            0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, // Top-right (green)
            -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, // Top-left (green)

            // Left face
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, // Bottom-left (blue)
            -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, // Bottom-right (blue)
            -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, // Top-right (blue)
            -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, // Top-left (blue)

            // Right face
            0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, // Bottom-left (yellow)
            0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f, // Bottom-right (yellow)
            0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f, // Top-right (yellow)
            0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, // Top-left (yellow)

            // Top face
            -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f, // Bottom-left (magenta)
            0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f, // Bottom-right (magenta)
            0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 1.0f, // Top-right (magenta)
            -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 1.0f, // Top-left (magenta)

            // Bottom face
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, // Bottom-left (cyan)
            0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, // Bottom-right (cyan)
            0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, // Top-right (cyan)
            -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f  // Top-left (cyan)
    };

    const GLuint indices[] = {
            // Front face
            0, 1, 2, 0, 2, 3,
            // Back face
            4, 5, 6, 4, 6, 7,
            // Left face
            8, 9, 10, 8, 10, 11,
            // Right face
            12, 13, 14, 12, 14, 15,
            // Top face
            16, 17, 18, 16, 18, 19,
            // Bottom face
            20, 21, 22, 20, 22, 23
    };

    glGenVertexArrays(1, &cube_vao);
    glBindVertexArray(cube_vao);

    // Vertex buffer
    glGenBuffers(1, &cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    // Index buffer
    glGenBuffers(1, &cube_index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    /* Position */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);

    /* Color */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));

    glBindVertexArray(0);
    /************************************************************************************************/

    glGenBuffers(1, &plane_vbo);
}

/* Runs on the GL thread */
void ARCoreManager::OnDrawFrame(int width, int height, int displayRotation) {
    if(!ar_session) return;

    screen_width = width;
    screen_height = height;

    ArSession_setDisplayGeometry(ar_session, displayRotation, width, height);
    ArStatus status = ArSession_update(ar_session, ar_frame);

    ArCamera* camera;
    ArFrame_acquireCamera(ar_session, ar_frame, &camera);
    ArCamera_getViewMatrix(ar_session, camera, glm::value_ptr(view));
    ArCamera_getProjectionMatrix(ar_session, camera, /*near*/ 0.1f, /*far*/ 100.0f, glm::value_ptr(proj));
    ArCamera_release(camera);
    mvp = proj * view;

    /* Render camera frame image texture using OpenGL */
    glUseProgram(camera_shader_program);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, cameraTextureId);
    glUniform1i(glGetUniformLocation(camera_shader_program, "u_Texture"), 0);

    GLuint rotationLocation = glGetUniformLocation(camera_shader_program, "u_Rotation");
    glUniform1i(rotationLocation, displayRotation);

    glBindVertexArray(camera_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glUseProgram(0);

    /*********************** Axis Rendering Starts *************************/
    glUseProgram(axis_shader_program);
    glEnable(GL_DEPTH_TEST);
    /* Accept fragment if it closer to the camera than the former one */
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Use identity model matrix to draw at world origin
    glm::mat4 axis_model = glm::mat4(1.0f);
    glm::mat4 axis_mvp = proj * view;

    GLuint mvpLocation = glGetUniformLocation(axis_shader_program, "mvp");
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(axis_mvp));

    glBindVertexArray(axis_vao);
    glLineWidth(5.0f); // Make lines thicker
    glDrawArrays(GL_LINES, 0, 6); // 6 vertices = 3 lines
    glBindVertexArray(0);

    glUseProgram(0);

    /*********************** Axis Rendering Stops *************************/

    /******************** Test Vector rendering starts *************************/

    float test_vector_vertices[] = {
            test_x_s, test_y_s, test_z_s, 1.0f, 1.0f, 0.0f,
            test_x_e, test_y_e, test_z_e, 1.0f, 1.0f, 0.0f
    };

    glBindVertexArray(test_vao);
    glBindBuffer(GL_ARRAY_BUFFER, test_vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(test_vector_vertices), test_vector_vertices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glUseProgram(axis_shader_program);
    glEnable(GL_DEPTH_TEST);
    /* Accept fragment if it closer to the camera than the former one */
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 test_mvp = proj * view;

    GLuint test_mvpLocation = glGetUniformLocation(axis_shader_program, "mvp");
    glUniformMatrix4fv(test_mvpLocation, 1, GL_FALSE, glm::value_ptr(test_mvp));

    glBindVertexArray(test_vao);
    glLineWidth(10.0f);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);

    /******************** Test Vector rendering ends *************************/

    /* Plane detection logic */
    ArTrackableList* planes;
    ArTrackableList_create(ar_session, &planes);
    ArSession_getAllTrackables(ar_session, AR_TRACKABLE_PLANE, planes);

    int count = 0;
    ArTrackableList_getSize(ar_session, planes, &count);

    /* Check if rest of the code can be optimized using SIMD */
    for(int i = 0; i < count; i++) {
        ArTrackable *trackable;
        ArTrackableList_acquireItem(ar_session, planes, i, &trackable);
        ArTrackingState state;
        ArTrackable_getTrackingState(ar_session, trackable, &state);

        if(state == AR_TRACKING_STATE_TRACKING) {
            /* Process plane data */
            ArPlane* plane = reinterpret_cast<ArPlane*>(trackable);
            ArPose* center_pose = nullptr;

            ArPose_create(ar_session, nullptr, &center_pose);
            ArPlane_getCenterPose(ar_session, plane, center_pose);

            float model_matrix[16];
            ArPose_getMatrix(ar_session, center_pose, model_matrix);

            float extent_x = 0.0f;
            float extent_z = 0.0f;
            ArPlane_getExtentX(ar_session, plane, &extent_x);
            ArPlane_getExtentZ(ar_session, plane, &extent_z);

            int32_t polygon_size = 0;
            ArPlane_getPolygonSize(ar_session, plane, &polygon_size);
            std::vector<float> polygon(polygon_size);
            ArPlane_getPolygon(ar_session, plane, polygon.data());

            /* Transforming the planes' polygon vertices from local to world coordinates */
            std::vector<float> world_vertices;
            for(int i = 0; i < polygon_size; i += 2) {
                float local_point[3] = { polygon[i], 0.0f, polygon[i + 1]};
                float world_point[3];
                TransformPoint(model_matrix, local_point, world_point);
                world_vertices.push_back(world_point[0]);
                world_vertices.push_back(world_point[1]);
                world_vertices.push_back(world_point[2]);
            }

            /* Draw the polygon (plane) */
            glUseProgram(plane_shader_program);
            /* Enable depth test */
            glEnable(GL_DEPTH_TEST);
            /* Accept fragment if it closer to the camera than the former one */
            glDepthFunc(GL_LESS);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            GLuint mvpLocation = glGetUniformLocation(plane_shader_program, "mvp");
            glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));

            glBindBuffer(GL_ARRAY_BUFFER, plane_vbo);
            glBufferData(GL_ARRAY_BUFFER, world_vertices.size() * sizeof(float) , world_vertices.data(), GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

            glDrawArrays(GL_TRIANGLE_FAN, 0, world_vertices.size() / 3);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glUseProgram(0);
        }
        ArTrackable_release(trackable);
    }
    ArTrackableList_destroy(planes);

    /* Render the object here */
    if(object_place) {
        glUseProgram(object_shader_program);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glm::mat4 cube_model = hit_pose_matrix;
        glm::mat4 cube_scale = glm::scale(glm::mat4(1.0f), glm::vec3(scaling_factor));
        glm::mat4 cube_rotation = glm::rotate(glm::mat4(1.0f), cube_rotation_angle, cube_rotation_axis);
        glm::mat4 cube_translation = glm::translate(glm::mat4(1.0f), cube_translation_vector);
        glm::mat4 cube_mvp = proj * view * cube_translation * cube_model * cube_rotation * cube_scale;

        GLuint mvpLocation = glGetUniformLocation(object_shader_program, "mvp");
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(cube_mvp));

        glBindVertexArray(cube_vao);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glUseProgram(0);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }
}