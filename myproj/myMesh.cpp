#include "myMesh.h"
#include "myVector3D.h"
#include <GL/glew.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <utility>
#include <cmath>

using namespace std;

myMesh::myMesh(void) {}

myMesh::~myMesh(void) { clear(); }

void myMesh::clear() {
  for (unsigned int i = 0; i < vertices.size(); i++)
    if (vertices[i])
      delete vertices[i];
  for (unsigned int i = 0; i < halfedges.size(); i++)
    if (halfedges[i])
      delete halfedges[i];
  for (unsigned int i = 0; i < faces.size(); i++)
    if (faces[i])
      delete faces[i];

  vector<myVertex *> empty_vertices;
  vertices.swap(empty_vertices);
  vector<myHalfedge *> empty_halfedges;
  halfedges.swap(empty_halfedges);
  vector<myFace *> empty_faces;
  faces.swap(empty_faces);
}

void myMesh::checkMesh() {
  vector<myHalfedge *>::iterator it;
  for (it = halfedges.begin(); it != halfedges.end(); it++) {
    if ((*it)->twin == NULL)
      break;
  }
  if (it != halfedges.end())
    cout << "Error! Not all edges have their twins!\n";
  else
    cout << "Each edge has a twin!\n";
}

bool myMesh::readFile(std::string filename) {
  string s, t, u;
  vector<int> faceids;
  myHalfedge **hedges;

  ifstream fin(filename);
  if (!fin.is_open()) {
    cout << "Unable to open file!\n";
    return false;
  }
  name = filename;

  map<pair<int, int>, myHalfedge *> twin_map;
  map<pair<int, int>, myHalfedge *>::iterator it;

  while (getline(fin, s)) {
    stringstream myline(s);
    myline >> t;
    if (t == "g") {
    } else if (t == "v") {
      float x, y, z;
      myline >> x >> y >> z;
      cout << "v " << x << " " << y << " " << z << endl;
      myPoint3D *p = new myPoint3D(x, y, z);
      myVertex *v = new myVertex();
      v->point = p;
      vertices.push_back(v);
    } else if (t == "mtllib") {
    } else if (t == "usemtl") {
    } else if (t == "s") {
    } else if (t == "f") {
      //-----------
      // HINTS
      //-----------
      faceids.clear();
      while (myline >> u) // read indices of vertices from a face into a
                          // container - it helps to access them later
        faceids.push_back(atoi((u.substr(0, u.find("/"))).c_str()) - 1);
      if (faceids.size() < 3) // ignore degenerate faces
        continue;

      hedges =
          new myHalfedge *[faceids.size()]; // allocate the array for storing
                                            // pointers to half-edges
      for (unsigned int i = 0; i < faceids.size(); i++)
        hedges[i] = new myHalfedge(); // pre-allocate new half-edges

      myFace *f = new myFace();         // allocate the new face
      f->adjacent_halfedge = hedges[0]; // connect the face with incident edge
      for (unsigned int i = 0; i < faceids.size(); i++) {
        int iplusone = (i + 1) % faceids.size();
        int iminusone = (i - 1 + faceids.size()) % faceids.size();

        // YOUR CODE COMES HERE!

        // connect prevs, and next
        hedges[i]->next = hedges[iplusone];
        hedges[i]->prev = hedges[iminusone];
        hedges[i]->adjacent_face = f;

        // search for the twins using twin_map
        int v_start = faceids[i];
        int v_end = faceids[iplusone];

        hedges[i]->source = vertices[v_start];
        vertices[v_start]->originof = hedges[i];

        auto it = twin_map.find(make_pair(v_end, v_start));
        if (it != twin_map.end()) {
          hedges[i]->twin = it->second;
          it->second->twin = hedges[i];
        } else {
          twin_map[make_pair(v_start, v_end)] = hedges[i];
        }

        // set originof
        // push edges to halfedges in myMesh
        halfedges.push_back(hedges[i]);
      }
      delete[] hedges;
      // push faces to faces in myMesh
      faces.push_back(f);
      cout << "f";
      while (myline >> u)
        cout << " " << atoi((u.substr(0, u.find("/"))).c_str());
      cout << endl;
    }
  }
  checkMesh();
  normalize();
  return true;
}

