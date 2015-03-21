// +-----------------------------------------------------------------------------------+
// | Client application for the volunteer computing project Acoustics@home             |
// +-----------------------------------------------------------------------------------+
// | Pacific Oceanological Institute, Institute for System Dynamics and Control Theory |
// +-----------------------------------------------------------------------------------+
// | Authors: Pavel Petrov, Oleg Zaikin                                                |
// +-----------------------------------------------------------------------------------+

#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <chrono>
#include "linalg.h"

using namespace std;

// layer data
struct layer
{
	vector<double> zend; // vector of finite depths
	double cbeg;         // velocity of a sound at the beginning of a layer
	double cend;         // velocity of a sound at the end of a layer
	double dbeg;         // density at the beginning of a layer
	double dend;         // density at the end of a layer
};




vector<double> compute_wnumbers(double &omeg, vector<double> &c, vector<double> &rho, vector<unsigned> &interface_idcs, vector<double> &meshsizes,unsigned flOnlyTrapped);

vector<double> compute_wnumbers_extrap(double &omeg, vector<double> &depths,vector<double> &c1s,vector<double> &c2s,vector<double> &rhos,vector<unsigned> &Ns_points, unsigned flOnlyTrapped,unsigned &ordRich);

int main( int argc, char **argv )
{

	double freq = 50;
    double c_w = 1500;
    double c_b = 2000;
    double rho_w = 1;
    double rho_b = 2;
    double dz = 1;
    unsigned nz = 501;
    unsigned ib = 90;    //at POINT = 89 we have water, at POINT = 90 we have bottom
                    //   ii = 89,                  at ii = 90

    cout.precision(15);

    double omeg = 2*3.141592653589793*freq;

    vector<double> input_c;
    vector<double> input_rho;
    vector<double> input_mesh { dz,dz };
    vector<unsigned> input_interf_idcs { ib };
    vector<double> out_wnum;

    for ( unsigned ii = 0; ii<nz; ii++ ){
        if (ii<ib){                   //
            input_c.push_back(c_w);
            input_rho.push_back(rho_w);
        }
        else {
            input_c.push_back(c_b);
            input_rho.push_back(rho_b);
        }

    }

    cout << "freq = " << freq << endl;
    cout << "omega = " << omeg << endl;

/*    out_wnum = compute_wnumbers(omeg, input_c, input_rho,input_interf_idcs, input_mesh,1);

   for (unsigned ii=0; ii<out_wnum.size();  ii++) {

            cout << ii << "->" << out_wnum.at(ii) << endl;


    }
*/
    cout << "NEW: extrapolation" << endl;

    vector<double> depths {90,500};
    vector<double> c1s  {1500,2000};
    vector<double> c2s  {1500,2000};
    vector<double> rhos  {1,2};
    vector<unsigned> Ns_points  {90,410};
    unsigned rord = 4;

    out_wnum = compute_wnumbers_extrap(omeg,depths,c1s,c2s,rhos,Ns_points,1,rord);

    cout << "Extrapolated ev:" << endl;
    for (unsigned ii=0; ii<out_wnum.size();  ii++) {

            cout << ii << "->" << out_wnum.at(ii) << endl;


    }

	return 0;


}




vector<double> compute_wnumbers_extrap(       double &omeg, // sound frequency
											  vector<double> &depths,
											  vector<double> &c1s,
											  vector<double> &c2s,
											  vector<double> &rhos,
											  vector<unsigned> &Ns_points,
											  unsigned flOnlyTrapped,
											  unsigned &ordRich)
/*  subroutine for computing wavenumbers for a given waveguide structure
    the computation is performed by the FD method for certain meshsize,
    Richardson extrapolation is used to improve the


Layer structure:

depths_{i-1}----c=c1s_i--
*
*
... <-Ns_points_i       rho = rhos_i
*
*
depths_{i}------c=c2s_i--

Other parameters:
omeg -- cyclic frequency, omeg = 2*Pi*f;
flOnlyTrapped -- flag to determine the mode subset: set to 0 to compute all propagating modes, i.e. such that k^2>=0, otherwise only trapped modes are computed
ordRich -- order of the Richardson extrapolation;

the top of the first layer is z=0
	*/


