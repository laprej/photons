#include <cstdio>
#include <cstdlib>

// Included files for OpenGL Rendering
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "glCanvas.h"
#include "boundingbox.h"
#include "camera.h"
#include "radiosity.h"
#include "raytracer.h"
#include "photon_mapping.h"
#include "mesh.h"
#include "raytree.h"
#include "utils.h"
#include "primitive.h"

// ========================================================
// static variables of GLCanvas class

int GLCanvas::display_list_index;
ArgParser* GLCanvas::args;
Mesh* GLCanvas::mesh;
Radiosity* GLCanvas::radiosity;
RayTracer* GLCanvas::raytracer;
PhotonMapping* GLCanvas::photon_mapping;

// State of the mouse cursor
int GLCanvas::mouseButton;
int GLCanvas::mouseX;
int GLCanvas::mouseY;
bool GLCanvas::shiftPressed;
bool GLCanvas::controlPressed;
bool GLCanvas::altPressed;

// params for the raytracing animation
int GLCanvas::raytracing_x;
int GLCanvas::raytracing_y;
int GLCanvas::raytracing_skip;

// ========================================================
// Initialize all appropriate OpenGL variables, set
// callback functions, and start the main event loop.
// This function will not return but can be terminated
// by calling 'exit(0)'
// ========================================================

void GLCanvas::initialize(ArgParser *_args, Mesh *_mesh, 
                          RayTracer *_raytracer, Radiosity *_radiosity, PhotonMapping *_photon_mapping) {
    
    args = _args;
    mesh = _mesh;
    raytracer = _raytracer;
    radiosity = _radiosity;
    photon_mapping = _photon_mapping;
    
    // setup glut stuff
    glutInitWindowSize(args->width, args->height);
    glutInitWindowPosition(100,100);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutCreateWindow("OpenGL Viewer");
    
    // basic rendering 
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    GLfloat ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient); 
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    
    display_list_index = glGenLists(1);
    
    // Initialize callback functions
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);
    
    Render();
    
    // Enter the main rendering loop
    glutMainLoop();
}

// ========================================================

void GLCanvas::InitLight() {
    // Set the last component of the position to 0 to indicate
    // a directional light source
    
    GLfloat position[4] = { 30,30,100, 1};
    GLfloat diffuse[4] = { 0.75,0.75,0.75,1};
    GLfloat specular[4] = { 0,0,0,1};
    GLfloat ambient[4] = { 0.2, 0.2, 0.2, 1.0 };
    
    GLfloat zero[4] = {0,0,0,0};
    glLightfv(GL_LIGHT1, GL_POSITION, position);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT1, GL_AMBIENT, zero);
    glEnable(GL_LIGHT1);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glEnable(GL_COLOR_MATERIAL);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    
    GLfloat spec_mat[4] = {1,1,1,1};
    float glexponent = 30;
    glMaterialfv(GL_FRONT, GL_SHININESS, &glexponent);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spec_mat);
    
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    float back_color[] = { 0.2,0.8,0.8,1};
    glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, back_color);
    glEnable(GL_LIGHT1);
}