void myMesh::computeNormals() {
  for (unsigned int i = 0; i < faces.size(); i++)
    faces[i]->computeNormal();

  for (unsigned int i = 0; i < vertices.size(); i++) {
    vertices[i]->computeNormal();
  }
}

void myMesh::normalize() {
  if (vertices.size() < 1)
    return;

  int tmpxmin = 0, tmpymin = 0, tmpzmin = 0, tmpxmax = 0, tmpymax = 0,
      tmpzmax = 0;

  for (unsigned int i = 0; i < vertices.size(); i++) {
    if (vertices[i]->point->X < vertices[tmpxmin]->point->X)
      tmpxmin = i;
    if (vertices[i]->point->X > vertices[tmpxmax]->point->X)
      tmpxmax = i;

    if (vertices[i]->point->Y < vertices[tmpymin]->point->Y)
      tmpymin = i;
    if (vertices[i]->point->Y > vertices[tmpymax]->point->Y)
      tmpymax = i;

    if (vertices[i]->point->Z < vertices[tmpzmin]->point->Z)
      tmpzmin = i;
    if (vertices[i]->point->Z > vertices[tmpzmax]->point->Z)
      tmpzmax = i;
  }

  double xmin = vertices[tmpxmin]->point->X, xmax = vertices[tmpxmax]->point->X,
         ymin = vertices[tmpymin]->point->Y, ymax = vertices[tmpymax]->point->Y,
         zmin = vertices[tmpzmin]->point->Z, zmax = vertices[tmpzmax]->point->Z;

  double scale = (xmax - xmin) > (ymax - ymin) ? (xmax - xmin) : (ymax - ymin);
  scale = scale > (zmax - zmin) ? scale : (zmax - zmin);

  for (unsigned int i = 0; i < vertices.size(); i++) {
    vertices[i]->point->X -= (xmax + xmin) / 2;
    vertices[i]->point->Y -= (ymax + ymin) / 2;
    vertices[i]->point->Z -= (zmax + zmin) / 2;

    vertices[i]->point->X /= scale;
    vertices[i]->point->Y /= scale;
    vertices[i]->point->Z /= scale;
  }
}

void myMesh::splitFaceTRIS(myFace *f, myPoint3D *p) { 
  /**** TODO ****/ 
  myVertex *vP = new myVertex;
  vP->point = p;
  vertices.push_back(vP);

  // 6 new halfedges (e0, e1, e2 + their twins) and 2 new faces (f1, f2)
  // old halfedges of the face f
  myHalfedge *h0 = f->adjacent_halfedge;
  myHalfedge *h1 = h0->next;
  myHalfedge *h2 = h1->next;

  // 6 new halfedges (2 per triangle, going to and from p)
  myHalfedge *e0 = new myHalfedge(); 
  myHalfedge *e0_twin = new myHalfedge(); 
  myHalfedge *e1 = new myHalfedge(); 
  myHalfedge *e1_twin = new myHalfedge(); 
  myHalfedge *e2 = new myHalfedge(); 
  myHalfedge *e2_twin = new myHalfedge(); 

  // sources
  e0->source = h1->source;
  e0_twin->source = vP;
  e1->source = h2->source;
  e1_twin->source = vP;
  e2->source = h0->source;
  e2_twin->source = vP;

  // twin 
  e0->twin = e0_twin; 
  e0_twin->twin = e0;
  e1->twin = e1_twin; 
  e1_twin->twin = e1;
  e2->twin = e2_twin; 
  e2_twin->twin = e2;

  //new faces
  myFace *f1 = new myFace();
  myFace *f2 = new myFace();

  // triangle 1
  f->adjacent_halfedge = h0;
  h0->next = e0; e0->prev = h0;
  e0->next = e0_twin; e0_twin->prev = e0;
  e0_twin->next = h0; h0->prev = e0_twin;
  h0->adjacent_face = f;
  e0->adjacent_face = f;
  e0_twin->adjacent_face = f;

  // triangle 2
  f1->adjacent_halfedge = h1;
  h1->next = e1; e1->prev = h1;
  e1->next = e1_twin; e1_twin->prev = e1;
  e1_twin->next = h1; h1->prev = e1_twin;
  h1->adjacent_face = f1;
  e1->adjacent_face = f1;
  e1_twin->adjacent_face = f1;

  // triangle 3
  f2->adjacent_halfedge = h2;
  h2->next = e2; e2->prev = h2;
  e2->next = e2_twin; e2_twin->prev = e2;
  e2_twin->next = h2; h2->prev = e2_twin;
  h2->adjacent_face = f2;
  e2->adjacent_face = f2;
  e2_twin->adjacent_face = f2;

  //twin
  e0->twin = e2_twin; 
  e2_twin->twin = e0;
  e1->twin = e0_twin; 
  e0_twin->twin = e1;
  e2->twin = e1_twin; 
  e1_twin->twin = e2;

  vP->originof = e0;

  halfedges.push_back(e0); halfedges.push_back(e0_twin);
  halfedges.push_back(e1); halfedges.push_back(e1_twin);
  halfedges.push_back(e2); halfedges.push_back(e2_twin);
  faces.push_back(f1);
  faces.push_back(f2);
}

