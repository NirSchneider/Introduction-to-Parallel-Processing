///// Guy: fill correctly the shared and the private variables in the #pragma omp parallel line
# include <stdlib.h>
# include <stdio.h>
# include <math.h> // Guy: compile with the flag -lm at the end of the line
# include <time.h>
# include <omp.h> // Guy: compile with gcc -fopenmp

int main ( );
int min_xy( int i1, int i2 );

/******************************************************************************/
int main ( )
{
    int threadsNum = 1; //NEW
    omp_set_num_threads(threadsNum); //NEW

    //====================== shared variables - start ======================
    int m = 500;
    int n = 500;
    int b[m][n];
    int count[m][n];
    int count_max = 20000;
    int g[m][n];
    char *output_filename = "mandelbrot_result.ppm";
    FILE *output_unit;
    int r[m][n];
    double x_max = 1.25;
    double x_min = -2.25;
    double y_max = 1.75;
    double y_min = -1.75;
    //====================== shared variables - end ======================


    //====================== private variables - start ======================
    int c;
    int i;
    int j;
    int jhi;
    int jlo;
    int k;
    double x;
    double x1, x2;
    double y;
    double y1, y2;
    //====================== private variables - end ======================

    double wtime; // time measurer

    //printf("Debug 01\n");
    wtime = omp_get_wtime ( ); //start time measuring

    /*
    Carry out the iteration for each pixel, determining COUNT.
    */
    # pragma omp parallel \
     shared (m, n, b, count, count_max, g, output_filename, output_unit, r, x_max, x_min, y_max, y_min) \
     private (c,i, j, jhi, jlo, k, x, x1, x2, y, y1, y2)
    {
        //====================== mandelbrot fractal calculation - start ======================
        # pragma omp for
        for ( i = 0; i < m; i++ )
        {
            y = ( ( double ) ( i - 1 ) * y_max
                  + ( double ) ( m - i ) * y_min )
                / ( double ) ( m - 1 );
            for ( j = 0; j < n; j++ )
            {
                x = ( ( double ) ( j - 1 ) * x_max
                      + ( double ) ( n - j ) * x_min )
                    / ( double ) ( n - 1 );
                count[i][j] = 0;
                x1 = x;
                y1 = y;
                for ( k = 1; k <= count_max; k++ )
                {
                    x2 = x1 * x1 - y1 * y1 + x;
                    y2 = 2 * x1 * y1 + y;
                    if ( x2 < -2.0 || 2.0 < x2 || y2 < -2.0 || 2.0 < y2 )
                    {
                        count[i][j] = k;
                        break;
                    }
                    x1 = x2;
                    y1 = y2;
                }
                if ( ( count[i][j] % 2 ) == 1 )
                {
                    r[i][j] = 255;
                    g[i][j] = 255;
                    b[i][j] = 255;
                }
                else
                {
                    c = ( int ) ( 255.0 * sqrt ( sqrt ( sqrt (
                            ( ( double ) ( count[i][j] ) / ( double ) ( count_max ) ) ) ) ) );
                    r[i][j] = 3 * c / 5;
                    g[i][j] = 3 * c / 5;
                    b[i][j] = c;
                }
            }
        }
        //====================== mandelbrot fractal calculation - end ======================
    }

    wtime = omp_get_wtime ( ) - wtime; //end time measuring
    //printf("Debug 02\n");
    printf ( "\n" );
    printf ( "Time = %g seconds.\n", wtime );

    /*
     Write data to an ASCII PPM file.
    */
    //====================== PPM's header - start ======================
    output_unit = fopen ( output_filename, "wt" ); //open file in write (text-default) mode
    fprintf ( output_unit, "P3\n" ); // write to output_unit "P3"
    fprintf ( output_unit, "%d %d\n", n, m ); // write to output_unit matrix dimensions nxm
    fprintf ( output_unit, "%d\n", 255 ); // write to output_unit 255 (max gray scale)
    //====================== PPM's header - end ======================

    //printf("Debug 03\n");

    //====================== PPM's content - start ======================
    for ( i = 0; i < m; i++ )
    {
        for ( jlo = 0; jlo < n; jlo = jlo + 4 )
        {
            jhi = min_xy( jlo + 4, n );
            for ( j = jlo; j < jhi; j++ )
            {
                fprintf ( output_unit, " %d %d %d", r[i][j], g[i][j], b[i][j] );
            }
            fprintf ( output_unit, "\n" );
        }
    }
    //====================== PPM's content - end ======================

    fclose ( output_unit );
    printf ( "\n" );
    printf ( "Image file written to \"%s\".\n", output_filename );

    /*
     Terminate.
    */
    return 0;
}

int min_xy( int i1, int i2 )
{
    int value;
    if ( i1 < i2 )
    {
        value = i1;
    }
    else
    {
        value = i2;
    }
    return value;
}
