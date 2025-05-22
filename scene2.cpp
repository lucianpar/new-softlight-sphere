
#include "Gamma/Domain.h"
#include "Gamma/Envelope.h"
#include "Gamma/Oscillator.h"
#include "Gamma/SamplePlayer.h"
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_VAO.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/math/al_Random.hpp"
#include "al/math/al_StdRandom.hpp"
#include "al/math/al_Vec.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_PresetSequencer.hpp"
#include "al/graphics/al_Light.hpp"
//#include "al/graphics/al_Asset.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_Graphics.hpp"
#include "al_ext/assets3d/al_Asset.hpp"
#include <cmath>
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



/* TO DO:

* fix jittering - need karls input. trying to get smooth 

* fix audio players

* set up scene containers for everything?

*/





int sceneIndex = 1;



// NEWER STAR FISH FUNCTION
// this function was gpt'd - i got lazy and am still rapid prototyping
void addStarfish(al::Mesh& m, int arms = 5, float armLength = 1.0f, float armWidth = 0.1f) {
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



//using namespace al;


class MyApp : public al::App {
 public:

  ////INITIAL OBJECTS AND DECLARATIONS////
  // ->
  al::Light light;
  //Light light;
  al::Material material;

  //// START DECLARATIONS FOR SCENE 1 ////

  //MESHES//
  al::VAOMesh blobMesh; 
  al::VAOMesh starCreatureMesh;
  std::vector<al::Nav> blobs;
  std::vector<al::Vec3f> velocity;
  std::vector<al::Vec3f> force;
  float scene2Boundary = 30.0f;
  bool inSphereScene2 = true;
  float blobSeperationThresh = 2.0f;
  int nAgentsScene2  = 30;
  float blobsSpeedScene2 = 0.1;
  float blobSizeScene2 = 1.8;

   std::vector<al::Vec3f> colorPallete = {{0.9f, 0.0f, 0.4}, {0.11, 0.2, 0.46}, {0.11, 0.44, 0.46}};


  //MESH EFFECTS//
  VertexEffectChain blobsEffectChain;
  RippleEffect blobsRippleX;
  RippleEffect blobsRippleY;
  RippleEffect blobsRippleZ;

  VertexEffectChain starEffectChain;
   RippleEffect starRipple;
  

  //set up auto pulse with base verts
  


  //// END DECLARATIONS FOR SCENE 1 ////



  //GLOBAL TIME PARAMS//
   double globalTime = 0;
   double sceneTime = 0;
   //int sceneIndex; -- moved out of app to be global
  




   ////DECLARE VALUES FOR EVENT TIMES////

   //SCENE 1 
    
    
    //karl code -- need to figure out where to put this -- maybe just in declerations in main file

    al::Vec3f randomVec3f(float scale) {
      return al::Vec3f(al::rnd::uniformS(), al::rnd::uniformS(), al::rnd::uniformS()) * scale;
    }


  //SCENE 2

    //float track2Start = 119;
    
    // i.e. -- moveOutEvent1, StartRipplingEvent1
  



  void onInit() override { 
    gam::sampleRate(audioIO().framesPerSecond()); 
  

    }

  void onCreate() override {
     ////BIOLERPLATE////
    nav().pos(al::Vec3d(0, 0, 0));  // Set the camera to view the scene
    sequencer().playSequence();

    // al::Vec3f randomVec3f(float scale) {
    //   return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * scale;
    // }

    // //INITIALIZE LIGHTING 

    nav().pos(0, 0, 0);

    // std::vector<al::Vec3f> colorPallete = {{0.9f, 0.0f, 0.4}, {0.4f, 0.0f, 0.9}};
    addSphere(blobMesh, blobSizeScene2, 40, 40);
    blobMesh.primitive(al::Mesh::TRIANGLES);
    for (int i = 0; i < blobMesh.vertices().size(); i++){
      // al::Vec3f newColor = colorPallete[i%2];
      // blobMesh.color(newColor.x,newColor.y,newColor.z, 0.6);
    }

    blobMesh.generateNormals();

    addStarfish(starCreatureMesh);

    starCreatureMesh.update();

    // blobMesh.update();

    for (int b = 0; b < nAgentsScene2; ++b) {
      al::Nav p;
      p.pos() = randomVec3f(5);
      p.quat()
          .set(al::rnd::uniformS(), al::rnd::uniformS(), al::rnd::uniformS(),
               al::rnd::uniformS())
          .normalize();
      // p.set(randomVec3f(5), randomVec3f(1));
      blobs.push_back(p);
      // velocity.push_back(al::Vec3f(0));
      // force.push_back(al::Vec3f(0));
    }

    blobMesh.update();
   



    blobsRippleX.setParams( 0.2,  0.1, 1.0, 'x');
    blobsRippleZ.setParams( 0.4,  0.1, 1.0, 'z');
    //blobsRippleY.setParams( 0.01,  0.1, 1.0, 'y');
    blobsEffectChain.pushBack(&blobsRippleZ);
    //blobsEffectChain.pushBack(&blobsRippleY);
    blobsEffectChain.pushBack(&blobsRippleX);

    starRipple.setParams( 1.0, 1.0, 1.0, 'z');

    starEffectChain.pushBack(&starRipple);




    scene2Boundary = 1.0;
    




    //end lighting test




    //BOUNDARY MESH
    // addSphere(boundarySphere, 7.5); 
    // boundarySphere.primitive((al::Mesh::LINES));


    ///////


    // SCENE 1 STUFF INITS

    ////CREATE MY MESHES////

    // NEED TO RESET TO RELATIVE PATHS, NOT HARDCODED
    



     // SCENE 2 STUFF INITS
   

   

  }

  ////BASIC TRIGGERING////
  bool onKeyDown(const al::Keyboard& k) override {

    //if (k.key() == '1') {
      // If the space key is pressed, we will trigger the sequencer
      //sequencer().playSequence();
      if (k.key() >=49 && k.key() <= 54){
      sceneIndex = k.key()-48;
      std::cout << "pressed key: " << sceneIndex << std::endl;
      }
    
    return true;
  }
//  float newSpeed = 0.0f;
  void onAnimate(double dt) override {

    // SET SCENES AND TIME TRANSITIONS ///
     globalTime += dt;
     sceneTime += dt;
     if (globalTime == 118) {
      sceneIndex = 2;
      sceneTime = 0;
     }
     if (globalTime == 334) {
      sceneIndex = 3;
      sceneTime = 0;
     }


    // Update the sequencer
    sequencer().update(globalTime); // XXX important to call this
    std::cout << "global time: " << globalTime << std::endl;
     fflush(stdout);

    if(sceneTime >=0){
      //boundary is initially set super small so they are constrained 
      scene2Boundary = 15.0;
    }
    if(sceneTime >=0 && sceneTime <=1.0){
      blobsSpeedScene2 = 3.0;
    }
    if(sceneTime >=1.0 && sceneTime <=5.0){
       blobsSpeedScene2 = 0.1;
    }



    if (sceneTime >= 5.0){
      blobsSpeedScene2 = 0.2;
    }
    if (sceneTime >= 8.0){
      blobsSpeedScene2 = 0.05;
    }
    if (sceneTime >= 11.0){
      blobsSpeedScene2 = 0.3;
    }
    // should we call in the audio callback instead?


    //// PROCESS MESH EFFECTS ////
    // bodyEffectChain.process(bodyMesh, globalTime);
    //openingSphereEffectChain.process(openingSphereMesh, globalTime);
    //openingSphereScatter.stop(true);
    //bodyScatter.triggerOut(true, bodyMesh);

    //bodyAttractor.processThomas(bodyMesh, globalTime, 0.001);
    //SCENE SPECIFIC // 

    for (int i = 0; i < blobs.size(); ++i) {
    //inSphereScene2 = true;
    if (blobs[i].pos().mag() >= scene2Boundary) {
     blobs[i].faceToward(blobs[i].uf()*(-1.0), 0.1);
     blobs[i].moveF(blobSizeScene2);

    }
    //checking blob against every other blob. turn around if too close to eachother
    // for (int j = 0; j < blobs.size(); ++j){
    //   if (blobs[i].pos() != blobs[j].pos()){
    //    float distance = (blobs[i].pos() - blobs[j].pos()).mag();
    //     if(distance <= blobSeperationThresh)
    //     blobs[i].faceToward(blobs[i].uf()*-1.0, 0.1);
    //     blobs[i].moveF(10.0);
        
    //   }
    // }

    
    //if inSphere()
    //agent[i].faceToward(clusterCenter*invertDir*run, 0.03);
    //blobs[i].faceToward(blobs[i-1].pos() * (-1), 0.04);
    blobs[i].moveF(1.0);

    blobs[i].step(sceneTime * blobsSpeedScene2 / (sceneTime + 0.1)); // scene time vs dt - unsure what works better for this
    // times time step -- make this "react to audio changes". division is to deal with step accumulation
    //blobs[i].nudge()
    


     }

    blobsEffectChain.process(blobMesh, sceneTime);

    starEffectChain.process(starCreatureMesh, sceneTime);
    
    blobMesh.update();

  

// velocity[k] += force[k] * dt;
// blobs[k].pos() += velocity[k] * dt;
    


    //MESH EFFECT SEQUENCING//

    //SCENE 1 -- from 

    // if (globalTime == 1){
    //   bodyScatter.triggerOut(true, bodyMesh);
    // }

    // bodyAttractor.processThomas(bodyMesh, globalTime, 0.0001);
    // bodyMesh.scale(1.01);

  
    // newSpeed;
    
  
    //SCENE 2 -- from

  }
  //END OF ANIMATE CALLBACK 

  //// INCREMENT VALUES FOR DRAW ////
  float shellIncrementScene1;
  float particleIncrementScene1;
  float pointSize = 3.0;
  
  void onDraw(al::Graphics& g) override {



    //// SCENE 1 START OF DRAW /////
    if (sceneIndex == 1){

    //THIS SEQUENCE MAKES THE SHELL APPEAR 
    glEnable(GL_BLEND);
    g.blendTrans();
    g.depthTesting(true);  
    g.clear(0.0,0.0,0.09 + ((sceneTime / (118-334))*0.8));

    g.lighting(true);
    // lighting from karl's example
    light.globalAmbient(al::RGB(0.5, (1.0), 1.0));
    light.ambient(al::RGB(0.5, (1.0), 1.0));
    light.diffuse(al::RGB(1, 1, 0.5));
    g.light(light);
    material.specular(light.diffuse() * 0.3);
    material.shininess(50);
    g.material(material);
    //end of lighting for karl's example

    glDepthMask(GL_FALSE); // re enable later if needed
   
  


    //PARTICLES SEQUENCE 1 
    
    g.pointSize(pointSize);
    //g.color(1.0,1.0,1.0,0.2);

    for (int i = 0; i < blobs.size(); ++i) {
      al::Vec3f newColor = colorPallete[i%3];
       
      g.pushMatrix();
     // g.color(1.0,sin(0.0+fearColorReact),0.5);
      g.translate(blobs[i].pos());
      g.rotate(blobs[i].quat());
      //g.meshColor();
      // g.color(newColor.x, newColor.y, newColor.z, 0.25);
      //draw blobs:
      if (i % 2 == 1){
      g.color(newColor.x, newColor.y, newColor.z, 0.4 + (sin(sceneTime*2.0) * 0.1));
      g.draw(blobMesh);
      }
      //draw creatures:
      else {
         g.color(newColor.x + 0.4, newColor.y + 0.4, newColor.z + 0.4, 0.4 + (sin(sceneTime*0.5) * 0.1));
        g.draw(starCreatureMesh);
        // g.color(newColor.x + 0.4, newColor.y + 0.4, newColor.z + 0.4, 1.0);
      }
      g.popMatrix();
      //blobMesh.update();
    }
    //g.meshColor();

    //if (sceneTime >= particlesAppearEvent) {
    //g.meshColor();
   // }
    

    // MAIN COLOR SEQUENCE //
    //color sequence
    // if(sceneTime<=particlesAppearEvent){
    // g.color(0+(sceneTime/particlesAppearEvent));}
    // else{
    //   //g.color(1.0);
    // }


   //only draw once particles have dispersed


    //g.draw(boundarySphere);

  } //// SCENE 1 END OF DRAW /////



    
    

    
    mSequencer.render(g);
  }

  void onSound(al::AudioIOData& io) override { mSequencer.render(io); }

  al::SynthSequencer& sequencer() { return mSequencer; }
  al::SynthSequencer mSequencer;
};

int main() {
  MyApp app;
  //int sceneIndex;


  //can this section move to onCreate or initial app declarations?? or is it fine here//

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

  // ^^^



  //constants for testing
  double g = 0.7;
  float a = 1.4;
  float b = 1.6;
  float c = 1.0;
  float d = 0.7;



  //group audio by every scene. make multiple sequencers to trigger when every scene index is switched
  
  //assign trajectories in the sequencer!!

  // UNCOMMENT AUDIO !!!!!!
  // this logic doesn't work, will  have to change
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
  if (sceneIndex == 2) {
  app.sequencer().add<SoundObject>(0, 44000).set( 0, 0, 0, 0.5,soundObjectVisual ,(songFiles[1][0]).c_str(), [&](double t, const al::Vec3f& p) -> al::Vec3f { 
    return al::Vec3f(
    //body of lambda logic. will replace this will header calls
           (cos(a * p.y) + c * cos(a * p.x)),
          (sin(b * p.x) + d * sin(b * p.y)), 
            p.z
        );
  });
  }


  app.start();
  return 0;
}
