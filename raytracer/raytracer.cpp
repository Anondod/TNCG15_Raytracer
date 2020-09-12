#include "BmpSave.h"

int main()
{
    printf("start");
    const int height = 800;
    const int width = 800;
    static unsigned char image[height][width][BYTES_PER_PIXEL];
    char* imageFileName = (char*)"result.bmp";

    int i, j;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            image[i][j][2] = (unsigned char)(i);            ///red
            image[i][j][1] = (unsigned char)(j);              ///green
            image[i][j][0] = (unsigned char)(j+i);              ///blue
        }
    }
    printf("Generating...  ");
    generateBitmapImage((unsigned char*)image, height, width, imageFileName);
    printf("Image generated!!");
}

