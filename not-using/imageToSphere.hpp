#include "al/graphics/al_Graphics.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_Texture.hpp"
#include "al/graphics/al_Image.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/io/al_File.hpp"
#include "al/ui/al_Parameter.hpp"

struct ImageSphereLoader {
    al::Texture tex;
    al::ParameterBool imageShow{"imageShow", "", true};
    al::Parameter sphereRadius = { "sphereRadius", "", 3.f, 0.f,10.f}; 
    al::Parameter pointSize = {"pointSize", "", 1.0f, 0.f, 10.f};

    void addCustomSphere(al::VAOMesh& m, float radius, int maxVertices, const al::Image& image) {
        m.primitive(al::Mesh::POINTS);
        m.reset(); // Clear any existing vertices

        // Fibonacci sphere for evenly distributed vertices - CHAT GPT FOR THIS MATH
        for (int i = 0; i < maxVertices; ++i) {
            float phi = M_PI * (3.0 - sqrt(5.0)); // Golden angle in radians
            float y = 1.0 - (i / float(maxVertices - 1)) * 2.0; // y ranges from 1 to -1
            float radiusAtY = sqrt(1.0 - y * y); // Radius at given y

            float theta = phi * i;

            // Convert spherical to Cartesian coordinates
            float x = cos(theta) * radiusAtY;
            float z = sin(theta) * radiusAtY;

            // Scale to the sphere radius
            m.vertex(radius * x, radius * y, radius * z);
            m.normal(x, y, z);

            // Map image pixel to sphere point (polar projection)
            int imgX = (theta / (2 * M_PI)) * image.width();
            int imgY = ((y + 1.0f) / 2.0f) * image.height();
            imgX = std::min(std::max(0, imgX), static_cast<int>(image.width() - 1));
            imgY = std::min(std::max(0, imgY), static_cast<int>(image.height() - 1));

            auto pixel = image.at(imgX, imgY);
            float r = pixel.r / 255.0f;
            float g = pixel.g / 255.0f;
            float b = pixel.b / 255.0f;
            m.color(r, g, b);
        }

        printf("Sphere generated with exactly %d vertices\n", maxVertices);
    }

    void loadImage(std::string imagePath, al::VAOMesh& mMesh, int maxVertices) {
        // Load texture (as image)
        al::Image image;
        if (!image.load(imagePath)) {
            printf("Failed to load texture from %s\n", imagePath.c_str());
            return;
        }

        // Generate sphere with color mapping from the image
        addCustomSphere(mMesh, sphereRadius, maxVertices, image);
        
        // Center the mesh
        al::Vec3f center(0.0f, 0.0f, 0.0f);
        for (auto& vertex : mMesh.vertices()) {
            center += vertex;
        }
        center /= mMesh.vertices().size();
        for (auto& vertex : mMesh.vertices()) {
            vertex -= center;
        }

        mMesh.update();
    }

    void transferColors(al::VAOMesh& sourceMesh, al::VAOMesh& targetMesh) {
    // Ensure both meshes have the same number of vertices
    if (sourceMesh.colors().size() != targetMesh.vertices().size()) {
        printf("Error: Source and target meshes have different vertex counts!\n");
        return;
    }

    // Ensure target mesh has a color buffer
    if (targetMesh.colors().size() != targetMesh.vertices().size()) {
        targetMesh.colors().resize(targetMesh.vertices().size());
    }

    // Copy colors from source to target
    for (int i = 0; i < sourceMesh.colors().size(); ++i) {
        targetMesh.colors()[i] = sourceMesh.colors()[i];
    }

    // Update target mesh to apply changes
    targetMesh.update();
    printf("Colors transferred from source to target mesh.\n");
}

    void draw(al::Graphics &g, al::VAOMesh& mMesh) {
        if (!imageShow) return;

        al::gl::depthTesting(true);
        g.pushMatrix();
        g.pointSize(pointSize);
        g.draw(mMesh);
        g.popMatrix();
    }
};