void myMesh::splitEdge(myHalfedge *e1, myPoint3D *p) { 
  /**** TODO ****/ 
  myHalfedge *e2 = new myHalfedge;
  myHalfedge *e2_twin = new myHalfedge;

  myVertex *vP = new myVertex;
  vP->point = p;
  vertices.push_back(vP);

  myHalfedge *old_twin = e1->twin;
  myHalfedge *old_e1_next = e1->next;
  myHalfedge *old_twin_next = old_twin->next;

  // e2
  e2->source = vP;
  e2->next = old_e1_next;
  e2->prev = e1;
  e2->twin = old_twin;
  e2->adjacent_face = e1->adjacent_face;
  old_e1_next->prev = e2;

  // old_twin
  // source = v2
  old_twin->next = e2_twin;
  old_twin->twin = e2;

  // e2_twin 
  e2_twin->source = vP;
  e2_twin->next = old_twin_next;
  e2_twin->prev = old_twin;
  e2_twin->twin = e1;
  e2_twin->adjacent_face = old_twin->adjacent_face;
  old_twin_next->prev = e2_twin;

  // update e1
  e1->next = e2;
  e1->twin = e2_twin;
  // e1->source = v1

  vP->originof = e2;
  halfedges.push_back(e2);
  halfedges.push_back(e2_twin);
}

//temp
int myMesh::ringSize(myFace *f) {
    int count = 0;
    myHalfedge *cur = f->adjacent_halfedge;
    do { count++; cur = cur->next; } while (cur != f->adjacent_halfedge);
    return count;
}

