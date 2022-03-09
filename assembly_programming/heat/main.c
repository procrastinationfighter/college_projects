#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

void start(int width, int height, void *M, float C, float weight);
void place(int count, int *x, int *y, float *temp);
void step();

void print_matrix(int width, int height, void *m) {
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            printf("%4.3f ", ((float *) m)[j * width + i]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("Not enough program arguments.");
        exit(1);
    }

    char *file_name = argv[1];
    float cond = strtod(argv[2], NULL);
    int steps = strtol(argv[3], NULL, 10);

    FILE *file = fopen(file_name, "r");

    int width, height;
    float freeze;
    fscanf(file, "%i %i %f", &width, &height, &freeze);

    // 9 bytes = 4 bytes (temps) + 4 bytes (changes) + 1 byte (heaters)
    void *m = malloc(9 * width * height);
    memset(m, 0, 9 * width * height);
    if (m == NULL) {
        printf("Failed malloc");
        exit(1);
    }

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            fscanf(file, "%f", (float *) (m + sizeof(float) * (j * width + i)));
        }
    }

    int heater_no;
    fscanf(file, "%i", &heater_no);

    int *hx = malloc(heater_no * sizeof(int));
    int *hy = malloc(heater_no * sizeof(int));
    float *h_temp = malloc(heater_no * sizeof(float));

    for (int i = 0; i < heater_no; i++) {
        fscanf(file, "%i %i %f", hx + i, hy + i, h_temp + i);
    }

    start(width, height, m, freeze, cond);
    place(heater_no, hx, hy, h_temp);

    free(hx);
    free(hy);
    free(h_temp);
    fclose(file);

    printf("Initial state:\n");
    print_matrix(width, height, m);
    for (int i = 0; i < steps; i++) {
        printf("Press any key to continue...\n");
        int ch = getchar();
        while (ch != '\n') {
            ch = getchar();
        }
        step();
        printf("After step %i\n", i + 1);
        print_matrix(width, height, m);
    }

    free(m);

    return 0;
}
