#ifndef _ELEMENT_H_
#define _ELEMENT_H_

#include <Eigen/Core>
#include "HalfEdgeMesh.h"

class Face;

class Element{
private:

protected:
	int Id;
	Eigen::MatrixXd V;
	Eigen::MatrixXi F;
	int NumVertex;
	int NumFace;

public:
	Eigen::MatrixXd GetV() const;
	Eigen::MatrixXi GetF() const;
	int GetNumVertex() const;
	int GetNumFace() const;
};


class TriPrism : public Element{
private:
public:
	double Height;
	TriPrism(int, Face*, double);
};


#endif
