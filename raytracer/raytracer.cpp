#include "BmpSave.h"
#include "Structures.h"

#include <thread>
#include <array>
#include <vector>
#include <iostream>
#include <mutex>


// TODO:
// - make raytracing recursive
// - add randomization in the camera ray directions
// - add transparent objects with refracting + reflecting rays
// - something about rays hitting another surface with t=0 (on edges)

const int n_threads = 8;
const int drawinterval = 5; // percentage
char* imageFileName = (char*)"result.bmp";

static unsigned char image[CAMSIZE][CAMSIZE][BYTES_PER_PIXEL];

// variables for drawing (to bmp) during the image rendering process
int drawmod = drawinterval / 100.0f * CAMSIZE * CAMSIZE;
int drawnpercentage = 0;
int drawnpixels = 0;

std::mutex mtx;
std::mutex index_lock;
int pixel_index = 0;

Camera camera;
Scene scene;


void renderScene();
void renderSegment(int starty, int endy);
void renderAlternatingPixels(int n_threads, int thread_index);
void renderPixelsInOrder();
void drawImageToBMP();
void addRoom();
void addObjects();
void addBox(Vector3 mid_point, float size, int material);

int main()
{
    addRoom();
    addObjects();
    Vector3 midPoint = Vector3(5.0, 4.9, 0.0);
    Vector3 v0 = midPoint + Vector3(-0.5, 0.0, -0.5);
    Vector3 v1 = midPoint + Vector3(0.5, 0.0, -0.5);
    Vector3 v2 = midPoint + Vector3(-0.5, 0.0, 0.5);
    scene.add_light_source(LightSource(v0, v1, v2, Vector3(1.0, 1.0, 1.0), 1000));
    camera.setScene(&scene);
    renderScene();
}

void renderScene()
{
    std::array<std::thread, n_threads> threads;
    for (int i = 0; i < n_threads; i++) {
        int start = i * CAMSIZE / n_threads;
        int end = (i + 1) * CAMSIZE / n_threads;
        std::cout << "thread " << i << " assigned to height " << start << "-" << end << "\n";

        //threads[i] = std::thread(renderSegment, start, end);
        threads[i] = std::thread(renderAlternatingPixels, n_threads, i);
        //threads[i] = std::thread(renderPixelsInOrder);
    }

    std::cout << "Rendering started:\n";

    for (int i = 0; i < n_threads; i++)
        threads[i].join();

    generateBitmapImage((unsigned char*)image, CAMSIZE, CAMSIZE, imageFileName);
    std::cout << "Image completed.";
}

