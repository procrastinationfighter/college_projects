#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

float LEAST_PRECISION = 1.0f / 256;

void start(int w, int h, int32_t *M, int32_t weight);
void step(int32_t T[]);

static inline int32_t float_to_int(float f) {
    return (int) round(f / LEAST_PRECISION);
}

static inline float int_to_float(int32_t i) {
    return (float) (i) * LEAST_PRECISION;
}

void print_matrix(int32_t *M, int w, int h) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            printf("%4.3f ", int_to_float(M[j * w + i]));
        }
        printf("\n");
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Correct usage: ./%s <input file>\n", argv[0]);
        exit(1);
    }

    FILE *file = fopen(argv[1], "r");

    int width, height;
    float weight;

    fscanf(file, "%i %i %f", &width, &height, &weight);

    if (width <= 0 || height <= 0) {
        printf("Incorrect matrix dimensions.\n");
        exit(1);
    }

    int32_t *M = malloc(sizeof(int32_t) * width * height);
    int32_t *new_wave = malloc(sizeof(int32_t) * height);
    if (M == NULL || new_wave == NULL) {
        printf("Failed malloc");
        exit(1);
    }

    float temp;

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            fscanf(file, "%f", &temp);
            M[j * width + i] = float_to_int(temp);
        }
    }

    start(width, height, M, float_to_int(weight));

    int steps;
    fscanf(file, "%i", &steps);

    printf("INITIAL STATE: \n");
    print_matrix(M, width, height);

    for (int i = 0; i < steps; i++) {
        for (int j = 0; j < height; j++) {
            fscanf(file, "%f", &temp);
            new_wave[j] = float_to_int(temp);
        }

        printf("Input any key to continue...\n");
        int ch = getchar();
        while (ch != '\n') {
            ch = getchar();
        }

        step(new_wave);

        printf("AFTER STEP %i\n", i + 1);
        print_matrix(M, width, height);
    }

    free(new_wave);
    free(M);
    fclose(file);
    return 0;
}
