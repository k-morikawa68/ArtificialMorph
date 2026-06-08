#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <unistd.h>
#include "HalfEdgeMesh.h"
#include "Element.h"
#include <igl/write_triangle_mesh.h>
#include <igl/doublearea.h>

std::string InputFileName3d; 
std::string InputFileFormat3d;
int Input2dShapeMode; 
std::string InputFileName2d; 
std::string InputFileFormat2d;

HalfEdgeMesh HEM_3d, HEM_2d, HEM_sep, HEM_tgt;
Eigen::VectorXd AreaChangeRate2dTo3d; 
double MaxAreaChangeRate2dTo3d;
double UniformScalingFactor; 

Eigen::VectorXd TargetShrinkRatePerTriangle; 

double FilmShrinkRate; 

double PrintWidth; 
double Height; 
double AreaOfTargetShape; 


void ReadInput(){
	std::cout << "ReadInput ... " << std::endl;
	std::string fname = "input/input.txt";
	std::ifstream fin(fname);
	if(!fin){
		std::cout << "Error: cannot opne input.txt file." << std::endl;
		exit(1);
	}
	std::string dummy;
	fin >> dummy;	fin >> InputFileName3d;
	fin >> dummy;	fin >> InputFileFormat3d;
	std::cout << "InputFile3d: " << InputFileName3d + "." + InputFileFormat3d << std::endl;

	fin >> dummy;	fin >> Input2dShapeMode;	std::cout << "Input2dShapeMode: " << Input2dShapeMode << std::endl;

	fin >> dummy;	fin >> InputFileName2d;
	fin >> dummy;	fin >> InputFileFormat2d;
	if(Input2dShapeMode == 1){
		std::cout << "InputFile2d: " << InputFileName2d + "." + InputFileFormat2d << std::endl;
	}else{
		std::cout << "InputFile2d: none" << std::endl;
	}

	fin >> dummy;	fin >> FilmShrinkRate;	std::cout << "FilmShrinkRate(Length): " << FilmShrinkRate << std::endl;
	
	fin >> dummy;	fin >> PrintWidth;	std::cout << "PrintWidth(mm): " << PrintWidth << std::endl;
	fin >> dummy;	fin >> Height;	std::cout << "Height(mm): " << Height << std::endl;
	fin >> dummy;	fin >> AreaOfTargetShape;	std::cout << "AreaOfTargetShape(mm^2): " << AreaOfTargetShape << std::endl;

	std::string check;
	fin >> check;
	if(check != "End"){
		std::cout << "Error: invalid input file format." << std::endl;
		exit(1);
	}
	std::cout << "done." << std::endl;
}


void CalcAreaChangeRate2dTo3d(){
	std::cout << "CalcAreaChangeRate2dTo3d ... " << std::flush;
	int n_f = (int)HEM_2d.p_f.size();
	AreaChangeRate2dTo3d.resize(n_f);
	for(int i = 0; i < n_f; i++){
		AreaChangeRate2dTo3d(i) = HEM_3d.p_f[i]->Area / HEM_2d.p_f[i]->Area;
	}
	MaxAreaChangeRate2dTo3d = AreaChangeRate2dTo3d.maxCoeff();

	std::cout << "done." << std::endl;
}

