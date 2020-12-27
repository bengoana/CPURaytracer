/*---------------------------------------------------------------------
Copyright (c) 2020 Pablo Bengoa (bengoana)
https://github.com/bengoana

This software is released under the MIT license.

This program is a college project uploaded for showcase purposes.
---------------------------------------------------------------------*/

#ifndef __RENDERER_H__
#define __RENDERER_H__ 1

#include "glm/glm.hpp"
#include <vector>

#include "px_sched.h"

class Geometry;

struct TScreen {
  unsigned int width;
  unsigned int height;
  int stride;
  unsigned int* pixels;
};

struct Camera {
  glm::vec3 pos;
  float focal_length;
  float u, v;
};

struct Light {
  glm::vec3 pos;
  glm::vec3 color;
  float intensity;
};

struct Ray {
  glm::vec3 origin;
  glm::vec3 dir;
  int ignored_index_ = -1;

  glm::vec3 at(float dist) {
    return origin + dir * dist;
  }
};

struct RayInfo {
  glm::vec3 pos;
  glm::vec3 normal;
  glm::vec3 color;
  float dist;
  int geometry_index_;
};

class Renderer {
public:

  Renderer();
  ~Renderer();

  void Init(TScreen *screen);

  void Update();

  void Clean();

  void SetLightRotation(float X, float Y, float Z);

  Camera camera_;
  TScreen *screen_;

  std::vector<Geometry*> geometries;
  std::vector<Light> lights_; //Later implemented

  glm::vec3 up = { 0.0f,1.0f,0.0 };
  glm::vec3 forward = { 0.0f,0.0f,-1.0f };
  glm::vec3 right = { 1.0f,0.0f,0.0f };
  glm::vec3 lower_left_corner;
  glm::vec3 horizontal;
  glm::vec3 vertical;

  glm::vec3 directional_dir_;
  float directional_inrtensity_;
  unsigned int light_samples_;
  float light_offset_;

  unsigned int num_threads_;
  unsigned int num_bounces_;

private:
  glm::vec3 ComputeLighting(RayInfo ray);

  void UpdateStep(int startrow, int endrow,int thread/*for tracing*/);
  
  RayInfo ComputeRay(Ray& ray, int depth);
  //For faster compute
  float LengthSquared(glm::vec3 v);
  float Rand();
  float RandRange(float min,float max);
  glm::vec3 RandVec(float lengthmin, float lengthmax);

  glm::vec3 RandSphere();

  px_sched::Scheduler schd;
  px_sched::Sync sync_obj;
  std::vector<glm::vec3> directional_dir_samples_;
};



#endif //__RENDERER_H__

