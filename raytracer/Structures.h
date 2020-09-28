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
     Vector3 mult(Vector3 inV)
    {
        return Vector3(inV.x * x, inV.y * y, inV.z * z);
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

class Triangle{
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

class LightSource {
public:
    Vector3 position;
    Vector3 color;
    LightSource(Vector3 in_position, Vector3 in_color)
    {
        position = in_position;
        color = in_color;
    }
};


class Scene{
public:
    std::vector<LightSource> lights;
    std::vector<Triangle> triangles;
    //std::vector<Sphere> spheres;

    void add_triangle(Triangle inT)
    {
        triangles.push_back(inT);
	}

    
    void add_light_source(LightSource inL)
    {
        lights.push_back(inL);
	}

    void rayIntersectAll(Ray& r, int& triangle_index, float& t)
    {
        // calculate closest triangle hit
        int min_triangle_index = -1;
        float min_t = INFINITY;
        for (int i = 0; i < triangles.size(); i++) {
            float temp_t = triangles[i].rayIntersection(r);
            if (temp_t > 0.0001 && temp_t < min_t) {
                min_t = temp_t;
                min_triangle_index = i;
            }
        }

        t = min_t;
        triangle_index = min_triangle_index;
    }

    Vector3 getLightInfluence(Vector3 hitpoint, Vector3 normal) {
        Vector3 accumulated_light = Vector3();
        for (size_t light_index = 0; light_index < lights.size(); ++light_index)
        {

            Vector3 to_lightsource = lights.at(light_index).position - hitpoint;
            float light_distance = to_lightsource.abs();
            to_lightsource = to_lightsource.normalize();

            float intensity = to_lightsource.dot(normal);
            intensity /= light_distance * light_distance;
            accumulated_light += lights.at(light_index).color.mult(intensity);
        }
        return accumulated_light;
    }

};

const int CAMSIZE = 800;

class Camera {
public:
    Vector3 camera_position;
    double offset = 1.0;

    Vector3 view_direction;
    Scene* current_scene;

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

    void setScene(Scene* s)
    {
        current_scene = s;
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
        float out_t;
        int out_index;

        current_scene->rayIntersectAll(r, out_index, out_t);

        //if ray hit a surface
        if (out_t > 0.0001 && out_index != -1) {
            Vector3 rayhit = r.start_point + (r.end_point - r.start_point).mult(out_t);
            Vector3 t_normal = current_scene->triangles[out_index].normal;

            int type = current_scene->triangles[out_index].material;
            if (type == DIFFUSE)
            {
                Vector3 accumulated_light = current_scene->getLightInfluence(rayhit, t_normal);
                return current_scene->triangles[out_index].color.mult(accumulated_light);
			}
            else if (type == MIRROR)
            {
                Vector3 in_dir = (r.end_point - r.start_point);
                // r=d-2(d*n)*n
                Vector3 reflect_dir = in_dir - t_normal.mult(2.0 * in_dir.dot(t_normal));

                Ray mirror_ray;
                mirror_ray.end_point = rayhit + reflect_dir;
                mirror_ray.start_point = rayhit;
                float mirror_ray_t;
                int mirror_ray_index;

                current_scene->rayIntersectAll(mirror_ray, mirror_ray_index, mirror_ray_t);

                // NEED EXCEPTION FOR RAYS THAT IMMEDIATELY HIT ANOTHER SURFACE AT T=0 (maybe)
                if (mirror_ray_t > 0.0001 && mirror_ray_index != -1) {
                    Vector3 mirror_rayhit = mirror_ray.start_point + (mirror_ray.end_point - mirror_ray.start_point).mult(mirror_ray_t);
                    Vector3 accumulated_light = current_scene->getLightInfluence(mirror_rayhit, current_scene->triangles[mirror_ray_index].normal);

                    return current_scene->triangles[mirror_ray_index].color.mult(accumulated_light);
                }
                else
                    return Vector3();
                
			}
            else if(type == GLASS)
            {
                std::cout << "refraction is not yet implemented, FOOL";
			}
            else
            {
                std::cout << "ERROR: No known material";
			}
        

        }
        else
            return Vector3(); //Vector3(100,0,0) to test if rays go "out of bounds"
    }

    // xy image plane, not 3d space coordinates
    void render_pixel(int x, int y, Vector3 color) 
    {
        screen[x][y] = color;
    }
};