#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// Constants for the semi-empirical mass formula according to Least-squares fit (1) in MeV units
const double a_V = 15.8;
const double a_S = 18.3;
const double a_C = 0.714;
const double a_A = 23.2;
const double a_P = 12;

// compute delta(A, Z) according to the formula
double delta_residue(int A, int Z)
{
    double delta = a_P * (pow(A, 0.5));
    if (A % 2 == 0 && Z % 2 == 0)  // even-even
    {
        return delta;
    }
    else if (A % 2 != 0 && Z % 2 != 0)  // odd-odd
    {
        return -delta;
    }
    else  // even-odd, odd-even
    {
        return 0;
    }
}

// binding energy of N and Z
double binding_energy(int N, int Z)
{
    int A = N + Z;
    double E_B = a_V * A - a_S*pow(A, 2.0/3.0) - a_C*pow(Z, 2)/pow(A, 1.0/3.0) - a_A*(pow(A-2*Z, 2)/A) + delta_residue(A, Z);
    return E_B;
}

int main(int argc, char* argv[])
{
    int Z = atoi(argv[1]);  // scan value for Z
    char binding_energies[200][40];
    int i;
    int N;
    // loop over N with a constant Z (per run) and calculate the mass
    for (N = 1; N <= 200; N++)
    {
        double M = binding_energy(N, Z);
        if (M <= 0)
        {
            M = -1;
        }
        sprintf(binding_energies[N-1], "%d - %d - %f", N, Z, M);
    }

    // output
    for (i = 0; i < 200; i++)
    {
        printf("%s\n", binding_energies[i]);
    }

    return 0;
}
