#ifdef _MSC_VER
#include <SDL2.h>
#else
#include <SDL2/SDL.h>
#endif /* _MSC_VER */

#include <stdio.h>
#include <math.h>
#include <complex.h>

#define WIDTH 800
#define HEIGHT 600

#define START_POS   -0.5
#define START_ZOOM  (WIDTH * 0.25296875f)

#define BAIL_OUT        2.0
#define FLIPS           24

#define ZOOM_FACTOR     4

void sdl_draw_mandelbrot(SDL_Window *window, complex double center, double zoom)
{
    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    int f,x,y,n;
    int maxiter = (width/2) * 0.049715909 * log10(zoom);
    complex double z, c;
    float C;

    SDL_Rect rects[height/FLIPS];
    SDL_Surface *surface;

    surface = SDL_GetWindowSurface(window);

    if ( surface == NULL )
    {
        fprintf(stderr, "Could not get surface : %s\n", SDL_GetError());
        exit(1);
    }

    fprintf(stderr, "zoom: %f\n", zoom);
    fprintf(stderr, "center point: %f %+fi\n", creal(center),
                                              cimag(center) );
    fprintf(stderr, "iterations: %d\n", maxiter);

    for (f = 0; f < FLIPS; f++)
    {
        for  (y = f; y < height; y += FLIPS)
        {
            for (x = 0; x < width; x++)
            {
                /* Get the complex poing on gauss space to be calculate */
                z = c = creal(center) + ((x - (width/2))/zoom) +
                    ((cimag(center) + ((y - (height/2))/zoom))*_Complex_I);

                #define X creal(z)
                #define Y cimag(z)

                /* Check if point lies within the main cardiod or
                   in the period-2 buld */
                if ( (pow(X-.25,2) + pow(Y,2))*(pow(X,2) + (X/2) + pow(Y,2) - .1875) < pow(Y,2)/4 ||
                     pow(X+1,2) + pow(Y,2) < .0625 )
                    n = maxiter;
                else
                    /* Applies the actual mandelbrot formula on that point */
                    for (n = 0; n <= maxiter && cabs(z) < BAIL_OUT; n ++)
                        z = cpow(z, 2) + c;

                C = n - log2f(logf(cabs(z)) / M_LN2 );

                /* Paint the pixel calculated depending on the number
                   of iterations found */
                ((Uint32*)surface->pixels)[(y*surface->w) + x] =
                    (n >= maxiter) ? 0 :
                    SDL_MapRGB(
                        surface->format,
                        (1+sin(C*0.27 + 5))*127.,
                        (1+cos(C*0.85))*127.,
                        (1+sin(C*0.15))*127.);
            }
            rects[y/FLIPS].x = 0;
            rects[y/FLIPS].y = y;
            rects[y/FLIPS].w = width;
            rects[y/FLIPS].h = 1;
        }
        SDL_UpdateWindowSurfaceRects(window, rects, height/FLIPS);
    }

    if (width % FLIPS != 0) {
        SDL_UpdateWindowSurface(window);
    }
}

int main(int argc, char **argv)
{
    /* SDL SEtup */
    if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
    {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);

    SDL_Window *window;

    window = SDL_CreateWindow(
        "sdlmandel",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WIDTH, HEIGHT,
        0);

    if ( window == NULL )
    {
        fprintf(stderr, "Couldn\'t create window : %s\n",
                SDL_GetError());
        exit(1);
    }

    /* Initialize variables */
    double complex center = START_POS;
    double zoom = START_ZOOM;

    sdl_draw_mandelbrot(window, center, zoom);

    SDL_Event event;
    while(1)
    {
        SDL_PollEvent(&event);
        switch (event.type)
        {
            case SDL_QUIT:
                exit(0);
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == ' ')
                {
                    center = START_POS;
                    zoom = START_ZOOM;
                    sdl_draw_mandelbrot(window, center, zoom);
                }
                else if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    exit(0);
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                center = creal(center) + ((event.button.x - (WIDTH/2))/zoom) +
                         ((cimag(center) + ((event.button.y - (HEIGHT/2))/zoom))
                          *_Complex_I);

                if (event.button.button == 1)
                    zoom *= ZOOM_FACTOR;
                else if (event.button.button == 3)
                    zoom /= ZOOM_FACTOR;

                sdl_draw_mandelbrot(window, center, zoom);
                break;
        }
    }

    return 0;
}
