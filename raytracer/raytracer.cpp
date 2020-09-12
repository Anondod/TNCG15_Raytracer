#include "BmpSave.h"

#include <thread>
#include <array>
#include <iostream>
#include <mutex>

const int n_threads = 4;

const int height = 800;
const int width = 800;
char* imageFileName = (char*)"result.bmp";

const int drawinterval = 20; // percentage


static unsigned char image[height][width][BYTES_PER_PIXEL];

int drawmod = drawinterval / 100.0f * width * height;
int drawnpercentage = 0;
int drawnpixels = 0;

std::mutex mtx;

//bad wait function
void badwait() {
    for (int i = 0; i < 1000000000; i++);
}

// render a vertical segment of the image, placeholder rgb values instead of raytraced values
void rendersegment(int starty, int endy, int r, int g, int b) {
    for (int i = starty; i < endy; i++) {
        for (int j = 0; j < width; j++) {
            //decide pixel color here (aka do raytracing)

            //draw pixel in image and ocasionally render to bmp
            mtx.lock();
            {
                //draw pixel
                image[i][j][2] = (unsigned char)(r);            ///red
                image[i][j][1] = (unsigned char)(g);            ///green
                image[i][j][0] = (unsigned char)(b);            ///blue

                //render to bmp in interval
                if (++drawnpixels % drawmod == 0) {
                    generateBitmapImage((unsigned char*)image, height, width, imageFileName);
                    drawnpercentage += drawinterval;
                    std::cout << drawnpercentage << "% rendered\n";
                    badwait(); //for testing
                }
            }
            mtx.unlock();
        }
    }
}

int main()
{
    std::array<std::thread, n_threads> threads;
    for (int i = 0; i < n_threads; i++) {
        int start = i * height / n_threads;
        int end = (i + 1) * height / n_threads;
        std::cout << "thread " << i << " assigned to height " << start << "-" << end << "\n";
        threads[i] = std::thread(rendersegment, start, end, 0, 0, i * 255 / n_threads);
    }

    std::cout << "Rendering started:\n";
        
    for (int i = 0; i < n_threads; i++)
        threads[i].join();

    generateBitmapImage((unsigned char*)image, height, width, imageFileName);
    std::cout << "Image completed.";
}


