#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    // Seed the random number generator with the current time
    srand(time(NULL));

    // Create a binary file
    FILE *file = fopen("binary_file.bin", "wb");
    if (!file) {
        printf("Unable to open file.\n");
        return 1;
    }

    // Write 10 random integers between 0 and 4 to the binary file
    for (int i = 0; i < 10; i++) {
        int num = rand() % 5; // Generate a random integer between 0 and 4
        fwrite(&num, sizeof(int), 1, file);
    }

    // Close the binary file
    fclose(file);

    printf("Binary file generated successfully.\n");
    return 0;
}
