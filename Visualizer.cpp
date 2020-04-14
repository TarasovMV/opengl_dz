#include "Visualizer.h"


//rainbow colormap for scalar value
void rainbow(float value,float &R, float &G, float &B)
{
  const float dx = 0.8;
  value = value < 0 ? 0 : value > 1 ? 1 : value;
  float g = (6 - 2 * dx) * value + dx;
  R = (fmaxf(0.0f, (3.0f - fabsf(g - 4.0f) - fabsf(g - 5.0f))/2.0f));
  G = (fmaxf(0.0f, (4.0f - fabsf(g - 2.0f) - fabsf(g - 4.0f))/2.0f));
  B = (fmaxf(0.0f, (3.0f - fabsf(g - 1.0f) - fabsf(g - 2.0f))/2.0f));
}

void yellowToRed(float value, float& R, float& G, float& B) {
	double a = (1 - 1 / (value * 100));
	R = 1; G = a; B = 0;
}

void redToBlue(float value, float& R, float& G, float& B) {
	double a = (1 - 1 / (value * 100));
	R = a; G = 0; B = 1;
}

void ocean(float value, float& R, float& G, float& B) {
	double a = (1 - 1 / (value * 100));
	R = a; G = 1; B = 1;
}


void Visualizer::Colormap(float val, float &R, float &G, float &B)
{
  if (colormap_scalar == WHITE)
    R = G = B = val;
  else if (colormap_scalar == RAINBOW)
    rainbow(val, R, G, B);
  else if (colormap_scalar == BANDED_RAINBOW)
  {
    const int NLEVELS = 7;
    val *= NLEVELS;
    val = ceilf(val) / NLEVELS;
    rainbow(val, R, G, B);
  }
}

void Visualizer::Direction2DToColor(float x, float y, int method, float& r, float& g, float& b)
{
  float f;
  if (method == RAINBOW)
  {
    f = atan2f(y,x) / 3.1415927f + 1.0f;
    r = f;

    if(r > 1)
      r = 2 - r;

    g = f + .66667f;
    if(g > 2)
      g -= 2;

    if(g > 1) g = 2 - g;

    b = f + 2 * .66667f;
    if(b > 2)
      b -= 2;

    if(b > 1)
      b = 2 - b;
  }
  else
  {
    r = g = b = 1;
  }
}

void Visualizer::ScalarToColor(float val, int colormap_scalar, float& R, float& G, float& B)
{
	double colorize = 0.8;
	if (colormap_scalar == WHITE)
		R = G = B = val;
	else if (colormap_scalar == RAINBOW)
		rainbow(val, R, G, B);
	else if (colormap_scalar == BANDED_RAINBOW)
	{
		const int NLEVELS = 7;
		val *= NLEVELS;
		val = ceilf(val) / NLEVELS;
		rainbow(val, R, G, B);
	}
	else if (colormap_scalar == YELLOW_TO_RED) {
		yellowToRed(val, R, G, B);
	}
	else if (colormap_scalar == BLUE_TO_RED) {
		redToBlue(val, R, G, B);
	}
	else if (colormap_scalar == OCEAN) {
		ocean(val, R, G, B);
	}
	R *= colorize; G *= colorize; B *= colorize;
}

