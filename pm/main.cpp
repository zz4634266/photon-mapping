#include <iostream>
#include "rtweekend.h"

#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include "moving_sphere.h"
#include "aarect.h"
#include "box.h"
#include "pdf.h"
#include "time.h"
struct photon
{
    point3 pos;
    color c;
    bool visited=false;
    bool caustic=false;

    int specular_hit_times=0;//純紀錄
};
photon photons[100000];
point3 first_hit;
int cnt=1;
bool specular_hit=false;
//int specular_hit_times=0;
hittable_list cornell_box() {
    hittable_list objects;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<flip_face>(make_shared<xz_rect>(213, 343, 227, 332, 554, light)));
    //objects.add(make_shared<xz_rect>(278, 279, 227, 278, 279, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    auto glass = make_shared<dielectric>(1.5);
    objects.add(make_shared<sphere>(point3(190,90,190), 90 , glass));

    auto glass2 = make_shared<dielectric>(2.0);
    objects.add(make_shared<sphere>(point3(400,90,300), 90 , glass2));
    //objects.add(make_shared<sphere>(point3(278,90,443), 90 , glass2));
    /*shared_ptr<hittable> box1 = make_shared<box>(point3(0,0,0), point3(165,330,165), white);
    //shared_ptr<material> aluminum = make_shared<metal>(color(0.8, 0.85, 0.88), 0.0);
    //shared_ptr<hittable> box1 = make_shared<box>(point3(0,0,0), point3(165,330,165), aluminum);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));
    objects.add(box1);*/

    return objects;
    /*hittable_list objects;


    objects.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(color(0.5,0.5,0.5))));
    objects.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(color(0.5,0.5,0.5))));

    auto difflight = make_shared<diffuse_light>(color(4,4,4));
    objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));

    return objects;*/
}

color ray_color(
    const ray& r,
    const color& background,
    const hittable& world,
    shared_ptr<hittable>& lights,
    int depth
) {
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return color(0,0,0);
    // If the ray hits nothing, return the background color.
    if (!world.hit(r, 0.001, infinity, rec))
        return background;



    /*ray scattered;
    color attenuation;
    color emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);
    double pdf_val;
    color albedo;
    if (!rec.mat_ptr->scatter(r, rec, albedo, scattered, pdf_val))
        return emitted;

    hittable_pdf light_pdf(lights, rec.p);
    scattered = ray(rec.p, light_pdf.generate(), r.time());
    pdf_val = light_pdf.value(scattered.direction());

    return emitted
         + albedo * rec.mat_ptr->scattering_pdf(r, rec, scattered)
                  * ray_color(scattered, background, world, lights, depth-1) / pdf_val;*/


scatter_record srec;
    color emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);
    if (!rec.mat_ptr->scatter(r, rec, srec))
        return emitted;

    if(cnt==1)
    {
        first_hit=rec.p;
        cnt++;
    }

    if (srec.is_specular) {
        return srec.attenuation
             * ray_color(srec.specular_ray, background, world, lights, depth-1);
    }




    auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
    mixture_pdf p(light_ptr, srec.pdf_ptr,true);
    //auto p=light_ptr;
    ray scattered = ray(rec.p, p.generate(), r.time());
    auto pdf_val = p.value(scattered.direction());

    if(pdf_val<0.001)return color(0,0,0);
    return emitted
        + srec.attenuation * rec.mat_ptr->scattering_pdf(r, rec, scattered)
                           * ray_color(scattered, background, world, lights, 1) / pdf_val;

    /*return emitted
        + srec.attenuation * rec.mat_ptr->scattering_pdf(r, rec, scattered)
                           * ray_color(scattered, background, world, lights, depth-1) / pdf_val;*/
}