// render a vertical segment of the image, placeholder rgb values instead of raytraced values
void renderSegment(int starty, int endy)
{
    // xy for 2d image, not representative of actual 3d-space
    for (int y = starty; y < endy; y++) {
        for (int x = 0; x < CAMSIZE; x++) {
            //decide pixel color here (aka do raytracing)
            Vector3 color = camera.calculate_pixel_color(x, y);

            //draw pixel in camera.screen and ocasionally render to bmp
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

// render alternating pixels in order, every thread renders every n_threads pixel
void renderAlternatingPixels(int n_threads, int thread_index)
{
    for (int i = thread_index; i < CAMSIZE*CAMSIZE; i+= n_threads) {
        int x = i % CAMSIZE;
        int y = floor(i / CAMSIZE);

        //decide pixel color here (aka do raytracing)
        Vector3 color = camera.calculate_pixel_color(x, y);

        //draw pixel in camera.screen and ocasionally render to bmp
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

// render first unrendered pixel every time the thread "gets active", SLIGHTLY less efficient MAYBE
void renderPixelsInOrder()
{
    int i = 0;
    while (i < CAMSIZE * CAMSIZE) {
        index_lock.lock();
        i = pixel_index++;
        index_lock.unlock();
        int x = i % CAMSIZE;
        int y = floor(i / CAMSIZE);

        //decide pixel color here (aka do raytracing)
        Vector3 color = camera.calculate_pixel_color(x, y);

        //draw pixel in camera.screen and ocasionally render to bmp
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

void drawImageToBMP() {
    double imax = 0;

    // double sqrt
    for (int y = 0; y < CAMSIZE; y++) {
        for (int x = 0; x < CAMSIZE; x++) {
            if (imax < sqrt(sqrt(camera.screen[x][y].x)))
                imax = sqrt(sqrt(camera.screen[x][y].x));
            if (imax < sqrt(sqrt(camera.screen[x][y].y)))
                imax = sqrt(sqrt(camera.screen[x][y].y));
            if (imax < sqrt(sqrt(camera.screen[x][y].z)))
                imax = sqrt(sqrt(camera.screen[x][y].z));
        }
    }

    for (int y = 0; y < CAMSIZE; y++) {
        for (int x = 0; x < CAMSIZE; x++) {
            //image is defined as image[y][x][c], c=2,1,0: rgb
            double red   = sqrt(sqrt(camera.screen[x][y].x));
            double green = sqrt(sqrt(camera.screen[x][y].y));
            double blue  = sqrt(sqrt(camera.screen[x][y].z)); 
            // scope now: 0 - imax
            red   = red   / imax * 255.99;
            green = green / imax * 255.99;
            blue  = blue  / imax * 255.99;

            image[y][x][2] = (unsigned char)(red);      // red
            image[y][x][1] = (unsigned char)(green);    // green
            image[y][x][0] = (unsigned char)(blue);     // blue
        }
    }

    // single sqrt
    /*
    for (int y = 0; y < CAMSIZE; y++) {
        for (int x = 0; x < CAMSIZE; x++) {
            if (imax < sqrt(camera.screen[x][y].x))
                imax = sqrt(camera.screen[x][y].x);
            if (imax < sqrt(camera.screen[x][y].y))
                imax = sqrt(camera.screen[x][y].y);
            if (imax < sqrt(camera.screen[x][y].z))
                imax = sqrt(camera.screen[x][y].z);
        }
    }

    for (int y = 0; y < CAMSIZE; y++) {
        for (int x = 0; x < CAMSIZE; x++) {
            //image is defined as image[y][x][c], where c=3: alpha, c=2,1,0: rgb
            image[y][x][2] = (unsigned char)(sqrt(camera.screen[x][y].x) * 255.99 / imax * 255.99);            ///red
            image[y][x][1] = (unsigned char)(sqrt(camera.screen[x][y].y) * 255.99 / imax) * 255.99;            ///green
            image[y][x][0] = (unsigned char)(sqrt(camera.screen[x][y].z) * 255.99 / imax) * 255.99;            ///blue
        }
    } 
    */

    generateBitmapImage((unsigned char*)image, CAMSIZE, CAMSIZE, imageFileName);
    drawnpercentage += drawinterval;
    std::cout << drawnpercentage << "% rendered\n";
}

void addRoom()
{
    // directions based on lec6 image for "the world"
    // BUT! y is the vertical axis here
    // AND! y = -z, x = x
    // as in: x goes right, y goes up, z goes out of your screen  (standard way? I think)

    // lower plane
    Vector3 bot0 = Vector3(5.0, -5.0, 0.0);     // mid
    Vector3 bot1 = Vector3(-3.0, -5.0, 0.0);    // left
    Vector3 bot2 = Vector3(0.0, -5.0, -6.0);     // up-left
    Vector3 bot3 = Vector3(10.0, -5.0, -6.0);    // up-right
    Vector3 bot4 = Vector3(13.0, -5.0, 0.0);    // right
    Vector3 bot5 = Vector3(10.0, -5.0, 6.0);   // down-right
    Vector3 bot6 = Vector3(0.0, -5.0, 6.0);    // down-left

    //upper plane
    Vector3 up0 = Vector3(5.0, 5.0, 0.0);     // mid
    Vector3 up1 = Vector3(-3.0, 5.0, 0.0);    // left
    Vector3 up2 = Vector3(0.0, 5.0, -6.0);     // up-left
    Vector3 up3 = Vector3(10.0, 5.0, -6.0);    // up-right
    Vector3 up4 = Vector3(13.0, 5.0, 0.0);    // right
    Vector3 up5 = Vector3(10.0, 5.0, 6.0);   // down-right
    Vector3 up6 = Vector3(0.0, 5.0, 6.0);    // down-left

    //colors
    Vector3 cl_white = Vector3(1.0, 1.0, 1.0);
    Vector3 cl_1 = Vector3(0.5, 0.2, 0.6);
    Vector3 cl_2 = Vector3(0.8, 0.3, 0.1);
    Vector3 cl_3 = Vector3(0.1, 0.4, 0.2);
    Vector3 cl_4 = Vector3(0.1, 0.7, 0.6);
    Vector3 cl_5 = Vector3(0.9, 0.9, 0.2);
    Vector3 cl_6 = Vector3(0.1, 0.0, 0.7);

   
    // lower plane triangles
    scene.add_triangle(Triangle(bot1, bot0, bot2, cl_white, DIFFUSE));
    scene.add_triangle(Triangle(bot2, bot0, bot3, cl_white, DIFFUSE));
    scene.add_triangle(Triangle(bot3, bot0, bot4, cl_white, DIFFUSE));
    scene.add_triangle(Triangle(bot4, bot0, bot5, cl_white, DIFFUSE));
    scene.add_triangle(Triangle(bot5, bot0, bot6, cl_white, DIFFUSE));
    scene.add_triangle(Triangle(bot6, bot0, bot1, cl_white, DIFFUSE));

    // upper plane triangles (opposite order for reversed normals
    scene.add_triangle(Triangle(up2, up0, up1, cl_white, DIFFUSE));
    scene.add_triangle(Triangle(up3, up0, up2, cl_white, DIFFUSE));
    scene.add_triangle(Triangle(up4, up0, up3, cl_white, DIFFUSE));
    scene.add_triangle(Triangle(up5, up0, up4, cl_white, DIFFUSE));
    scene.add_triangle(Triangle(up6, up0, up5, cl_white, DIFFUSE));
    scene.add_triangle(Triangle(up1, up0, up6, cl_white, DIFFUSE));

    // first set of wall triangles ("bottom left half")
    scene.add_triangle(Triangle(bot1, bot2, up1, cl_1, DIFFUSE));
    scene.add_triangle(Triangle(bot2, bot3, up2, cl_2, DIFFUSE));
    scene.add_triangle(Triangle(bot3, bot4, up3, cl_3, DIFFUSE));
    scene.add_triangle(Triangle(bot4, bot5, up4, cl_4, DIFFUSE));
    scene.add_triangle(Triangle(bot5, bot6, up5, cl_5, DIFFUSE));
    scene.add_triangle(Triangle(bot6, bot1, up6, cl_6, DIFFUSE));

    // second set of wall triangles ("top right half")
    scene.add_triangle(Triangle(bot1, up1, up6, cl_6, DIFFUSE));
    scene.add_triangle(Triangle(bot2, up2, up1, cl_1, DIFFUSE));
    scene.add_triangle(Triangle(bot3, up3, up2, cl_2, DIFFUSE));
    scene.add_triangle(Triangle(bot4, up4, up3, cl_3, DIFFUSE));
    scene.add_triangle(Triangle(bot5, up5, up4, cl_4, DIFFUSE));
    scene.add_triangle(Triangle(bot6, up6, up5, cl_5, DIFFUSE));
}

void addObjects() {
    // tetrahedron
    if(false)
    {
        Vector3 mid_point = Vector3(1.0, 0.0, 0.0);
        Vector3 tet1 = Vector3(0.0, 1.0, 1.0) + mid_point;
        Vector3 tet2 = Vector3(1.0, 0.0, 1.0) + mid_point;
        Vector3 tet3 = Vector3(1.0, 1.0, 0.0) + mid_point;
        Vector3 tet4 = Vector3(0.0, 0.0, 0.0) + mid_point;

        // tetrahedron
        scene.add_triangle(Triangle(tet1, tet4, tet2, Vector3(), GLASS));
        scene.add_triangle(Triangle(tet2, tet4, tet3, Vector3(), GLASS));
        scene.add_triangle(Triangle(tet1, tet2, tet3, Vector3(), GLASS));
        scene.add_triangle(Triangle(tet1, tet3, tet4, Vector3(), GLASS));
    }
    
    //TETRAHEDRON 2
    if(true)
    {
        Vector3 mid_point = Vector3(10.0, -3.0, 1.0);
        Vector3 tet1 = Vector3(-1.0, 0.0, 0.0) + mid_point; // cam-front
        Vector3 tet2 = Vector3(0.0, -2.0, -2.0) + mid_point; // cam-left
        Vector3 tet3 = Vector3(0.0, -2.0, 2.0) + mid_point; // cam-right
        Vector3 tet4 = Vector3(0.0, 2.0, 0.0) + mid_point; // cam top

        // tetrahedron
        scene.add_triangle(Triangle(tet2, tet3, tet1, Vector3(1, 0, 0), DIFFUSE));
        scene.add_triangle(Triangle(tet3, tet4, tet1, Vector3(1, 0, 0), DIFFUSE));
        scene.add_triangle(Triangle(tet4, tet2, tet1, Vector3(1, 0, 0), DIFFUSE));
        scene.add_triangle(Triangle(tet3, tet2, tet4, Vector3(1, 0, 0), DIFFUSE));
    }

    //TETRAHEDRON 2 copy
    if (true)
    {
        Vector3 mid_point = Vector3(3.0, 0.0, 0.0);
        Vector3 tet1 = Vector3(-1.0, 0.0, 0.0) + mid_point; // cam-front
        Vector3 tet2 = Vector3(0.0, -1.0, -1.0) + mid_point; // cam-left
        Vector3 tet3 = Vector3(0.0, -1.0, 1.0) + mid_point; // cam-right
        Vector3 tet4 = Vector3(0.0, 1.0, 0.0) + mid_point; // cam top

        // tetrahedron
        scene.add_triangle(Triangle(tet2, tet3, tet1, Vector3(1, 1, 1), GLASS));
        scene.add_triangle(Triangle(tet3, tet4, tet1, Vector3(1, 1, 1), GLASS));
        scene.add_triangle(Triangle(tet4, tet2, tet1, Vector3(1, 1, 1), GLASS));
        scene.add_triangle(Triangle(tet3, tet2, tet4, Vector3(1, 1, 1), GLASS));
    }

    if (true) {
        scene.add_sphere(Sphere(Vector3(4,-4,2), 1, Vector3(1,1,1), MIRROR));
    }
    if(true){
    
        Vector3 box1 = Vector3(6.0, 2.0, -3.0);
        Vector3 box2 = Vector3(6.0, 0.0, -3.0);
        Vector3 box3 = Vector3(6.0, -3.0, -3.0);
        
        addBox(box1, 1.0, DIFFUSE);
        addBox(box2, 2.0, DIFFUSE);
        addBox(box3, 3.0, DIFFUSE);
	}
}


void addBox(Vector3 mid_point, float size = 1.0, int material = DIFFUSE)
{
    
    mid_point -=  Vector3(0.5, 0.5, 0.5).mult(size);
    Vector3 box1 = Vector3(0.0, 0.0, 1.0).mult(size) + mid_point; //ner vänster
    Vector3 box2 = Vector3(1.0, 0.0, 1.0).mult(size) + mid_point; //ner höger
    Vector3 box3 = Vector3(1.0, 1.0, 1.0).mult(size) + mid_point; //upp höger
    Vector3 box4 = Vector3(0.0, 1.0, 1.0).mult(size) + mid_point; //upp vänster

    Vector3 box5 = Vector3(0.0, 0.0, 0.0).mult(size) + mid_point; //ner vänster
    Vector3 box6 = Vector3(1.0, 0.0, 0.0).mult(size) + mid_point; //ner höger
    Vector3 box7 = Vector3(1.0, 1.0, 0.0).mult(size) + mid_point; //upp höger
    Vector3 box8 = Vector3(0.0, 1.0, 0.0).mult(size) + mid_point; //upp vänster

    scene.add_triangle(Triangle(box1, box2, box3, Vector3(1,1,1), material));
    scene.add_triangle(Triangle(box1, box3, box4, Vector3(1,1,1), material));

    scene.add_triangle(Triangle(box2, box6, box7, Vector3(1,1,1), material));
    scene.add_triangle(Triangle(box2, box7, box3, Vector3(1,1,1), material));

    scene.add_triangle(Triangle(box6, box5, box8, Vector3(1, 1, 1), material));
    scene.add_triangle(Triangle(box6, box8, box7, Vector3(1, 1, 1), material));

    scene.add_triangle(Triangle(box5, box1, box4, Vector3(1, 1, 1), material));
    scene.add_triangle(Triangle(box5, box4, box8, Vector3(1, 1, 1), material));

    scene.add_triangle(Triangle(box4, box3, box7, Vector3(1, 1, 1), material));
    scene.add_triangle(Triangle(box4, box7, box8, Vector3(1, 1, 1), material));

    scene.add_triangle(Triangle(box5, box6, box2, Vector3(1, 1, 1), material));
    scene.add_triangle(Triangle(box5, box2, box1, Vector3(1, 1, 1), material));
	
}