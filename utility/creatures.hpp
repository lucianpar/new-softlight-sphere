#ifndef CREATURES_HPP
#define CREATURES_HPP

#include "al/graphics/al_VAOMesh.hpp"
#include "al/math/al_Complex.hpp"
#include "al/math/al_Vec.hpp"

class Creature {

public:
  void addStarfish(al::Mesh &m, int arms = 5, float armLength = 1.0f,
                   float armWidth = 0.1f) {
    m.primitive(al::Mesh::TRIANGLES_ADJACENCY);
    m.reset();

    al::Vec3f center(0, 0, 0);

    for (int i = 0; i < arms; ++i) {
      float angle = i * (2 * M_PI / arms);
      float nextAngle = angle + M_PI / arms; // for width offset

      // Tip direction
      al::Vec3f dir(cos(angle), sin(angle), 0);
      al::Vec3f perp(-dir.y, dir.x, 0); // perpendicular for width

      // Arm tip vertex
      al::Vec3f tip = dir * armLength;
      // Base left and right for width
      al::Vec3f baseLeft = center + perp * armWidth * 0.5f;
      al::Vec3f baseRight = center - perp * armWidth * 0.5f;

      // Triangle: baseLeft -> tip -> baseRight
      m.vertex(baseLeft);
      m.vertex(tip);
      m.vertex(baseRight);

      // Color per arm (optional gradient)
      float hue = float(i) / arms;
      al::Color c = al::HSV(hue, 0.8, 0.9);
      m.color(c);
      m.color(c);
      m.color(c);
    }
  }
};

#endif