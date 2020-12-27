/*---------------------------------------------------------------------
Copyright (c) 2020 Pablo Bengoa (bengoana)
https://github.com/bengoana

This software is released under the MIT license.

This program is a college project uploaded for showcase purposes.
---------------------------------------------------------------------*/

#include "renderer.h"
#include "geometry.h"
#include "glm\gtx\transform.hpp"

#include <algorithm>

//For cpu tracing
#include "minitrace.h"

static inline unsigned int ConvertToRGBA(glm::vec3 color) {
  unsigned int out_color_ = 0;

  for (int i = 0; i < 3; i++) {
    if (color[i] > 1.0f) color[i] = 1.0f;
  }

  out_color_ |= ((int)(color.r * 255.0f) << 16);
  out_color_ |= ((int)(color.g * 255.0f) << 8);
  out_color_ |= ((int)(color.b * 255.0f));

  return out_color_;
}

static glm::vec3 BackgroundColor(const Ray& ray) {

  glm::vec3 unit_direction = glm::normalize(ray.dir);
  float t = 0.5 * (unit_direction.y + 1.0);
  return (1.0f - t) * (glm::vec3(1.0, 1.0, 1.0) + t * glm::vec3(0.2, 0.2, 0.6));
}

static inline glm::vec3 Reflect(const glm::vec3& v, const glm::vec3& n) {
  return v - 2 * dot(v, n) * n;
}

