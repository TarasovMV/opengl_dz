#include "common.h"
#include "ShaderProgram.h"
#include "Simulator.h"
#include "Visualizer.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <memory>
#include "main.h"

static const GLsizei WIDTH = 1024, HEIGHT = 1024;

bool g_filling = true;

std::shared_ptr<Simulator>  pSim;
std::shared_ptr<Visualizer> pVis;


void drag(GLFWwindow*  window, double mx, double my)
{
  int xi, yi, X, Y;
  double  dx, dy, len;
  static int last_mouse_x = 0;
  static int last_mouse_y = 0;

  // Compute the array index that corresponds to the cursor location
  xi = (int)clamp((double)(pSim->DIM() + 1) * ((double)mx / (double)WIDTH));
  yi = (int)clamp((double)(pSim->DIM() + 1) * ((double)(HEIGHT - my) / (double)HEIGHT));

  X = xi;
  Y = yi;

  if (X > (pSim->DIM() - 1))
    X = pSim->DIM() - 1;
  if (Y > (pSim->DIM() - 1))
    Y = pSim->DIM() - 1;

  if (X < 0)
    X = 0;
  if (Y < 0)
    Y = 0;

  // Add force at the cursor location
  my = HEIGHT - my;
  dx = mx - last_mouse_x; dy = my - last_mouse_y;
  len = sqrt(dx * dx + dy * dy);
  if (len != 0.0)
  {
    dx *= 0.1 / len;
    dy *= 0.1 / len;
  }

  pSim->AddForce(X, Y, dx, dy);
  pSim->SetDensity(X, Y, 10.0f);

  last_mouse_x = mx;
  last_mouse_y = my;
}

static void key(GLFWwindow* window, int k, int s, int action, int mods)
{
  if (action != GLFW_PRESS)
    return;

  switch (k)
  {
    case GLFW_KEY_R: pSim->DecreaseTimestep(0.001); break;
    case GLFW_KEY_T: pSim->IncreaseTimestep(0.001); break;
    case GLFW_KEY_V: pSim->MultiplyViscosity(5.0); break;
    case GLFW_KEY_B: pSim->MultiplyViscosity(0.2); break;
    case GLFW_KEY_N: pSim->ToggleFreezeSim(); break;

    case GLFW_KEY_C: pVis->NextDirectionColormap(); break;
    case GLFW_KEY_S: pVis->MultiplyGlyphLength(1.2f); break;
    case GLFW_KEY_A: pVis->MultiplyGlyphLength(0.8f); break;
    case GLFW_KEY_X: pVis->ToggleDrawSmoke(); break;
    case GLFW_KEY_Z: pVis->ToggleDrawGlyphs(); break;

	/// Переключение цветовой карты
    case GLFW_KEY_M: pVis->NextScalarColormap(); break;
	/// Переключение скалярной величины
    case GLFW_KEY_P: pVis->NextScalarObject(); break;
	/// Переключение скалярной величины
	case GLFW_KEY_U: pVis->NextGlyphType(); break;
    
    case GLFW_KEY_ESCAPE: exit(0);
    case GLFW_KEY_SPACE:
        if (g_filling == 0)
        {
          glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
          g_filling = true;
        }
        else
        {
          glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
          g_filling = false;
        }
        break;
    default:
      return;
  }
}

int initGL()
{
	int res = 0;
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		return -1;
	}

	std::cout << "Vendor: "   << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: "  << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL: "     << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	return 0;
}

int main(int argc, char** argv)
{
	///частота расстановки глифов
  int grid_size = 128;

	if(!glfwInit())
    return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  GLFWwindow*  window = glfwCreateWindow(WIDTH, HEIGHT, "Fluid sim visualization", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	
	glfwMakeContextCurrent(window); 
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetCursorPosCallback(window, drag);
	glfwSetKeyCallback(window, key);

	if(initGL() != 0) 
		return -1;
	
  //Reset any OpenGL errors which could be present for some reason
	GLenum gl_error = glGetError();
	while (gl_error != GL_NO_ERROR)
		gl_error = glGetError();

	std::unordered_map<GLenum, std::string> shaders;
	shaders[GL_VERTEX_SHADER]   = "vertex.vert";
	shaders[GL_FRAGMENT_SHADER] = "fragment.frag";
	ShaderProgram program(shaders); GL_CHECK_ERRORS;

//  glfwSwapInterval(1); // force 60 frames per second

  pSim = std::make_shared<Simulator>();
  pVis = std::make_shared<Visualizer>();

  pSim->InitSimulation(grid_size);
  pVis->Init(grid_size, WIDTH, HEIGHT);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

    glViewport  (0, 0, WIDTH, HEIGHT);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear     (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pSim->StepSimulation();

    program.StartUseShader();
    pVis->UpdateBuffers(pSim);
    pVis->Draw();
    program.StopUseShader();

    glFlush();

		glfwSwapBuffers(window); 
	}

  pSim = nullptr;
	pVis = nullptr;

	glfwTerminate();
	return 0;
}
