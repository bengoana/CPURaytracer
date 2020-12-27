/*---------------------------------------------------------------------
Copyright (c) 2020 Pablo Bengoa (bengoana)
https://github.com/bengoana

This software is released under the MIT license.

This program is a college project uploaded for showcase purposes.
---------------------------------------------------------------------*/


#include <stdio.h>

#include <SDL.h>
#include "SDL_timer.h"
#include "renderer.h"
#include "geometry.h"

#define PX_SCHED_IMPLEMENTATION
#include "px_sched.h"

#include <Windows.h>

float g_scale_ = 0.5f;

struct sState {
  Renderer renderer_;
  Sphere sphere1_;
  Sphere sphere2_;
  Sphere sphere3_;
  Sphere sphere4_;
  Plane floor_;
  CustomGeometry cube_;
  CustomGeometry cube2_;
  CustomGeometry teapot_;
  Light light1_;
  float z_angle_;

  bool config_mode = true;
  bool light_rotation = true;
} state;

void Prepare() {
  state.renderer_.camera_.pos = {0.0f,0.0f,0.0f};
  state.renderer_.camera_.focal_length = 1.0f;
  state.renderer_.camera_.u = 1.77f;
  state.renderer_.camera_.v = 1.0f;

  state.renderer_.light_offset_ = 0.01f;
  state.renderer_.light_samples_ = 2;

 state.sphere1_.pos_ = glm::vec3(0.0f, 0.0f, -20.0f);
 state.sphere1_.color_ = glm::vec3(1.0f,0.32f,0.36f);
 state.sphere1_.radius_ = 4.0f;
 state.sphere1_.diffuse_ = 0.0f;
 state.sphere1_.specular_ = 1.0f;
 state.sphere1_.InitAABB();

 state.sphere2_.pos_ = glm::vec3(5.0f, -1.0f, -15.0f);
 state.sphere2_.color_ = glm::vec3(0.9f, 0.76f, 0.46f);
 state.sphere2_.radius_ = 2.0f;
 state.sphere2_.diffuse_ = 0.0f;
 state.sphere2_.specular_ = 1.0f;
 state.sphere2_.InitAABB();

 state.sphere3_.pos_ = glm::vec3(5.0f, 0.0f, -25.0f);
 state.sphere3_.color_ = glm::vec3(0.65f, 0.77f, 0.97f);
 state.sphere3_.radius_ = 3.0f;
 state.sphere3_.diffuse_ = 0.0f;
 state.sphere3_.specular_ = 1.0f;
 state.sphere3_.InitAABB();

 state.sphere4_.pos_ = glm::vec3(-5.5f,0.0f,-15.0f);
 state.sphere4_.color_ = glm::vec3(0.90f, 0.90f, 0.90f);
 state.sphere4_.radius_ = 3.0f;
 state.sphere4_.diffuse_ = 1.0f;
 state.sphere4_.specular_ = 0.0f;
 state.sphere4_.InitAABB();

 state.floor_.pos_ = glm::vec3(0.0f, -10.0f, 0.0f);
 state.floor_.color_ = glm::vec3(1.0f, 0.0f, 0.0f);
 state.floor_.diffuse_ = 1.0f;
 state.floor_.specular_ = 1.0f;

 state.cube_.pos_ = { 3.0f,0.0f,-5.0f };
 state.cube_.color_ = { 1.0f,1.0f,0.0f };
 state.cube_.diffuse_ = 0.0f;
 state.cube_.specular_ = 1.0f;
 state.cube_.use_fast_normal_ = false;
 state.cube_.LoadObj("../../data/cube.obj");

 state.cube2_.pos_ = { -5.0f,0.0f,-10.0f };
 state.cube2_.color_ = { 0.0f,1.0f,0.0f };
 state.cube2_.diffuse_ = 1.0f;
 state.cube2_.specular_ = 1.0f;
 state.cube2_.use_fast_normal_ = false;
 state.cube2_.LoadObj("../../data/cube.obj");

 state.teapot_.pos_ = { 2.0f,1.0f,-20.0f };
 state.teapot_.color_ = { 0.6f,0.2f,0.4f };
 state.teapot_.diffuse_ = 1.0f;
 state.teapot_.specular_ = 0.0f;
 state.teapot_.use_fast_normal_ = false;
 state.teapot_.LoadObj("../../data/teapot.obj");

 state.renderer_.geometries.push_back(&state.sphere1_);
 state.renderer_.geometries.push_back(&state.sphere2_);
 state.renderer_.geometries.push_back(&state.sphere3_);
 state.renderer_.geometries.push_back(&state.sphere4_);
 state.renderer_.geometries.push_back(&state.floor_);

 state.light1_.pos = glm::vec3(1.0f, -1.0f, -4.0f);
 state.light1_.intensity = 1.0f;
 state.light1_.color = { 0.5f,0.0f,0.0f };

 state.renderer_.lights_.push_back(state.light1_);
 state.z_angle_ = 0.0f;
}

