// Author: APD team, except where source was noted

#include "helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>


#define CONTOUR_CONFIG_COUNT    16
#define FILENAME_MAX_SIZE       50
#define STEP                    8
#define SIGMA                   200
#define RESCALE_X               2048
#define RESCALE_Y               2048

#define CLAMP(v, min, max) if(v < min) { v = min; } else if(v > max) { v = max; }

typedef struct {
    int thread_id;
    int step_x;
    int step_y;
    int no_threads;
    ppm_image *image;
    ppm_image **contour_map;
    ppm_image *new_image;
    pthread_barrier_t *barrier;
    unsigned char **grid;
} arguments;

unsigned char **alloc_grid(int p, int q) {
    unsigned char **grid = (unsigned char **)malloc((p + 1) * sizeof(unsigned char*));
    if (!grid) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    for (int i = 0; i <= p; i++) {
        grid[i] = (unsigned char *)malloc((q + 1) * sizeof(unsigned char));
        if (!grid[i]) {
            fprintf(stderr, "Unable to allocate memory\n");
            exit(1);
        }
    }
    return grid;
}

// Creates a map between the binary configuration (e.g. 0110_2) and the corresponding pixels
// that need to be set on the output image. An array is used for this map since the keys are
// binary numbers in 0-15. Contour images are located in the './contours' directory.
ppm_image **init_contour_map() {
    ppm_image **map = (ppm_image **)malloc(CONTOUR_CONFIG_COUNT * sizeof(ppm_image *));
    if (!map) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    for (int i = 0; i < CONTOUR_CONFIG_COUNT; i++) {
        char filename[FILENAME_MAX_SIZE];
        sprintf(filename, "./contours/%d.ppm", i);
        // sprintf(filename, "../checker/contours/%d.ppm", i);
        map[i] = read_ppm(filename);
    }

    return map;
}

// Updates a particular section of an image with the corresponding contour pixels.
// Used to create the complete contour image.
void  update_image(ppm_image *image, ppm_image *contour, int x, int y) {
    for (int i = 0; i < contour->x; i++) {
        for (int j = 0; j < contour->y; j++) {
            int contour_pixel_index = contour->x * i + j;
            int image_pixel_index = (x + i) * image->y + y + j;

            image->data[image_pixel_index].red = contour->data[contour_pixel_index].red;
            image->data[image_pixel_index].green = contour->data[contour_pixel_index].green;
            image->data[image_pixel_index].blue = contour->data[contour_pixel_index].blue;
        }
    }
}

// Corresponds to step 1 of the marching squares algorithm, which focuses on sampling the image.
// Builds a p x q grid of points with values which can be either 0 or 1, depending on how the
// pixel values compare to the `sigma` reference value. The points are taken at equal distances
// in the original image, based on the `step_x` and `step_y` arguments.
unsigned char **sample_grid(ppm_image *image, int step_x, int step_y, unsigned char sigma, int thread_id, int no_threads , unsigned char **grid) {

    int p = image->x / step_x;
    int q = image->y / step_y;

    int start = thread_id * p;
    int end = (thread_id + 1) * p;

    for (int i = 0; i < p; i++) {
        for (int j = 0; j < q; j++) {
            ppm_pixel curr_pixel = image->data[i * step_x * image->y + j * step_y];

            unsigned char curr_color = (curr_pixel.red + curr_pixel.green + curr_pixel.blue) / 3;

            if (curr_color > sigma) {
                grid[i][j] = 0;
            } else {
                grid[i][j] = 1;
            }
        }
    }

    // last sample points have no neighbors below / to the right, so we use pixels on the
    // last row / column of the input image for them
    for (int i = 0; i < p; i++) {
        ppm_pixel curr_pixel = image->data[i * step_x * image->y + image->x - 1];

        unsigned char curr_color = (curr_pixel.red + curr_pixel.green + curr_pixel.blue) / 3;

        if (curr_color > sigma) {
            grid[i][q] = 0;
        } else {
            grid[i][q] = 1;
        }
    }
    for (int j = 0; j < q; j++) {
        ppm_pixel curr_pixel = image->data[(image->x - 1) * image->y + j * step_y];

        unsigned char curr_color = (curr_pixel.red + curr_pixel.green + curr_pixel.blue) / 3;

        if (curr_color > sigma) {
            grid[p][j] = 0;
        } else {
            grid[p][j] = 1;
        }
    }

    return grid;
}

// Corresponds to step 2 of the marching squares algorithm, which focuses on identifying the
// type of contour which corresponds to each subgrid. It determines the binary value of each
// sample fragment of the original image and replaces the pixels in the original image with
// the pixels of the corresponding contour image accordingly.
void march(ppm_image *image, unsigned char **grid, ppm_image **contour_map, int step_x, int step_y, int thread_id, int no_threads) {
    int p = (image->x / step_x) / no_threads;
    int q = image->y / step_y;

    int start = thread_id * p;
    int end = (thread_id + 1) * p;   

    for (int i = start; i < end; i++) {
        for (int j = 0; j < q; j++) {
            unsigned char k = 8 * grid[i][j] + 4 * grid[i][j + 1] + 2 * grid[i + 1][j + 1] + 1 * grid[i + 1][j];
            update_image(image, contour_map[k], i * step_x, j * step_y);
        }
    }
}

