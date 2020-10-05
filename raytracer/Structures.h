#pragma once
#include <string>
#include <math.h>
#include <vector>
#include <iostream>

#define PI 3.14159265358979323846

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

class Sphere {
public:
    Vector3 position;
    float radius;

    Vector3 color;
    int material;

    Sphere(Vector3 in_position, float in_radius, Vector3 in_color, int in_material) {
        position = in_position;
        radius = in_radius;
        color = in_color;
        material = in_material;
    }

    float rayIntersection(Ray& r)
    {
        // https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
    
        Vector3 L = position - r.start_point;
        Vector3 rayVector = r.end_point - r.start_point;
        float rayAbs = rayVector.abs();
        Vector3 D = rayVector.mult(1.0 / rayAbs);

        float t_ca = L.dot(D);
        if(t_ca < 0) return -1; //dont do this cause ray could start inside sphere

        float d = sqrt(L.dot(L) - t_ca*t_ca);
        if (d > radius) return -1;

        float t_hc = sqrt(radius * radius - d * d);

        // first contact
        //std::cout << "t_ca:" << t_ca << " " << "t_hc:" << t_hc << "   ";
        float t0 = t_ca - t_hc;
        if (t0 > 0) {
            //std::cout << t0* rayAbs << " FIRST \n";
            return t0 / rayAbs; // needs * rayabs because the return float t goes from ray_start = 0, to ray_end = 1, not normalised
        }
        //second contact
        float t1 = t_ca + t_hc;
        if (t1 > 0) {
            //std::cout << t1 * rayAbs << " SECOND " << r.start_point << "\n";
            return t1 / rayAbs;
        }

        //no hit
        return -1;
    }

    Vector3 getNormalAt(Vector3 inV) {
        return (inV - position).normalize();
    }

};
// Kill only Lambertian / Oren Nayar rays. But Refractions have to be killed too. Kill refractions/reflections INSIDE transparent object after a set amount of bounces. 3 or 4 is okay
// we want 2 -10 bounces, threshold depends on how dark the object is
bool russianRoulette(float threshold) {
    float P = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    return P < threshold;

}


class LightSource {
public:
    Vector3 v0;
    Vector3 v1;
    Vector3 v2;
    Vector3 normal;
    Vector3 color;
    float area;
    Vector3 side1;
    Vector3 side2;

    LightSource(const Vector3& in0, const Vector3& in1, const Vector3& in2,
        Vector3 in_color, float watts)
    {
        v0 = in0;
        v1 = in1;
        v2 = in2;
        side1 = v1 - v0;
        side2 = v2 - v0;

        area = side1.cross(side2).abs();

        normal = side1.cross(side2).normalize();

        color = in_color.mult(watts / (2*PI));
    }

    float rayIntersection(Ray& r)
    {
        Vector3 T = r.start_point - v0;
        Vector3 E1 = v1 - v0;
        Vector3 E2 = v2 - v0;
        Vector3 D = r.end_point - r.start_point;
        Vector3 P = D.cross(E2);
        Vector3 Q = T.cross(E1);

        Vector3 tuv = Vector3(Q.dot(E2), P.dot(T), Q.dot(D)).mult(1.0 / P.dot(E1));

        // if ray intersected triangle return t, else return -1 for miss
        if (tuv.y >= 0 && tuv.z >= 0 && tuv.y <= 1 && tuv.z <= 1)
            return tuv.x;
        else
            return -1.0;
    }

    Vector3 randPoint() {
        float U = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float V = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        return v0 + side1.mult(U) + side2.mult(V);
    }

};


const int TRIANGLE = 0;
const int SPHERE = 1;
const int LIGHT = 2;

class Scene{
public:
    std::vector<LightSource> lights;
    std::vector<Triangle> triangles;
    std::vector<Sphere> spheres;

    void add_triangle(Triangle inT)
    {
        triangles.push_back(inT);
	}
    void add_sphere(Sphere inS)
    {
        spheres.push_back(inS);
    }
    
    void add_light_source(LightSource inL)
    {
        lights.push_back(inL);
	}

    void rayIntersectAll(Ray& r, int& index, float& t, int& hit_type)
    {
        // calculate closest triangle hit
        int min_index = -1;
        float min_t = INFINITY;
        for (int i = 0; i < triangles.size(); i++) {
            float temp_t = triangles[i].rayIntersection(r);
            if (temp_t > 0.0001 && temp_t < min_t) {
                min_t = temp_t;
                min_index = i;
                hit_type = TRIANGLE;
            }
        }
        for (int i = 0; i < spheres.size(); i++) {
            float temp_t = spheres[i].rayIntersection(r);
            if (temp_t > 0.0001 && temp_t < min_t) {
                min_t = temp_t;
                min_index = i;
                hit_type = SPHERE;
            }
        }
        for (int i = 0; i < lights.size(); i++) {
            float temp_t = lights[i].rayIntersection(r);
            if (temp_t > 0.0001 && temp_t < min_t) {
                min_t = temp_t;
                min_index = i;
                hit_type = LIGHT;
            }
        }

        t = min_t;
        index = min_index;
    }