void Visualizer::Init(int n, uint32_t width, uint32_t height)
{
  m_width = width;
  m_height = height;
  m_grid_dim = n;

  vec_pos.resize(n * n * 4);
  vec_col.resize(n * n * 6);

  glGenBuffers(1, &sim_vbo_vec);                                                        GL_CHECK_ERRORS;
  glGenBuffers(1, &vbo_vec_col);                                                        GL_CHECK_ERRORS;
  glBindBuffer(GL_ARRAY_BUFFER, sim_vbo_vec);                                           GL_CHECK_ERRORS;
  glBufferData(GL_ARRAY_BUFFER, n * n * 4 * sizeof(GLfloat), vec_pos.data(), GL_STREAM_DRAW); GL_CHECK_ERRORS;

  glBindBuffer(GL_ARRAY_BUFFER, vbo_vec_col);                                           GL_CHECK_ERRORS;
  glBufferData(GL_ARRAY_BUFFER, n * n * 6 * sizeof(GLfloat), vec_col.data(), GL_STREAM_DRAW); GL_CHECK_ERRORS;

  glGenVertexArrays(1, &sim_vao_vec);                                                   GL_CHECK_ERRORS;
  glBindVertexArray(sim_vao_vec);                                                       GL_CHECK_ERRORS;

  glBindBuffer(GL_ARRAY_BUFFER, sim_vbo_vec);                                           GL_CHECK_ERRORS;
  glEnableVertexAttribArray(vertexLocation);                                            GL_CHECK_ERRORS;
  glVertexAttribPointer(vertexLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);                   GL_CHECK_ERRORS;

  glBindBuffer(GL_ARRAY_BUFFER, vbo_vec_col);                                           GL_CHECK_ERRORS;
  glEnableVertexAttribArray(colorLocation);                                            GL_CHECK_ERRORS;
  glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);                   GL_CHECK_ERRORS;

  glBindVertexArray(0);

  //////////////////////////////////////////////////////
  int n_verts_grid = n * n;
  smoke_pos.resize(n_verts_grid * 2);
  smoke_col.resize(n_verts_grid * 3);

  glGenBuffers(1, &sim_vbo_smoke);                                                        GL_CHECK_ERRORS;
  glBindBuffer(GL_ARRAY_BUFFER, sim_vbo_smoke);                                           GL_CHECK_ERRORS;
  glBufferData(GL_ARRAY_BUFFER, n_verts_grid * 2 * sizeof(GLfloat), smoke_pos.data(), GL_STREAM_DRAW); GL_CHECK_ERRORS;

  glGenBuffers(1, &vbo_smoke_col);                                                        GL_CHECK_ERRORS;
  glBindBuffer(GL_ARRAY_BUFFER, vbo_smoke_col);                                           GL_CHECK_ERRORS;
  glBufferData(GL_ARRAY_BUFFER, n_verts_grid * 3 * sizeof(GLfloat), smoke_col.data(), GL_STREAM_DRAW); GL_CHECK_ERRORS;

  glGenVertexArrays(1, &sim_vao_smoke);                                                   GL_CHECK_ERRORS;
  glBindVertexArray(sim_vao_smoke);                                                       GL_CHECK_ERRORS;

  glBindBuffer(GL_ARRAY_BUFFER, sim_vbo_smoke);                                           GL_CHECK_ERRORS;
  glEnableVertexAttribArray(vertexLocation);                                            GL_CHECK_ERRORS;
  glVertexAttribPointer(vertexLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);                   GL_CHECK_ERRORS;

  glBindBuffer(GL_ARRAY_BUFFER, vbo_smoke_col);                                           GL_CHECK_ERRORS;
  glEnableVertexAttribArray(colorLocation);                                            GL_CHECK_ERRORS;
  glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);                   GL_CHECK_ERRORS;

  int numIndices = 2 * n * (n - 1) + n - 1;

  indices.resize(numIndices);

  int primRestart = n_verts_grid;
  int k = 0;
  for (int x = 0; x < n - 1; ++x)
  {
    for (int y = 0; y < n - 1; ++y)
    {
      int offset = x * n + y;

      if (y == 0)
      {
        indices[k] = offset + 0;     k++;
        indices[k] = offset + n;     k++;
        indices[k] = offset + 1;     k++;
        indices[k] = offset + n + 1; k++;
      }
      else
      {
        indices[k] = offset + 1;       k++;
        indices[k] = offset + n + 1; k++;

        if (y == n - 2)
        {
          indices[k] = primRestart; k++;
        }
      }
    }
  }

  GLuint vboIndices;
  glGenBuffers(1, &vboIndices);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices); GL_CHECK_ERRORS;
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLuint), indices.data(), GL_STREAM_DRAW); GL_CHECK_ERRORS;

  glBindVertexArray(0);
}


