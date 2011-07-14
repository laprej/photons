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
#include <stack>
#include "sphere.h"

Vec3f global_energy;

// ==========
// DESTRUCTOR
PhotonMapping::~PhotonMapping() {
    // cleanup all the photons
    delete kdtree;
}


// ========================================================================
// Recursively trace a single photon

void PhotonMapping::TracePhoton(const Vec3f &position, const Vec3f &direction, 
                                const Vec3f &energy, int iter) const {
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
        
        if (Primitive *p = h.getPrim()) {
            Photon ph(position, direction, energy, iter);
            p->addPhoton(ph);
        }
                
        Vec3f pos = r.pointAtParameter(h.getT());
        
        // Take care of some things here...
        Material *m = h.getMaterial();
        assert(m != NULL);
        
        // Multiply by material consants
        Vec3f diffuse = m->getDiffuseColor();
        //std::cout << "Diffuse: " << diffuse << "\n";
        diffuse = diffuse * energy;
        Vec3f reflective = m->getReflectiveColor();
        //std::cout << "Reflective: " << reflective << "\n";
        reflective = reflective * energy;
        Vec3f transmissive = m->getTransmissiveColor();
        transmissive = transmissive * energy;
        
        
        static const Vec3f zero = Vec3f(0,0,0);
        //double photon_prob = GLOBAL_mtrand.rand();
        
        if (diffuse != zero) {
            //std::cout << "This material diffuse\n";
            // Diffuse
            //Vec3f normal = h.getNormal();
            //Vec3f V = r.getDirection();
            Vec3f R_dir = RandomDiffuseDirection(h.getNormal());
            //R_dir.Normalize(); RandomDiffuseDirection normalizes b4 return
            Ray R(pos, R_dir);
            TracePhoton(pos, R_dir, diffuse, iter+1);
            if (iter != 0) {
                Photon p(pos, direction, diffuse, iter);
                kdtree->AddPhoton(p);
            }
        }
        if (reflective != zero) {
            //std::cout << "This material is reflective\n";
            // Reflection
            //Vec3f normal = h.getNormal();
            //Vec3f V = r.getDirection();
            Vec3f R_dir = MirrorDirection(h.getNormal(), r.getDirection());
            R_dir.Normalize();
            Ray R(pos, R_dir);
            TracePhoton(pos, R_dir, reflective, iter+1);
            if (iter != 0) {
                Photon p(pos, direction, reflective, iter);
                kdtree->AddPhoton(p);
            }
        }
        if (transmissive != zero) {
            Vec3f pos2 = r.pointAtParameter(h.getT2() + EPSILON);
            
            //Vec3f normal = h.getNormal();
            //Vec3f V = r.getDirection();
            //Vec3f R_dir = r.getDirection();
            //std::cout << "R_dir.Length() is " << R_dir.Length() << "\n";
            //R_dir.Normalize();
            Ray R(pos2, r.getDirection());
            TracePhoton(pos2, r.getDirection(), transmissive, iter+1);
            if (iter != 0) {
                Photon p(pos, direction, transmissive, iter);
                kdtree->AddPhoton(p);
            }
        }
    }
}



// ========================================================================
// Trace the specified number of photons through the scene

