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
#include <fstream>
#include <cassert>
#include <string>
#include <utility>
#include "vertex.h"
#include "boundingbox.h"
#include "mesh.h"
#include "edge.h"
#include "face.h"
#include "glCanvas.h"
#include "primitive.h"
#include "sphere.h"
#include "cylinder_ring.h"
#include "ray.h"
#include "hit.h"
#include "camera.h"

// =======================================================================
// DESTRUCTOR
// =======================================================================

Mesh::~Mesh() {
    unsigned int i;
    for (i = 0; i < rasterized_primitive_faces.size(); i++) {
        Face *f = rasterized_primitive_faces[i];
        removeFaceEdges(f);
        delete f;
    }
    if (subdivided_quads.size() != original_quads.size()) {
        for (i = 0; i < subdivided_quads.size(); i++) {
            Face *f = subdivided_quads[i];
            removeFaceEdges(f);
            delete f;
        }
    }
    for (i = 0; i < original_quads.size(); i++) {
        Face *f = original_quads[i];
        removeFaceEdges(f);
        delete f;
    }
    for (i = 0; i < primitives.size(); i++) { delete primitives[i]; }
    for (i = 0; i < materials.size(); i++) { delete materials[i]; }
    for (i = 0; i < vertices.size(); i++) { delete vertices[i]; }
    delete bbox;
}

// =======================================================================
// MODIFIERS:   ADD & REMOVE
// =======================================================================

Vertex* Mesh::addVertex(const Vec3f &position) {
    int index = numVertices();
    vertices.push_back(new Vertex(index,position));
    // extend the bounding box to include this point
    if (bbox == NULL) 
        bbox = new BoundingBox(position,position);
    else 
        bbox->Extend(position);
    return vertices[index];
}

void Mesh::addPrimitive(Primitive* p) {
    primitives.push_back(p);
    p->addRasterizedFaces(this,args);
}

void Mesh::addFace(Vertex *a, Vertex *b, Vertex *c, Vertex *d, Material *material, enum FACE_TYPE face_type) {
    // create the face
    Face *f = new Face(material);
    // create the edges
    Edge *ea = new Edge(a,b,f);
    Edge *eb = new Edge(b,c,f);
    Edge *ec = new Edge(c,d,f);
    Edge *ed = new Edge(d,a,f);
    // point the face to one of its edges
    f->setEdge(ea);
    // connect the edges to each other
    ea->setNext(eb);
    eb->setNext(ec);
    ec->setNext(ed);
    ed->setNext(ea);
    // verify these edges aren't already in the mesh 
    // (which would be a bug, or a non-manifold mesh)
    assert (edges.find(std::make_pair(a,b)) == edges.end());
    assert (edges.find(std::make_pair(b,c)) == edges.end());
    assert (edges.find(std::make_pair(c,d)) == edges.end());
    assert (edges.find(std::make_pair(d,a)) == edges.end());
    // add the edges to the master list
    edges[std::make_pair(a,b)] = ea;
    edges[std::make_pair(b,c)] = eb;
    edges[std::make_pair(c,d)] = ec;
    edges[std::make_pair(d,a)] = ed;
    // connect up with opposite edges (if they exist)
    edgeshashtype::iterator ea_op = edges.find(std::make_pair(b,a)); 
    edgeshashtype::iterator eb_op = edges.find(std::make_pair(c,b)); 
    edgeshashtype::iterator ec_op = edges.find(std::make_pair(d,c)); 
    edgeshashtype::iterator ed_op = edges.find(std::make_pair(a,d)); 
    if (ea_op != edges.end()) { ea_op->second->setOpposite(ea); }
    if (eb_op != edges.end()) { eb_op->second->setOpposite(eb); }
    if (ec_op != edges.end()) { ec_op->second->setOpposite(ec); }
    if (ed_op != edges.end()) { ed_op->second->setOpposite(ed); }
    // add the face to the appropriate master list
    if (face_type == FACE_TYPE_ORIGINAL) {
        original_quads.push_back(f);
        subdivided_quads.push_back(f);
    } else if (face_type == FACE_TYPE_RASTERIZED) {
        rasterized_primitive_faces.push_back(f); 
    } else {
        assert (face_type == FACE_TYPE_SUBDIVIDED);
        subdivided_quads.push_back(f);
    }
    // if it's a light, add it to that list too
    if (material->getEmittedColor().Length() > 0 && face_type == FACE_TYPE_ORIGINAL) {
        original_lights.push_back(f);
    }
}

void Mesh::removeFaceEdges(Face *f) {
    // helper function for face deletion
    Edge *ea = f->getEdge();
    Edge *eb = ea->getNext();
    Edge *ec = eb->getNext();
    Edge *ed = ec->getNext();
    assert (ed->getNext() == ea);
    Vertex *a = ea->getStartVertex();
    Vertex *b = eb->getStartVertex();
    Vertex *c = ec->getStartVertex();
    Vertex *d = ed->getStartVertex();
    // remove elements from master lists
    edges.erase(std::make_pair(a,b)); 
    edges.erase(std::make_pair(b,c)); 
    edges.erase(std::make_pair(c,d)); 
    edges.erase(std::make_pair(d,a)); 
    // clean up memory
    delete ea;
    delete eb;
    delete ec;
    delete ed;
}