void ScalingPrintRange(){
	std::cout << "ScalingPrintRange ... " << std::endl;
	double max_x = HEM_2d.p_v[0]->Pos(0);
	double min_x = HEM_2d.p_v[0]->Pos(0);
	double max_y = HEM_2d.p_v[0]->Pos(1);
	double min_y = HEM_2d.p_v[0]->Pos(1);
	for(int i = 1; i < HEM_2d.p_v.size(); i++){
		if(HEM_2d.p_v[i]->Pos(0) > max_x) max_x = HEM_2d.p_v[i]->Pos(0);
		if(HEM_2d.p_v[i]->Pos(0) < min_x) min_x = HEM_2d.p_v[i]->Pos(0);
		if(HEM_2d.p_v[i]->Pos(1) > max_y) max_y = HEM_2d.p_v[i]->Pos(1);
		if(HEM_2d.p_v[i]->Pos(1) < min_y) min_y = HEM_2d.p_v[i]->Pos(1);
	}
	double width_x = max_x - min_x;
	double width_y = max_y - min_y;
	double range_scale;
	if(width_x > width_y){
		range_scale = PrintWidth / width_x;
	}else{
		range_scale = PrintWidth / width_y;
	}

	for(int i = 0; i < HEM_2d.p_v.size(); i++){
		HEM_2d.p_v[i]->Pos = range_scale * HEM_2d.p_v[i]->Pos;	
		HEM_3d.p_v[i]->Pos = range_scale * HEM_3d.p_v[i]->Pos;	
	}

	std::cout << "x range: [" << min_x * range_scale << ", " << max_x * range_scale << "]\t" << width_x * range_scale << " mm\n";
	std::cout << "y range: [" << min_y * range_scale << ", " << max_y * range_scale << "]\t" << width_y * range_scale << " mm\n";
	std::cout << "done." << std::endl;
}

void MakeTriangles(){
	std::cout << "MakeTriangles ... " << std::flush;
	int n_f = HEM_sep.p_f.size();
	HEM_sep.CalcFaceAreas();
	
	double sum = 0.0;
	for(int i = 0; i < n_f; i++){
		sum += AreaChangeRate2dTo3d(i) * HEM_sep.p_f[i]->Area;	
	}
	UniformScalingFactor = AreaOfTargetShape / sum;


	TargetShrinkRatePerTriangle = UniformScalingFactor * AreaChangeRate2dTo3d; 
	Eigen::VectorXd scaling;
	scaling.resize(n_f);
	double filmShrinkRate_A = FilmShrinkRate * FilmShrinkRate;
	for(int i = 0; i < n_f; i++){
		scaling(i) = (TargetShrinkRatePerTriangle(i) - filmShrinkRate_A) / (1.0 - filmShrinkRate_A);
		if(TargetShrinkRatePerTriangle(i) < filmShrinkRate_A){
			std::cout << "Warning: target shrink rate " << i << "(= " << TargetShrinkRatePerTriangle(i) << ") is too small to achieve that shrinkage with this shrink film." << std::endl;
			scaling(i) = 0.0;
		}
	}


	for(int i = 0; i < n_f; i++){
		scaling(i) = sqrt(scaling(i));
	}

	HEM_sep.CalcCentroids();
	for(int i = 0; i < n_f; i++){
		HalfEdge* he = HEM_sep.p_f[i]->p_he;
		do{
			Eigen::Vector3d c_to_v = he->p_v->Pos - HEM_sep.p_f[i]->Centroid;
			c_to_v *= scaling(i);
			he->p_v->Pos = HEM_sep.p_f[i]->Centroid + c_to_v;

			he = he->next;
		}while(he != HEM_sep.p_f[i]->p_he);
	}

	std::cout << "done." << std::endl;
}


void WriteTarget3dShape(){
	std::cout << "WriteTarget3dShape ... " << std::flush;
	HEM_3d.CopyHalfEdgeMesh(&HEM_tgt);
	int n_v = HEM_tgt.p_v.size();
	double scaling = sqrt(UniformScalingFactor);
	for(int i = 0; i < n_v; i++){
		HEM_tgt.p_v[i]->Pos = scaling * HEM_tgt.p_v[i]->Pos;
	}
	//HEM_tgt.WriteVTK("target_3d_shape.vtk");
	HEM_tgt.WriteTriangleMesh("target_3d_shape.off");
	std::cout << "done." << std::endl;
}