static inline unsigned int BlendARGB(unsigned int pix0, unsigned int pix1, int alphafx8)
{
  // Version rapida. Trabajamos en pares de componentes, operando varios
  // bitfields a la vez
  unsigned int ag0 = (pix0 & 0xff00ff00) >> 8;
  unsigned int rb0 = pix0 & 0x00ff00ff;
  unsigned int ag1 = (pix1 & 0xff00ff00) >> 8;
  unsigned int rb1 = pix1 & 0x00ff00ff;

  unsigned int ag = ((ag0 * alphafx8 + ag1 * (0x100 - alphafx8)) >> 8) & 0xff00ff;
  unsigned int rb = ((rb0 * alphafx8 + rb1 * (0x100 - alphafx8)) >> 8) & 0xff00ff;

  return ((ag << 8) | rb);
}

static inline unsigned int BilinearSampling(unsigned int pix_h0v0,
  unsigned int pix_h1v0,
  unsigned int pix_h0v1,
  unsigned int pix_h1v1,
  int alpha_hor_fx8,
  int alpha_ver_fx8)
{
  // La funcion muestrado (sampling) bilinear
  // Devuelve una muestra ponderada entre los 4 texels vecinos a la coordenada dada
  // Blend de los pixels en vertical
  unsigned int s0 = BlendARGB(pix_h0v0, pix_h0v1, alpha_ver_fx8);
  unsigned int s1 = BlendARGB(pix_h1v0, pix_h1v1, alpha_ver_fx8);
  // Blend horizontal de los dos anteriores
  unsigned int s = BlendARGB(s0, s1, alpha_hor_fx8);
  return s;
}

void Scale(SDL_Surface *src, SDL_Surface *dst) {
  
  float stepy = 0;

  for (int y = 0; y < dst->h; ++y) {
    unsigned int* dst_line = (unsigned int*)dst->pixels + (y * (dst->pitch >> 2));
  
    unsigned int* src_line0 = (unsigned int*)src->pixels + (int)stepy * (src->pitch >> 2);
    unsigned int* src_line1 = (unsigned int*)src->pixels + ((int)stepy+1) * (src->pitch >> 2);

    float stepx = 0;

    unsigned int factor = (unsigned int)(g_scale_ * 256.0f);

    for (int x = 0; x < dst->w; ++x) {
      dst_line[x] = BilinearSampling(
        src_line0[(int)stepx],
        src_line0[(int)(stepx + 1) % src->w],
        src_line1[(int)stepx],
        src_line1[(int)(stepx + 1) % src->w],
        factor,
        factor);

      stepx += g_scale_;
    }
  
    stepy += g_scale_;
  }


}

void Config(unsigned int time) {
  const char* string = R"STR(
  ----------------CONFIG MODE-------------------
    
  Config mode increases delta time considerably

  Press C to disable/enable config mode

  Controls:
    - Camera left: A
    - Camera right: D
    - Camera up: W
    - Camera down: S

    - Enable/Disable light rotation: Q
    - Cycle Threads used (number of total tasks launched): T

    - Load Base scene: B
    - Load Heavy Scene With Objs: N
    
    - Enable Upscaling render optimisation: U

  )STR";
  if (state.config_mode) {
    system("cls");
    printf(string);
    printf("Current max number of processing tasks: %d\n", state.renderer_.num_threads_);
  }
  
  printf("Delta time: %d ms \n", SDL_GetTicks() - time);
}

