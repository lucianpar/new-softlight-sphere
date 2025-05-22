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
//#include "softlight-sphere/meshMorph.hpp"
#include "softlight-sphere/parseObj.hpp"
#include "softlight-sphere/loadAudioScene.hpp"
#include "softlight-sphere/soundObject.hpp"
#include "softlight-sphere/loadAudioScene.hpp"
#include "softlight-sphere/eoys-mesh-fx/vfxMain.hpp"
#include "softlight-sphere/eoys-mesh-fx/scatter.hpp"
//#include "softlight-sphere/imageToMesh.hpp"
//#include "softlight-sphere/imageToSphere.hpp"
#include "softlight-sphere/attractors.hpp"
#include "softlight-sphere/imageColorToMesh.hpp"



/* scene 1 to do 
adjust sequencing 
put into container 
fix shader???? - ask karl
fix ripple speed and scaling
*/

// Shader Program
al::ShaderProgram glowShader;

int sceneIndex = 1;

class MyApp : public al::App {
public:
    // Meshes and Effects
    al::VAOMesh attractorMesh;
    al::VAOMesh bodyMesh;
    objParser newObjParser;
    Attractor mainAttractor;
    VertexEffectChain mainEffectChain;
  RippleEffect mainRippleY;
  RippleEffect mainRippleX;
  RippleEffect mainRippleZ;


  VertexEffectChain bodyEffectChain;;
  ScatterEffect bodyScatter;

    // Global Time
    double globalTime = 0;
    double sceneTime = 0;
    float pointSize = 5.0f; // Particle size


    ////DECLARE VALUES FOR EVENT TIMES////

   //SCENE 1 
    float rippleAmplitudeScene1 = 0.0;
    float rippleSpeedXScene1 = 0.2;
    float rippleSpeedYScene1 = 0.2;
    float rippleSpeedZScene1 = 0.3;
    float attractorSpeedScene1;
    float startingBodyAlpha = 0.00001;

    float shellTurnsWhiteEvent = 15.0f;
  
    float particlesAppearEvent = 15.0f;

    float particlesSlowRippleEvent = 10.0f;

    float rippleSpeedUpEvent = 30.0f;

    float stopSpeedUpEvent = 62.0f;

    float startTurnShellBlack = 62.0f;

    float stopTurnShellBlack = 64.0f;

    float bodyCloudAppear = 65.0f;

    float bodyCloudMoveOut = 70.0f;

    //float startAttractor = 70.0f;

  

    float moveInEvent = 85.0;

    


    void onInit() override {
        gam::sampleRate(audioIO().framesPerSecond());
    }

