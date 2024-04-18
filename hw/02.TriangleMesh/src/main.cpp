﻿///// main.cpp
///// OpenGL 3+, GLSL 1.20, GLEW, GLFW3

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <fstream>
#include <cassert>

// include glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// model hpp
#include "models/cube_triangle_soup.hpp"
#include "models/avocado_triangle_soup.hpp"
#include "models/donut_triangle_soup.hpp"
#include "models/cube_vlist_triangles.hpp"
#include "models/avocado_vlist_triangles.hpp"
#include "models/donut_vlist_triangles.hpp"

////////////////////////////////////////////////////////////////////////////////
/// Model 관련 설정
////////////////////////////////////////////////////////////////////////////////
enum MeshModel { kCube, kAvocado, kDonut }; 
enum MeshType { kTriangleSoup, kVlistTriangles };

MeshModel g_mesh_model = kCube;
MeshType  g_mesh_type = kTriangleSoup;

GLsizeiptr  g_position_size, g_color_size, g_index_size;      // the size in bytes of vbo & ibo
void        *g_position_data, *g_color_data, *g_index_data;   // a pointer to data will be copied
size_t      g_num_position = 0;   // # of vertex info. in a triangle soup
size_t      g_num_index = 0;    // # of index info. in a vertex list & triangles


////////////////////////////////////////////////////////////////////////////////
/// 쉐이더 관련 변수 및 함수
////////////////////////////////////////////////////////////////////////////////
GLuint  program;          // 쉐이더 프로그램 객체의 레퍼런스 값
GLint   loc_a_position;   // attribute 변수 a_position 위치
GLint   loc_a_color;      // attribute 변수 a_color 위치

GLint   loc_u_PVM;        // uniform 변수 u_PVM 위치

GLuint  position_buffer;  // GPU 메모리에서 position_buffer의 위치
GLuint  color_buffer;     // GPU 메모리에서 color_buffer의 위치
GLuint  index_buffer;     // GPU 메모리에서 index_buffer의 위치

GLuint create_shader_from_file(const std::string& filename, GLuint shader_type);
void init_shader_program();
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// 변환 관련 변수 및 함수
////////////////////////////////////////////////////////////////////////////////
glm::mat4     mat_model, mat_view, mat_proj;
glm::mat4     mat_PVM;

float g_angle = 0.0f;

void set_transform();
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
/// 렌더링 관련 변수 및 함수
////////////////////////////////////////////////////////////////////////////////

void init_buffer_objects();     // VBO init 함수: GPU의 VBO를 초기화하는 함수.
void update_buffer_objects();
void render_object();           // rendering 함수: 물체(삼각형)를 렌더링하는 함수.
////////////////////////////////////////////////////////////////////////////////


// GLSL 파일을 읽어서 컴파일한 후 쉐이더 객체를 생성하는 함수
GLuint create_shader_from_file(const std::string& filename, GLuint shader_type)
{
  GLuint shader = 0;

  shader = glCreateShader(shader_type);

  std::ifstream shader_file(filename.c_str());
  std::string shader_string;

  shader_string.assign(
    (std::istreambuf_iterator<char>(shader_file)),
    std::istreambuf_iterator<char>());

  // Get rid of BOM in the head of shader_string
  // Because, some GLSL compiler (e.g., Mesa Shader compiler) cannot handle UTF-8 with BOM
  if (shader_string.compare(0, 3, "\xEF\xBB\xBF") == 0)  // Is the file marked as UTF-8?
  {
    std::cout << "Shader code (" << filename << ") is written in UTF-8 with BOM" << std::endl;
    std::cout << "  When we pass the shader code to GLSL compiler, we temporarily get rid of BOM" << std::endl;
    shader_string.erase(0, 3);                  // Now get rid of the BOM.
  }

  const GLchar* shader_src = shader_string.c_str();
  glShaderSource(shader, 1, (const GLchar * *)& shader_src, NULL);
  glCompileShader(shader);

  GLint is_compiled;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);
  if (is_compiled != GL_TRUE)
  {
    std::cout << "Shader COMPILE error: " << std::endl;

    GLint buf_len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &buf_len);

    std::string log_string(1 + buf_len, '\0');
    glGetShaderInfoLog(shader, buf_len, 0, (GLchar *)log_string.c_str());

    std::cout << "error_log: " << log_string << std::endl;

    glDeleteShader(shader);
    shader = 0;
  }

  return shader;
}