void FrontMatter(int argc, char **argv){
	if(argc != 2){
		std::cout << "Error: the number of values in console command\n";
		std::cout << "Please put the directory name of workspace.\n"; 
		std::cout << std::endl;
		exit(0);
	}
	char* dirName = argv[1];
	std::cout << "Workspace name : " << dirName << std::endl;
	
	if(chdir(dirName) != 0){ 
		std::cout << "Error: moving the directory of workspace" << std::endl;
		exit(1);
	}
	char dirPath[256];
	if(getcwd(dirPath,256) == NULL){ 
		std::cout << "Error: getting workspace" << std::endl;
		exit(1);
	}
	std::cout << "Workspace path : " << dirPath << std::endl;
}

void ConstructTriPrisms(){
	std::cout << "ConstructTriPrisms ... " << std::flush;

	HalfEdgeMesh hem_ref_sep;
	HEM_tgt.SeparateByTriangle(&hem_ref_sep);
	HEM_sep.CalcCentroids();
	HEM_sep.CalcFaceAreas();

	hem_ref_sep.CalcFaceAreas();

	int n_f = (int)hem_ref_sep.p_f.size();
	for(int i = 0; i < n_f; i++){
		Face* fi = HEM_tgt.p_f[i];
		double scaling = sqrt(HEM_sep.p_f[i]->Area / hem_ref_sep.p_f[i]->Area);
		Eigen::Vector3d v0 = HEM_tgt.p_f[i]->p_he->p_v->Pos;
		Eigen::Vector3d v1 = HEM_tgt.p_f[i]->p_he->next->p_v->Pos;
		Eigen::Vector3d v2 = HEM_tgt.p_f[i]->p_he->prev->p_v->Pos;
		Eigen::Vector3d e1 = v1 - v0;
		Eigen::Vector3d e2 = v2 - v0;
		double len1 = scaling * e1.norm();
		double len2 = scaling * e2.norm();
		double theta = acos(e1.normalized().dot(e2.normalized()));
		hem_ref_sep.p_f[i]->p_he->p_v->Pos(0) = 0.0;
		hem_ref_sep.p_f[i]->p_he->p_v->Pos(1) = 0.0;
		hem_ref_sep.p_f[i]->p_he->p_v->Pos(2) = 0.0;
		hem_ref_sep.p_f[i]->p_he->next->p_v->Pos(0) = len1;
		hem_ref_sep.p_f[i]->p_he->next->p_v->Pos(1) = 0.0;
		hem_ref_sep.p_f[i]->p_he->next->p_v->Pos(2) = 0.0;
		hem_ref_sep.p_f[i]->p_he->prev->p_v->Pos(0) = len2 * cos(theta);
		hem_ref_sep.p_f[i]->p_he->prev->p_v->Pos(1) = len2 * sin(theta);
		hem_ref_sep.p_f[i]->p_he->prev->p_v->Pos(2) = 0.0;

		hem_ref_sep.CalcFaceArea(hem_ref_sep.p_f[i]);

		Eigen::Vector3d translation;
		hem_ref_sep.CalcCentroid(hem_ref_sep.p_f[i]);
		translation = -hem_ref_sep.p_f[i]->Centroid;
		HalfEdge* he = hem_ref_sep.p_f[i]->p_he;
		do{
			he->p_v->Pos = he->p_v->Pos + translation;
			he = he->next;
		}while(he != hem_ref_sep.p_f[i]->p_he);
	}

	for(int i = 0; i < n_f; i++){
		Eigen::MatrixXd jacobi(2,2);
		Eigen::MatrixXd rot_m90(2,2);
		rot_m90 << 0.0, 1.0, -1.0, 0.0;
		Eigen::Vector2d t[3];
		Eigen::Vector2d e[3];
		Face* f_ref = hem_ref_sep.p_f[i];
		Face* f_cur = HEM_sep.p_f[i];
		e[0] = f_ref->p_he->prev->p_v->Pos.head(2) - f_ref->p_he->next->p_v->Pos.head(2);
		e[1] = f_ref->p_he->p_v->Pos.head(2) - f_ref->p_he->prev->p_v->Pos.head(2);
		e[2] = f_ref->p_he->next->p_v->Pos.head(2) - f_ref->p_he->p_v->Pos.head(2);
		for(int j = 0; j < 3; j++){
			t[j] = rot_m90 * e[j];
		}
		jacobi.setZero();
		HalfEdge* he = f_cur->p_he;
		int j = 0;
		do{
			jacobi.block(0,0,1,2)  += (he->next->p_v->Pos(0) - f_cur->Centroid(0) + he->prev->p_v->Pos(0) - f_cur->Centroid(0)) / (2.0*f_ref->Area) * t[j].transpose();
			jacobi.block(1,0,1,2) += (he->next->p_v->Pos(1) - f_cur->Centroid(0) + he->prev->p_v->Pos(1) - f_cur->Centroid(0)) / (2.0*f_ref->Area) * t[j].transpose();
			he = he->next;
			j++;
		}while(he != f_cur->p_he);


		Eigen::MatrixXd u, sigma, v;
		Eigen::JacobiSVD<Eigen::MatrixXd> SVD(jacobi, Eigen::ComputeFullU | Eigen::ComputeFullV);
		sigma = SVD.singularValues().asDiagonal();
		u = SVD.matrixU();
		v = SVD.matrixV();
		Eigen::MatrixXd r;
		r = u * v.transpose();

		Eigen::Vector2d x[3];
		x[0] = r * f_ref->p_he->p_v->Pos.head(2);
		x[1] = r * f_ref->p_he->next->p_v->Pos.head(2);
		x[2] = r * f_ref->p_he->prev->p_v->Pos.head(2);
		f_cur->p_he->p_v->Pos.head(2) = f_cur->Centroid.head(2) + x[0];
		f_cur->p_he->next->p_v->Pos.head(2) = f_cur->Centroid.head(2) + x[1];
		f_cur->p_he->prev->p_v->Pos.head(2) = f_cur->Centroid.head(2) + x[2];
	}
	//HEM_sep.WriteTriangleMesh("triangles.off");
	//HEM_sep.WriteVTK("triangles.vtk");


	std::vector<TriPrism*> p_tp;
	int n_t = HEM_sep.p_f.size();
	for(int i = 0; i < n_t; i++){
		TriPrism* tmp = new TriPrism(i, HEM_sep.p_f[i], Height);
		p_tp.push_back(tmp);
	}

	int numVertexPerElement = p_tp[0]->GetNumVertex();
	int numFacePerElement = p_tp[0]->GetNumFace();
	int numElement = n_t;
	int numVertex = numVertexPerElement * numElement;
	int numFace = numFacePerElement * numElement;

	Eigen::MatrixXd V_tp;
	Eigen::MatrixXi F_tp;
	V_tp.resize(numVertex, 3);
	F_tp.resize(numFace, 3);

	for(int i = 0; i < numElement; i++){
		Eigen::MatrixXd Vi = p_tp[i]->GetV();
		Eigen::MatrixXi Fi = p_tp[i]->GetF();
		V_tp.block(i * numVertexPerElement, 0, numVertexPerElement, 3) = Vi;
		Eigen::MatrixXi fi = Fi + i * numVertexPerElement * Eigen::MatrixXi::Ones(numFacePerElement, 3);
		F_tp.block(i * numFacePerElement, 0, numFacePerElement, 3) = fi;
	}

	igl::write_triangle_mesh("result_triprism.stl", V_tp, F_tp);

	for(int i = 0; i < n_t; i++){
		delete(p_tp[i]);
	}
	p_tp.clear();
	p_tp.shrink_to_fit();
	std::cout << "done." << std::endl;
}

