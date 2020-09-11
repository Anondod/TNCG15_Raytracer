#include "BmpSave.h"

int main()
{
    const int height = 361;
    const int width = 867;
    unsigned char image[height][width][BYTES_PER_PIXEL];
    char* imageFileName = (char*)"result.bmp";

    int i, j;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            image[i][j][2] = (unsigned char)(i * 255 / height);             ///red
            image[i][j][1] = (unsigned char)(j * 255 / width);              ///green
            image[i][j][0] = (unsigned char)((i + j) * 255 / (height + width)); ///blue
        }
    }

    generateBitmapImage((unsigned char*)image, height, width, imageFileName);
    printf("Image generated!!");
}

