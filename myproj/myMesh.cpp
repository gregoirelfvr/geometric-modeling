#include "myMesh.h"
#include "myVector3D.h"
#include <GL/glew.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <utility>

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

void myMesh::splitFaceTRIS(myFace *f, myPoint3D *p) { /**** TODO ****/ }

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

  // new halfedges
  myHalfedge *e2 = new myHalfedge();
  myHalfedge *e2_twin = new myHalfedge();

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

void myMesh::splitFaceQUADS(myFace *f, myPoint3D *p) { /**** TODO ****/ }

void myMesh::subdivisionCatmullClark() { /**** TODO ****/ }

void myMesh::surfaceOfRevolution() { /**** TODO ****/ }

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
            if (isValidEar) { h0 = he; break; }
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
