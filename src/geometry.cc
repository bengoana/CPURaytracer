/*---------------------------------------------------------------------
Copyright (c) 2020 Pablo Bengoa (bengoana)
https://github.com/bengoana

This software is released under the MIT license.

This program is a college project uploaded for showcase purposes.
---------------------------------------------------------------------*/

#include "geometry.h"
#include "renderer.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

bool Geometry::AABBIntersection(Ray ray) {
  float t1 = (vboxMin[0] - ray.origin[0]) / ray.dir[0];
  float t2 = (vboxMax[0] - ray.origin[0]) / ray.dir[0];

  float tmin = std::min(t1, t2);
  float tmax = std::max(t1, t2);

  for (int i = 1; i < 3; ++i) {
    t1 = (vboxMin[i] - ray.origin[i]) / ray.dir[i];
    t2 = (vboxMax[i] - ray.origin[i]) / ray.dir[i];

    tmin = std::max(tmin, std::min(t1, t2));
    tmax = std::min(tmax, std::max(t1, t2));
  }

  return tmax > std::max(tmin, 0.0f);
}

Sphere::Sphere() {
  radius_ = 1.0f;
  pos_ = glm::vec3(0.0f, 0.0f, 0.0f);
  color_ = glm::vec3(1.0f,0.0f,0.0f);
}


Sphere::~Sphere(){
}

float Sphere::ComputeRay(Ray& r){
  //if (!AABBIntersection(r)) return -1;

  glm::vec3 oc = r.origin - pos_;
  float a = glm::dot(r.dir, r.dir);
  float b = 2.0 * glm::dot(oc, r.dir);
  float c = glm::dot(oc, oc) - radius_ * radius_;
  float discriminant = b * b - 4.0f * a * c;
  if (discriminant < 0) {
    return -1.0;
  }
  else {
    return (-b - sqrt(discriminant)) / (2.0f * a);
  }
}

glm::vec3 Sphere::GetNormal(glm::vec3& collision_spot){
  return glm::normalize(collision_spot - pos_);
}

void Sphere::InitAABB(){
  vboxMax.x = pos_.x + radius_;
  vboxMax.y = pos_.y + radius_;
  vboxMax.z = pos_.z + radius_;

  vboxMin.x = pos_.x - radius_;
  vboxMin.y = pos_.y - radius_;
  vboxMin.z = pos_.z - radius_;
}

Plane::Plane(){
  normal_ = { 0.0f,1.0f,0.0f };
}

Plane::~Plane(){
}

float Plane::ComputeRay(Ray& ray){
  glm::vec3 p = { normal_.x,normal_.y ,normal_.z };
  float w = glm::length(pos_);
  return -(glm::dot(ray.origin, p) + w) / glm::dot(ray.dir, p);
}

glm::vec3 Plane::GetNormal(glm::vec3& collision_spot){

  return normal_;
}

CustomGeometry::CustomGeometry(){
  use_fast_normal_ = false;
}

CustomGeometry::~CustomGeometry(){
}

void CustomGeometry::LoadObj(const char* filePath){
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string error;

  bool result = tinyobj::LoadObj(shapes, materials, error, filePath);

  if (shapes.size() < 1) {
    printf("Error loading obj\n");
    return;
  }
  if (!error.empty()) {
    printf("[OBJLoader]: %s", error.c_str() + 6);;
  }

  //Only single shape obj supported
  vertices_.resize((int)(shapes[0].mesh.positions.size() / 3));

  glm::vec3 vmin = pos_;
  glm::vec3 vmax = pos_;

  for (int i = 0; i < vertices_.size(); i++) {
    vertices_[i].position[0] = shapes[0].mesh.positions[i * 3];
    vertices_[i].position[1] = shapes[0].mesh.positions[i * 3 + 1];
    vertices_[i].position[2] = shapes[0].mesh.positions[i * 3 + 2];

    vmin = glm::min(vmin, vertices_[i].position + pos_);
    vmax = glm::max(vmax, vertices_[i].position + pos_);

    if (shapes[0].mesh.normals.size() > 0) {
      vertices_[i].normal[0] = shapes[0].mesh.normals[i * 3];
      vertices_[i].normal[1] = shapes[0].mesh.normals[i * 3 + 1];
      vertices_[i].normal[2] = shapes[0].mesh.normals[i * 3 + 2];
    }
  }
  

  vboxMin = vmin;
  vboxMax = vmax;

  indices_.resize(shapes[0].mesh.indices.size());
  
  for (int i = 0; i < indices_.size(); ++i) {
    indices_[i] = shapes[0].mesh.indices[i];
  }

  shapes.clear();
  materials.clear();
}

float CustomGeometry::ComputeRay(Ray& ray){
  if (!AABBIntersection(ray)) return -1;


  float result = 999999.0f;
  for (int i = 0; i < indices_.size(); i += 3) {
    float distance = TriCollision(
    ray.origin,ray.dir,
      vertices_[indices_[i]].position + pos_,
      vertices_[indices_[i+1]].position + pos_,
      vertices_[indices_[i+2]].position + pos_);

    if (distance > -1 && distance < result) {
      result = distance;

      last_normal_ = glm::cross((vertices_[indices_[i]].position + pos_) - (vertices_[indices_[i + 1]].position + pos_),
      (vertices_[indices_[i]].position + pos_) - (vertices_[indices_[i + 2]].position + pos_));
      last_normal_ = glm::normalize(last_normal_);
      last_normal_ = { 1.0f,0.0f,0.0f };
    }
  }

  if (result >= 999997.0f) result = -1;

  return result;
}

glm::vec3 CustomGeometry::GetNormal(glm::vec3& collision_spot){
  
  if(use_fast_normal_)  
    return last_normal_;
  
  float dist_ = 99999.0f;
  int vertex_index_ = 0;
  float main_dot = -2.0;
  for (int i = 0; i < vertices_.size();++i) {
    float dot = glm::dot(collision_spot-(vertices_[i].position + pos_), vertices_[i].normal);
    if (dot > main_dot) {
      float temp = glm::distance(collision_spot, vertices_[i].position + pos_);
      if (dist_ > temp) {
        temp = dist_;
        main_dot = dot;
        vertex_index_ = i;
      }
    }
  }

  return vertices_[vertex_index_].normal;
}

inline float CustomGeometry::TriCollision(glm::vec3& ro, glm::vec3& rd, glm::vec3& v0, glm::vec3& v1, glm::vec3& v2){
  glm::vec3 v1v0 = v1 - v0;
  glm::vec3 v2v0 = v2 - v0;
  glm::vec3 rov0 = ro - v0;
  glm::vec3  n = cross(v1v0, v2v0);
  glm::vec3  q = cross(rov0, rd);
  float d = 1.0f / dot(rd, n);
  float u = d * dot(-q, v2v0);
  float v = d * dot(q, v1v0);
  float t = d * dot(-n, rov0);
  if (u < 0.0f || u>1.0f || v < 0.0f || (u + v)>1.0f) t = -1.0f;
  return t;
}