int main(int argc, char** argv) {

  SDL_Surface* g_SDLSrf;
  SDL_Surface* g_LowScale;
  int req_w = 1280;
  int req_h = 720;


  //Initialize all the systems of SDL
  SDL_Init(SDL_INIT_EVERYTHING);

  //Create a window with a title, "Getting Started", in the centre
  //(or undefined x and y positions), with dimensions of 800 px width
  //and 600 px height and force it to be shown on screen
  SDL_Window* window = SDL_CreateWindow("RayTracer", SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED, req_w, req_h, SDL_WINDOW_SHOWN);

  //Create a renderer for the window created above, with the first display driver present
  //and with no additional settings
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

  //Set the draw color of renderer to green
  SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

  //Clear the renderer with the draw color
  SDL_RenderClear(renderer);

  //Update the renderer which will show the renderer cleared by the draw color which is green
  int end = 0;

  // if all this hex scares you, check out SDL_PixelFormatEnumToMasks()!
  g_SDLSrf = SDL_CreateRGBSurface(0, req_w, req_h, 32,
    0x00FF0000,
    0x0000FF00,
    0x000000FF,
    0xFF000000);

  g_LowScale = SDL_CreateRGBSurface(0, req_w * g_scale_, req_h * g_scale_, 32,
    0x00FF0000,
    0x0000FF00,
    0x000000FF,
    0xFF000000);

  SDL_Texture* sdlTexture = SDL_CreateTexture(renderer,
    SDL_PIXELFORMAT_ARGB8888,
    SDL_TEXTUREACCESS_STREAMING,
    req_w, req_h);

  TScreen screen;
  screen.pixels = (unsigned int*)g_LowScale->pixels;
  screen.width = g_LowScale->w;
  screen.height = g_LowScale->h;
  screen.stride = g_LowScale->pitch >> 2; // >> 2 if pixels are int

  Prepare();
  state.renderer_.Init(&screen);

  unsigned int time = SDL_GetTicks();
  while (!end) {
    SDL_Event event;

    SDL_LockSurface(g_SDLSrf);
    SDL_LockSurface(g_LowScale);
    state.renderer_.Update();

    if (g_scale_ <= 0.5f) {
      Scale(g_LowScale, g_SDLSrf);
    }

    SDL_UnlockSurface(g_LowScale);
    SDL_UnlockSurface(g_SDLSrf);

    SDL_UpdateTexture(sdlTexture, NULL, g_SDLSrf->pixels, g_SDLSrf->pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);

    SDL_RenderPresent(renderer);
  


    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_KEYDOWN:
        if(event.key.keysym.sym == SDLK_c)
          state.config_mode = !state.config_mode;
        if (event.key.keysym.sym == SDLK_w)
          state.renderer_.camera_.pos.y += 0.3f;
        if (event.key.keysym.sym == SDLK_a)
          state.renderer_.camera_.pos.x -= 0.3f;
        if (event.key.keysym.sym == SDLK_s)
          state.renderer_.camera_.pos.y -= 0.3f;
        if (event.key.keysym.sym == SDLK_d)
          state.renderer_.camera_.pos.x += 0.3f;
        if (event.key.keysym.sym == SDLK_q)
          state.light_rotation = !state.light_rotation;
        if (event.key.keysym.sym == SDLK_t) {
          state.renderer_.num_threads_ = (state.renderer_.num_threads_ + 10);
          if (state.renderer_.num_threads_ >= 100) state.renderer_.num_threads_ = 1;
        }
        if (event.key.keysym.sym == SDLK_b) {
          state.renderer_.geometries.clear();
          state.renderer_.geometries.push_back(&state.sphere1_);
          state.renderer_.geometries.push_back(&state.sphere2_);
          state.renderer_.geometries.push_back(&state.sphere3_);
          state.renderer_.geometries.push_back(&state.sphere4_);
          state.renderer_.geometries.push_back(&state.floor_);
        }
        if (event.key.keysym.sym == SDLK_n) {
          state.renderer_.geometries.clear();
          state.renderer_.geometries.push_back(&state.cube_);
          state.renderer_.geometries.push_back(&state.cube2_);
          state.renderer_.geometries.push_back(&state.teapot_);
          state.renderer_.geometries.push_back(&state.floor_);
        }
        if (event.key.keysym.sym == SDLK_u) {
          if (g_scale_ == 0.5f) {
            screen.pixels = (unsigned int*)g_SDLSrf->pixels;
            screen.width = g_SDLSrf->w;
            screen.height = g_SDLSrf->h;
            screen.stride = g_SDLSrf->pitch >> 2; // >> 2 if pixels are int
            g_scale_ = 1.0f;
          } else {
            screen.pixels = (unsigned int*)g_LowScale->pixels;
            screen.width = g_LowScale->w;
            screen.height = g_LowScale->h;
            screen.stride = g_LowScale->pitch >> 2; // >> 2 if pixels are int
            g_scale_ = 0.5f;
          }
        }
        break;
      case SDL_QUIT:
        end = 1;
        break;
      }
    }

    if (state.light_rotation) {
      state.z_angle_ += 0.05f * (SDL_GetTicks() - time) * 0.01f;
      state.renderer_.SetLightRotation(-0.4f, state.z_angle_, 0.0f);
    }
    Config(time);
    time = SDL_GetTicks();

  }
  

  //Pause for 3 seconds (or 3000 milliseconds)
  //SDL_Delay(3000);

  state.renderer_.Clean();

  //Destroy the renderer created above
  SDL_DestroyRenderer(renderer);

  //Destroy the window created above
  SDL_DestroyWindow(window);

  //Close all the systems of SDL initialized at the top
  SDL_Quit();

  return 0;
}