void WriteParamAndTriangles(){
	Eigen::MatrixXd V;
	Eigen::MatrixXi F;

	int n_v_p = HEM_2d.p_v.size();
	int n_f_p = HEM_2d.p_f.size();
	int n_v_t = HEM_sep.p_v.size();
	int n_f_t = HEM_sep.p_f.size();
	int n_v = n_v_p + n_v_t;
	int n_f = n_f_p + n_f_t;

	V.resize(n_v, 3);
	F.resize(n_f, 3);

	for(int i = 0; i < n_v_p; i++){
		V.row(i) = HEM_2d.p_v[i]->Pos;
	}
	for(int i = n_v_p; i < n_v; i++){
		V.row(i) = HEM_sep.p_v[i-n_v_p]->Pos;
	}

	for(int i = 0; i < n_f_p; i++){
		F(i, 0) = HEM_2d.p_f[i]->p_he->p_v->Id;
		F(i, 1) = HEM_2d.p_f[i]->p_he->next->p_v->Id;
		F(i, 2) = HEM_2d.p_f[i]->p_he->prev->p_v->Id;
	}
	for(int i = n_f_p; i < n_f; i++){
		F(i, 0) = n_v_p + HEM_sep.p_f[i-n_f_p]->p_he->p_v->Id;
		F(i, 1) = n_v_p + HEM_sep.p_f[i-n_f_p]->p_he->next->p_v->Id;
		F(i, 2) = n_v_p + HEM_sep.p_f[i-n_f_p]->p_he->prev->p_v->Id;
	}

	igl::write_triangle_mesh("param_and_triangles.off", V, F);

	/*
	std::ofstream fout("param_and_triangles.vtk");
	fout << "# vtk DataFile Version 3.0\n";
	fout << "ParamAndTriangles\n";
	fout << "ASCII\n";
	fout << "DATASET UNSTRUCTURED_GRID\n";
	fout << "POINTS " << n_v << " float\n";
	fout << V << "\n";
	fout << "CELLS " << n_f << " " << 4 * n_f << "\n";
	for(int i = 0; i < n_f; i++){
		fout << "3 " << F.row(i) << "\n";
	}
	fout << "CELL_TYPES " << n_f << "\n";
	for(int i = 0; i < n_f; i++){
		fout << "5\n";
	}
	fout << "CELL_DATA " << n_f << "\n";
	fout << "SCALARS TargetShrinkRate float\n";
	fout << "LOOKUP_TABLE default\n";
	fout << TargetShrinkRatePerTriangle << "\n";
	fout << TargetShrinkRatePerTriangle << "\n";
	*/
}