// Calls `free` method on the utilized resources.
void free_resources(ppm_image *image, ppm_image **contour_map, unsigned char **grid, int step_x , arguments *args) {
    for (int i = 0; i < CONTOUR_CONFIG_COUNT; i++) {
        free(contour_map[i]->data);
        free(contour_map[i]);
    }
    free(contour_map);

    for (int i = 0; i <= image->x / step_x; i++) {
        free(grid[i]);
    }
    free(grid);

    if (image != args->new_image) {
        free(image->data);
        free(image);
        free(args->new_image->data);
        free(args->new_image);
    } else {
        free(args->new_image->data);
        free(args->new_image);
    }
}

ppm_image *alloc_rescale_image(){
    ppm_image *new_image = (ppm_image *)malloc(sizeof(ppm_image));
    if (!new_image) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }
    new_image->x = RESCALE_X;
    new_image->y = RESCALE_Y;

    new_image->data = (ppm_pixel*)malloc(new_image->x * new_image->y * sizeof(ppm_pixel));
    if (!new_image) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }
    return new_image;
}

ppm_image *rescale_image(ppm_image *image , int thread_id , int no_threads , ppm_image *new_image) {

    uint8_t sample[3];

    int start = thread_id * new_image->x / no_threads;
    int end = (thread_id + 1) * new_image->x / no_threads;

    // we only rescale downwards
    if (image->x <= RESCALE_X && image->y <= RESCALE_Y) {
        return image;
    }    

    // use bicubic interpolation for scaling
    for (int i = start; i < end; i++) {
        for (int j = 0; j < new_image->y; j++) {
            float u = (float)i / (float)(new_image->x - 1);
            float v = (float)j / (float)(new_image->y - 1);
            sample_bicubic(image, u, v, sample);

            new_image->data[i * new_image->y + j].red = sample[0];
            new_image->data[i * new_image->y + j].green = sample[1];
            new_image->data[i * new_image->y + j].blue = sample[2];
        }
    }

    return new_image;
}

void *f (void *arg) {
    arguments *args = (arguments *)arg;
    int thread_id = args->thread_id;
    int step_x = args->step_x;
    int step_y = args->step_y;
    ppm_image *image = args->image;
    pthread_barrier_t *barrier = args->barrier;
    ppm_image *new_image = args->new_image;

    // 1. Rescale image
    if (image != new_image) {
        image = rescale_image(image , thread_id , args->no_threads , new_image);
    }

    // 2. Sample the grid
    args->grid = sample_grid(image, step_x, step_y, SIGMA , thread_id , args->no_threads , args->grid);

    pthread_barrier_wait(barrier);

    // 3. March the squares
    march(image, args->grid, args->contour_map , step_x, step_y, thread_id , args->no_threads);

    pthread_barrier_wait(barrier);

    return;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: ./tema1 <in_file> <out_file> <P>\n");
        return 1;
    }

    ppm_image *image = read_ppm(argv[1]);
    int step_x = STEP;
    int step_y = STEP;

    pthread_t threads[atoi(argv[3])];
    arguments args[atoi(argv[3])];

    pthread_barrier_t barrier;


    pthread_barrier_init(&barrier , NULL , atoi(argv[3]));

    // 0. Initialize contour map
    ppm_image **contour_map = init_contour_map();

    // 1. Rescale image
    ppm_image *scaled_image;    

    if (image->x > RESCALE_X || image->y > RESCALE_Y) {
        scaled_image = alloc_rescale_image();
    } else {
        scaled_image = image;
    }

    unsigned char **grid = alloc_grid(scaled_image->x / step_x, scaled_image->y / step_y);

    for (int i = 0; i < atoi(argv[3]); i++) {
        args[i].thread_id = i;
        args[i].step_x = step_x;
        args[i].step_y = step_y;
        args[i].image = image;
        args[i].new_image = scaled_image;
        args[i].no_threads = atoi(argv[3]);
        args[i].contour_map = contour_map;
        args[i].barrier = &barrier;
        args[i].grid = grid;
        pthread_create(&threads[i], NULL, f, &args[i]);
    } 

    for (int i = 0; i < atoi(argv[3]); i++) {
        pthread_join(threads[i], NULL);
    }

    // 4. Write output
    write_ppm(scaled_image, argv[2]);

    free_resources(scaled_image, contour_map, grid, step_x , args);
    pthread_barrier_destroy(&barrier);

    return 0;
}
