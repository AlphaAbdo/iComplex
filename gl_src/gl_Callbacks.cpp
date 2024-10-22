
#include <macros.hpp>
#include "gl_Callbacks.hpp"

void char_callback(GLFWwindow* window, unsigned int codepoint) {
    #ifdef __USE_IMGUI
    if(betaWindow::p_io && betaWindow::p_io->WantTextInput)
        return;
    #endif
    char c = static_cast<char>(codepoint);

    switch (c) {
        case '\033':
        case 'q':
        case 'Q':
            printf("Quitting\n");
            glfwSetWindowShouldClose(window, true);
            break;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
            #ifdef __USE_IMGUI
            case GLFW_KEY_F1:
                betaWindow::enableIMGUI = !betaWindow::enableIMGUI;
                break;
            #endif
            case GLFW_KEY_LEFT:
                cOff[pX] -= 0.05f * scale;
                frameState = fSTATE::FRAME_SHIFT;
                break;

            case GLFW_KEY_UP:
                cOff[pY] += 0.05f * scale;
                frameState = fSTATE::FRAME_SHIFT;
                break;

            case GLFW_KEY_RIGHT:
                cOff[pX] += 0.05f * scale;
                frameState = fSTATE::FRAME_SHIFT;
                break;

            case GLFW_KEY_DOWN:
                cOff[pY] -= 0.05f * scale;
                frameState = fSTATE::FRAME_SHIFT;
                break;

            case GLFW_KEY_KP_ADD:
            case GLFW_KEY_EQUAL:  // Plus key
                scale /= 1.1f;
                frameState = fSTATE::FULL_COMPUTE;
                break;

            case GLFW_KEY_KP_SUBTRACT:
            case GLFW_KEY_MINUS:
                scale *= 1.1f;
                frameState = fSTATE::FULL_COMPUTE;
                break;

            default:
                break;
        }
    }
}


void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    new_SCR_SIZE[pX] = width;
    new_SCR_SIZE[pY] = height;

    glfwGetWindowPos(window, bufferOffset.data() + pX, bufferOffset.data() + pY);

}


void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    #ifdef __USE_IMGUI
    bool accuredOnIMGUI = (betaWindow::p_io)? ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) : false;
    #else
    bool accuredOnIMGUI = false;
    #endif
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !accuredOnIMGUI) {
        leftClicked = true;
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        leftClicked = false;
    }

    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS && !accuredOnIMGUI) {
        middleClicked = true;
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
        middleClicked = false;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && !accuredOnIMGUI) {
        rightClicked = true;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        rightClicked = false;
    }

    // Get cursor position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    lastx = static_cast<int>(xpos);
    lasty = static_cast<int>(ypos);

    mOff[pX] = 0.0;
    mOff[pY] = 0.0;
    dscale = 1.0;
}


void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    double fx = (xpos - lastx) / SCR_SIZE[pX] *avgLoopTime*10;
    double fy = (lasty - ypos) / SCR_SIZE[pY] *avgLoopTime*10;
    // printf("fx: %f\t, fy: %f\n", (xpos - lastx), (lasty - ypos));
    fx = std::log(std::abs(fx) + 1) * (fx < 0 ? -1 : 1);
    fy= std::log(std::abs(fy) + 1) * (fy < 0 ? -1 : 1);

    // printf("fx: %f\t, fy: %f\n", fx, fy);
    if (leftClicked) {
        mOff[pX] = fx * scale;
        mOff[pY] = fy * scale;
    } else {
        if (middleClicked) {
            
            if (fy > 0.0) {
                dscale = 1.0 - fy;
				mOff[pX] =  lastx;
				mOff[pY] =  (SCR_SIZE[pY] - lasty);
                // printf("pos: %d\t, %d\n", lastx, lasty);
            } else {
                dscale = 1.0 / (1.0 + fy);
                double s = SCR_SIZE[pY] * scale / SCR_SIZE[pX];
				mOff[pX] =  (0.5)* (1-dscale)*scale;
				mOff[pY] =  (0.5)* (1-dscale)*s;
            }
        } else {
            dscale = 1.0;
            mOff[pX] = 0.0;
            mOff[pY] = 0.0;
        }
    }

}



void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}