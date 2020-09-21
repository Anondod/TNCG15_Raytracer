#include "BmpSave.h"
#include "Structures.h"

#include <thread>
#include <array>
#include <vector>
#include <iostream>
#include <mutex>

const int n_threads = 4;

const int width = 800;
char* imageFileName = (char*)"result.bmp";

const int drawinterval = 20; // percentage


static unsigned char image[width][width][BYTES_PER_PIXEL];

int drawmod = drawinterval / 100.0f * width * width;
int drawnpercentage = 0;
int drawnpixels = 0;

std::mutex mtx;

std::vector<Triangle> scene;

Camera camera;



//bad wait function
void badwait() {
    for (int i = 0; i < 1000000000; i++);
}

void drawImageToBMP() {
    //screen = onormaliserad;
    //find max och min
    //screen/max
    
    double imax = 0;
    for (int y = 0; y < width; y++) {
        for (int x = 0; x < width; x++) {
            if (imax < camera.screen[x][y].x)
                imax = camera.screen[x][y].x;
            if (imax < camera.screen[x][y].y)
                imax = camera.screen[x][y].y;
            if (imax < camera.screen[x][y].z)
                imax = camera.screen[x][y].z;
        }
    }
    for (int y = 0; y < width; y++) {
        for (int x = 0; x < width; x++) {
            //draw pixel
            image[x][y][2] = (unsigned char)(camera.screen[x][y].x * 255.99 / imax);            ///red
            image[x][y][1] = (unsigned char)(camera.screen[x][y].y * 255.99 / imax);            ///green
            image[x][y][0] = (unsigned char)(camera.screen[x][y].z * 255.99 / imax);            ///blue
        }
    }

    generateBitmapImage((unsigned char*)image, width, width, imageFileName);
    drawnpercentage += drawinterval;
    std::cout << drawnpercentage << "% rendered\n";
    //badwait(); //for testing
}

// render a vertical segment of the image, placeholder rgb values instead of raytraced values
void rendersegment(int starty, int endy, int r, int g, int b) {
    // xy for 2d image, not representative of actual 3d-space
    for (int y = starty; y < endy; y++) {
        for (int x= 0; x < width; x++) {
            //decide pixel color here (aka do raytracing)
            Vector3 color = camera.calculate_pixel_color(x, y);

            //draw pixel in image and ocasionally render to bmp
            mtx.lock();
            {
                camera.render_pixel(x, y, color);

                //render to bmp in interval
                if (++drawnpixels % drawmod == 0) {
                    drawImageToBMP();
                }
            }
            mtx.unlock();
        }
    }
}



int main()
{

    Vector3 vertex1 = Vector3(4.0, -1.0, -1.0);
    Vector3 vertex2 = Vector3(4.0, -1.0, 1.0);
    Vector3 vertex3 = Vector3(4.0, 1.0, 0.0);
    Vector3 color = Vector3(0.8, 0.6, 0.1);
    Triangle test_triangle = Triangle(vertex1, vertex2, vertex3, color, 0);

    triangles.push_back(test_triangle);


    /*
    Vector3 test_vector_add = test_vector1 + test_vector2;
    Vector3 test_vector_cross = test_vector1.cross(test_vector2);
    test_vector2 += test_vector1;
    std::cout << test_vector1<<"\n";
    std::cout << test_vector2<<"\n";
    
    
    std::cout << "Addition" << test_vector_add<< "\n";
    std::cout << "+=" <<test_vector2.to_string()<< "\n";
    std::cout << "cross" <<test_vector_cross.to_string()<< "\n"; // 0 -1 0
    std::cout << "dot product (1.0, 2.0, 3.0) dot  (0.0, 1.0, 2.0) = " << Vector3(1.0, 2.0, 3.0).dot(Vector3(0.0, 1.0, 2.0)) << "\n";
    */

    std::array<std::thread, n_threads> threads;
    for (int i = 0; i < n_threads; i++) {
        int start = i * width / n_threads;
        int end = (i + 1) * width / n_threads;
        std::cout << "thread " << i << " assigned to height " << start << "-" << end << "\n";
        threads[i] = std::thread(rendersegment, start, end, 0, 0, i * 255 / n_threads);
    }

    std::cout << "Rendering started:\n";

    for (int i = 0; i < n_threads; i++)
        threads[i].join();

    generateBitmapImage((unsigned char*)image, width, width, imageFileName);
    std::cout << "Image completed.";
}


