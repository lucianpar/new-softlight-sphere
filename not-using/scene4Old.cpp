#include "Gamma/Domain.h"
#include "Gamma/Envelope.h"
#include "Gamma/Oscillator.h"
#include "Gamma/SamplePlayer.h"
#include "al/app/al_App.hpp"
#include "al/graphics/al_Graphics.hpp"
#include "al/graphics/al_Light.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_VAO.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/io/al_ControlNav.hpp"
#include "al/math/al_Random.hpp"
#include "al/math/al_Vec.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_PresetSequencer.hpp"
#include "al_ext/assets3d/al_Asset.hpp"
#include <iostream>
#include <string>

// MY CUSTOM INCLUDES:
#include "softlight-sphere/eoys-mesh-fx/ripple.hpp"
#include "softlight-sphere/eoys-mesh-fx/scatter.hpp"
#include "softlight-sphere/eoys-mesh-fx/vfxUtility.hpp"
// #include "softlight-sphere/meshMorph.hpp"
#include "softlight-sphere/eoys-mesh-fx/scatter.hpp"
#include "softlight-sphere/eoys-mesh-fx/vfxMain.hpp"
#include "softlight-sphere/utility/loadAudioScene.hpp"
#include "softlight-sphere/utility/parseObj.hpp"
#include "softlight-sphere/utility/soundObject.hpp"
// #include "softlight-sphere/imageToMesh.hpp"
// #include "softlight-sphere/imageToSphere.hpp"
#include "softlight-sphere/utility/attractors.hpp"
#include "softlight-sphere/utility/imageColorToMesh.hpp"

/*
* sequence
* work on removing oldest mesh vertices
*update world color to flow into next scene
* add more mirrors
*figure out spacings

*/

void makePointLine(al::VAOMesh &mMesh, const al::Nav &head, float width,
                   al::Vec3f inputColor) {
  mMesh.vertex(head.pos() + head.ur() * width);
  mMesh.vertex(head.pos() + head.ur() * (width / 2));
  mMesh.vertex(head.pos() + head.ur() * (width / 4));
  mMesh.vertex(head.pos());
  mMesh.vertex(head.pos() - head.ur() * width);
  mMesh.vertex(head.pos() - head.ur() * (width / 2));
  mMesh.vertex(head.pos() - head.ur() * (width / 4));

  mMesh.color(inputColor.x, inputColor.y, inputColor.z, 0.7);
  mMesh.color(inputColor.x, inputColor.y, inputColor.z, 0.6);
  mMesh.color(inputColor.x, inputColor.y, inputColor.z, 0.5);
  mMesh.color(inputColor.x, inputColor.y, inputColor.z, 0.3);
  mMesh.color(inputColor.x, inputColor.y, inputColor.z, 0.7);
  mMesh.color(inputColor.x, inputColor.y, inputColor.z, 0.6);
  mMesh.color(inputColor.x, inputColor.y, inputColor.z, 0.5);

  // mMesh.update();
}

class MyApp : public al::App {
public:
  al::Nav head; // for main ribbon
  al::Nav reflectedHead;
  al::Vec3f target;
  al::Vec3f reflectedTarget;
  float t{0};
  float a{0};
  al::Parameter width{"Width", 0.05, 0, 0.2};

  // Meshes and Effects

  al::VAOMesh ribbon{al::Mesh::POINTS};
  al::VAOMesh reflectedRibbon{al::Mesh::TRIANGLES};
  al::VAOMesh newAttractor;

  al::Mesh test;
  al::VAOMesh referenceMesh;
  al::Vec3f color1{0.1, 0.3, 0.4};
  al::Vec3f color2{0.9, 0.9, 0.9};
  Attractor mainAttractor;
  float updatedSpeed = 1.0;

  al::Light light;
  // Light light;
  al::Material material;

  // Global Time
  double globalTime = 0;
  double sceneTime = 0;
  float pointSize = 1.0f; // Particle size