// vertex shader와 fragment shader를 링크시켜 program을 생성하는 함수
void init_shader_program()
{
  GLuint vertex_shader
    = create_shader_from_file("./shader/vertex.glsl", GL_VERTEX_SHADER);

  std::cout << "vertex_shader id: " << vertex_shader << std::endl;
  assert(vertex_shader != 0);

  GLuint fragment_shader
    = create_shader_from_file("./shader/fragment.glsl", GL_FRAGMENT_SHADER);

  std::cout << "fragment_shader id: " << fragment_shader << std::endl;
  assert(fragment_shader != 0);

  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  GLint is_linked;
  glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
  if (is_linked != GL_TRUE)
  {
    std::cout << "Shader LINK error: " << std::endl;

    GLint buf_len;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &buf_len);

    std::string log_string(1 + buf_len, '\0');
    glGetProgramInfoLog(program, buf_len, 0, (GLchar *)log_string.c_str());

    std::cout << "error_log: " << log_string << std::endl;

    glDeleteProgram(program);
    program = 0;
  }

  std::cout << "program id: " << program << std::endl;
  assert(program != 0);

  loc_u_PVM = glGetUniformLocation(program, "u_PVM");

  loc_a_position = glGetAttribLocation(program, "a_position");
  loc_a_color = glGetAttribLocation(program, "a_color");
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_1 && action == GLFW_PRESS)
  {
    g_mesh_model = kCube;
    g_mesh_type = kTriangleSoup;
    update_buffer_objects();
  }
  if (key == GLFW_KEY_2 && action == GLFW_PRESS)
  {
    g_mesh_model = kAvocado;
    g_mesh_type = kTriangleSoup;
    update_buffer_objects();
  }
  if (key == GLFW_KEY_3 && action == GLFW_PRESS)
  {
    g_mesh_model = kDonut;
    g_mesh_type = kTriangleSoup;
    update_buffer_objects();
  }
  if (key == GLFW_KEY_4 && action == GLFW_PRESS)
  {
    g_mesh_model = kCube;
    g_mesh_type = kVlistTriangles;
    update_buffer_objects();
  }
  if (key == GLFW_KEY_5 && action == GLFW_PRESS)
  {
    g_mesh_model = kAvocado;
    g_mesh_type = kVlistTriangles;
    update_buffer_objects();
  }
  if (key == GLFW_KEY_6 && action == GLFW_PRESS)
  {
    g_mesh_model = kDonut;
    g_mesh_type = kVlistTriangles;
    update_buffer_objects();
  }
}

void update_buffer_objects()
{
  /////////////////////////////////////////////////////////////////////
  /// TODO: 아래 코드를 적절히 수정하여 프로그램을 완성하시오.
  /////////////////////////////////////////////////////////////////////

  if (g_mesh_model == kCube)
  {
    if (g_mesh_type == kTriangleSoup)
    {
      g_position_size = sizeof(cube::triangle_soup::position);
      g_position_data = cube::triangle_soup::position;

      g_color_size = sizeof(cube::triangle_soup::color);
      g_color_data = cube::triangle_soup::color;

      assert(g_position_size == g_color_size);
      g_num_position = cube::triangle_soup::num_position;
    } 
    else
    {
      g_position_size = sizeof(cube::vlist_triangles::position);
      g_position_data = cube::vlist_triangles::position;

      g_index_size = sizeof(cube::vlist_triangles::index);
      g_index_data = cube::vlist_triangles::index;

      g_color_size = sizeof(cube::vlist_triangles::color);
      g_color_data = cube::vlist_triangles::color;

      assert(g_position_size == g_color_size);
      g_num_index = cube::vlist_triangles::num_index;
    }
  }
  else if (g_mesh_model == kAvocado)
  {
    if (g_mesh_type == kTriangleSoup)
    {
      g_position_size = sizeof(avocado::triangle_soup::position);
      g_position_data = avocado::triangle_soup::position;

      g_color_size = sizeof(avocado::triangle_soup::color);
      g_color_data = avocado::triangle_soup::color;

      assert(g_position_size == g_color_size);
      g_num_position = avocado::triangle_soup::num_position;
    }
    else
    {
      g_position_size = sizeof(avocado::vlist_triangles::position);
      g_position_data = avocado::vlist_triangles::position;

      g_index_size = sizeof(avocado::vlist_triangles::index);
      g_index_data = avocado::vlist_triangles::index;

      g_color_size = sizeof(avocado::vlist_triangles::color);
      g_color_data = avocado::vlist_triangles::color;

      assert(g_position_size == g_color_size);
      g_num_index = avocado::vlist_triangles::num_index;
    }
  }
  else
  {
    if (g_mesh_type == kTriangleSoup)
    {
      g_position_size = sizeof(donut::triangle_soup::position);
      g_position_data = donut::triangle_soup::position;

      g_color_size = sizeof(donut::triangle_soup::color);
      g_color_data = donut::triangle_soup::color;

      assert(g_position_size == g_color_size);
      g_num_position = donut::triangle_soup::num_position;
    }
    else
    {
      g_position_size = sizeof(donut::vlist_triangles::position);
      g_position_data = donut::vlist_triangles::position;

      g_index_size = sizeof(donut::vlist_triangles::index);
      g_index_data = donut::vlist_triangles::index;

      g_color_size = sizeof(donut::vlist_triangles::color);
      g_color_data = donut::vlist_triangles::color;

      assert(g_position_size == g_color_size);
      g_num_index = donut::vlist_triangles::num_index;
    }
  }


  // VBO
  glBindBuffer(GL_ARRAY_BUFFER, position_buffer); 
  glBufferData(GL_ARRAY_BUFFER, g_position_size, g_position_data, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
  glBufferData(GL_ARRAY_BUFFER, g_color_size, g_color_data, GL_STATIC_DRAW);

  // IBO
  if (g_mesh_type == kVlistTriangles)
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_index_size, g_index_data, GL_STATIC_DRAW);
  }
}


