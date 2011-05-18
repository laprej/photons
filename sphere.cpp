#include "utils.h"
#include "material.h"
#include "argparser.h"
#include "sphere.h"
#include "vertex.h"
#include "mesh.h"
#include "ray.h"
#include "hit.h"

// ====================================================================
// ====================================================================

bool Sphere::intersect(const Ray &r, Hit &h) const {
    
    // ==========================================
    // ASSIGNMENT:  IMPLEMENT SPHERE INTERSECTION
    // ==========================================
    
    // plug the explicit ray equation into the implict sphere equation and solve
    Vec3f temp = r.getOrigin() - center;
    
    double a = r.getDirection().Dot3(r.getDirection());
    double b = r.getDirection().Dot3(temp) * 2;
    double c = temp.Dot3(temp) - radius * radius;
    
    double disc = b * b - 4 * a * c;
    
    if (disc > 0) {
        //std::cout << "r: " << r << " hit " << *this << "\n";
        
        disc = sqrt(disc);
        double t1 = (-b - disc) / (2 * a);
        double t2 = (-b + disc) / (2 * a);
        
        if (t1 < 0 && t2 < 0) {
            return false;
        }
        
        if (t1 < 0 && t2 > 0) {
            return false;
            // t2 is closer than t1 so it must be the point we want
            Vec3f norm = r.getOrigin() + t2 * r.getDirection() - center;
            norm.Normalize();
            h.set(t2, material, norm);
        }
        
        if (t1 > 0 && t2 < 0) {
            return false;
            // t1 is closer than t2 so it must be the point we want
            Vec3f norm = r.getOrigin() + t1 * r.getDirection() - center;
            norm.Normalize();
            h.set(t1, material, norm);
        }
        
        if (t1 < t2 && t1 > 0.001) {
            // t1 is closer than t2 so it must be the point we want
            Vec3f norm = r.getOrigin() + t1 * r.getDirection() - center;
            norm.Normalize();
            h.set(t1, material, norm);
        }
        else if (t2 > 0.001) {
            // t2 is closer than t1 so it must be the point we want
            Vec3f norm = r.getOrigin() + t2 * r.getDirection() - center;
            norm.Normalize();
            h.set(t2, material, norm);
        }
        else
            return false;
        
        return true;
    }
    
    // return true if the sphere was intersected, and update the hit
    // data structure to contain the value of t for the ray at the
    // intersection point, the material, and the normal
    return false;
} 

// ====================================================================
// ====================================================================

// helper function to place a grid of points on the sphere
Vec3f ComputeSpherePoint(double s, double t, const Vec3f center, double radius) {
    double angle = 2*M_PI*s;
    double y = -cos(M_PI*t);
    double factor = sqrt(1-y*y);
    double x = factor*cos(angle);
    double z = factor*-sin(angle);
    Vec3f answer = Vec3f(x,y,z);
    answer *= radius;
    answer += center;
    return answer;
}

// for OpenGL rendering, and for radiosity, we need a patch based
// version of the sphere
void Sphere::addRasterizedFaces(Mesh *m, ArgParser *args) {
    
    int h = args->sphere_horiz;
    int v = args->sphere_vert;
    assert (h % 2 == 0);
    int i,j;
    int va,vb,vc,vd;
    Vertex *a,*b,*c,*d;
    int offset = m->numVertices();
    
    // place vertices
    m->addVertex(center+radius*Vec3f(0,-1,0));  // bottom vertex
    for (j = 1; j < v; j++) {  // middle vertices
        for (i = 0; i < h; i++) {
            double s = i / double(h);
            double t = j / double(v);
            m->addVertex(ComputeSpherePoint(s,t,center,radius));
        }
    }
    m->addVertex(center+radius*Vec3f(0,1,0));  // top vertex
    
    // create the middle patches
    for (j = 1; j < v-1; j++) {
        for (i = 0; i < h; i++) {
            va = 1 +  i      + h*(j-1);
            vb = 1 + (i+1)%h + h*(j-1);
            vc = 1 +  i      + h*(j);
            vd = 1 + (i+1)%h + h*(j);
            a = m->getVertex(offset + va);
            b = m->getVertex(offset + vb);
            c = m->getVertex(offset + vc);
            d = m->getVertex(offset + vd);
            m->addRasterizedPrimitiveFace(a,b,d,c,material);
        }
    }
    
    for (i = 0; i < h; i+=2) {
        // create the bottom patches
        va = 0;
        vb = 1 +  i;
        vc = 1 + (i+1)%h;
        vd = 1 + (i+2)%h;
        a = m->getVertex(offset + va);
        b = m->getVertex(offset + vb);
        c = m->getVertex(offset + vc);
        d = m->getVertex(offset + vd);
        m->addRasterizedPrimitiveFace(d,c,b,a,material);
        // create the top patches
        va = 1 + h*(v-1);
        vb = 1 +  i      + h*(v-2);
        vc = 1 + (i+1)%h + h*(v-2);
        vd = 1 + (i+2)%h + h*(v-2);
        a = m->getVertex(offset + va);
        b = m->getVertex(offset + vb);
        c = m->getVertex(offset + vc);
        d = m->getVertex(offset + vd);
        m->addRasterizedPrimitiveFace(b,c,d,a,material);
    }
}