{
    vector<double> coeff_extrap;
    if (ordRich == 1) {
        coeff_extrap.assign({1});
    }
    else if (ordRich == 2){
        coeff_extrap.assign({-1,2});
    }
    else if (ordRich == 3){
        coeff_extrap.assign({0.5, -4, 4.5});
    }
    else if (ordRich == 4){
        coeff_extrap.assign({-1/double(6),4,-13.5,32/double(3)});
    }
    else {
        ordRich = 3;
        coeff_extrap.assign({0.5, -4, 4.5});
    }

    cout << "Richardson coeffs" << endl;
    for (int ii=0; ii<coeff_extrap.size() ; ii++ ){
        cout << coeff_extrap.at(ii) << endl;
    }


    vector<double> input_c;
    vector<double> input_rho;
    vector<double> input_mesh;
    vector<unsigned> input_interf_idcs;
    vector<double> out_wnum;
    vector<double> wnum_extrapR;
    double zc = 0;
    double zp = 0;
    double dz = 0;
    unsigned m_wnum = 1000;

    unsigned n_layers = depths.size();
    unsigned n_points_total = 0;
    unsigned n_points_layer = 0;
// outer loop for Richardson coefficient rr
    for (unsigned rr = 1; rr <= ordRich; rr++){

        input_c.clear();
        input_rho.clear();
        input_interf_idcs.clear();
        input_mesh.clear();
        out_wnum.clear();

        input_c.push_back(0);
        input_rho.push_back(0);
        n_points_total = 1;
        zp = 0;

        for (unsigned ll = 0; ll<n_layers; ll++){
            zc = depths.at(ll);
            n_points_layer = Ns_points.at(ll)*rr;
            dz = (zc - zp)/(  n_points_layer  );
            input_mesh.push_back(  dz  );
            input_c.at(n_points_total-1) = c1s.at(ll) ;
            input_rho.at(n_points_total-1) = rhos.at(ll) ;

            n_points_total = n_points_total + n_points_layer;

            for (unsigned kk=1; kk<= n_points_layer; kk++) {
                input_rho.push_back(rhos.at(ll));
                input_c.push_back( c1s.at(ll) + (c2s.at(ll) - c1s.at(ll))*kk/n_points_layer );
            }

            if (ll < n_layers - 1) {
                input_interf_idcs.push_back(n_points_total-1);
            }
            zp = zc;
        }

        cout << "rr=" << rr << endl;

        out_wnum = compute_wnumbers(omeg, input_c, input_rho,input_interf_idcs, input_mesh,flOnlyTrapped);
        m_wnum = min(m_wnum, out_wnum.size() );

        if (rr == 1) { wnum_extrapR.assign(m_wnum,0);}


        for (unsigned mm=0; mm<m_wnum; mm++ ) {
            wnum_extrapR.at(mm) = wnum_extrapR.at(mm) + out_wnum.at(mm)*coeff_extrap.at(rr-1);
        }



            for (unsigned ii=0; ii<out_wnum.size();  ii++) {

                cout << ii << "->" << out_wnum.at(ii) << endl;
            }

    }


return wnum_extrapR;
}



