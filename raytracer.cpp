#include "raytracer.h"
#include "material.h"
#include "vectors.h"
#include "argparser.h"
#include "raytree.h"
#include "utils.h"
#include "mesh.h"
#include "face.h"
#include "primitive.h"
#include "photon_mapping.h"

// ===========================================================================
// casts a single ray through the scene geometry and finds the closest hit
bool RayTracer::CastRay(Ray &ray, Hit &h, bool use_rasterized_patches) const {
    bool answer = false;
    
    // intersect each of the true quads 
    for (int i = 0; i < mesh->numOriginalQuads(); i++) {
        Face *f = mesh->getOriginalQuad(i);
        if (f->intersect(ray,h,args->intersect_backfacing)) answer = true;
    }
    
    // intersect each of the primitives (either the patches, or the original primitives)
    if (use_rasterized_patches) {
        for (int i = 0; i < mesh->numRasterizedPrimitiveFaces(); i++) {
            Face *f = mesh->getRasterizedPrimitiveFace(i);
            if (f->intersect(ray,h,args->intersect_backfacing)) answer = true;
        }
    } else {
        int num_primitives = mesh->numPrimitives();
        for (int i = 0; i < num_primitives; i++) {
            if (mesh->getPrimitive(i)->intersect(ray,h)) answer = true;
        }
    }
    return answer;
}

// ===========================================================================
// does the recursive (shadow rays & recursive rays) work
Vec3f RayTracer::TraceRay(Ray &ray, Hit &hit, int bounce_count) const {
    
    // First cast a ray and see if we hit anything.
    hit = Hit();
    bool intersect = CastRay(ray,hit,false);
    
    // if there is no intersection, simply return the background color
    if (intersect == false) {
        const Vec3f &bg = mesh->getBackgroundColor();
        return Vec3f(srgb_to_linear(bg.r()),
                     srgb_to_linear(bg.g()),
                     srgb_to_linear(bg.b()));    
    }
    
    // otherwise decide what to do based on the material
    Material *m = hit.getMaterial();
    assert (m != NULL);
    
    // rays coming from the light source are set to white, don't bother to ray trace further.
    if (m->getEmittedColor().Length() > 0.001) {
        return Vec3f(1,1,1);
    } 
    
    Vec3f normal = hit.getNormal();
    Vec3f point = ray.pointAtParameter(hit.getT());
    Vec3f answer;
    
    // ----------------------------------------------
    // start with the indirect light (ambient light)
    Vec3f diffuse_color = m->getDiffuseColor(hit.get_s(),hit.get_t());
    if (args->gather_indirect) {
        // photon mapping for more accurate indirect light
        answer = diffuse_color * photon_mapping->GatherIndirect(point, normal, ray.getDirection());
    } else {
        // the usual ray tracing hack for indirect light
        answer = diffuse_color * args->ambient_light;
    }      
    
    // ----------------------------------------------
    // add contributions from each light that is not in shadow
    int num_lights = mesh->getLights().size();
    for (int i = 0; i < num_lights; i++) {
        Face *f = mesh->getLights()[i];
        Vec3f pointOnLight = f->computeCentroid();
        Vec3f lightColor = f->getMaterial()->getEmittedColor() * f->getArea();
        Vec3f dirToLight = pointOnLight - point;
        double dist = dirToLight.Length();
        dirToLight.Normalize();
        lightColor /= M_PI*dist*dist;
       
        if (args->num_shadow_samples == 1) {
            Ray lightRay(point, dirToLight);
            
            bool allClear = true;
            
            int num_primitives = mesh->numPrimitives();
            for (int j = 0; j < num_primitives; ++j) {
                Primitive *p = mesh->getPrimitive(j);
                Hit lightHit;
                if (p->intersect(lightRay,lightHit)) {
                    // Hit.  Now check to see if it's between us and our light source
                    Vec3f shadowRayPoint = lightRay.pointAtParameter(lightHit.getT());
                    Vec3f shadowRayVec = pointOnLight - shadowRayPoint;
                    double x = shadowRayVec.Length();
                    if (x < dist) {
                        allClear = false;
                        shadowRayVec.Normalize();
                        RayTree::AddShadowSegment(Ray(shadowRayPoint, shadowRayVec), 0, dist);
                        break;
                    }
                }
            }
            
            if (allClear) {
                answer += m->Shade(ray,hit,dirToLight,lightColor,args);
            }
        }
        else if (args->num_shadow_samples > 1) {
            Vec3f tempAnswer;
            for (int s = 0; s < args->num_shadow_samples; ++s) {
                Vec3f newPoint = f->RandomPoint();
                Vec3f dir = newPoint - point;
                dist = dir.Length();
                dir.Normalize();
                lightColor = f->getMaterial()->getEmittedColor() * f->getArea();
                lightColor /= M_PI*dist*dist;
                Ray lightRay(point, dir);
                
                bool allClear = true;
                
                int num_primitives = mesh->numPrimitives();
                for (int j = 0; j < num_primitives; ++j) {
                    Primitive *p = mesh->getPrimitive(j);
                    Hit lightHit;
                    if (p->intersect(lightRay,lightHit)) {
                        // Hit.  Now check to see if it's between us and our light source
                        Vec3f shadowRayPoint = lightRay.pointAtParameter(lightHit.getT());
                        Vec3f shadowRayVec = pointOnLight - shadowRayPoint;
                        double x = shadowRayVec.Length();
                        if (x < dist) {
                            allClear = false;
                            shadowRayVec.Normalize();
                            RayTree::AddShadowSegment(Ray(shadowRayPoint, shadowRayVec), 0, dist);
                            //break;
                        }
                    }
                }
                
                if (allClear) {
                    tempAnswer += m->Shade(ray,hit,dirToLight,lightColor,args);
                }
            }
            tempAnswer /= args->num_shadow_samples;
            answer += tempAnswer;
        }
        else {
            answer += m->Shade(ray,hit,dirToLight,lightColor,args);
        }
       
        // ===========================================
        // ASSIGNMENT:  ADD SHADOW & SOFT SHADOW LOGIC
        // ===========================================
        // add the lighting contribution from this particular light at this point
        // (fix this to check for blockers between the light & this surface)
    }
    
    // ----------------------------------------------
    // add contribution from reflection, if the surface is shiny
    Vec3f reflectiveColor = m->getReflectiveColor();
    
    // =================================
    // ASSIGNMENT:  ADD REFLECTIVE LOGIC
    // =================================
    
    if (args->num_bounces > bounce_count) {
        Vec3f V = ray.getDirection();
        Vec3f R_dir = V - 2 * V.Dot3(normal) * normal;
        R_dir.Normalize();
        //R_dir *= -1.0;
        Ray R(point, R_dir);
        Hit nHit;
        answer += TraceRay(R, nHit, bounce_count+1) * reflectiveColor;
        RayTree::AddReflectedSegment(R, 0, nHit.getT());
    }
    
    return answer; 
}
