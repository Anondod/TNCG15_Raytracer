#pragma once
#include <string>
#include <math.h>
#include <vector>
#include <iostream>


class Vector3 {
public:
    double x;
    double y;
    double z;
    Vector3()
    {
        x = 0.0;
        y = 0.0;
        z = 0.0;
	}
    Vector3(double in_x, double in_y, double in_z)
    {
        x = in_x;
        y = in_y;
        z = in_z;
    }

    Vector3& operator+=(const Vector3 inV)
    {
        x = x + inV.x;
        y = y + inV.y;
        z = z + inV.z;

        return *this;
	}

    friend Vector3 operator+(Vector3 lhs, const Vector3& rhs)
    {
        lhs += rhs;
        return lhs;
	}

    Vector3& operator-=(const Vector3 inV)
    {
        x = x - inV.x;
        y = y - inV.y;
        z = z - inV.z;

        return *this;
	}

    friend Vector3 operator-(Vector3 lhs, const Vector3& rhs)
    {
        lhs -= rhs;
        return lhs;
	}

    double abs()
    {
        return sqrt(x * x + y * y + z * z);
    }

    Vector3 normalize()
    {
        return (*this).mult(1/abs());
    }

    double dot(Vector3 inV)
    {
        return x * inV.x + y * inV.y + z * inV.z;
    }
    Vector3 cross(Vector3 inV)
    {
        return Vector3(y * inV.z - inV.y* z, z * inV.x - inV.z * x, x * inV.y - inV.x * y);
    }

    Vector3 mult(double factor)
    {
        return Vector3(factor * x, factor * y, factor * z);
	}
    std::string to_string()
    {
        std::string out_string = "<";
        out_string += std::to_string(x) + ", "
            + std::to_string(y) + ", "
            + std::to_string(z) + ">";

        return out_string;
	}
    friend std::ostream& operator<<(std::ostream& os, Vector3& inV)
    {
        os << inV.to_string();
        return os;
    }
};



struct Ray {
    Vector3 start_point;
    Vector3 end_point;
    Vector3 ray_color;
};

const int DIFFUSE = 0;
const int MIRROR = 1;
const int GLASS = 2;

struct Triangle{
public:
    Vector3 v0;
    Vector3 v1;
    Vector3 v2;

    Vector3 normal;

    Vector3 color;

    int material;

    Triangle() = delete;
	
    Triangle(const Vector3& in0, const Vector3& in1, const Vector3& in2, const Vector3& inc, int inm)
    {
        v0 = in0;
        v1 = in1;
        v2 = in2;
        color = inc;
        material = inm;

        Vector3 side1 = v1 - v0;
        Vector3 side2 = v2 - v0;

        normal = side1.cross(side2).normalize();
        std::cout << side1 << " cross ";
        std::cout << side2 << "  : ";
        std::cout << normal << "\n";
	}

    float rayIntersection(Ray &r)
    {
        Vector3 T = r.start_point - v0;
        Vector3 E1 = v1 - v0;
        Vector3 E2 = v2 - v0;
        Vector3 D = r.end_point - r.start_point;
        Vector3 P = D.cross(E2);
        Vector3 Q = T.cross(E1);

        Vector3 tuv = Vector3(Q.dot(E2), P.dot(T), Q.dot(D)).mult(1.0 / P.dot(E1));

        // if ray intersected triangle return t, else return -1 for miss
        if (tuv.y >= 0 && tuv.z >= 0 && tuv.y + tuv.z <= 1)
            return tuv.x;
        else
            return -1.0;
    }
};

std::vector<Triangle> triangles;

const int CAMSIZE = 800;

class Camera {
public:
    Vector3 camera_position;
    double offset = 1.0;

    Vector3 view_direction;


    Vector3 screen[CAMSIZE][CAMSIZE];

    double  pixel_size = 1.0 / (CAMSIZE / 2.0);;

    Camera() 
    {
       //fill screen with zero
        for (int i = 0; i < CAMSIZE; i++) {
            for (int j = 0; j < CAMSIZE; j++) {
                screen[i][j] = Vector3(); // zero vector
            }
        }
    }

    // xy image plane, not 3d space coordinates
    Vector3 calculate_pixel_color(int x, int y) 
    {
        // calculate pixel-plane position
        Vector3 pixel_position = camera_position + Vector3(offset, (y - CAMSIZE /2) * pixel_size, (x - CAMSIZE /2) * pixel_size);
        //ABOVE PIXEL COORDS MIGHT BE WRONG

        // Send ray towards position
        Ray r;
        r.end_point = pixel_position;
        r.start_point = camera_position;
        

        // calculate closest triangle hit
        int triangle_index = -1;
        float min_t = 1000000000;
        for (int i = 0; i < triangles.size(); i++) {
            float temp_t = triangles[i].rayIntersection(r);
            if (temp_t > 0.0001 && temp_t < min_t) {
                min_t = temp_t;
                triangle_index = i;
            }
        }

        //TEMP return triangle color if hit or black if miss
        if (min_t > 0.0001 && triangle_index != -1)
            return triangles[triangle_index].color; //.mult(triangles[triangle_index].normal.dot(r.end_point - r.start_point));
        else
            return Vector3();
    }

    // xy image plane, not 3d space coordinates
    void render_pixel(int x, int y, Vector3 color) 
    {
        screen[x][y] = color;
    }
};

class Scene {

};


/*
ColorDbl getLightFromDirection(Vertex start, Vertex dir) {
    Ray r = raytrace(start, dir);
    hitpos;
    hitriangle
    hitriangle.normal

    Direction reflectiondir = reflect(r.end, r.hittriangle.normal);
    Direction refractiondir = refract(r.end, r.hittriangle);

    color += hitriangle.color * getDiffuseLight(r, lightlist); //shadow rays
    // point light position + n | dir2light


    color += hitriangle.specular_color * getLightFromDirection(r, reflectiondir); //reflection
    color += hitriangle.color * getLightFromDirection(r, refractiondir); //refraction

    return color;
}

ColorDbl getDiffuseLight(Vertex hit_position, Triangle inT) {

    for (light_source in light_source_list) {
        raytrace(hit_position, light_source.position);

        hitpoint->ljus->triangel  if (t < 1)
    }
}

ColorDbl color = getLightFromDirection(start, dir, importance, count_bounces);
*/