//
// Created by xflajs00 on 23.10.2021.
//

#include "PerlinNoiseGenerator.h"
#include <ctime>
#include <glm/geometric.hpp>
#include <glm/vec2.hpp>
#include <utils/Random.h>

float hash(float n) { return glm::fract(sin(n) * 1e4); }
float hash(glm::vec2 p) { return glm::fract(1e4 * sin(17.0 * p.x + p.y * 0.1) * (0.1 + glm::abs(sin(p.y * 13.0 + p.x)))); }



pf::mc::PerlinNoiseGenerator::PerlinNoiseGenerator() : PerlinNoiseGenerator(generateSeed()) {
}


pf::mc::PerlinNoiseGenerator::PerlinNoiseGenerator(double seed) : seed(seed) {
}

void pf::mc::PerlinNoiseGenerator::setSeed(double seed) {
  PerlinNoiseGenerator::seed = seed;
}
float pnoise(glm::vec3 x) {
  const glm::vec3 step = glm::vec3(110, 241, 171);

  glm::vec3 i = glm::floor(x);
  glm::vec3 f = glm::fract(x);

  float n = glm::dot(i, step);

  glm::vec3 u = f * f * (3.0f - 2.0f * f);
  return glm::mix(glm::mix(glm::mix(hash(n + glm::dot(step, glm::vec3(0, 0, 0))), hash(n + glm::dot(step, glm::vec3(1, 0, 0))), u.x),
                           glm::mix(hash(n + glm::dot(step, glm::vec3(0, 1, 0))), hash(n + glm::dot(step, glm::vec3(1, 1, 0))), u.x), u.y),
                  glm::mix(glm::mix(hash(n + glm::dot(step, glm::vec3(0, 0, 1))), hash(n + glm::dot(step, glm::vec3(1, 0, 1))), u.x),
                           glm::mix(hash(n + glm::dot(step, glm::vec3(0, 1, 1))), hash(n + glm::dot(step, glm::vec3(1, 1, 1))), u.x), u.y),
                  u.z);
}
double pf::mc::PerlinNoiseGenerator::noise(glm::vec3 coord) const {
  constexpr auto div = 50.0;
  coord.x += seed;
  coord.z -= seed * 5;
  coord.y += 40;
  return -coord.y + (pnoise(coord / 10.f) * 10.f + pnoise(coord / 20.f) * 20.f + pnoise(coord / 40.f) * 40.f + pnoise(coord / 80.f) * 80.f);
  //return generator.accumulatedOctaveNoise3D(coord.x / div, coord.y / div, coord.z / div, 5);
}