void myMesh::splitFaceQUADS(myFace *f, myPoint3D *p) { 
  /**** TODO ****/ 

  myVertex *vP = new myVertex;
  vP->point = p;
  vertices.push_back(vP);

  // + 3 faces and + 16 new halfedges 
  //old halfedges of the face f
  myHalfedge *h0 = f->adjacent_halfedge;
  myHalfedge *h1 = h0->next;
  myHalfedge *h2 = h1->next;
  myHalfedge *h3 = h2->next;

  //splitting the edges of the quad 
  myPoint3D *m0 = new myPoint3D(
      (h0->source->point->X + h1->source->point->X) / 2,
      (h0->source->point->Y + h1->source->point->Y) / 2,
      (h0->source->point->Z + h1->source->point->Z) / 2
  );
  //cout << "before any split: " << ringSize(f) << endl; debug
  splitEdge(h0, m0);
  myHalfedge *s0 = h0->next;
  //cout << "after split 0: " << ringSize(f) << endl; debug 
  myPoint3D *m1 = new myPoint3D(
      (h1->source->point->X + h2->source->point->X) / 2,
      (h1->source->point->Y + h2->source->point->Y) / 2,
      (h1->source->point->Z + h2->source->point->Z) / 2
  );
  splitEdge(h1, m1);
  myHalfedge *s1 = h1->next;
  myPoint3D *m2 = new myPoint3D(
      (h2->source->point->X + h3->source->point->X) / 2,
      (h2->source->point->Y + h3->source->point->Y) / 2,
      (h2->source->point->Z + h3->source->point->Z) / 2
  );
  splitEdge(h2, m2);
  myHalfedge *s2 = h2->next;
  myPoint3D *m3 = new myPoint3D(
      (h3->source->point->X + h0->source->point->X) / 2,
      (h3->source->point->Y + h0->source->point->Y) / 2,
      (h3->source->point->Z + h0->source->point->Z) / 2
  );
  splitEdge(h3, m3);
  myHalfedge *s3 = h3->next;

  // 
    myHalfedge *in0 = new myHalfedge(); // s0->source → vP
    myHalfedge *out0 = new myHalfedge(); // vP → h0->source
    myHalfedge *in1 = new myHalfedge(); // s1->source → vP
    myHalfedge *out1 = new myHalfedge(); // vP → h1->source
    myHalfedge *in2 = new myHalfedge(); // s2->source → vP
    myHalfedge *out2 = new myHalfedge(); // vP → h2->source
    myHalfedge *in3 = new myHalfedge(); // s3->source → vP
    myHalfedge *out3 = new myHalfedge(); // vP → h3->source

    // source
    in0->source = s0->source;
    out0->source = vP;
    in1->source = s1->source;
    out1->source = vP;
    in2->source = s2->source;
    out2->source = vP;
    in3->source = s3->source;
    out3->source = vP;

    // twin
    in0->twin = out2; out2->twin = in0;
    in1->twin = out3; out3->twin = in1;
    in2->twin = out0; out0->twin = in2;
    in3->twin = out1; out1->twin = in3;

    // 3 new faces + f
    myFace *f1 = new myFace();
    myFace *f2 = new myFace();
    myFace *f3 = new myFace();

    // quad 0
    f->adjacent_halfedge = h0;
    h0->next = s0;   
    s0->prev = h0;
    s0->next = in0;  
    in0->prev = s0;
    in0->next = out0; 
    out0->prev = in0;
    out0->next = h0; 
    h0->prev = out0;
    h0->adjacent_face = f;
    s0->adjacent_face = f;
    in0->adjacent_face = f;
    out0->adjacent_face = f;

    // quad 1
    f1->adjacent_halfedge = h1;
    h1->next = s1;   
    s1->prev = h1;
    s1->next = in1;  
    in1->prev = s1;
    in1->next = out1; 
    out1->prev = in1;
    out1->next = h1; 
    h1->prev = out1;
    h1->adjacent_face = f1;
    s1->adjacent_face = f1;
    in1->adjacent_face = f1;
    out1->adjacent_face = f1;

    // quad 2
    f2->adjacent_halfedge = h2;
    h2->next = s2;   
    s2->prev = h2;
    s2->next = in2;  
    in2->prev = s2;
    in2->next = out2; 
    out2->prev = in2;
    out2->next = h2; 
    h2->prev = out2;
    h2->adjacent_face = f2;
    s2->adjacent_face = f2;
    in2->adjacent_face = f2;
    out2->adjacent_face = f2;

    // quad 3
    f3->adjacent_halfedge = h3;
    h3->next = s3;   s3->prev = h3;
    s3->next = in3;  in3->prev = s3;
    in3->next = out3; out3->prev = in3;
    out3->next = h3; h3->prev = out3;
    h3->adjacent_face = f3;
    s3->adjacent_face = f3;
    in3->adjacent_face = f3;
    out3->adjacent_face = f3;

    vP->originof = out0;

    halfedges.push_back(in0);  
    halfedges.push_back(out0);
    halfedges.push_back(in1);  
    halfedges.push_back(out1);
    halfedges.push_back(in2);  
    halfedges.push_back(out2);
    halfedges.push_back(in3);  
    halfedges.push_back(out3);
    faces.push_back(f1);
    faces.push_back(f2);
    faces.push_back(f3);
}