vector<double> compute_wnumbers(           double &omeg, // sound frequency
											  vector<double> &c,
											  vector<double> &rho,
											  vector<unsigned> &interface_idcs,
											  vector<double> &meshsizes,
											  unsigned flOnlyTrapped                 // set flOnlyTrapped = 0 to compute all propagating modes, i.e. such that k^2>=0
											  )
{
	// prepare the three diagonals of the matrix to be passed to the EIG function
    // for the c = c_j, j=0... N_points
    // interfaces are at z = z_m,  interface_idcs = {m}, if empty then we have NO interfaces
    // mesh size in the j-th layer is meshsizes.at(j); this vector has at least one element,
    // for the k-th interface interface_idcs.at(k-1) we have meshsizes meshsizes.at(k-1) and meshsizes.at(k) below and above the interface respectively
    // for c(interface_idcs.at(k-1)) the value of c is the one BELOW the k-th interface
    //(i.e. for the water-bottom interface at the boundary we take the value from the bottom)


	vector<double> md;
	vector<double> ud;
	vector<double> ld;

    int N_points = c.size();
    unsigned layer_number = 0;
    double dz = meshsizes.at(layer_number);
    double dz_next = dz;
    double q = 0;
    double cp, cm, dp, dm, cmin, cmax, kappamax, kappamin;
    int next_interface_idx;

    if ( interface_idcs.size() > 0 )
    {
        next_interface_idx = interface_idcs.at(0)-1;
    }
    else
    {
        next_interface_idx = N_points;
    }

    cmin = c.at(0);
    cmax = c.at(0);

    for( int ii=0; ii < N_points; ii++ ){
        if (c.at(ii) < cmin) { cmin = c.at(ii); }
        if (c.at(ii) > cmax) { cmax = c.at(ii); }

    }

    kappamax = omeg/cmin;
    kappamin = omeg/cmax;

    if (flOnlyTrapped == 0 )
        kappamin = 0;


    for(int ii=0; ii < N_points-2; ii++ ){

        // ordinary point


        ud.push_back( 1/(dz*dz) );
        ld.push_back( 1/(dz*dz) );
        md.push_back(-2/(dz*dz)  + omeg*omeg/(c.at(ii+1)*c.at(ii+1)) );


        // special case of the point at the interface

        if (ii == next_interface_idx) {         //ii -- z(ii+1), z(0) = 0
            layer_number = layer_number + 1;    // ������ ii=89 -- ����, � ii=90 -���,
                                                // ����� ii = 89 -- ���������, ��� ���

            cp = c.at(ii+1);
            dp = rho.at(ii+1);
            cm = c.at(ii);
            dm = rho.at(ii);
            q = 1/( dz_next*dm + dz*dp );

            dz_next = meshsizes.at(layer_number);


            ld.at(ii) = 2*q*dp/dz;
            md.at(ii) = -2*q*( dz_next*dp + dz*dm )/(dz*dz_next) + omeg*omeg*q*( dz*dp*cp*cp + dz_next*dm*cm*cm )/( cp*cp*cm*cm ) ;
            ud.at(ii) = 2*q*dm/dz_next;

            if ( interface_idcs.size() > layer_number )
            {
                next_interface_idx = interface_idcs.at(layer_number) - 1;
            }
            else
            {
                next_interface_idx = N_points;
            }

            dz = dz_next;
        }
    }

    // HERE WE CALL THE EIG ROUTINE!!!
    //input: diagonals ld, md, ud + interval [0 k_max]
    //output: wnumbers2 = wave numbers squared

    alglib::real_2d_array A, eigenvectors; // V - ������ ������
	alglib::real_1d_array eigenvalues; // Lm -������ ����
	A.setlength(N_points-2,N_points-2);
	eigenvectors.setlength(N_points-2,N_points-2);
	eigenvalues.setlength(N_points-2);
    alglib::ae_int_t eigen_count = 0;




	// fill matrix by zeros
	for (int ii=0; ii < N_points-2; ii++ )
		for ( int jj=0; jj < N_points-2; jj++ )
			A[ii][jj] = 0.0;


	// fill tridiagonal matrix
	// make Dirichlet case matrix
	// �� ������� ��������� ����� ����� -2/(h^2), �� ���- � ���- ���������� ����� ����� 1/(h^2).

	// matrix is symmetrized: a_{j,j} = m_j; a_{j,j+1} = a_{j+1,j} = \sqrt(u_j*l_{j+1});


	/*for ( int ii=0; ii < N_points-2; ii++ ) {
		A[ii][ii] = md.at(ii);


		if ( ii >0 ) {
            A[ii][ii-1] = ld.at(ii);
            //A[ii-1][ii] = ld.at(ii);
		}

		if ( ii < N_points-3 ) {
            A[ii][ii+1] = ud.at(ii);
            //A[ii+1][ii] = ud.at(ii);
		}

	}*/

    for ( int ii=0; ii < N_points-3; ii++ ) {
		A[ii][ii] = md.at(ii);
        A[ii][ii+1] = sqrt(ud.at(ii)*ld.at(ii+1));
        A[ii+1][ii] = A[ii][ii+1];
	}
    A[N_points-3][N_points-3] = md.at(N_points-3);


    ofstream myFile("thematrixdiags.txt");
    for (int ii=0; ii<N_points-2; ii++){
        myFile << ld.at(ii) << "  " << md.at(ii) << "  " << ud.at(ii) << endl;
    }

/*    ofstream myFile1("thematrixdiags_A.txt");
    myFile1 << "W" << "  " << A[1][1] << "  " << A[1][2] << endl;
    for (int ii=1; ii<N_points-3; ii++){
        myFile1 << A[ii][ii-1] << "  " << A[ii][ii] << "  " << A[ii][ii+1] << endl;
    }
    myFile1 << A[N_points-3][N_points-4] << "  " << A[N_points-3][N_points-3] << "  " << "W" << endl;
*/

	vector<double> wnumbers;




    //alglib::smatrixevdr( A, N_points-2, 0, 0, kappamin, kappamax, eigen_count, eigenvalues, eigenvectors); // CHECK CALL ARGUMENTS!


    alglib::smatrixevdr( A, N_points-2, 0, 0, kappamin*kappamin, kappamax*kappamax, eigen_count, eigenvalues, eigenvectors); // CHECK CALL ARGUMENTS!

    for (int ii=0; ii<eigen_count; ii++) {
            wnumbers.push_back( sqrt(eigenvalues[eigen_count-ii-1]) );

    }


    return wnumbers;
}


