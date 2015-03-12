// +-----------------------------------------------------------------------------------+
// | Client application for the volunteer computing project Acoustics@home             |                       
// +-----------------------------------------------------------------------------------+
// | Pacific Oceanological Institute, Institute for System Dynamics and Control Theory |   
// +-----------------------------------------------------------------------------------+
// | Authors: Pavel Petrov, Oleg Zaikin                                                |
// +-----------------------------------------------------------------------------------+

#include <iostream>
#include <vector>
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

// data of corresponding function output 
struct eigenfunctions_velocities
{
	vector<double> eigenfunctions;
	vector<double> velocities;
};

// prototypes of functions
vector<double> calc_chanel_mods_wave_numbers( double &depth_step, double &freq, vector<double> &sound_velocity,
											  vector<double> &density, vector<double> &point_indexes );
vector<double> calc_wave_numbers_richardson( layer  &cur_layer, double &initial_depth_step, double &freq );
eigenfunctions_velocities calc_eigenfunctions_velocities( layer  &cur_layer, double &initial_depth_step,   
														  double &freq, double &wave_number );
int main( int argc, char **argv )
{
	layer  cur_layer;
	double initial_depth_step;
	double freq;
	double wave_number;

	// test filling
	initial_depth_step = 2.0;
	freq = 0.1;
	cur_layer.cbeg = 0.2;
	cur_layer.cend = 0.8;
	cur_layer.dbeg = 0.5;
	cur_layer.dend = 0.9;
	for ( unsigned i=0; i < 5; i++ )
		cur_layer.zend.push_back( i );
	//
	
	vector<double> cur_wave_numbers;
	eigenfunctions_velocities cur_eigenfunc_veloc;
	cur_wave_numbers = calc_wave_numbers_richardson( cur_layer, initial_depth_step, freq );
	cout << "calc_wave_numbers_richardson() done" << endl;
	cur_eigenfunc_veloc = calc_eigenfunctions_velocities( cur_layer, initial_depth_step, freq, wave_number );
	cout << "calc_eigenfunctions_velocities() done" << endl;
	
	return 0;
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
vector<double> calc_chanel_mods_wave_numbers( double &depth_step, 
										      double &freq, // sound frequency
											  vector<double> &sound_velocity,
											  vector<double> &density,
											  vector<double> &point_indexes )
{
	vector<double> spectr_problem_eigenvalues;
	
	// make tridiagonal matrix and find its eigenvalues in given interval
	// ...

	// fill vector wave_numbers
	// test filling
	spectr_problem_eigenvalues.resize(10);
	for( unsigned i=0; i < spectr_problem_eigenvalues.size(); i++ )
		spectr_problem_eigenvalues[i] = i;
	// 
	
	return spectr_problem_eigenvalues;
}

// function for calculating wave numbers with Richardson extrapolation
vector<double> calc_wave_numbers_richardson( layer  &cur_layer, // info about current layer
									         double &initial_depth_step, // initial step of depth
									         double &freq ) // sound frequency
{
	vector<double> wave_numbers;
	
	// call function
	vector<double> channel_mods_wave_numbers, sound_velocity, density, point_indexes;
	// fill vectors sound_velocity, density, point_indexes
	unsigned channel_mods_iterations = 10; // test filling
	double cur_depth_step;
	for ( unsigned i = 0; i < channel_mods_iterations; i++ ) {
		cur_depth_step = initial_depth_step + i*0.1;
		channel_mods_wave_numbers = calc_chanel_mods_wave_numbers( cur_depth_step, freq, sound_velocity, density, point_indexes );
	}

	// fill vector wave_number
	// ...
	
	return wave_numbers;
}

/* Function for calculating eigenfunctions and group velocities
������� ��� ������� ����������� �������� � ��������� ���������:
����:
��� � ������� (2) + �������� ��������� �����;
������:
-- ����� ����������� ������� ������� �����-����� ��� ���� � ������� ������� ����.
-- ���������������� �� � ����������� �� 1.
-- ����� ��������� ��������. 
�����:
����������� ������� psij, ������������� �� 1.
��������� �������� ����.*/
eigenfunctions_velocities calc_eigenfunctions_velocities( layer  &cur_layer,          // info about current layer
									                      double &initial_depth_step, // initial step of depth
									                      double &freq,               // sound frequency
											              double &wave_number )       // value of a wave number
{
	eigenfunctions_velocities eigenfunc_veloc; // for output data
	
	// fill eigenfunctions and velocities
	// test filling
	eigenfunc_veloc.eigenfunctions.resize(10);
	for( unsigned i=0; i < eigenfunc_veloc.eigenfunctions.size(); i++ )
		eigenfunc_veloc.eigenfunctions[i] = i;
	eigenfunc_veloc.velocities.resize(10);
	for( unsigned i=0; i < eigenfunc_veloc.velocities.size(); i++ )
		eigenfunc_veloc.velocities[i] = i;
	
	return eigenfunc_veloc;
}