    void onCreate() override {
        nav().pos(al::Vec3d(0, 0, 0)); 
        sequencer().playSequence();

        //initialize body 
        newObjParser.parse("/Users/lucian/Desktop/201B/allolib_playground/softlight-sphere/assets/BaseMesh.obj", bodyMesh);
        bodyMesh.translate(0,3.5,-4);
        for (int i = 0; i < bodyMesh.vertices().size(); ++i) {
            bodyMesh.color(1.0, 0.6, 0.3, startingBodyAlpha); // Orange particles with alpha transparency
        }
        bodyMesh.primitive(al::Mesh::POINTS);
        // bodyMesh.update();

        // Initialize attractor
        al::addSphere(attractorMesh, 10.0f, 100, 100);
        attractorMesh.primitive(al::Mesh::LINES); //switch back to lines
        for (int i = 0; i < attractorMesh.vertices().size(); ++i) {
            attractorMesh.color(1.0, 0.6, 0.2, 0.4); // Orange particles with alpha transparency
        }
        std::cout << "made attractor sphere with vertices # : " << attractorMesh.vertices().size() << std::endl;
        // attractorMesh.update();



        //effect setting

        bodyScatter.setBaseMesh(bodyMesh.vertices());
        bodyScatter.setParams(0.5, 20.0);
        bodyScatter.setScatterVector(bodyMesh);
        mainRippleX.setParams(4, 0.2, 4.0, 'y');
        mainRippleX.setParams(4, 0.2, 6.0, 'x');
        mainRippleX.setParams(4, 0.2, 5.0, 'z');

        mainEffectChain.pushBack(&mainRippleX);
        mainEffectChain.pushBack(&mainRippleY);
        mainEffectChain.pushBack(&mainRippleZ);

        bodyEffectChain.pushBack(&bodyScatter);

        bodyScatter.triggerOut(true, bodyMesh);

         bodyMesh.update();

         attractorMesh.update();



        
        // Compile Shader


        //shader currently not working
    glowShader.compile(R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec4 aColor;
    
    out vec4 vColor;
    out vec2 fragUV;
    
    uniform highp mat4 u_model;
    uniform highp mat4 u_view;
    uniform highp mat4 u_projection;

    void main() {
        vColor = aColor;
        gl_Position = u_projection * u_view * u_model * vec4(aPos, 1.0);
        fragUV = (gl_Position.xy / gl_Position.w) * 0.5 + 0.5; 
        gl_PointSize = 12.0;
    }
)", 

R"(
    #version 330 core
    in vec4 vColor;
    in vec2 fragUV;
    out vec4 fragColor;
    
    uniform highp float u_time;
    uniform highp vec2 u_resolution;
    uniform highp vec2 u_center;

    void main() {
        // Ensure the uniform is actively used
        float dist = length(fragUV - u_center);
        float glowIntensity = exp(-dist * dist * 10.0); 

        // Apply glow effect
        fragColor = vColor * glowIntensity;
        fragColor.a = vColor.a * glowIntensity;
    }
)");
glowShader.link();


    }
    float bodyAlphaIncScene1 = 0.0;
    void onAnimate(double dt) override {
        globalTime += dt;
        sceneTime += dt;
        //std::cout << sceneTime << std::endl;

  
        

        // Apply Attractor Effect
        if (sceneTime < particlesSlowRippleEvent){
        mainAttractor.processThomas(attractorMesh, sceneTime, 0);
        }

        if (sceneTime >= particlesSlowRippleEvent && sceneTime <= rippleSpeedUpEvent) {
          attractorSpeedScene1 = 0.00005;
          mainAttractor.processThomas(attractorMesh, sceneTime, attractorSpeedScene1);
        }

         if (sceneTime>=rippleSpeedUpEvent && sceneTime <= stopSpeedUpEvent){
          attractorSpeedScene1 = 0.00015;
          mainAttractor.processThomas(attractorMesh, sceneTime, attractorSpeedScene1);
         }
          if (sceneTime>=stopSpeedUpEvent && sceneTime <= moveInEvent){
          attractorSpeedScene1 = 0.00001;
          mainAttractor.processThomas(attractorMesh, sceneTime, attractorSpeedScene1);
          

         }

         if (sceneTime>=stopSpeedUpEvent){
          attractorSpeedScene1 = 0.000001;
          mainAttractor.processThomas(attractorMesh, sceneTime, attractorSpeedScene1);
          mainEffectChain.process(attractorMesh, sceneTime);
         }



        attractorMesh.update();
        //body mesh effects
      if (sceneTime >=bodyCloudAppear) {
        if (bodyAlphaIncScene1 < 1.0){
        bodyAlphaIncScene1 +=0.0001;
        for (int i = 0; i < bodyMesh.vertices().size(); ++i) {
              al::Color currentColor = bodyMesh.colors()[i];
              currentColor.a = startingBodyAlpha + bodyAlphaIncScene1;  // Increment alpha only
              bodyMesh.colors()[i] = currentColor;
              } 
          // bodyMesh.update();
            }
          }
      if (sceneTime >= moveInEvent){
        attractorSpeedScene1 = 0.0000001;
          mainAttractor.processThomas(attractorMesh, sceneTime, attractorSpeedScene1);
        bodyScatter.setParams(5.0, 20.0);
        bodyScatter.triggerIn(true);
        attractorMesh.scale(0.9995);
        attractorMesh.translate(0,(3.5)*((sceneTime-moveInEvent)/(moveInEvent - 118)),(-4)*((sceneTime-moveInEvent)/(moveInEvent - 118)));
        }




      bodyEffectChain.process(bodyMesh, sceneTime);
      bodyMesh.update();







        
    }

     float shellIncrementScene1;
  float particleIncrementScene1;

    void onDraw(al::Graphics& g) override {

      if (sceneTime < shellTurnsWhiteEvent) {
    shellIncrementScene1 = ((sceneTime) / (shellTurnsWhiteEvent));
    g.clear(1.0-shellIncrementScene1);
    }
    if (sceneTime >= shellTurnsWhiteEvent){
    g.clear(0.0);
    }
    g.depthTesting(true);
    g.blending(true);
        g.blendAdd(); // Additive blending for glowing effect

        // Use Custom Glow Shader
        glowShader.begin();
        glowShader.uniform("u_time", (float)sceneTime);
    glowShader.uniform("u_resolution", al::Vec2f(width(), height()));
    glowShader.uniform("u_center", al::Vec3f(0.0, 0.0, 0.0)); // Center of the glow effect
        g.pointSize(pointSize);
        g.meshColor();
        g.draw(attractorMesh);
       
    if (sceneTime >= bodyCloudAppear){
    g.draw(bodyMesh);
    }
        glowShader.end();
    }

    void onSound(al::AudioIOData& io) override { mSequencer.render(io); }
    al::SynthSequencer& sequencer() { return mSequencer; }
    al::SynthSequencer mSequencer;
};

int main() {
    MyApp app;

    bool soundObjectVisual = false; 
  AudioLoader audioLoader;
  std::vector<std::vector<std::string>> songFiles(5); //vector for each song scene . //NOTE -- VEC IS 0 INDEX, BUT SONG FOLDERS ARE 1 INDEX FOR CLARITY
  audioLoader.loadSceneAudio(songFiles, 1);
  audioLoader.loadSceneAudio(songFiles, 2);
  //audioLoader.loadSceneAudio(songFiles, 2);
  //audioLoader.loadSceneAudio(songFiles, 3);
  //audioLoader.loadSceneAudio(songFiles, 4);
  //audioLoader.loadSceneAudio(songFiles, 5);
  //audioLoader.loadSceneAudio(songFiles, 6);

   double g = 0.7;
  float a = 1.4;
  float b = 1.6;
  float c = 1.0;
  float d = 0.7;

  if (sceneIndex ==1){
  app.sequencer().add<SoundObject>(0, 44000).set( 0, 0, 0, 0.5,soundObjectVisual ,(songFiles[0][0]).c_str(), [&](double t, const al::Vec3f& p) -> al::Vec3f { 
    return al::Vec3f(
    //body of lambda logic. will replace this will header calls
           (sin(a * p.y) + c * cos(a * p.x)),
          (sin(b * p.x) + d * cos(b * p.y)), 
            p.z
        );
  });
  }
    app.start();
    return 0;
}
