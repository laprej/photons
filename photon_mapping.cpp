#include <cstdio>
#include <cstdlib>

// Included files for OpenGL Rendering
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <iostream>
#include <algorithm>
#include "photon_mapping.h"
#include "mesh.h"
#include "face.h"
#include "primitive.h"
#include "kdtree.h"
#include "utils.h"
#include "raytracer.h"


// ==========
// DESTRUCTOR
PhotonMapping::~PhotonMapping() {
    // cleanup all the photons
    delete kdtree;
}


// ========================================================================
// Recursively trace a single photon

void PhotonMapping::TracePhoton(const Vec3f &position, const Vec3f &direction, 
                                const Vec3f &energy, int iter) {
    if (iter > 5) {
        return;
    }
    // ==============================================
    // ASSIGNMENT: IMPLEMENT RECURSIVE PHOTON TRACING
    // ==============================================
    
    
    // Trace the photon through the scene.  At each diffuse or
    // reflective bounce, store the photon in the kd tree.
    
    // One optimization is to *not* store the first bounce, since that
    // direct light can be efficiently computed using classic ray
    // tracing.
    
    Ray r(position, direction);
    Hit h;
    
    if (raytracer->CastRay(r, h, 0)) {
        // If we hit something...
        Vec3f v = raytracer->TraceRay(r, h, 0);
        
        //std::cout << h << "\n";
        
        Vec3f pos = r.pointAtParameter(h.getT());
        
        // Take care of some things here...
        Material *m = h.getMaterial();
        assert(m != NULL);
        
        // Multiply by material consants
        Vec3f diffuse = m->getDiffuseColor();
        std::cout << "Diffuse: " << diffuse << "\n";
        diffuse = diffuse * energy;
        Vec3f reflective = m->getReflectiveColor();
        std::cout << "Reflective: " << reflective << "\n";
        reflective = reflective * energy;
        
        if (reflective == Vec3f(0,0,0)) {
            std::cout << "This material is not reflective\n";
            // Diffuse
            Vec3f normal = h.getNormal();
            Vec3f V = r.getDirection();
            //Vec3f R_dir = V - 2 * V.Dot3(normal) * normal;
            //Vec3f R_dir = MirrorDirection(normal, V);
            Vec3f R_dir = RandomDiffuseDirection(normal);
            R_dir.Normalize();
            //R_dir *= -1.0;
            Ray R(pos, R_dir);
            TracePhoton(pos, R_dir, diffuse, iter+1);
            if (iter != 0) {
                Photon p(pos, direction, diffuse, iter);
                kdtree->AddPhoton(p);
            }
        }
        else {
            std::cout << "This material is reflective\n";
            // Reflection
            Vec3f normal = h.getNormal();
            Vec3f V = r.getDirection();
            //Vec3f R_dir = V - 2 * V.Dot3(normal) * normal;
            Vec3f R_dir = MirrorDirection(normal, V);
            R_dir.Normalize();
            //R_dir *= -1.0;
            Ray R(pos, R_dir);
            TracePhoton(pos, R_dir, reflective, iter+1);
            if (iter != 0) {
                Photon p(pos, direction, reflective, iter);
                kdtree->AddPhoton(p);
            }
        }
        
        // And handle those differently
        
//        // Reflection
//        Vec3f normal = h.getNormal();
//        Vec3f V = r.getDirection();
//        Vec3f R_dir = V - 2 * V.Dot3(normal) * normal;
//        R_dir.Normalize();
//        //R_dir *= -1.0;
//        Ray R(pos, R_dir);
//        Hit nHit;
//        TracePhoton(pos, R_dir, reflective, iter+1);
        
//        Photon p(pos, direction, energy, iter);
//        kdtree->AddPhoton(p);
    }
    
    
//    Photon(const Vec3f &p, const Vec3f &d, const Vec3f &e, int b) :
//    position(p),direction_from(d),energy(e),bounce(b) {}
}



// ========================================================================
// Trace the specified number of photons through the scene

void PhotonMapping::TracePhotons() {
    std::cout << "trace photons" << std::endl;
    
    // first, throw away any existing photons
    delete kdtree;
    
    // consruct a kdtree to store the photons
    BoundingBox *bb = mesh->getBoundingBox();
    Vec3f min = bb->getMin();
    Vec3f max = bb->getMax();
    Vec3f diff = max-min;
    min -= 0.001*diff;
    max += 0.001*diff;
    kdtree = new KDTree(BoundingBox(min,max));
    
    // photons emanate from the light sources
    const std::vector<Face*>& lights = mesh->getLights();
    
    // compute the total area of the lights
    double total_lights_area = 0;
    for (unsigned int i = 0; i < lights.size(); i++) {
        total_lights_area += lights[i]->getArea();
    }
    
    // shoot a constant number of photons per unit area of light source
    // (alternatively, this could be based on the total energy of each light)
    for (unsigned int i = 0; i < lights.size(); i++) {  
        double my_area = lights[i]->getArea();
        int num = (int)ceil(args->num_photons_to_shoot * my_area / total_lights_area);
        // the initial energy for this photon
        Vec3f energy = my_area/double(num) * lights[i]->getMaterial()->getEmittedColor();
        Vec3f normal = lights[i]->computeNormal();
        for (int j = 0; j < num; j++) {
            Vec3f start = lights[i]->RandomPoint();
            // the initial direction for this photon (for diffuse light sources)
            Vec3f direction = RandomDiffuseDirection(normal);
            TracePhoton(start,direction,energy,0);
        }
    }
}


