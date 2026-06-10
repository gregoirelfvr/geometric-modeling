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

void myMesh::splitEdge(myHalfedge *e1, myPoint3D *p) { /**** TODO ****/ }

void myMesh::splitFaceQUADS(myFace *f, myPoint3D *p) { /**** TODO ****/ }

void myMesh::subdivisionCatmullClark() { /**** TODO ****/ }

void myMesh::triangulate() {
  /**** TODO ****/
  vector<myFace *> remaining_faces(faces.begin(), faces.end());
  for (myFace *f : remaining_faces) {
    while (triangulate(f)) {
      // anchor + part of soon to be triangle
      myHalfedge *h0 = f->adjacent_halfedge;
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
      myHalfedge *last = f->adjacent_halfedge;
      while (last->next != h0)
        last = last->next;
      last->next = new_he_twin;
      new_he_twin->prev = last;
      new_he_twin->adjacent_face = f;
      // all the remaining he are all linked to f still
      // only h0, h1 needed to change

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
