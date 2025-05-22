#include "Gamma/Domain.h"
#include "Gamma/Envelope.h"
#include "Gamma/Oscillator.h"
#include "Gamma/SamplePlayer.h"
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_VAO.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/math/al_Random.hpp"
#include "al/math/al_Vec.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_PresetSequencer.hpp"
#include "al/graphics/al_Light.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_Graphics.hpp"
#include "al_ext/assets3d/al_Asset.hpp"
#include <iostream>
#include <string>

// MY CUSTOM INCLUDES:
#include "softlight-sphere/eoys-mesh-fx/ripple.hpp"
#include "softlight-sphere/eoys-mesh-fx/scatter.hpp"
#include "softlight-sphere/eoys-mesh-fx/vfxUtility.hpp"
#include "softlight-sphere/attractors.hpp"

// Shader Program
al::ShaderProgram glowShader;

class MyApp : public al::App {
public:
    // Meshes and Effects
    al::VAOMesh mainMesh;
    Attractor mainAttractor;

    // Global Time
    double globalTime = 0;
    double sceneTime = 0;
    float pointSize = 5.0f; // Particle size

    void onInit() override {
        gam::sampleRate(audioIO().framesPerSecond());
    }

    void onCreate() override {
        nav().pos(al::Vec3d(0, 0, 20)); // Move the camera back for view

        // Initialize Mesh
        al::addSphere(mainMesh, 10.0f, 200, 200);
        mainMesh.primitive(al::Mesh::LINES);
        for (int i = 0; i < mainMesh.vertices().size(); ++i) {
            mainMesh.color(1.0, 0.6, 0.2, 0.4); // Orange particles with alpha transparency
        }
        mainMesh.update();
        
        // Compile Shader
        glowShader.compile(R"(
            #version 330 core
            in vec3 vPosition;
            in vec4 vColor;
            out vec4 fragColor;
            uniform float u_time;
            uniform vec2 u_resolution;

            float glow(float distance, float radius, float intensity) {
                return exp(-distance * distance * intensity) * radius;
            }

            void main() {
                vec2 uv = gl_FragCoord.xy / u_resolution;
                vec2 center = vec2(0.5, 0.5);
                float dist = length(uv - center);
                float intensity = glow(dist, 0.7, 10.0);
                fragColor = vColor * intensity;
                fragColor.a = intensity * vColor.a;
            }
        )", R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec4 aColor;
            out vec3 vPosition;
            out vec4 vColor;
            uniform mat4 u_model;
            uniform mat4 u_view;
            uniform mat4 u_projection;

            void main() {
                vPosition = aPos;
                vColor = aColor;
                gl_Position = u_projection * u_view * u_model * vec4(aPos, 1.0);
                gl_PointSize = 8.0; // Size of each particle
            }
        )");
        glowShader.link();
    }

    void onAnimate(double dt) override {
        globalTime += dt;
        sceneTime += dt;

        // Apply Attractor Effect
        mainAttractor.processThomas(mainMesh, sceneTime, 0.0001);
        mainMesh.update();
    }

    void onDraw(al::Graphics& g) override {
        g.clear(0.0, 0.0, 0.0, 1.0);
        g.depthTesting(true);
        g.blending(true);
        g.blendAdd(); // Additive blending for glowing effect

        // Use Custom Glow Shader
        glowShader.begin();
        glowShader.uniform("u_time", (float)globalTime);
        glowShader.uniform("u_resolution", al::Vec2f(width(), height()));
        g.pointSize(pointSize);
        g.meshColor();
        g.draw(mainMesh);
        glowShader.end();
    }

    void onSound(al::AudioIOData& io) override { mSequencer.render(io); }
    al::SynthSequencer& sequencer() { return mSequencer; }
    al::SynthSequencer mSequencer;
};

int main() {
    MyApp app;
    app.start();
    return 0;
}
