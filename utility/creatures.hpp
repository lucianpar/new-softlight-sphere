#ifndef CREATURES_HPP
#define CREATURES_HPP

#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/math/al_Complex.hpp"
#include "al/math/al_Constants.hpp"
#include "al/math/al_Vec.hpp"
class Creature {

public:
  void makeJellyfish(al::VAOMesh &mesh, float headRadius = 0.5f,
                     int headSlices = 30, int headStacks = 30, int numLegs = 12,
                     int segmentsPerLeg = 50, float amplitude = 0.15f,
                     float frequency = 5.0f,
                     float legRatio = 6.0f // 👈 scales legs relative to head
  ) {
    mesh.reset(); // clear geometry
    mesh.primitive(al::Mesh::POINTS);

    // Generate a temporary mesh for the sphere
    al::Mesh head;
    head.primitive(al::Mesh::POINTS);
    addSphere(head, 1.0f, headSlices,
              headStacks); // radius=1.0f, apply scale later

    // Find the topmost vertex (highest y)
    int topIndex = 0;
    float maxY = head.vertices()[0].y;
    for (int i = 1; i < head.vertices().size(); ++i) {
      if (head.vertices()[i].y > maxY) {
        maxY = head.vertices()[i].y;
        topIndex = i;
      }
    }

    // Move top vertex to index 0
    std::vector<al::Vec3f> reorderedVerts = head.vertices();
    std::swap(reorderedVerts[0], reorderedVerts[topIndex]);

    // Apply head scaling: radius and squish
    for (auto &v : reorderedVerts) {
      v.x *= headRadius;
      v.y *= headRadius * 0.7f;
      v.z *= headRadius * 0.9f;
    }

    // Add to final mesh with head color
    al::RGB headColor(0.8, 0.5, 1.0);
    for (const auto &v : reorderedVerts) {
      mesh.vertex(v);
      mesh.color(headColor);
    }

    // Compute leg length from ratio
    float legLength = headRadius * legRatio;

    // Add sinusoidal legs
    float angleStep = M_2PI / numLegs;
    for (int i = 0; i < numLegs; ++i) {
      float angle = i * angleStep;
      float xOrigin = std::cos(angle) * headRadius * 0.5f;
      float zOrigin = std::sin(angle) * headRadius * 0.5f;

      for (int j = 0; j < segmentsPerLeg; ++j) {
        float y = -j * (legLength / segmentsPerLeg);
        float offsetX = std::sin(j * frequency * 0.1f) * amplitude * headRadius;
        float offsetZ = std::cos(j * frequency * 0.1f) * amplitude * headRadius;

        float x = xOrigin + offsetX;
        float z = zOrigin + offsetZ;

        mesh.vertex(al::Vec3f(x, y, z));
        mesh.color(al::RGB(0.2, 0.4, 0.8)); // leg color
      }
    }

    mesh.update(); // finalize VAO
  }

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