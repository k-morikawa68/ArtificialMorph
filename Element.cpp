#include <iostream>
#include <Eigen/Core>
#include <Eigen/Dense>
#include "Element.h"

Eigen::MatrixXd Element::GetV() const{
	return V;
}
Eigen::MatrixXi Element::GetF() const{
	return F;
}
int Element::GetNumVertex() const{
	return NumVertex;
}
int Element::GetNumFace() const{
	return NumFace;
}

TriPrism::TriPrism(int id, Face* f, double h){
	Id = id;
	NumVertex = 6;
	NumFace = 8;
	V.resize(NumVertex, 3);
	F.resize(NumFace, 3);
	Height = h;

	Eigen::Vector3d offset(0.0, 0.0, Height);
	HalfEdge* he = f->p_he;
	int i = 0;
	do{
		V.row(i) = he->p_v->Pos.transpose();
		V.row(i+3) = (he->p_v->Pos + offset).transpose();
		he = he->next;
		i++;
	}while(he != f->p_he);

	F.row(0) = Eigen::Vector3i(1, 0, 2);
	F.row(1) = Eigen::Vector3i(3, 4, 5);
	F.row(2) = Eigen::Vector3i(0, 3, 2);
	F.row(3) = Eigen::Vector3i(3, 5, 2);
	F.row(4) = Eigen::Vector3i(1, 2, 5);
	F.row(5) = Eigen::Vector3i(1, 5, 4);
	F.row(6) = Eigen::Vector3i(3, 1, 4);
	F.row(7) = Eigen::Vector3i(3, 0, 1);
}