    Vector3 directLight(Vector3 hitpoint, Vector3 hitnormal) {
        Vector3 accumulated_light = Vector3();
        int M = 5;
        for (size_t light_index = 0; light_index < lights.size(); light_index++)
        {
            float light_intensity = 0.0f;
            for (size_t k = 0; k < M; k++)
            {
                Vector3 light_point = lights[light_index].randPoint();
                Vector3 Sk = light_point - hitpoint;

                // surface is turned away from light source ray
                if (hitnormal.dot(Sk) < 0) continue;

                // shadow ray comes from the back of light surface
                if (lights[light_index].normal.dot(Sk) > 0) continue;

                Ray shadow_ray;
                shadow_ray.start_point = hitpoint;
                shadow_ray.end_point = light_point;

                float out_t;
                int out_index;
                int hit_type;
                rayIntersectAll(shadow_ray, out_index, out_t, hit_type);

                //if (hit_type == SPHERE) std::cout << "SPHERE";

                // this is equivalent to Vk
                if (out_index == light_index && hit_type == LIGHT) {
                    float Dk = Sk.abs();

                    float cosak = -Sk.dot(lights[light_index].normal) / Dk;
                    float cosbk = Sk.dot(hitnormal) / Dk;

                    light_intensity += cosak * cosbk / (Dk * Dk);
                }
                
            }
            accumulated_light += lights[light_index].color.mult(lights[light_index].area * light_intensity);
        }

        return accumulated_light.mult(1.0 / M);
    }

    /*Vector3 directLight(Vector3 hitpoint, Vector3 normal) {
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
    }*/

};

const int CAMSIZE = 800;

class Camera {
public:
    Vector3 camera_position = Vector3(-1,0,0); //Vector3(-2,0,0)
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
        
        Vector3 lightresult = recursive(r, 0);
        return lightresult;
    }

    Vector3 recursive(Ray inRay, int time_to_die) {
        float P = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float myValue = 0.8;
        if(P > myValue)

        if (time_to_die > 10) {
            return Vector3();
        }

        float out_t;
        int out_index;
        int hit_type;
        current_scene->rayIntersectAll(inRay, out_index, out_t, hit_type);
        if (hit_type == LIGHT)
        {
            return current_scene->lights[out_index].color;
        }
        //if ray hit a surface
        else if (out_t > 0.0001 && out_index != -1) {
            Vector3 rayhit = inRay.start_point + (inRay.end_point - inRay.start_point).mult(out_t);
            Vector3 in_dir = (inRay.end_point - inRay.start_point).normalize();
            //if sphere get normal somehow else
            Vector3 t_normal;
            Vector3 color;
            int type;
            if(hit_type == SPHERE)
            {
                t_normal = current_scene->spheres[out_index].getNormalAt(rayhit);
                color = current_scene->spheres[out_index].color;
                type = current_scene->spheres[out_index].material;
            }
            else { // hit_type == TRIANGLE
                t_normal = current_scene->triangles[out_index].normal;
                color = current_scene->triangles[out_index].color;
                type = current_scene->triangles[out_index].material;
            }
            
            if (type == DIFFUSE)
            {
                //if (P)
                //    recursive(normalish riktning);

                // DO DIRECT LIGHT MONTE CARLO INTEGRATION
                Vector3 accumulated_light = current_scene->directLight(rayhit, t_normal);
                return color.mult(accumulated_light);
            }
            else if (type == MIRROR)
            {
                // r=d-2(d*n)*n
                Vector3 reflect_dir = in_dir - t_normal.mult(2.0 * in_dir.dot(t_normal));

                Ray mirror_ray;
                mirror_ray.end_point = rayhit + reflect_dir;
                mirror_ray.start_point = rayhit;

                Vector3 lightresult = recursive(mirror_ray, time_to_die + 1);
                return lightresult;
            }
            else if (type == GLASS)
            {
                float n1, n2;
                // from glass into air
                if (in_dir.dot(t_normal) > 0.0) {
                    n1 = 1.5;
                    n2 = 1.0;
                    // invert normal since the ray hits the backside of the triangle;
                    t_normal = t_normal.mult(-1.0);
                }
                // from air into glass
                else {
                    n1 = 1.0;
                    n2 = 1.5;
				}

                float am = asinf(n2 / n1);
                float theta = acosf(t_normal.dot(in_dir.normalize().mult(-1.0)));


                // r=d-2(d*n)*n
                Vector3 reflect_dir = in_dir - t_normal.mult(2.0 * in_dir.dot(t_normal));

                Ray mirror_ray;
                mirror_ray.end_point = rayhit + reflect_dir;
                mirror_ray.start_point = rayhit;

                Vector3 out_light = recursive(mirror_ray, time_to_die + 1);

                // do refraction if:
                // - if ray hits surface with higher refraction coefficient
                // - incoming angle is smaller than brewster angle
                if (n2 > n1 || (theta <= am)) {
                    //if((theta <= am))
                        //std::cout << "theta: " << theta << " , am: " << am << "\n";
                    float R0 = pow((n1 - n2) / (n1 + n2), 2);

                    float Rc = R0 + (1 - R0) * pow((1 - cos(theta)), 5);
                    float T = 1.0 - Rc;

                    float ndiv = n1 / n2;
                    float NI_dot = in_dir.dot(t_normal);

                    Vector3 refract_dir = in_dir.mult(ndiv) + t_normal.mult(-1.0 * ndiv * NI_dot - sqrt(1.0 - ndiv * ndiv * (1.0 - NI_dot * NI_dot)));
                
                    Ray refract_ray;
                    refract_ray.end_point = rayhit + refract_dir;
                    refract_ray.start_point = rayhit;

                    Vector3 lightresult_refract = recursive(refract_ray, time_to_die + 1);
                
                    out_light = out_light.mult(Rc) + lightresult_refract.mult(T);
                }

                return out_light; 
            }
            else
            {
                std::cout << "ERROR: No known material";
                return Vector3(1000000, 0, 0);
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