  void onInit() override { gam::sampleRate(audioIO().framesPerSecond()); }

  void onCreate() override {
    nav().pos(al::Vec3d(head.pos())); //

    al::addSphere(test);

    // Initialize Mesh
    mainAttractor.makeNoiseCube(newAttractor, 5.0, 2500);
    newAttractor.primitive(al::Mesh::LINE_LOOP);
    newAttractor.update();
    // std::cout << referenceMesh.vertices()[0] << referenceMesh.vertices()[1]
    //           << std::endl;
    // random generated values that produced good deterministic results
    // attempting to use the smallest number
    referenceMesh.vertex(0.532166, 3.68314, -2.96492);
    referenceMesh.vertex(-1.21797, -0.745106, 2.07905);

    // al::addSphere(referenceMesh, 10.0, 10.0);
    referenceMesh.primitive(al::Mesh::POINTS);

    head.moveF(1.0);
    head.spinF(0.3);
    // head
    // target.set(al::rnd::uniformS(), al::rnd::uniformS(),
    // al::rnd::uniformS());

    // makePointLine()

    ribbon.vertex(width, 0, 0);
    ribbon.normal(width, 0, 0);
    ribbon.vertex(0, 0, 0);
    ribbon.normal(0, 1, 0);
    ribbon.vertex(-width, 0, 0);
    ribbon.normal(-width, 0, 0);

    reflectedRibbon.vertex(width, 0, 0);
    reflectedRibbon.normal(width, 0, 0);
    reflectedRibbon.vertex(0, 0, 0);
    reflectedRibbon.normal(0, 1, 0);
    reflectedRibbon.vertex(-width, 0, 0);
    reflectedRibbon.normal(-width, 0, 0);

    ribbon.update();
    reflectedRibbon.update();
    referenceMesh.update();
  }

  void onAnimate(double dt) override {
    globalTime += dt;
    sceneTime += dt;

    /// trying newer effect
    // mainAttractor.processRossler(newAttractor, dt, 1.0);
    // mainAttractor.processLorenz(newAttractor, dt, 1.0);

    mainAttractor.processAizawa(newAttractor, dt, 1.0);
    newAttractor.update();

    // Apply Attractor Effect
    mainAttractor.processRossler(referenceMesh, dt, 1.0);
    target = referenceMesh.vertices()[0];
    reflectedTarget = target * al::Vec3f(-1.0, 1.0, 1.0);
    // mainAttractor.processChen(ribbon, dt, 1.0);

    head.faceToward(target, 1.0);
    reflectedHead = head;
    reflectedHead.faceToward(target, 1.0);
    reflectedHead.pos(head.pos() * al::Vec3f(-1, 1, 1)); // mirror across X

    // nav().pos(al::Vec3d(head.pos()));
    //  nav().faceToward(nav().pos() * -0.5);

    //
    makePointLine(ribbon, head, width, color1);
    // ribbon.vertex(head.pos() + head.ur() * width);
    // ribbon.vertex(head.pos());
    // ribbon.vertex(head.pos() - head.ur() * width);

    // ribbon.color(color1.x, color1.y, color1.z, 0.7);
    // ribbon.color(color1.x, color1.y, color1.z, 0.3);
    // ribbon.color(color1.x, color1.y, color1.z, 0.7);

    reflectedRibbon.vertex(reflectedHead.pos() + reflectedHead.ur() * width);
    reflectedRibbon.vertex(reflectedHead.pos());
    reflectedRibbon.vertex(reflectedHead.pos() - reflectedHead.ur() * width);
    reflectedRibbon.color(color1.x, color1.y, color1.z, 0.5);
    reflectedRibbon.color(color1.x, color1.y, color1.z, 0.05);
    reflectedRibbon.color(color1.x, color1.y, color1.z, 0.4);

    //     if ((int)(globalTime * 10) % 50 == 0) {
    //     addBranch(ribbon, head, width);
    // }

    if (true) {
      // sort of curved surface shading
      ribbon.normal(al::Vec3f(head.uu()).lerp(head.ur(), 0.1));
      ribbon.normal(head.uu());
      ribbon.normal(al::Vec3f(head.uu()).lerp(head.ur(), -0.1));
    } else {
      // flat shading....
      ribbon.normal(head.uu());
      ribbon.normal(head.uu());
      ribbon.normal(head.uu());
    }

    const int n = ribbon.vertices().size();
    if (n > 3) {
      ribbon.index(n - 1);
      ribbon.index(n - 2);
      ribbon.index(n - 4);
      ribbon.index(n - 2);
      ribbon.index(n - 5);
      ribbon.index(n - 4);
      ribbon.index(n - 2);
      ribbon.index(n - 3);
      ribbon.index(n - 5);
      ribbon.index(n - 3);
      ribbon.index(n - 6);
      ribbon.index(n - 5);

      reflectedRibbon.index(n - 1);
      reflectedRibbon.index(n - 2);
      reflectedRibbon.index(n - 4);
      reflectedRibbon.index(n - 2);
      reflectedRibbon.index(n - 5);
      reflectedRibbon.index(n - 4);
      reflectedRibbon.index(n - 2);
      reflectedRibbon.index(n - 3);
      reflectedRibbon.index(n - 5);
      reflectedRibbon.index(n - 3);
      reflectedRibbon.index(n - 6);
      reflectedRibbon.index(n - 5);
    }

    head.step(dt * updatedSpeed);
    if (sceneTime < 2.0) {
      // ribbon.scale(1.1);
      ribbon.scale(1.001);
      reflectedRibbon.scale(1.001);
    }
    // ribbon.scale(1.0001);
    ribbon.update();

    // reflectedRibbon.scale(1.000001);
    // reflectedHead.step(dt * updatedSpeed);
    // reflectedRibbon.scale(1.001);
    reflectedRibbon.update();

    referenceMesh.update();
  }