color photon_color(const ray& r,const color& background,const hittable& world,int depth,int num)
{
    hit_record rec;
    if(depth==0)return color(0,0,0);
    if (!world.hit(r, 0.001, infinity, rec))
        return background;


    scatter_record srec;
    color emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);
    if (!rec.mat_ptr->scatter(r, rec, srec))
        return emitted;

    if (srec.is_specular) {
        if(srec.refract)
        {
            photons[num].caustic=true;
            photons[num].specular_hit_times++;
        }
        else photons[num].caustic=false;
        if(!srec.refract)
        {
            //photons[num].caustic=false;
            photons[num].specular_hit_times=0;
        }


        return srec.attenuation
             * photon_color(srec.specular_ray, background, world, depth-1,num);
    }

    mixture_pdf p(srec.pdf_ptr, srec.pdf_ptr,false);

    ray scattered = ray(rec.p, p.generate(), r.time());

    auto pdf_val = p.value(scattered.direction());
    if(pdf_val<0.001)return color(0,0,0);
    if(depth-1>0 && !photons[num].caustic)
    {
        photons[num].pos=rec.p;
        return srec.attenuation * rec.mat_ptr->scattering_pdf(r, rec, scattered)
                           * photon_color(scattered, background, world, depth-1,num) / pdf_val;
    }
    else
    {

        photons[num].pos=rec.p;
        return srec.attenuation * rec.mat_ptr->scattering_pdf(r, rec, scattered)/pdf_val;
    }


}
int main() {

    // Image
    const auto aspect_ratio = 1.0;
    const int image_width = 400;

    const int samples_per_pixel = 25;
    const int max_depth = 4;

    // World
    hittable_list world;


    // Camera

    point3 lookfrom(278,278,-800);
point3 lookat(278,278,0);
auto vfov = 40.0;
vec3 vup(0,1,0);
auto dist_to_focus = (lookfrom-lookat).length();
auto aperture = 2.0;
color background(0,0,0);
const int image_height = static_cast<int>(image_width / aspect_ratio);



world = cornell_box();
shared_ptr<hittable> lights =
        make_shared<xz_rect>(213, 343, 227, 332, 554, shared_ptr<material>());

int photon_num=100000;
int photon_depth=3;
for(int i=0;i<photon_num;i++)
{
    color pc(0,0,0);
        vec3 rd_start_dir=random_in_hemisphere(vec3(0.0,-1.0,0.0));
        ray p_ray(point3(random_double(213.0,343.0),554.0,random_double(332.0,554.0)),rd_start_dir,0.0);
        //std::cout<<p_ray.dir.x()<<" "<<p_ray.dir.y()<<" "<<p_ray.dir.z()<<std::endl;
        pc=photon_color(p_ray,background,world,photon_depth,i);
        //pc=pc*vec3(15,15,15);
        photons[i].c=pc;
        //std::cout<<photons[i].pos.x()<<" "<<photons[i].pos.y()<<" "<<photons[i].pos.z()<<std::endl;
        //std::cout<<photons[photon_num-1].c.x()<<" "<<photons[photon_num-1].c.y()<<" "<<photons[photon_num-1].c.z()<<std::endl;

}
std::cerr << "first pass done "  << std::endl;
camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);
    // Render
    clock_t start, end; // 儲存時間用的變數
    start = clock(); // 計算開始時間
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int j = image_height-1; j >= 0; --j) {
            std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) {
            color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                //auto u = (i + random_double()) / (image_width-1);
                //auto v = (j + random_double()) / (image_height-1);
                auto u = (double)(i ) / (image_width-1);
                auto v = (double)(j ) / (image_height-1);
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, background, world, lights, max_depth);
                //pixel_color=color(0,0,0);
            }
            //std::cout<<first_hit.x()<<" "<<first_hit.y()<<" "<<first_hit.z()<<std::endl;
            int visit_num=0;
            color photon_sum(0,0,0);
            for(int k=0;k<photon_num;k++)
            {
                //1.caustic map  2.global photon map
                //if(photons[k].specular_hit_times==2 && photons[k].caustic &&(fabs(first_hit.x()-photons[k].pos.x())<=5) && (fabs(first_hit.y()-photons[k].pos.y())<=5) && (fabs(first_hit.z()-photons[k].pos.z())<=5))
                if((fabs(first_hit.x()-photons[k].pos.x())<=1) && (fabs(first_hit.y()-photons[k].pos.y())<=1) && (fabs(first_hit.z()-photons[k].pos.z())<=1))
                {
                    if(!photons[k].visited)
                    {
                        visit_num++;
                        //std::cout<<"123"<<std::endl;
                        //std::cout<<fabs(first_hit.x()-photons[k].pos.x())<<std::endl;
                        //std::cout<<photons[k].pos.x()<<" "<<photons[k].pos.y()<<" "<<photons[k].pos.z()<<std::endl;
                        photons[k].visited=true;
                        pixel_color=pixel_color+photons[k].c*samples_per_pixel;
                       // photon_sum+=photons[k].c*samples_per_pixel;
                    }
                }
            }
            //if(visit_num>0)
            //    pixel_color=pixel_color+(photon_sum/(double)visit_num);
            first_hit=point3(0,0,0);
            cnt=1;
            write_color(std::cout, pixel_color, samples_per_pixel);
        }
    }
    end = clock(); // 計算結束時間
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC; // 計算
    std ::cerr<<"\n"<<cpu_time_used<<"\n";
    std::cerr << "\nDone.\n";
}
