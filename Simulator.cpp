#include "Simulator.h"

int clamp(float x)
{
  return ((x) >= 0.0 ? ((int)(x)) : (-((int)(1 - (x)))));
}

void Simulator::AddForce(uint32_t x, uint32_t y, double val_x, double val_y)
{
  fx[y * dim + x] += val_x;
  fy[y * dim + x] += val_y;
}

double Simulator::GetForce(uint32_t x, uint32_t y)
{
	return sqrt(pow(fx[y * dim + x], 2) + pow(fy[y * dim + x], 2));
}

double Simulator::GetForceX(uint32_t x, uint32_t y)
{
	return fx[y * dim + x];
}

double Simulator::GetForceY(uint32_t x, uint32_t y)
{
	return fy[y * dim + x];
}

void Simulator::InitSimulation(int n)
{
  int i; 
  size_t memSize;

  dim = n;
  
  memSize = n * 2 * (n / 2 + 1) * sizeof(fftw_complex);        
  vx      = (double*) fftw_malloc(memSize);
  vy      = (double*) fftw_malloc(memSize);
  vx0     = (double*) fftw_malloc(memSize);
  vy0     = (double*) fftw_malloc(memSize);

  memSize  = n * n * sizeof(double);
  fx      = (double*) fftw_malloc(memSize);
  fy      = (double*) fftw_malloc(memSize);
  rho     = (double*) fftw_malloc(memSize);
  rho0    = (double*) fftw_malloc(memSize);

  plan_rc_x = fftw_plan_dft_r2c_2d(n, n, (double*)vx0,(fftw_complex*)vx0, 0);
  plan_cr_x = fftw_plan_dft_c2r_2d(n, n, (fftw_complex*)vx0,(double*)vx0, 0);

  plan_rc_y = fftw_plan_dft_r2c_2d(n, n, (double*)vy0,(fftw_complex*)vy0, 0);
  plan_cr_y = fftw_plan_dft_c2r_2d(n, n, (fftw_complex*)vy0,(double*)vy0, 0);

  for (i = 0; i < n * n; i++)                      
  { vx[i] = vy[i] = vx0[i] = vy0[i] = fx[i] = fy[i] = rho[i] = rho0[i] = 0.0f; }
}

Simulator::~Simulator()
{
  if(vx != nullptr)
  {
    fftw_free(vx);
    vx = nullptr;
  }
  if(vy != nullptr)
  {
    fftw_free(vy);
    vy = nullptr;
  }

  if(vx0 != nullptr)
  {
    fftw_free(vx0);
    vx0 = nullptr;
  }

  if(vy0 != nullptr)
  {
    fftw_free(vy0);
    vy0 = nullptr;
  }

  if(fx != nullptr)
  {
    fftw_free(fx);
    fx = nullptr;
  }

  if(fy != nullptr)
  {
    fftw_free(fy);
    fy = nullptr;
  }

  if(rho != nullptr)
  {
    fftw_free(rho);
    rho = nullptr;
  }

  if(rho0 != nullptr)
  {
    fftw_free(rho0);
    rho0 = nullptr;
  }
  fftw_destroy_plan(plan_cr_x);
  fftw_destroy_plan(plan_rc_x);
  fftw_destroy_plan(plan_cr_y);
  fftw_destroy_plan(plan_rc_y);
}

void Simulator::FFT(int direction)
{
  if(direction == 1)
  {
    fftw_execute(plan_rc_x);
    fftw_execute(plan_rc_y);
  }
  else
  {
    fftw_execute(plan_cr_x);
    fftw_execute(plan_cr_y);
  }
}

