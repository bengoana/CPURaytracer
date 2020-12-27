/*---------------------------------------------------------------------
Copyright (c) 2020 Pablo Bengoa (bengoana)
https://github.com/bengoana

This software is released under the MIT license.

This program is a college project uploaded for showcase purposes.
---------------------------------------------------------------------*/

#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__ 1

#include "glm/glm.hpp"

#include <vector>

struct Ray;

struct sVertex {
  glm::vec3 position;
  glm::vec3 normal;
};

class Geometry {
public:
  Geometry() {}
  ~Geometry() {}

  virtual float ComputeRay(Ray& ray) = 0;
  virtual glm::vec3 GetNormal(glm::vec3& collision_spot) = 0;

  glm::vec3 pos_;
  glm::vec3 color_;
  float diffuse_;
  float specular_;

protected:
  glm::vec3 vboxMin;
  glm::vec3 vboxMax;
  bool AABBIntersection(Ray ray);
};

class Sphere : public Geometry {
public:
  Sphere();
  ~Sphere();

  float ComputeRay(Ray& ray) override;

  glm::vec3 GetNormal(glm::vec3& collision_spot) override;

  void InitAABB();

  float radius_;
};

class Plane : public Geometry {
public:
  Plane();
  ~Plane();

  float ComputeRay(Ray& ray) override;
  glm::vec3 GetNormal(glm::vec3& collision_spot) override;

  glm::vec3 normal_;
};

class CustomGeometry : public Geometry {
public:
  CustomGeometry();
  ~CustomGeometry();

  void LoadObj(const char* filePath);

  float ComputeRay(Ray& ray) override;
  glm::vec3 GetNormal(glm::vec3& collision_spot) override;

  std::vector<sVertex> vertices_;
  std::vector<unsigned short> indices_;
  glm::vec3 last_normal_;
  bool use_fast_normal_;

private:


  float inline TriCollision(glm::vec3& ro, glm::vec3& rd, glm::vec3& v0, glm::vec3& v1, glm::vec3& v2);
};



#endif //__GEOMETRY_H__