void myMesh::subdivisionCatmullClark() {
  /**** TODO ****/

  splitFaceQUADS(faces[0], new myPoint3D(1, 1, 0));
}

void myMesh::surfaceOfRevolution(const std::vector<myPoint3D> &profile, int slices) {
  /**** TODO ****/
  if (profile.size() < 2 || slices < 3)
    return;

  int numPoints = profile.size();

  std::vector<myVertex *> generated_verts;

  for (int s = 0; s < slices; s++) {
    double theta = s * (2.0 * M_PI / slices);

    for (int i = 0; i < numPoints; i++) {
      double px = profile[i].X;
      double py = profile[i].Y;
      double pz = profile[i].Z;

      double nx = px * cos(theta) - pz * sin(theta);
      double ny = py;
      double nz = px * sin(theta) + pz * cos(theta);

      myVertex *v = new myVertex();
      v->point = new myPoint3D(nx, ny, nz);

      vertices.push_back(v);
      generated_verts.push_back(v);
    }
  }

  std::map<std::pair<int, int>, myHalfedge *> twin_map;

  auto getIndex = [&](int s, int i) { return s * numPoints + i; };

  for (int s = 0; s < slices; s++) {
    int next_s = (s + 1) % slices;

    for (int i = 0; i < numPoints - 1; i++) {

      int v0_idx = getIndex(s, i);
      int v1_idx = getIndex(next_s, i);
      int v2_idx = getIndex(next_s, i + 1);
      int v3_idx = getIndex(s, i + 1);

      std::vector<int> faceids = {v0_idx, v1_idx, v2_idx, v3_idx};

      myFace *f = new myFace();
      myHalfedge **hedges = new myHalfedge *[4];
      for (int j = 0; j < 4; j++)
        hedges[j] = new myHalfedge();

      f->adjacent_halfedge = hedges[0];

      for (int j = 0; j < 4; j++) {
        int jplusone = (j + 1) % 4;
        int jminusone = (j - 1 + 4) % 4;

        hedges[j]->next = hedges[jplusone];
        hedges[j]->prev = hedges[jminusone];
        hedges[j]->adjacent_face = f;

        int v_start = faceids[j];
        int v_end = faceids[jplusone];

        hedges[j]->source = generated_verts[v_start];
        generated_verts[v_start]->originof = hedges[j];

        auto it = twin_map.find(std::make_pair(v_end, v_start));
        if (it != twin_map.end()) {
          hedges[j]->twin = it->second;
          it->second->twin = hedges[j];
        } else {
          twin_map[std::make_pair(v_start, v_end)] = hedges[j];
        }
        halfedges.push_back(hedges[j]);
      }

      delete[] hedges;
      faces.push_back(f);
    }
  }
}

