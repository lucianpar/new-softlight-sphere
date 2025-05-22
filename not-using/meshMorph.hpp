#ifndef MESH_MORPH_HPP
#define MESH_MORPH_HPP


#include "al/graphics/al_VAOMesh.hpp"
#include "al/math/al_Complex.hpp"
#include "al/math/al_Vec.hpp"

struct MeshToNewMesh {
    public:
    int nCurrentVertices;
    int nTargetVertices;

    // void setTarget(al::VAOMesh& currentMesh, al::VAOMesh& targetMesh){

    // }
    void setTarget(al::VAOMesh& currentMesh, al::VAOMesh& targetMesh ){
        nTargetVertices = targetMesh.vertices().size();
        nCurrentVertices = currentMesh.vertices().size();
    }

    void process(al::VAOMesh& currentMesh, al::VAOMesh& targetMesh, float time, float speed){
    // al::lerp(const Vec<N, T> &input, const Vec<N, T> &target, T amt)
    for (int i = 0; i < nTargetVertices; ++i) {
    // currentMesh.vertices()[i] = al::lerp(
    //         currentMesh.vertices()[i], // current position
    //         targetMesh.vertices()[i],  // target position
    //         speed * time               // interpolation amount
        // );
        currentMesh.vertices()[i] += (targetMesh.vertices()[i]-currentMesh.vertices()[i])*speed;


    }
    }


};






#endif