  void onDraw(al::Graphics &g) override {
    glEnable(GL_BLEND);
    // g.blendTrans();
    g.depthTesting(true);
    // g.clear(1.0, 1.0, 0.9, 1.0);
    // g.clear(sin(sceneTime));
    // g.clear(0.9);
    // g.clear(sin(sceneTime / 2.0));
    g.clear(0.0);

    // g.depthTesting(true);
    g.blending(true);
    glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      // g.blendTrans();
      g.depthTesting(true);


      g.lighting(true);
      // lighting from karl's example
      light.globalAmbient(al::RGB(1.0, 1.0, 1.0));
      light.ambient(al::RGB(1.0, 1.0, 1.0));
      light.diffuse(al::RGB(1, 1, 1.0));
      g.light(light);
      material.specular(1.0);
      material.shininess(128);
      g.material(material);
    // g.blendAdd(); // Additive blending for glowing effect
    // g.lighting(true);
    // // lighting from karl's example
    // light.globalAmbient(al::RGB(1.0, (1.0), 1.0));
    // light.ambient(al::RGB(1.0, (1.0), 1.0));
    // light.diffuse(al::RGB(1, 1, 1.0));
    // g.light(light);
    // material.specular(light.diffuse());
    // material.shininess(50);

    // g.material(material);

    // g.meshColor();
    // g.color(0.0);
    g.pointSize(pointSize);
    g.color(0.871, 0.467, 0.192, 0.1);
    g.draw(newAttractor);
    // g.draw(referenceMesh);
    // g.draw(ribbon);
    // g.draw(reflectedRibbon);
    //   glowShader.end();
  }

  void onSound(al::AudioIOData &io) override { mSequencer.render(io); }
  al::SynthSequencer &sequencer() { return mSequencer; }
  al::SynthSequencer mSequencer;
};

int main() {
  MyApp app;
  app.start();
  return 0;
}