void Visualizer::UpdateBuffers(const std::shared_ptr<Simulator> &pSim)
{
  int i, j, idx; 
  double px, py;

  double wn = (double)m_width  / (double)(m_grid_dim + 1);   // Grid cell width
  double hn = (double)m_height / (double)(m_grid_dim + 1);  // Grid cell height

  float coeff_x = 2.0f / m_width;
  float coeff_y = 2.0f / m_height;

  if (draw_smoke)
  {
    int k = 0;
    int c = 0;
    for (i = 0; i < m_grid_dim; i++)			//draw smoke
    {
      for (j = 0; j < m_grid_dim; j++)
      {
        px = wn + (double)i * wn;
        py = hn + (double)j * hn;

        float r, g, b;
        Colormap(pSim->GetDensity(i, j), r, g, b);
        smoke_col[c] = r; c++;
        smoke_col[c] = g; c++;
        smoke_col[c] = b; c++;

        smoke_pos[k] = coeff_x * px - 1.0f; k++;
        smoke_pos[k] = coeff_y * py - 1.0f; k++;
      }
    }

    glBindVertexArray(sim_vao_smoke);                                                        GL_CHECK_ERRORS;
    glBindBuffer(GL_ARRAY_BUFFER, sim_vbo_smoke);                                            GL_CHECK_ERRORS;
    glBufferData(GL_ARRAY_BUFFER, m_grid_dim * m_grid_dim * 2 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); GL_CHECK_ERRORS;
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_grid_dim * m_grid_dim * 2 * sizeof(GLfloat), smoke_pos.data());      GL_CHECK_ERRORS;

    glBindBuffer(GL_ARRAY_BUFFER, vbo_smoke_col);                                            GL_CHECK_ERRORS;
    glBufferData(GL_ARRAY_BUFFER, m_grid_dim * m_grid_dim * 3 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); GL_CHECK_ERRORS;
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_grid_dim * m_grid_dim * 3 * sizeof(GLfloat), smoke_col.data());      GL_CHECK_ERRORS;

