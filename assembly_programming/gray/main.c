#include <stdio.h>
#include <stdlib.h>

unsigned char R_WEIGHT = 77, G_WEIGHT = 151, B_WEIGHT = 28;

extern void make_gray(unsigned char *matrix, int width, int height);

int main(int argc, char *argv[]) {
    switch (argc) {
        case 1:
            printf("Program usage: ./%s <file>.ppm R? G? B?\n", argv[0]);
            exit(1);
        case 2:
            break;
        case 5:
            B_WEIGHT = atoi(argv[4]);
        case 4:
            G_WEIGHT = atoi(argv[3]);
        case 3:
            R_WEIGHT = atoi(argv[2]);
            break;
        default:
            printf("Too many arguments.\n");
            exit(1);
    }

    FILE *input = fopen(argv[1], "r");

    char f, s;
    fscanf(input, "%c%c", &f, &s);
    if (f != 'P' || s != '3') {
        printf("File not in P3 format. \n");
        exit(1);
    }

    int width, height;
    fscanf(input, "%i %i", &width, &height);
    unsigned char *matrix = malloc(3 * width * height * sizeof(unsigned char));
    if (matrix == NULL) {
        printf("Malloc failed.\n");
        exit(1);
    }
    int first, second, third;
    fscanf(input, "%i", &first);

    for (int i = 0; i < width * height * 3; i += 3) {
        fscanf(input, "%i %i %i", &first, &second, &third);
        matrix[i] = (unsigned char) first;
        matrix[i + 1] = (unsigned char) second;
        matrix[i + 2] = (unsigned char) third;
    }
    fclose(input);

    make_gray(matrix, width, height);

    // Change extension of output file.
    int k = 0;
    while (argv[1][k] != '.') {
        k++;
    }
    argv[1][k + 2] = 'g';
    FILE *output = fopen(argv[1], "w");
    fprintf(output, "P2\n%i %i\n255\n", width, height);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            fprintf(output, "%i ", matrix[i * width + j]);
        }
        fprintf(output, "\n");
    }

    fclose(output);
    free(matrix);
    return 0;
}