void PhotonMapping::TracePhotons() {
    std::cout << "trace photons" << std::endl;
    
    // first, throw away any existing photons
    delete kdtree;
    int num_prims = mesh->numPrimitives();
    for (int i = 0; i < num_prims; ++i) {
        Primitive *p = mesh->getPrimitive(i);
        p->resetPhotons();
    }
    
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
    
    global_energy = Vec3f();
    
    // shoot a constant number of photons per unit area of light source
    // (alternatively, this could be based on the total energy of each light)
    for (unsigned int i = 0; i < lights.size(); i++) {  
        double my_area = lights[i]->getArea();
        int num = (int)ceil(args->num_photons_to_shoot * my_area / total_lights_area);
        // the initial energy for this photon
        Vec3f energy = my_area/double(num) * lights[i]->getMaterial()->getEmittedColor();
        Vec3f normal = lights[i]->computeNormal();
        std::cout << "emitted energy for light " << i << ": " << num * energy << "\n";
        std::cout << "energy per photon: " << energy << "\n";
        global_energy += num*energy;
#pragma omp parallel for
        for (int j = 0; j < num; j++) {
            Vec3f start = lights[i]->RandomPoint();
            // the initial direction for this photon (for diffuse light sources)
            Vec3f direction = RandomDiffuseDirection(normal);
            TracePhoton(start,direction,energy,0);
        }
    }
    std::cout << "end trace photons" << std::endl;
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

void PhotonMapping::RenderEnergy()
{
    for (int i = 0; i < mesh->numPrimitives(); ++i) {
        Primitive *p = mesh->getPrimitive(i);
        if (Sphere *s = dynamic_cast<Sphere*> (p)) {
            double r = s->getRadius();
            Vec3f v = s->getCenter();
            double intensity = s->getIntensity();
            glColor3f(1.0-intensity, intensity, 0.0);
            glTranslated(v.x(), v.y(), v.z());
            GLUquadricObj *quad = gluNewQuadric();
            gluSphere(quad, r, 30, 30);
        }
    }
}

bool sort(const std::pair<int, double> &l, const std::pair<int, double> &r)
{
    if (l.second < r.second) {
        return true;
    }
    return false;
}

unsigned long count_photons(const KDTree *kd)
{
    if (kd->isLeaf()) {
        return kd->getPhotons().size();
    }
    
    unsigned long num = kd->getPhotons().size();
    
    return num + count_photons(kd->getChild1()) + count_photons(kd->getChild2());
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
    
    const unsigned int collect = args->num_photons_to_collect;
    
    // Temporary photon holder
    std::vector<Photon> photons;
    
    const Vec3f exp = Vec3f(0.5, 0.5, 0.5);
    
    Vec3f min = point - exp;
    Vec3f max = point + exp;
    
    std::vector<std::pair<int, double> > pairs;
        
    BoundingBox b(min, max);
        
    while (1) {
        kdtree->CollectPhotonsInBox(b, photons);
        
        while (photons.size() < collect) {
            photons.clear();
            min -= exp;
            max += exp;
            b.Set(min, max);
            kdtree->CollectPhotonsInBox(b, photons);
        }
        
        //std::cout << "We have " << photons.size() << " neighbors\n";
        
        for (unsigned int i = 0; i < photons.size(); ++i) {
            double d = DistanceBetweenTwoPoints2(point, photons[i].getPosition());
            // TODO: Should maxDim below be divided by 2.0 or not???
            if (d > b.maxDim() * b.maxDim() /* / 2.0 */ ) {
                //std::cout << "throwing this one out, too far!\n";
                //std::cout << "d is " << d << " and maxDim is " << b.maxDim() << "\n";
                continue;
            }
            pairs.push_back(std::make_pair(i, d));
        }
        
        if (pairs.size() >= collect) {
            break;
        }
        
        photons.clear();
        min -= exp;
        max += exp;
        b.Set(min, max);
    }
    
    // Just a sanity check.
    assert(pairs.size() >= collect);
    
    std::sort(pairs.begin(), pairs.end(), sort);
    
    while (pairs.size() > collect) {
        pairs.pop_back();
    }
    
    double maxDist = pairs.back().second;
    
    Vec3f ne;
    
    for (unsigned int i = 0; i < pairs.size(); ++i) {
        Vec3f te = photons[pairs[i].first].getEnergy();
        // TODO: Is the following multiplication good or bad???
        te /= maxDist;// * maxDist;
        ne += te;
    }
    
    return ne;
    
    // collect the closest args->num_photons_to_collect photons
    // determine the radius that was necessary to collect that many photons
    // average the energy of those photons over that radius
    return Vec3f(0,0,0);
}