void myMesh::triangulate() {
  /**** TODO ****/
  vector<myFace *> remaining_faces(faces.begin(), faces.end());
  for (myFace *f : remaining_faces) {
    while (triangulate(f)) {
      //Ear clipping 
      //normal fix since the triangulate wasn't working well for back faces
      myVector3D face_normal(0, 0, 0);
      myHalfedge *cur = f->adjacent_halfedge;
      do {
          myPoint3D *c = cur->source->point;
          myPoint3D *n = cur->next->source->point;
          face_normal.dX += (c->Y - n->Y) * (c->Z + n->Z);
          face_normal.dY += (c->Z - n->Z) * (c->X + n->X);
          face_normal.dZ += (c->X - n->X) * (c->Y + n->Y);
          cur = cur->next;
      } while (cur != f->adjacent_halfedge);


      vector<myHalfedge *> face_he;
      myHalfedge *h0 = f->adjacent_halfedge;
      myHalfedge *he = f->adjacent_halfedge;

      do {
        myPoint3D *a = he->source->point;
        myPoint3D *b = he->next->source->point;
        myPoint3D *c = he->next->next->source->point;
        
        myVector3D ab(b->X-a->X, b->Y-a->Y, b->Z-a->Z);
        myVector3D bc(c->X-b->X, c->Y-b->Y, c->Z-b->Z);
        myVector3D cross; 
        
        cross.crossproduct(ab, bc);
        double dot = cross.dX*face_normal.dX + cross.dY*face_normal.dY + cross.dZ*face_normal.dZ;

        if (dot > 0) {
            bool isValidEar = true;
            myHalfedge *temp = he->next->next->next;
            while (temp != he) {
                myPoint3D *p = temp->source->point;
                myVector3D ap(p->X-a->X, p->Y-a->Y, p->Z-a->Z);
                myVector3D bp(p->X-b->X, p->Y-b->Y, p->Z-b->Z);
                myVector3D cp(p->X-c->X, p->Y-c->Y, p->Z-c->Z);
                myVector3D c1; 
                c1.crossproduct(ab, ap);
                myVector3D edge_bc(c->X-b->X, c->Y-b->Y, c->Z-b->Z);
                myVector3D c2; 
                c2.crossproduct(edge_bc, bp);
                myVector3D ca(a->X-c->X, a->Y-c->Y, a->Z-c->Z);
                myVector3D c3; 
                c3.crossproduct(ca, cp);

                double d1 = c1.dX*face_normal.dX + c1.dY*face_normal.dY + c1.dZ*face_normal.dZ;
                double d2 = c2.dX*face_normal.dX + c2.dY*face_normal.dY + c2.dZ*face_normal.dZ;
                double d3 = c3.dX*face_normal.dX + c3.dY*face_normal.dY + c3.dZ*face_normal.dZ;
                
                if (d1 >= 0 && d2 >= 0 && d3 >= 0) { 
                  isValidEar = false; break; 
                }
                temp = temp->next;
          }
          if (isValidEar) {
            h0 = he;
            break;
          }
        }
        he = he->next;
      } while (he != f->adjacent_halfedge);

      myHalfedge *h1 = h0->next;
      myHalfedge *h2 = h1->next;

      // new halfedge and it's twin
      myHalfedge *new_he = new myHalfedge;
      new_he->source = h2->source;
      new_he->adjacent_face = f;
      new_he->next = h0;
      new_he->prev = h1;

      myHalfedge *new_he_twin = new myHalfedge;
      new_he_twin->source = h0->source;
      new_he_twin->next = h2;
      // new_he_twin->prev = ;

      new_he->twin = new_he_twin;
      new_he_twin->twin = new_he;

      // triangle
      myFace *new_face = new myFace;
      new_face->adjacent_halfedge = h0;
      h1->next = new_he;

      h0->prev = new_he;

      h0->adjacent_face = new_face;
      h1->adjacent_face = new_face;
      new_he->adjacent_face = new_face;

      // old face
      f->adjacent_halfedge = new_he_twin;
      new_he_twin->next = h2;
      // loop to find last he to link the previous for new_he_twin
      myHalfedge *last = h2;
      while (last->next != h0)
        last = last->next;
      last->next = new_he_twin;
      new_he_twin->prev = last;
      new_he_twin->adjacent_face = f;
      // all the remaining he are all linked to f still
      myHalfedge *remaining_he = h2;
      while (remaining_he != new_he_twin) {
          remaining_he->adjacent_face = f;
          remaining_he = remaining_he->next;
      }

      halfedges.push_back(new_he);
      halfedges.push_back(new_he_twin);
      faces.push_back(new_face);
    }
  }
}

// return false if already triangle, true othewise.
bool myMesh::triangulate(myFace *f) {
  /**** TODO ****/
  if (f->adjacent_halfedge->next->next->next == f->adjacent_halfedge) {
    return false;
  }
  return true;
}