void init_buffer_objects()
{

  // generate VBO & IBO
  glGenBuffers(1, &position_buffer);
  glGenBuffers(1, &color_buffer);
  glGenBuffers(1, &index_buffer);

  update_buffer_objects();

}

void set_transform() 
{
  mat_view = glm::mat4(1.0f);
  mat_proj = glm::mat4(1.0f); 

  mat_model = glm::rotate(glm::mat4(1.0f), g_angle, glm::vec3(0.0f, 1.0f, 0.0f));
  mat_model = mat_model * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
}


void render_object()
{
  // 특정 쉐이더 프로그램 사용
  glUseProgram(program);

  mat_PVM = mat_proj * mat_view * mat_model;
  glUniformMatrix4fv(loc_u_PVM, 1, GL_FALSE, glm::value_ptr(mat_PVM));

  /////////////////////////////////////////////////////////////////////
  /// TODO: 아래 코드를 적절히 수정하여 프로그램을 완성하세요.
  /////////////////////////////////////////////////////////////////////

  // 앞으로 언급하는 배열 버퍼(GL_ARRAY_BUFFER)는 position_buffer로 지정
  glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
  // 버텍스 쉐이더의 attribute 중 a_position 부분 활성화
  glEnableVertexAttribArray(loc_a_position);
  // 현재 배열 버퍼에 있는 데이터를 버텍스 쉐이더 a_position에 해당하는 attribute와 연결
  glVertexAttribPointer(loc_a_position, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

  // 앞으로 언급하는 배열 버퍼(GL_ARRAY_BUFFER)는 color_buffer로 지정
  glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
  // 버텍스 쉐이더의 attribute 중 a_color 부분 활성화
  glEnableVertexAttribArray(loc_a_color);
  // 현재 배열 버퍼에 있는 데이터를 버텍스 쉐이더 a_color에 해당하는 attribute와 연결
  glVertexAttribPointer(loc_a_color, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

  if (g_mesh_type == kTriangleSoup)
  {
    glDrawArrays(GL_TRIANGLES, 0, g_num_position);
  }
  else
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,index_buffer);
    glDrawElements(GL_TRIANGLES, g_num_index, GL_UNSIGNED_INT, (void*) 0);
  }

  // 정점 attribute 배열 비활성화
  glDisableVertexAttribArray(loc_a_position);
  glDisableVertexAttribArray(loc_a_color);

  // 쉐이더 프로그램 사용해제
  glUseProgram(0);
}


int main(void)
{
  GLFWwindow* window;

  // Initialize GLFW library
  if (!glfwInit())
    return -1;

  // Create a GLFW window containing a OpenGL context
  window = glfwCreateWindow(500, 500, "Render Mesh with VBOs & IBOs", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    return -1;
  }

  // Make the current OpenGL context as one in the window
  glfwMakeContextCurrent(window);

  // Initialize GLEW library
  if (glewInit() != GLEW_OK)
    std::cout << "GLEW Init Error!" << std::endl;

  // Print out the OpenGL version supported by the graphics card in my PC
  std::cout << glGetString(GL_VERSION) << std::endl;

  // keyboard callback
  glfwSetKeyCallback(window, key_callback);

  init_shader_program();
  init_buffer_objects();

  // Loop until the user closes the window
  while (!glfwWindowShouldClose(window))
  {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable (GL_DEPTH_TEST);

    set_transform();
    render_object();

    g_angle += 0.01f;

    // Swap front and back buffers
    glfwSwapBuffers(window);

    // Poll for and process events
    glfwPollEvents();
  }

  glfwTerminate();

  return 0;
}
