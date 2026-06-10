#include "myVertex.h"
#include "myFace.h"
#include "myHalfedge.h"
#include "myVector3D.h"

myVertex::myVertex(void) {
  point = NULL;
  originof = NULL;
  normal = new myVector3D(1.0, 1.0, 1.0);
}

myVertex::~myVertex(void) {
  if (point)
    delete point;
  if (normal)
    delete normal;
}

void myVertex::computeNormal() {
  /**** TODO ****/
  myHalfedge *step = originof;
  normal->dX = normal->dY = normal->dZ = 0.0;
  int counter = 0;
  do {
    myVector3D *fn = step->adjacent_face->normal;
    *normal += *fn;
    counter++;
    step = step->twin->next;
  } while (step != originof);
  normal->dX /= counter;
  normal->dY /= counter;
  normal->dZ /= counter;
}