int main(int argc, char** argv){
	FrontMatter(argc, argv);

	ReadInput();
	std::string fname3d = "input/" + InputFileName3d + "." + InputFileFormat3d;
	HEM_3d.ReadTriangleMesh(fname3d);
	if(Input2dShapeMode == 0){
		HEM_3d.HarmonicParam(&HEM_2d);
	}else if(Input2dShapeMode == 1){
		std::string fname2d = "input/" + InputFileName2d + "." + InputFileFormat2d;
		HEM_2d.ReadTriangleMesh(fname2d);
	}
	ScalingPrintRange();
	HEM_2d.SeparateByTriangle(&HEM_sep);
	HEM_3d.CalcFaceAreas();
	HEM_2d.CalcFaceAreas();
	//HEM_2d.WriteVTK("param.vtk");
	HEM_2d.WriteTriangleMesh("param.off");


	CalcAreaChangeRate2dTo3d();
	MakeTriangles();
	ConstructTriPrisms();

	WriteTarget3dShape();

	WriteParamAndTriangles();

	// area ---------------------------------------------
	double A_before(0), A_after(0);
	HEM_2d.CalcFaceAreas();
	HEM_tgt.CalcFaceAreas();
	for(int i = 0; i < HEM_2d.p_f.size(); i++){
		A_before += HEM_2d.p_f[i]->Area;
		A_after += HEM_tgt.p_f[i]->Area;
	}	
	std::cout << "Total Area (before shrink): " << A_before << std::endl;
	std::cout << "Total Area (after shrink): " << A_after << std::endl;
	std::cout << "Total Shrink Rate: " << A_after/A_before << std::endl;
	// --------------------------------------------------
	std::cout << "All done.\n" << std::endl;
	return 0;
}
