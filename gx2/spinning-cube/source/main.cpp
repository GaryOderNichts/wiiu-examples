#include <whb/proc.h>
#include <whb/gfx.h>

#include <gx2/draw.h>
#include <gx2/registers.h>
#include <gx2/swap.h>
#include <gx2r/buffer.h>
#include <gx2r/draw.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader_gsh.h"

static const float cubePositions[] = {
    -1.0f, -1.0f,  1.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 1.0f,
     1.0f,  1.0f,  1.0f, 1.0f,
    -1.0f,  1.0f,  1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f, 1.0f,
     1.0f, -1.0f, -1.0f, 1.0f,
     1.0f,  1.0f, -1.0f, 1.0f,
    -1.0f,  1.0f, -1.0f, 1.0f,
};

static const float cubeColors[] = {
    1.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, 0.5f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.5f, 1.0f,
};

static uint16_t cubeIndices[] = {
    0, 1, 2, 2, 3, 0,
    1, 5, 6, 6, 2, 1,
    7, 6, 5, 5, 4, 7,
    4, 0, 3, 3, 7, 4,
    4, 5, 1, 1, 0, 4,
    3, 2, 6, 6, 7, 3,
};

int main(int argc, char const* argv[])
{
    // Initialize ProcUI
    WHBProcInit();
    
    // Initialize the graphics backend
    WHBGfxInit();

    // Load the shader group
    WHBGfxShaderGroup shader;
    WHBGfxLoadGFDShaderGroup(&shader, 0, shader_gsh);

    // Initialize the shader attributes
    WHBGfxInitShaderAttribute(&shader, "aPosition", 0, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32_32_32);
    WHBGfxInitShaderAttribute(&shader, "aColor", 1, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32_32_32);
    
    // Initialize the fetch shader
    WHBGfxInitFetchShader(&shader);

    // Read the location of the mvp uniform
    uint32_t mvpLocation = GX2GetVertexUniformVar(shader.vertexShader, "uModelViewProj")->offset;

    // Create an attribute buffer for vertex position data
    GX2RBuffer positionBuffer;
    positionBuffer.flags = GX2R_RESOURCE_BIND_VERTEX_BUFFER |
                         GX2R_RESOURCE_USAGE_CPU_READ |
                         GX2R_RESOURCE_USAGE_CPU_WRITE |
                         GX2R_RESOURCE_USAGE_GPU_READ;
    positionBuffer.elemSize = 4 * sizeof(float);
    positionBuffer.elemCount = 8;
    GX2RCreateBuffer(&positionBuffer);

    // Copy the position data into the buffer
    void* data = GX2RLockBufferEx(&positionBuffer, GX2R_RESOURCE_BIND_NONE);
    memcpy(data, cubePositions, sizeof(cubePositions));
    GX2RUnlockBufferEx(&positionBuffer, GX2R_RESOURCE_BIND_NONE);

    // Create an attribute buffer for vertex color data
    GX2RBuffer colorBuffer;
    colorBuffer.flags = GX2R_RESOURCE_BIND_VERTEX_BUFFER |
                         GX2R_RESOURCE_USAGE_CPU_READ |
                         GX2R_RESOURCE_USAGE_CPU_WRITE |
                         GX2R_RESOURCE_USAGE_GPU_READ;
    colorBuffer.elemSize = 4 * sizeof(float);
    colorBuffer.elemCount = 8;
    GX2RCreateBuffer(&colorBuffer);

    // Copy the color data into the buffer
    data = GX2RLockBufferEx(&colorBuffer, GX2R_RESOURCE_BIND_NONE);
    memcpy(data, cubeColors, sizeof(cubeColors));
    GX2RUnlockBufferEx(&colorBuffer, GX2R_RESOURCE_BIND_NONE);

    // Create a model and view matrix
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f));

    glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.0f, 0.0f, -4.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // Create a projection matrix for the TV and DRC
    GX2Surface* tvSurface = &WHBGfxGetTVColourBuffer()->surface;
    glm::mat4 tvProjectionMatrix = glm::perspective(glm::radians(45.0f), (float) tvSurface->width / tvSurface->height, 0.1f, 10.0f);

    GX2Surface* drcSurface = &WHBGfxGetDRCColourBuffer()->surface;
    glm::mat4 drcProjectionMatrix = glm::perspective(glm::radians(45.0f), (float) drcSurface->width / drcSurface->height, 0.1f, 10.0f);

    while (WHBProcIsRunning()) {
        // Rotate the cube
        modelMatrix = glm::rotate(modelMatrix, glm::radians(2.0f), glm::vec3(1.0f, 1.0f, 0.0f));

        // Start rendering the frame
        WHBGfxBeginRender();

        // Render TV
        WHBGfxBeginRenderTV();
        {
            // Clear the screen
            WHBGfxClearColor(0.2f, 0.2f, 0.2f, 1.0f);

            // Set the shader
            GX2SetFetchShader(&shader.fetchShader);
            GX2SetVertexShader(shader.vertexShader);
            GX2SetPixelShader(shader.pixelShader);

            // Enable depth testing
            GX2SetDepthOnlyControl(TRUE, TRUE, GX2_COMPARE_FUNC_LESS);

            // Calculate and set the mvpMatrix
            glm::mat4 mvpMatrix = tvProjectionMatrix * viewMatrix * modelMatrix;
            GX2SetVertexUniformReg(mvpLocation, 16, glm::value_ptr(mvpMatrix));

            // Set the attribute buffers
            GX2RSetAttributeBuffer(&positionBuffer, 0, positionBuffer.elemSize, 0);
            GX2RSetAttributeBuffer(&colorBuffer, 1, colorBuffer.elemSize, 0);

            // Draw
            GX2DrawIndexedImmediateEx(GX2_PRIMITIVE_MODE_TRIANGLES, 36, GX2_INDEX_TYPE_U16, cubeIndices, 0, 1);
        }
        WHBGfxFinishRenderTV();

        // Render DRC
        WHBGfxBeginRenderDRC();
        {
            WHBGfxClearColor(0.2f, 0.2f, 0.2f, 1.0f);

            GX2SetFetchShader(&shader.fetchShader);
            GX2SetVertexShader(shader.vertexShader);
            GX2SetPixelShader(shader.pixelShader);

            GX2SetDepthOnlyControl(TRUE, TRUE, GX2_COMPARE_FUNC_LESS);

            glm::mat4 mvpMatrix = drcProjectionMatrix * viewMatrix * modelMatrix;
            GX2SetVertexUniformReg(mvpLocation, 16, glm::value_ptr(mvpMatrix));

            GX2RSetAttributeBuffer(&positionBuffer, 0, positionBuffer.elemSize, 0);
            GX2RSetAttributeBuffer(&colorBuffer, 1, colorBuffer.elemSize, 0);

            GX2DrawIndexedImmediateEx(GX2_PRIMITIVE_MODE_TRIANGLES, 36, GX2_INDEX_TYPE_U16, cubeIndices, 0, 1);
        }
        WHBGfxFinishRenderDRC();

        // Finish the frame
        WHBGfxFinishRender();
    }

    // Free data and shut down graphics backend
    WHBGfxFreeShaderGroup(&shader);

    GX2RDestroyBufferEx(&positionBuffer, GX2R_RESOURCE_BIND_NONE);
    GX2RDestroyBufferEx(&colorBuffer, GX2R_RESOURCE_BIND_NONE);

    WHBGfxShutdown();
    WHBProcShutdown();

    return 0;
}