void GLCanvas::display(void) {
    glDrawBuffer(GL_BACK);
    
    const Vec3f &bg = mesh->getBackgroundColor();
    // Clear the display buffer, set it to the background color
    glClearColor(bg.r(),bg.g(),bg.b(),0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set the camera parameters
    mesh->getCamera()->glInit(args->width, args->height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    mesh->getCamera()->glPlaceCamera();
    InitLight(); // light will be a headlamp!
    
    if (args->intersect_backfacing)
        glDisable(GL_CULL_FACE);
    else
        glEnable(GL_CULL_FACE);
    
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    
    glCallList(display_list_index);
    HandleGLError(); 
    
    // Swap the back buffer with the front buffer to display
    // the scene
    glutSwapBuffers();
}

// ========================================================
// Callback function for window resize
// ========================================================

void GLCanvas::reshape(int w, int h) {
    args->width = w;
    args->height = h;
    
    // Set the OpenGL viewport to fill the entire window
    glViewport(0, 0, (GLsizei)args->width, (GLsizei)args->height);
    
    // Set the camera parameters to reflect the changes
    mesh->getCamera()->glInit(args->width, args->height);
}

// ========================================================
// Callback function for mouse click or release
// ========================================================

void GLCanvas::mouse(int button, int state, int x, int y) {
    args->raytracing_animation = false;
    // Save the current state of the mouse.  This will be
    // used by the 'motion' function
    mouseButton = button;
    mouseX = x;
    mouseY = y;
    shiftPressed = glutGetModifiers() & GLUT_ACTIVE_SHIFT;
    controlPressed = glutGetModifiers() & GLUT_ACTIVE_CTRL;
    altPressed = glutGetModifiers() & GLUT_ACTIVE_ALT;
}

// ========================================================
// Callback function for mouse drag
// ========================================================

void GLCanvas::motion(int x, int y) {
    // Control or Shift or Alt pressed = zoom
    // (don't move the camera, just change the angle or image size)
    if (controlPressed || shiftPressed || altPressed) {
        mesh->getCamera()->zoomCamera(mouseY-y);
    }
    // Left button = rotation
    // (rotate camera around the up and horizontal vectors)
    else if (mouseButton == GLUT_LEFT_BUTTON) {
        mesh->getCamera()->rotateCamera(0.005*(mouseX-x), 0.005*(mouseY-y));
    }
    // Middle button = translation
    // (move camera perpendicular to the direction vector)
    else if (mouseButton == GLUT_MIDDLE_BUTTON) {
        mesh->getCamera()->truckCamera(mouseX-x, y-mouseY);
    }
    // Right button = dolly or zoom
    // (move camera along the direction vector)
    else if (mouseButton == GLUT_RIGHT_BUTTON) {
        mesh->getCamera()->dollyCamera(mouseY-y);
    }
    mouseX = x;
    mouseY = y;
    
    // Redraw the scene with the new camera parameters
    glutPostRedisplay();
}

// ========================================================
// Callback function for keyboard events
// ========================================================

void GLCanvas::keyboard(unsigned char key, int x, int y) {
    args->raytracing_animation = false;
    switch (key) {
            // RAYTRACING STUFF
        case 'r':  case 'R':
            // animate raytracing of the scene
            args->gather_indirect=false;
            args->raytracing_animation = !args->raytracing_animation;
            if (args->raytracing_animation) {
                raytracing_skip = std::max(args->width,args->height) / 10;
                if (raytracing_skip % 2 == 0) raytracing_skip++;
                assert (raytracing_skip >= 1);
                raytracing_x = raytracing_skip/2;
                raytracing_y = raytracing_skip/2;
                display(); // clear out any old rendering
                printf ("raytracing animation started, press 'R' to stop\n");
            } else
                printf ("raytracing animation stopped, press 'R' to start\n");    
            break;
        case 't':  case 'T': {
            // visualize the ray tree for the pixel at the current mouse position
            int i = x;
            int j = glutGet(GLUT_WINDOW_HEIGHT)-y;
            RayTree::Activate();
            raytracing_skip = 1;
            TraceRay(i,j);
            RayTree::Deactivate();
            // redraw
            Render();
            break; }
        case 'l':  case 'L': { 
            // toggle photon rendering
            args->render_photons = !args->render_photons;
            Render();
            break; }
        case 'k':  case 'K': { 
            // toggle photon rendering
            args->render_kdtree = !args->render_kdtree;
            Render();
            break; }
        case 'p':  case 'P': { 
            // toggle photon rendering
            photon_mapping->TracePhotons();
            Render();
            break; }
        case 'd': case 'D': {
            Vec3f en;
            int num_prims = mesh->numPrimitives();
            for (int i = 0; i < num_prims; ++i) {
                std::vector<Photon> phos = mesh->getPrimitive(i)->getPhotons();
                int q = mesh->getPrimitive(i)->getPhotons().size();
                std::cout << "Primitive " << i << " has " << q << " photons\n";
                for (int j = 0; j < q; ++j) {
                    en += phos[j].getEnergy();
                }
                std::cout << "en is " << en << "\n";
                double intensity = 0;
                for (int j = 0; j < 3; ++j) {
                    intensity += en[j];
                }
                intensity /= 3.0;
                if (intensity > 1.0) {
                    intensity = 1.0;
                }
                mesh->getPrimitive(i)->setIntensity(intensity);
                std::cout << "intensity is " << intensity << "\n";
            }
            args->render_photons = false;
            args->render_kdtree = false;
            args->render_energy = !args->render_energy;
            Render();
            break;
        }
        case 'g':  case 'G': { 
            args->gather_indirect = true;
            args->raytracing_animation = !args->raytracing_animation;
            if (args->raytracing_animation) {
                raytracing_skip = std::max(args->width,args->height) / 10;
                if (raytracing_skip % 2 == 0) raytracing_skip++;
                assert (raytracing_skip >= 1);
                raytracing_x = raytracing_skip/2;
                raytracing_y = raytracing_skip/2;
                display(); // clear out any old rendering
                printf ("photon mapping animation started, press 'G' to stop\n");
            } else
                printf ("photon mapping animation stopped, press 'G' to start\n");    
            break; 
        }
            
            // RADIOSITY STUFF
        case ' ': 
            // a single step of radiosity
            radiosity->Iterate();
            Render();
            break;
        case 'a': case 'A':
            // animate radiosity solution
            args->radiosity_animation = !args->radiosity_animation;
            if (args->radiosity_animation) 
                printf ("radiosity animation started, press 'A' to stop\n");
            else
                printf ("radiosity animation stopped, press 'A' to start\n");
            break;
        case 's': case 'S':
            // subdivide the mesh for radiosity
            radiosity->Cleanup();
            radiosity->getMesh()->Subdivision();
            radiosity->Reset();
            Render();
            break;
        case 'c': case 'C':
            // clear the radiosity solution
            radiosity->Reset();
            Render();
            break;
            
            // VISUALIZATIONS
        case 'w':  case 'W':
            // render wireframe mode
            args->wireframe = !args->wireframe;
            Render();
            break;
        case 'v': case 'V':
            // toggle the different visualization modes
            args->render_mode = RENDER_MODE((args->render_mode+1)%NUM_RENDER_MODES);
            switch (args->render_mode) {
                case RENDER_MATERIALS: printf ("RENDER_MATERIALS\n"); break;
                case RENDER_LIGHTS: printf ("RENDER_LIGHTS\n"); break;
                case RENDER_UNDISTRIBUTED: printf ("RENDER_UNDISTRIBUTED\n"); break;
                case RENDER_ABSORBED: printf ("RENDER_ABSORBED\n"); break;
                case RENDER_RADIANCE: printf ("RENDER_RADIANCE\n"); break;
                case RENDER_FORM_FACTORS: printf ("RENDER_FORM_FACTORS\n"); break;
                default: assert(0); }
            Render();
            break;
        case 'i':  case 'I':
            // interpolate patch illumination values
            args->interpolate = !args->interpolate;
            Render();
            break;
        case 'b':  case 'B':
            // interpolate patch illumination values
            args->intersect_backfacing = !args->intersect_backfacing;
            Render();
            break;
            
        case 'e':  case 'E': { 
            // output current camera position (eye)
            std::cout << *mesh->getCamera() << std::endl;
            break; }
            
        case 'q':  case 'Q':
            // quit
            delete GLCanvas::photon_mapping;
            delete GLCanvas::raytracer;
            delete GLCanvas::radiosity;
            delete GLCanvas::mesh;
            exit(0);
            break;
        default:
            printf("UNKNOWN KEYBOARD INPUT  '%c'\n", key);
    }
}


// trace a ray through pixel (i,j) of the image an return the color
Vec3f GLCanvas::TraceRay(int i, int j) {
    // compute and set the pixel color
    int max_d = std::max(args->width,args->height);
    
    if (args->num_antialias_samples <= 1) {
        assert (i >= 0 && i < args->width);
        assert (j >= 0 && j < args->height);
        // convert integer pixel coordinates from (0,1)->(width-1,height-1)
        // into floating point coordinates (0,0)->(1,1)
        double x = (i+0.5-args->width/2.0)/double(max_d)+0.5;
        double y = (j+0.5-args->height/2.0)/double(max_d)+0.5;
        assert (x >= 0.0 && x <= 1.0);
        assert (y >= 0.0 && y <= 1.0);
        Ray r = mesh->getCamera()->generateRay(x,y);
        Hit hit;
        Vec3f color = raytracer->TraceRay(r,hit);
        RayTree::AddMainSegment(r,0,hit.getT());
        return color;
    } else {
        // ==========================================
        // ASSIGNMENT:  IMPLEMENT ANTIALIASING
        // ==========================================
        Vec3f color;
        
        int N = args->num_antialias_samples;
        
        for (int h = 0; h < N; ++h) {
            assert (i >= 0 && i < args->width);
            assert (j >= 0 && j < args->height);
            
            double rand = GLOBAL_mtrand.rand();
            double rotation = 2 * M_PI * rand;
            
            double away = (double)h / N;
            double around = h * rotation;
            
            // convert integer pixel coordinates from (0,1)->(width-1,height-1)
            // into floating point coordinates (0,0)->(1,1)
            double I = i;
            double J = j;
            
            I += (std::cos(around) / 1.0) * away;
            J += (std::sin(around) / 1.0) * away;
            
            double x = (I+0.5-args->width/2.0)/double(max_d)+0.5;
            double y = (J+0.5-args->height/2.0)/double(max_d)+0.5;
            
            if (x < 0.0) {
                x = 0.0;
            } else if (x > 1.0) {
                x = 1.0;
            }
            if (y < 0.0) {
                y = 0.0;
            } else if (y > 1.0) {
                y = 1.0;
            }
            Ray r = mesh->getCamera()->generateRay(x,y);
            Hit hit;
            color += raytracer->TraceRay(r,hit) * rand;
            RayTree::AddMainSegment(r,0,hit.getT());
            //return color;
        }
        
        color /= N;
       
        // return the average color
        return color;        
    }
}



// Scan through the image from the lower left corner across each row
// and then up to the top right.  Initially the image is sampled very
// coarsely.  Increment the static variables that track the progress
// through the scans
int GLCanvas::DrawPixel() {
    if (raytracing_x >= args->width) {
        raytracing_x = raytracing_skip/2;
        raytracing_y += raytracing_skip;
    }
    if (raytracing_y >= args->height) {
        if (raytracing_skip == 1) return 0;
        raytracing_skip = raytracing_skip / 2;
        if (raytracing_skip % 2 == 0) raytracing_skip++;
        assert (raytracing_skip >= 1);
        raytracing_x = raytracing_skip/2;
        raytracing_y = raytracing_skip/2;
        glEnd();
        glPointSize(raytracing_skip);
        glBegin(GL_POINTS);
    }
    
    // compute the color and position of intersection
    Vec3f color= TraceRay(raytracing_x, raytracing_y);
    double r = linear_to_srgb(color.x());
    double g = linear_to_srgb(color.y());
    double b = linear_to_srgb(color.z());
    glColor3f(r,g,b);
    double x = 2 * (raytracing_x/double(args->width)) - 1;
    double y = 2 * (raytracing_y/double(args->height)) - 1;
    glVertex3f(x,y,-1);
    raytracing_x += raytracing_skip;
    return 1;
}


void GLCanvas::idle() {
    if (args->radiosity_animation) {
        double undistributed = radiosity->Iterate();
        if (undistributed < 0.001) {
            args->radiosity_animation = false;
            printf ("undistributed < 0.001, animation stopped\n");
        }
        Render();
    }
    if (args->raytracing_animation) {
        // draw 100 pixels and then refresh the screen and handle any user input
        glDisable(GL_LIGHTING);
        glDrawBuffer(GL_FRONT);
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glPointSize(raytracing_skip);
        glBegin(GL_POINTS);
        for (int i = 0; i < 100; i++) {
            if (!DrawPixel()) {
                args->raytracing_animation = false;
                break;
            }
        }
        glEnd();
        glFlush();
    }
}


void GLCanvas::Render() {
    glNewList(display_list_index, GL_COMPILE_AND_EXECUTE);
    // =========================================================
    // put your GL drawing calls inside the display list for efficiency
    radiosity->Paint(args);
    // Draw the ray tree
    glDisable(GL_LIGHTING);
    RayTree::paint();
    glEnable(GL_LIGHTING);
    // =========================================================
    if (args->render_photons) {
        photon_mapping->RenderPhotons();
    }
    if (args->render_kdtree) {
        photon_mapping->RenderKDTree();
    }
    if (args->render_energy) {
        photon_mapping->RenderEnergy();
    }
    
    glEndList();
    glutPostRedisplay();
}


// ========================================================
// ========================================================

int HandleGLError() {
    GLenum error;
    int i = 0;
    while ((error = glGetError()) != GL_NO_ERROR) {
        printf ("GL ERROR(%d):  %s\n", i, gluErrorString(error));
        i++;
    }
    if (i == 0) return 1;
    return 0;
}