// ==============================================================================
// EDGE HELPER FUNCTIONS

Edge* Mesh::getEdge(Vertex *a, Vertex *b) const {
    edgeshashtype::const_iterator iter = edges.find(std::make_pair(a,b));
    if (iter == edges.end()) return NULL;
    return iter->second;
}

Vertex* Mesh::getChildVertex(Vertex *p1, Vertex *p2) const {
    vphashtype::const_iterator iter = vertex_parents.find(std::make_pair(p1,p2)); 
    if (iter == vertex_parents.end()) return NULL;
    return iter->second; 
}

void Mesh::setParentsChild(Vertex *p1, Vertex *p2, Vertex *child) {
    assert (vertex_parents.find(std::make_pair(p1,p2)) == vertex_parents.end());
    vertex_parents[std::make_pair(p1,p2)] = child; 
}

//
// ===============================================================================
// the load function parses our (non-standard) extension of very simple .obj files
// ===============================================================================

void Mesh::Load(const std::string &input_file, ArgParser *_args) {
    args = _args;
    std::ifstream objfile(input_file.c_str());
    if (objfile == NULL) {
        std::cout << "ERROR! CANNOT OPEN " << input_file << std::endl;
        return;
    }
    
    std::string token;
    Material *active_material = NULL;
    camera = NULL;
    background_color = Vec3f(1,1,1);
    
    while (objfile >> token) {
        if (token == "v") {
            double x,y,z;
            objfile >> x >> y >> z;
            addVertex(Vec3f(x,y,z));
        } else if (token == "vt") {
            assert (numVertices() >= 1);
            double s,t;
            objfile >> s >> t;
            getVertex(numVertices()-1)->setTextureCoordinates(s,t);
        } else if (token == "f") {
            int a,b,c,d;
            objfile >> a >> b >> c >> d;
            a--;
            b--;
            c--;
            d--;
            assert (a >= 0 && a < numVertices());
            assert (b >= 0 && b < numVertices());
            assert (c >= 0 && c < numVertices());
            assert (d >= 0 && d < numVertices());
            assert (active_material != NULL);
            addOriginalQuad(getVertex(a),getVertex(b),getVertex(c),getVertex(d),active_material);
        } else if (token == "s") {
            double x,y,z,r;
            objfile >> x >> y >> z >> r;
            assert (active_material != NULL);
            addPrimitive(new Sphere(Vec3f(x,y,z),r,active_material));
        } else if (token == "r") {
            double x,y,z,h,r,r2;
            objfile >> x >> y >> z >> h >> r >> r2;
            assert (active_material != NULL);
            addPrimitive(new CylinderRing(Vec3f(x,y,z),h,r,r2,active_material));
        } else if (token == "background_color") {
            double r,g,b;
            objfile >> r >> g >> b;
            background_color = Vec3f(r,g,b);
        } else if (token == "PerspectiveCamera") {
            camera = new PerspectiveCamera();
            objfile >> *(PerspectiveCamera*)camera;
        } else if (token == "OrthographicCamera") {
            camera = new OrthographicCamera();
            objfile >> *(OrthographicCamera*)camera;
        } else if (token == "m") {
            // this is not standard .obj format!!
            // materials
            int m;
            objfile >> m;
            assert (m >= 0 && m < (int)materials.size());
            active_material = materials[m];
        } else if (token == "material") {
            // this is not standard .obj format!!
            std::string texture_file = "";
            Vec3f diffuse(0,0,0);
            double r,g,b;
            objfile >> token;
            if (token == "diffuse") {
                objfile >> r >> g >> b;
                diffuse = Vec3f(r,g,b);
            } else {
                assert (token == "texture_file");
                objfile >> texture_file;
            }
            Vec3f reflective,emitted,transmitted;      
            objfile >> token >> r >> g >> b;
            assert (token == "reflective");
            reflective = Vec3f(r,g,b);
            objfile >> token >> r >> g >> b;
            assert (token == "emitted");
            emitted = Vec3f(r,g,b);
            objfile >> token >> r >> g >> b;
            assert (token == "transmitted");
            transmitted = Vec3f(r,g,b);
            materials.push_back(new Material(texture_file,diffuse,reflective,emitted,transmitted));
        } else {
            std::cout << "UNKNOWN TOKEN " << token << std::endl;
            exit(0);
        }
    }
    std::cout << " mesh loaded " << numFaces() << std::endl;
    
    if (camera == NULL) {
        // if not initialized, position a perspective camera and scale it so it fits in the window
        assert (bbox != NULL);
        Vec3f point_of_interest = bbox->getCenter();
        double max_dim = bbox->maxDim();
        Vec3f camera_position = point_of_interest + Vec3f(0,0,4*max_dim);
        Vec3f up = Vec3f(0,1,0);
        camera = new PerspectiveCamera(camera_position, point_of_interest, up, 20 * M_PI/180.0);    
    }
}