/* function for calculating wave number of channel mods
������� ��� ������� �������� ����� ��������� ���
����:
dz -- ��� �� �������;
f -- ������� �����;
������ c = (ci); -- �������� ����� � ����� dz; ����� = N. �������� N -- "�������".
������ d = (di); -- ���������; ����� N.
������ m = (mj); -- ������� �����, ��� ��������� ����� ������ ������
                     (�����, ��� ������������� ���� ���� � ���������� ������). ����� M <=10.
������: ������������ ���������������� �������, ��������� ������ ����������� �������� ��� ������� [omega/cmax omega/cmin].
�����: ����������� �������� kj^2 ������������ ������������ ������ */

/*

vector<double> calc_chanel_mods_wave_numbers( double &depth_step,
										      double &freq, // sound frequency
											  vector<double> &sound_velocity,
											  vector<double> &density,
											  vector<double> &point_indexes )
{
	vector<double> spectr_problem_eigenvalues;

	// make tridiagonal matrix and find its eigenvalues in given interval
	// ...
	stringstream sstream;
	// calculate eigenvalues and eigenvectors
	int n=2000;
	double from = -0.001, to = 0.001; // interval for eigenvalues

	sstream << "n : " << n << endl;
	alglib::real_2d_array A, eigenvectors; // V - ������ ������
	alglib::real_1d_array eigenvalues; // Lm -������ ����
	A.setlength(n,n);
	eigenvectors.setlength(n,n);
	eigenvalues.setlength(n);

	// fill matrix by zeros
	for ( int i=0; i < n; i++ )
		for ( int j=0; j < n; j++ )
			A[i][j] = 0.0;

	// fill tridiagonal matrix
	// make Dirichlet case matrix
	// �� ������� ��������� ����� ����� -2/(h^2), �� ���- � ���- ���������� ����� ����� 1/(h^2).
	double h = 2.0;
	for ( int i=0; i < n; i++ ) {
		A[i][i] = -2/pow(h,2);
		if ( i != n-1 ) {
			A[i+1][i] = 1/pow(h,2);
			A[i][i+1] = 1/pow(h,2);
		}
	}

	sstream << "A :" << endl;
	sstream << "first diagonal above main :" << endl;
	for ( int i=0; i < n-1; i++ )
		sstream << A[i][i+1] << " ";
	sstream << endl;

	sstream << "main diagonal :" << endl;
	for ( int i=0; i < n; i++ )
		sstream << A[i][i] << " ";
	sstream << endl;

	sstream << "first diagonal below main :" << endl;
	for ( int i=0; i < n-1; i++ )
		sstream << A[i+1][i] << " ";
	sstream << endl;

	sstream << "interval : (" << from << ", " << to << "]" << endl;
	alglib::ae_int_t eigen_count = 0;
	//alglib::smatrixevd(A,n,1,0,eigenvalues,eigenvectors);
    chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	alglib::smatrixevdr( A, n, 1, 0, from, to, eigen_count, eigenvalues, eigenvectors); // bisection method
	chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);

	sstream << "solving time : " << time_span.count() << endl;
	sstream << "eigenvalues count in interval : " << eigen_count << endl;
	//cout << "eigenvalues count : " << eigenvalues.length() << endl;

	for ( int i=0; i < eigenvalues.length(); i++ ) {
		sstream << "eigenvalue # " << i << " : " << eigenvalues[i] << endl;
		sstream << "eigenvector # " << i << " : " << endl;
		for ( int j=0; j < n; j++ )
			sstream << eigenvectors[i][j] << " ";
		if ( i < eigenvalues.length() - 1 )
			sstream << endl;
	}

	ofstream ofile( "out" );
	ofile << sstream.str();
	ofile.close();
	sstream.clear(); sstream.str("");

	cout << "finding eigenvalues done" << endl;

	// fill vector wave_numbers
	// test filling
	spectr_problem_eigenvalues.resize(10);
	for( unsigned i=0; i < spectr_problem_eigenvalues.size(); i++ )
		spectr_problem_eigenvalues[i] = i;
	//

	return spectr_problem_eigenvalues;
}


*/