// ======================================================================
// PHOTON VISUALIZATION FOR DEBUGGING
// ======================================================================

void PhotonMapping::RenderPhotons() {
    RenderPhotonPositions();
    RenderPhotonDirections();
}

// render the position of each photon
void PhotonMapping::RenderPhotonPositions() {  
    if (kdtree == NULL) return;
    glDisable(GL_LIGHTING);
    glPointSize(3);
    glBegin(GL_POINTS);
    // walk through all the cells of the kdtree
    std::vector<const KDTree*> todo;  
    todo.push_back(kdtree);
    while (!todo.empty()) {
        const KDTree *node = todo.back();
        todo.pop_back(); 
        if (node->isLeaf()) {
            const std::vector<Photon> &photons = node->getPhotons();
            int num_photons = photons.size();
            for (int i = 0; i < num_photons; i++) {
                const Photon &p = photons[i];
                Vec3f energy = p.getEnergy()*args->num_photons_to_shoot;
                glColor3f(energy.x(),energy.y(),energy.z());
                const Vec3f &position = p.getPosition();
                // draw each photon as a gl point
                glVertex3f(position.x(),position.y(),position.z());
            }
        } else {
            todo.push_back(node->getChild1());
            todo.push_back(node->getChild2());
        } 
    }
    glEnd();
    glEnable(GL_LIGHTING);
}


// render the incoming direction of each photon
void PhotonMapping::RenderPhotonDirections() {
    if (kdtree == NULL) return;
    glDisable(GL_LIGHTING);
    glLineWidth(1);
    glBegin(GL_LINES);
    // walk through all the cells of the kdtree
    std::vector<const KDTree*> todo;  
    todo.push_back(kdtree);
    BoundingBox *bb = mesh->getBoundingBox();
    double max_dim = bb->maxDim();
    while (!todo.empty()) {
        const KDTree *node = todo.back();
        todo.pop_back(); 
        if (node->isLeaf()) {
            const std::vector<Photon> &photons = node->getPhotons();
            int num_photons = photons.size();
            for (int i = 0; i < num_photons; i++) {
                const Photon &p = photons[i];
                const Vec3f a = p.getPosition();
                Vec3f b = p.getPosition()-(p.getDirectionFrom()*0.02*max_dim);
                Vec3f energy = p.getEnergy()*args->num_photons_to_shoot;
                glColor3f(energy.x(),energy.y(),energy.z());
                // draw each photon direction as a small line segment
                glVertex3f(a.x(),a.y(),a.z());
                glVertex3f(b.x(),b.y(),b.z());
            }
        } else {
            todo.push_back(node->getChild1());
            todo.push_back(node->getChild2());
        } 
    }
    glEnd();
    glEnable(GL_LIGHTING);
}


// ======================================================================
// render a simple wireframe of the KD tree
void PhotonMapping::RenderKDTree() {
    if (kdtree == NULL) return;
    glDisable(GL_LIGHTING);
    glLineWidth(1);
    glColor3f(0,0,0);
    glBegin(GL_LINES);
    std::vector<const KDTree*> todo;  
    todo.push_back(kdtree);
    while (!todo.empty()) {
        const KDTree *node = todo.back();
        todo.pop_back(); 
        if (node->isLeaf()) {
            const Vec3f& min = node->getMin();
            const Vec3f& max = node->getMax();
            
            glVertex3f(min.x(),min.y(),min.z());
            glVertex3f(max.x(),min.y(),min.z());
            glVertex3f(min.x(),min.y(),min.z());
            glVertex3f(min.x(),max.y(),min.z());
            glVertex3f(max.x(),max.y(),min.z());
            glVertex3f(max.x(),min.y(),min.z());
            glVertex3f(max.x(),max.y(),min.z());
            glVertex3f(min.x(),max.y(),min.z());
            
            glVertex3f(min.x(),min.y(),min.z());
            glVertex3f(min.x(),min.y(),max.z());
            glVertex3f(min.x(),max.y(),min.z());
            glVertex3f(min.x(),max.y(),max.z());
            glVertex3f(max.x(),min.y(),min.z());
            glVertex3f(max.x(),min.y(),max.z());
            glVertex3f(max.x(),max.y(),min.z());
            glVertex3f(max.x(),max.y(),max.z());
            
            glVertex3f(min.x(),min.y(),max.z());
            glVertex3f(max.x(),min.y(),max.z());
            glVertex3f(min.x(),min.y(),max.z());
            glVertex3f(min.x(),max.y(),max.z());
            glVertex3f(max.x(),max.y(),max.z());
            glVertex3f(max.x(),min.y(),max.z());
            glVertex3f(max.x(),max.y(),max.z());
            glVertex3f(min.x(),max.y(),max.z());
            
        } else {
            todo.push_back(node->getChild1());
            todo.push_back(node->getChild2());
        } 
    }
    glEnd();
    glEnable(GL_LIGHTING);
}


// ======================================================================

Vec3f PhotonMapping::GatherIndirect(const Vec3f &point, const Vec3f &normal, const Vec3f &direction_from) const {
    
    if (kdtree == NULL) { 
        std::cout << "WARNING: Photons have not been traced throughout the scene." << std::endl;
        return Vec3f(0,0,0); 
    }
    
    
    
    // ================================================================
    // ASSIGNMENT: GATHER THE INDIRECT ILLUMINATION FROM THE PHOTON MAP
    // ================================================================
    
    
    
    // collect the closest args->num_photons_to_collect photons
    // determine the radius that was necessary to collect that many photons
    // average the energy of those photons over that radius
    return Vec3f(0,0,0);
}