// =======================================================================
// PAINT
// =======================================================================

void Mesh::PaintWireframe() {
    
    glDisable(GL_LIGHTING);
    
    // draw all the interior edges
    glLineWidth(1);
    glColor3f(0,0,0);
    glBegin (GL_LINES);
    for (edgeshashtype::iterator iter = edges.begin();
         iter != edges.end(); iter++) {
        Edge *e = iter->second;
        if (e->getOpposite() == NULL) continue;
        Vec3f a = e->getStartVertex()->get();
        Vec3f b = e->getEndVertex()->get();
        glVertex3f(a.x(),a.y(),a.z());
        glVertex3f(b.x(),b.y(),b.z());
    }
    glEnd();
    
    // draw all the boundary edges
    glLineWidth(3);
    glColor3f(1,0,0);
    glBegin (GL_LINES);
    for (edgeshashtype::iterator iter = edges.begin();
         iter != edges.end(); iter++) {
        Edge *e = iter->second;
        if (e->getOpposite() != NULL) continue;
        Vec3f a = e->getStartVertex()->get();
        Vec3f b = e->getEndVertex()->get();
        glVertex3f(a.x(),a.y(),a.z());
        glVertex3f(b.x(),b.y(),b.z());
    }
    glEnd();
    
    glEnable(GL_LIGHTING);
    
    HandleGLError(); 
}

// =================================================================
// SUBDIVISION
// =================================================================

Vertex* Mesh::AddEdgeVertex(Vertex *a, Vertex *b) {
    Vertex *v = getChildVertex(a,b);
    if (v != NULL) return v;
    Vec3f pos = 0.5*a->get() + 0.5*b->get();
    double s = 0.5*a->get_s() + 0.5*b->get_s();
    double t = 0.5*a->get_t() + 0.5*b->get_t();
    v = addVertex(pos);
    v->setTextureCoordinates(s,t);
    setParentsChild(a,b,v);
    return v;
}

Vertex* Mesh::AddMidVertex(Vertex *a, Vertex *b, Vertex *c, Vertex *d) {
    Vec3f pos = 0.25*a->get() + 0.25*b->get() + 0.25*c->get() + 0.25*d->get();
    double s = 0.25*a->get_s() + 0.25*b->get_s() + 0.25*c->get_s() + 0.25*d->get_s();
    double t = 0.25*a->get_t() + 0.25*b->get_t() + 0.25*c->get_t() + 0.25*d->get_t();
    Vertex *v = addVertex(pos);
    v->setTextureCoordinates(s,t);
    return v;
}

void Mesh::Subdivision() {
    
    bool first_subdivision = false;
    if (original_quads.size() == subdivided_quads.size()) {
        first_subdivision = true;
    }
    
    std::vector<Face*> tmp = subdivided_quads;
    subdivided_quads.clear();
    
    for (unsigned int i = 0; i < tmp.size(); i++) {
        Face *f = tmp[i];
        
        Vertex *a = (*f)[0];
        Vertex *b = (*f)[1];
        Vertex *c = (*f)[2];
        Vertex *d = (*f)[3];
        // add new vertices on the edges
        Vertex *ab = AddEdgeVertex(a,b);
        Vertex *bc = AddEdgeVertex(b,c);
        Vertex *cd = AddEdgeVertex(c,d);
        Vertex *da = AddEdgeVertex(d,a);
        // add new point in the middle of the patch
        Vertex *mid = AddMidVertex(a,b,c,d);
        
        assert (getEdge(a,b) != NULL);
        assert (getEdge(b,c) != NULL);
        assert (getEdge(c,d) != NULL);
        assert (getEdge(d,a) != NULL);
        
        // copy the color and emission from the old patch to the new
        Material *material = f->getMaterial();
        if (!first_subdivision) {
            removeFaceEdges(f);
            delete f;
        }
        
        // create the new faces
        addSubdividedQuad(a,ab,mid,da,material);
        addSubdividedQuad(b,bc,mid,ab,material);
        addSubdividedQuad(c,cd,mid,bc,material);
        addSubdividedQuad(d,da,mid,cd,material);
        
        assert (getEdge(a,ab) != NULL);
        assert (getEdge(ab,b) != NULL);
        assert (getEdge(b,bc) != NULL);
        assert (getEdge(bc,c) != NULL);
        assert (getEdge(c,cd) != NULL);
        assert (getEdge(cd,d) != NULL);
        assert (getEdge(d,da) != NULL);
        assert (getEdge(da,a) != NULL);
    }
}