Renderer::Renderer(){
  num_threads_ = 64;
  num_bounces_ = 4;


  glm::mat4 X = glm::rotate(-1.5f, glm::vec3(1.0f, 0.0f, 0.0f));
  glm::mat4 Y = glm::rotate(0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 Z = glm::rotate(0.0f,glm::vec3(0.0f,0.0f,1.0f));


  directional_dir_ = ((Y * X) * Z) * glm::vec4(0.0f,0.0f,-1.0f,0.0f);
  directional_inrtensity_ = 0.7f;
}

void Renderer::SetLightRotation(float x, float y, float z){
  glm::mat4 X = glm::rotate(x, glm::vec3(1.0f, 0.0f, 0.0f));
  glm::mat4 Y = glm::rotate(y, glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 Z = glm::rotate(z, glm::vec3(0.0f, 0.0f, 1.0f));


  directional_dir_ = ((Y * X) * Z) * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
}

Renderer::~Renderer(){
}

void Renderer::Init(TScreen *screen){
  screen_ = screen;

  horizontal = glm::vec3(camera_.u, 0, 0);
  vertical = -glm::vec3(0.0f, camera_.v, 0.0f);

  schd.init();
  srand(time(NULL));

  directional_dir_samples_.resize(light_samples_);
  //For tracing
  mtr_init("../../../trace.json");
}

void Renderer::Clean(){
  geometries.clear();
  mtr_shutdown();
}

glm::vec3 Renderer::ComputeLighting(RayInfo info){
  glm::vec3 total_light_ = { 0.0f,0.0f,0.0f };
  
  //Directional

  for (int i = 0; i < light_samples_; ++i) {
    Ray ray;
    ray.origin = info.pos;
    ray.dir = -directional_dir_samples_[i];
    ray.ignored_index_ = info.geometry_index_;
    //Diffuse
    RayInfo result = ComputeRay(ray, 0);

    float diffuse_strength = 0.0f;
    float specular_strength = 0.0f;
    if (result.dist == -1) { //if collision then shadow
      diffuse_strength = glm::max(glm::dot(info.normal, -directional_dir_samples_[i]), 0.0f) * directional_inrtensity_;

      glm::vec3 viewDir = glm::normalize(camera_.pos - info.pos);
      glm::vec3 reflectDir = glm::reflect(directional_dir_samples_[i], info.normal);
      specular_strength = glm::pow(glm::max(glm::dot(viewDir, reflectDir), 0.0f), 64) * directional_inrtensity_;

    }

    total_light_ += info.color * 0.2f + diffuse_strength * geometries[info.geometry_index_]->diffuse_
      + specular_strength * geometries[info.geometry_index_]->specular_;
  }


  total_light_ /= light_samples_;

  return total_light_;
}

RayInfo Renderer::ComputeRay(Ray& ray, int depth){
  
  float distance_ = 99999999.f;
  glm::vec3 last_pos = { 0.0f,0.0f,0.0f };
  glm::vec3 normal = { 0.0f,0.0f,0.0f };
  glm::vec3 color_ = { 0.0f,0.0f,0.0f };
  int geo_index_;
  for (int k = 0; k < geometries.size(); ++k) {
    if (k == ray.ignored_index_) continue;
    float result = geometries[k]->ComputeRay(ray);
    if (result == -1) continue;
    
    if (result < distance_ && result > 0) { //Intersected with a closer object
      distance_ = result;
      normal = geometries[k]->GetNormal(ray.at(result));
      color_ = geometries[k]->color_;
      last_pos = ray.at(result);
      geo_index_ = k;
    }
  }

  RayInfo out_var;

  if (distance_ >= 99999997.f) {
    out_var.dist = -1;
    return out_var;
  }
    
  out_var.dist = distance_;
  out_var.pos = last_pos;
  out_var.normal = normal;
  out_var.geometry_index_ = geo_index_;

  out_var.color = color_;
  
  if (depth <= 0)
    return out_var;
  //Reflectance

  if (geometries[geo_index_]->specular_ > 0.0f) {
    Ray reflect;
    reflect.origin = last_pos;
    reflect.ignored_index_ = geo_index_;
    reflect.dir = glm::reflect(ray.dir, normal);

    RayInfo reflection = ComputeRay(reflect, 0);
    if (reflection.dist > -1.0f) {
      out_var.color += ComputeLighting(reflection) * geometries[geo_index_]->specular_;
    } else {
      out_var.color += BackgroundColor(reflect) * geometries[geo_index_]->specular_ ;

    }
  }
  
  return out_var;
}

void Renderer::Update() {
  MTR_BEGIN("Render", "MainCore");
  while (screen_->height % num_threads_ != 0) {
    num_threads_++;
  }
  int step = screen_->height / num_threads_;

  int base_pos_ = 0;
  int end_pos = step;
  //Offset for softer shadows
  directional_dir_samples_[0] = {
    directional_dir_.x + light_offset_,
      directional_dir_.y,
      directional_dir_.z };
  directional_dir_samples_[1] = {
  directional_dir_.x - light_offset_,
    directional_dir_.y,
    directional_dir_.z };

  lower_left_corner = camera_.pos -
    horizontal / 2.0f - vertical / 2.0f - glm::vec3(0, 0, camera_.focal_length);
  
  schd.run([this, base_pos_, end_pos] {UpdateStep(base_pos_, end_pos, 0); }, &sync_obj);

  for (int i = 0; i < num_threads_; ++i) {


    base_pos_ += step;
    end_pos += step;
    if ((i+1) == num_threads_)
      end_pos = screen_->height;

    schd.run([this, base_pos_, end_pos, i] {UpdateStep(base_pos_, end_pos, i); }, &sync_obj);
  }

  schd.waitFor(sync_obj);
  MTR_END("Render", "MainCore");

}

void Renderer::UpdateStep(int startrow, int endrow, int thread){
  MTR_SCOPE("Render", "StepUpdate");

  for (int i = startrow; i < endrow; ++i) {
    for (int j = 0; j < screen_->width; ++j) {
      float u = float(j) / (screen_->width - 1);
      float v = float(i) / (screen_->height - 1);
      Ray ray;
      ray.origin = camera_.pos;
      ray.dir = lower_left_corner + u * horizontal + v * vertical - camera_.pos;

      RayInfo info = ComputeRay(ray, num_bounces_);
      glm::vec3 color_;
      if (info.dist != -1.0f)
        color_ = ComputeLighting(info);
      else color_ = BackgroundColor(ray);

      screen_->pixels[i * screen_->stride + j] = ConvertToRGBA(color_);

      

    }
  }
}

float Renderer::LengthSquared(glm::vec3 v){
  return v.x * v.x + v.y * v.y + v.z * v.z;
}

float Renderer::Rand(){
  return rand() / (RAND_MAX + 1.0f);
}

float Renderer::RandRange(float min, float max){
  return min + (max - min) * Rand();
}

glm::vec3 Renderer::RandVec(float lengthmin, float lengthmax){
  return glm::vec3(RandRange(lengthmin, lengthmax), RandRange(lengthmin, lengthmax), RandRange(lengthmin, lengthmax));
}

glm::vec3 Renderer::RandSphere(){
  while (true) {
    glm::vec3 p = RandVec(-1, 1);
    if (LengthSquared(p) >= 1) continue;
    return p;
  }
  return glm::vec3();
}



