#ifndef MAIN_SIMULATOR_H
#define MAIN_SIMULATOR_H

#include <fftw3.h>
#include <cmath>
#include <cstdint>


struct Simulator
{
    Simulator() = default;
    ~Simulator();

    void InitSimulation(int n);
    void StepSimulation();
    void ToggleFreezeSim() { frozen = !frozen; }
    void MultiplyViscosity(double mul) { visc *= mul; }
    void IncreaseTimestep(float val) { dt += val;}
    void DecreaseTimestep(float val) { dt -= val;}
	void AddForce(uint32_t x, uint32_t y, double val_x, double val_y);
	double GetForce(uint32_t x, uint32_t y);
	double GetForceX(uint32_t x, uint32_t y);
	double GetForceY(uint32_t x, uint32_t y);
    void SetDensity(uint32_t x, uint32_t y, double val) { rho[y * dim + x] = val; }
    double GetDensity(uint32_t x, uint32_t y) { return rho[y * dim + x];}
    double GetDensityIso(uint32_t x) { return rho[x];}
    double GetVelocityX(uint32_t x, uint32_t y) { return vx[y * dim + x];}
    double GetVelocityY(uint32_t x, uint32_t y) { return vy[y * dim + x];}
	double GetVelocity(uint32_t x, uint32_t y) { return (sqrt(pow(GetVelocityX(x, y), 2) + pow(GetVelocityY(x, y), 2))); }

    uint32_t DIM() { return dim;}

private:
    double visc   = 0.001;		  //fluid viscosity
    uint32_t dim  = 128;				//size of simulation grid
    double dt     = 0.4;				//simulation time step

    //(vx,vy)   = velocity field at the current moment
    double *vx = nullptr;
    double *vy = nullptr;

    //(vx0,vy0) = velocity field at the previous moment
    double *vx0 = nullptr;
    double *vy0 = nullptr;

    //(fx,fy)   = simulation forces
    double *fx = nullptr;
    double *fy = nullptr;

    //smoke density at the current (rho) and previous (rho0) moment
    double *rho  = nullptr;
    double *rho0 = nullptr;

    //simulation domain discretization
    fftw_plan plan_rc_x, plan_cr_x;
    fftw_plan plan_rc_y, plan_cr_y;

    bool frozen = false;

    //execute the Fast Fourier Transform
    //direction = 1 for direct transform and direction = - 1 for inverse transform
    void FFT(int direction);
    void Solve(int n);

    // diffuse matter in the velocity field. 
    // reads input densities from rho0 and writes the result into rho.
    void DiffuseMatter(int n);

    // set user-controlled forces, dampen forces and matter density to get a stable simulation
    void SetForces();
};

int clamp(float x);


#endif //MAIN_SIMULATOR_H