//    glEnable(GL_PRIMITIVE_RESTART); GL_CHECK_ERRORS;
//    glPrimitiveRestartIndex(primRestart); GL_CHECK_ERRORS;
//    glDrawElements(GL_TRIANGLE_STRIP, numIndices, GL_UNSIGNED_INT, nullptr);
//    glDisable(GL_PRIMITIVE_RESTART); GL_CHECK_ERRORS;
//    glDrawArrays(GL_TRIANGLE_STRIP, 0, n_verts_grid);
    glBindVertexArray(0);

  }

  if (draw_vecs)
  {
    for (i = 0; i < m_grid_dim; i++)
    {
      for (j = 0; j < m_grid_dim; j++)
      {
        float r, g, b;
		//Direction2DToColor(pSim->GetVelocityX(i, j), pSim->GetVelocityY(i, j), colormap_dir, r, g, b);
		
		/// Карта по вектору скорости
		if (object_scalar == VELOCITY) {
			ScalarToColor(pSim->GetVelocity(i, j) * ColorMapKfVelocity, colormap_dir, r, g, b);
		}
		/// Карта по вектору силы
		else if (object_scalar == FORCE) {
			ScalarToColor(pSim->GetForce(i, j) * ColorMapKfForce, colormap_dir, r, g, b);
		}
		/// Карта по вектору плотности
		else if (object_scalar == DENSITY) {
			ScalarToColor(pSim->GetDensity(i, j), colormap_dir, r, g, b);
		}

        double x1, y1, x2, y2;
        x1 = wn + (double) i * wn;
        y1 = hn + (double) j * hn;

		/// величина и направление глифа
        x2 = x1 + vec_scale * pSim->GetVelocityX(i, j);
        y2 = y1 + vec_scale * pSim->GetVelocityY(i, j);
		//x2 = x1 + vec_scale * pSim->GetForceX(i, j);
		//y2 = y1 + vec_scale * pSim->GetForceY(i, j);

        vec_pos[(i * m_grid_dim + j) * 4 + 0] = coeff_x * x1 - 1.0f;
        vec_pos[(i * m_grid_dim + j) * 4 + 1] = coeff_y * y1 - 1.0f;
        vec_pos[(i * m_grid_dim + j) * 4 + 2] = coeff_x * x2 - 1.0f;
        vec_pos[(i * m_grid_dim + j) * 4 + 3] = coeff_x * y2 - 1.0f;

        vec_col[(i * m_grid_dim + j) * 6 + 0] = r;
        vec_col[(i * m_grid_dim + j) * 6 + 1] = g;
        vec_col[(i * m_grid_dim + j) * 6 + 2] = b;

        vec_col[(i * m_grid_dim + j) * 6 + 3] = r;
        vec_col[(i * m_grid_dim + j) * 6 + 4] = g;
        vec_col[(i * m_grid_dim + j) * 6 + 5] = b;
      }
    }
    glBindVertexArray(sim_vao_vec);                                                          GL_CHECK_ERRORS;

    glBindBuffer(GL_ARRAY_BUFFER, sim_vbo_vec);                                              GL_CHECK_ERRORS;
    glBufferData(GL_ARRAY_BUFFER, m_grid_dim * m_grid_dim * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); GL_CHECK_ERRORS;
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_grid_dim * m_grid_dim * 4 * sizeof(GLfloat), vec_pos.data());        GL_CHECK_ERRORS;

    glBindBuffer(GL_ARRAY_BUFFER, vbo_vec_col);                                              GL_CHECK_ERRORS;
    glBufferData(GL_ARRAY_BUFFER, m_grid_dim * m_grid_dim * 6 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); GL_CHECK_ERRORS;
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_grid_dim * m_grid_dim * 6 * sizeof(GLfloat), vec_col.data());        GL_CHECK_ERRORS;

    glDrawArrays(GL_LINES, 0, m_grid_dim * m_grid_dim * 2);
    glBindVertexArray(0);
  }
}

void Visualizer::Draw()
{
  int numIndices = 2 * m_grid_dim *(m_grid_dim - 1) + m_grid_dim - 1;
  int primRestart = m_grid_dim * m_grid_dim;

  if(draw_smoke)
  {
    glBindVertexArray(sim_vao_smoke); GL_CHECK_ERRORS;
    glEnable(GL_PRIMITIVE_RESTART); GL_CHECK_ERRORS;
    glPrimitiveRestartIndex(primRestart); GL_CHECK_ERRORS;
    glDrawElements(GL_TRIANGLE_STRIP, numIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
    glDisable(GL_PRIMITIVE_RESTART); GL_CHECK_ERRORS;
    glBindVertexArray(0);
  }

  if(draw_vecs)
  {
    glBindVertexArray(sim_vao_vec); GL_CHECK_ERRORS;
	/// Способы отрисовски глифов
    glDrawArrays(GL_POINTS, 0, m_grid_dim * m_grid_dim * 2); GL_CHECK_ERRORS;
    //glDrawArrays(GL_TRIANGLES, 0, m_grid_dim * m_grid_dim * 2); GL_CHECK_ERRORS;
    //glDrawArrays(GL_TRIANGLES_ADJACENCY, 0, m_grid_dim * m_grid_dim * 2); GL_CHECK_ERRORS;
    //glDrawArrays(GL_LINES, 0, m_grid_dim * m_grid_dim * 2); GL_CHECK_ERRORS;
    glBindVertexArray(0);
  }
}