void Simulator::Solve(int n)
{
  double x, y, x0, y0, f, r, U[2], V[2], s, t;
  int i, j, i0, j0, i1, j1;

  for (i = 0; i < n * n; i++)
  { 
	  vx[i] += dt * vx0[i]; 
	  vx0[i] = vx[i]; vy[i] += dt * vy0[i]; 
	  vy0[i] = vy[i]; 
  }

  for (x = (0.5f/n), i = 0 ; i < n ; i++, x += (1.0f / n))
  {
    for (y = (0.5f/n), j = 0; j < n; j++, y += (1.0f / n))
    {
      x0 = n * (x - dt * vx0[i + n * j]) - 0.5f;
      y0 = n * (y - dt * vy0[i + n * j]) - 0.5f;

      i0 = clamp(x0);
      s = x0 - i0;
      i0 = (n + (i0 % n)) % n;
      i1 = (i0 + 1) % n;

      j0 = clamp(y0);
      t = y0 - j0;
      j0 = (n + (j0 % n)) % n;
      j1 = (j0 + 1) % n;

      vx[i + n * j] = (1 - s) * ((1 - t) * vx0[i0 + n * j0] + t * vx0[i0 + n * j1])
                      + s * ((1 - t) * vx0[i1 + n * j0] + t * vx0[i1 + n * j1]);

      vy[i + n * j] = (1 - s) * ((1 - t) * vy0[i0 + n * j0] + t * vy0[i0 + n * j1])
                      + s * ((1 - t) * vy0[i1 + n * j0] + t * vy0[i1 + n * j1]);
    }
  }

  for(i = 0; i < n; i++)
  {
    for (j = 0; j < n; j++)
    {
      vx0[i + (n + 2) * j] = vx[i + n * j];
      vy0[i + (n + 2) * j] = vy[i + n * j];
    }
  }

  FFT(1);

  for (i = 0; i <= n;i += 2)
  {
    x = 0.5f * i;
    for (j = 0; j < n; j++)
    {
      y = j <= n/2 ? (double)j : (double)j - n;
      r = x * x + y * y;

      if (r == 0.0f) continue;

      f = (double)exp(-r * dt * visc);
      U[0] = vx0[i + 0 + (n + 2) * j]; V[0] = vy0[i + 0 + (n + 2) * j];
      U[1] = vx0[i + 1 + (n + 2) * j]; V[1] = vy0[i + 1 + (n + 2) * j];

      vx0[i + 0 + (n + 2) * j] = f * ((1 - x * x / r) * U[0] - x * y / r * V[0]);
      vx0[i + 1 + (n + 2) * j] = f * ((1 - x * x / r) * U[1] - x * y / r * V[1]);

      vy0[i + 0 + (n + 2) * j] = f * (-y * x / r * U[0] + (1 - y * y / r) * V[0]);
      vy0[i + 1 + (n + 2) * j] = f * (-y * x / r * U[1] + (1 - y * y / r) * V[1]);
    }
  }

  FFT(-1);

  f = 1.0 / (n * n);
  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
    {
      vx[i + n * j] = f * vx0[i + (n + 2) * j];
      vy[i + n * j] = f * vy0[i + (n + 2) * j];
    }
}


void Simulator::DiffuseMatter(int n)
{
  double x, y, x0, y0, s, t;
  int i, j, i0, j0, i1, j1;

  for (x = 0.5f / n, i = 0 ; i < n ; i++, x += 1.0f / n)
  {
    for (y = 0.5f / n, j = 0; j < n; j++, y += 1.0f / n)
    {
      x0 = n * (x - dt * vx[i + n * j]) - 0.5f;
      y0 = n * (y - dt * vy[i + n * j]) - 0.5f;

      i0 = clamp(x0);
      s = x0 - i0;
      i0 = (n + (i0 % n)) % n;
      i1 = (i0 + 1) % n;

      j0 = clamp(y0);
      t = y0 - j0;
      j0 = (n + (j0 % n)) % n;
      j1 = (j0 + 1) % n;

      rho[i + n * j] = (1 - s) * ((1 - t) * rho0[i0 + n * j0] + t * rho0[i0 + n * j1]) +
                       s * ((1 - t) * rho0[i1 + n * j0] + t * rho0[i1 + n * j1]);
    }
  }
}

void Simulator::SetForces()
{
  int i;
  for (i = 0; i < dim * dim; i++)
  {
    rho0[i] = 0.995 * rho[i];
    fx[i]  *= 0.85;
    fy[i]  *= 0.85;
    vx0[i]  = fx[i];
    vy0[i]  = fy[i];
  }
}

void Simulator::StepSimulation()
{
  if (!frozen)
  {
    SetForces();
    Solve(dim);
    DiffuseMatter(dim);
  }
}