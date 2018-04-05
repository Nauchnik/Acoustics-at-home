#ifndef SEQUENTIAL_H
#define SEQUENTIAL_H

#include <vector>
#include <complex>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <math.h>
#include "linalg.h"

using namespace std;

const double LOCAL_M_PI = 3.14159265358979323846;
const complex<double> Iu(0.0,1.0);
const double START_HUGE_VALUE = 1e100;
const double START_CW_VALUE = 1481;

struct search_space_point
{
	double R;
	double rhob;
	double cb;
	double tau;
	vector<double> cws;
	double residual;
	vector<double> depths;

	bool operator==(const search_space_point& a) const
	{
		return (R == a.R && tau == a.tau && rhob == a.rhob && cb == a.cb && cws == a.cws && depths == a.depths);
	}
};

struct reduced_search_space_attribute
{
	bool R;
	bool rhob;
	bool cb;
	bool tau;
	vector<bool> cws;
};

class CAMBALA_sequential
{
public:
	CAMBALA_sequential();
	double H;
	unsigned long long nh;
	unsigned long long ncb;
	unsigned long long nrhob;
	unsigned long long nR;
	unsigned long long ntau;  //tau_comment: added tau to the class declaration
	double cb1;
	double cb2;
	vector<double> cw1_arr;
	vector<double> cw2_arr;
	vector<double> cw1_init_arr;
	vector<double> cw2_init_arr;
	vector<double> d1_arr;
	vector<double> d2_arr;
	vector<double> d_step;
	vector<unsigned long long> ncpl_init_arr;
	vector<unsigned long long> ncpl_arr;
	string object_function_type;
	unsigned ppm;
	double h1;
	double h2;
	double R1;
	double R2;
	double tau1;    // tau to the class declaration
	double tau2;  
	double rhob1;
	double rhob2;
	unsigned long long n_layers_w;
	unsigned long long iterated_local_search_runs;
	vector<unsigned> mode_numbers;
	vector<vector<double>> modal_delays;
	vector<double> freqs;
	vector<double> c1s;
	vector<double> c2s;
	vector<double> rhos;
	vector<unsigned> Ns_points;
	vector<vector<double>> weight_coeffs;
	int verbosity;
	string dtimesFileName;
	string spmagFileName;
	int rank;
	bool isTimeDelayPrinting;
	string launch_type; // bruteforce | ils
	stringstream input_params_sstream;
	string output_filename;
	string depths_filename;
	
	// Oleg's functions
	vector<vector<double>> search_space; // values of variables which form a search space
	int readScenario(string scenarioFileName);
	int readInputDataFromFiles();
	int init(vector<double> depths);
	void solve();
	int createDepthsArray(const double h, vector<vector<double>> &depths_vec);
	void loadValuesToSearchSpaceVariables();
	double getRecordResidual();
	search_space_point getNonRandomStartPoint( vector<double> depths );
	double fillDataComputeResidual( search_space_point &point );
	vector<search_space_point> getSearchSpacePointsVec(vector<double> depths);
	search_space_point findLocalMinHillClimbing(vector<double> depths);
	void findGlobalMinBruteForce(vector<double> depths);
	void reportFinalResult();
	void getThreeValuesFromStr(string str, double &val1, double &val2, double &val3);
	void reduceSearchSpace(reduced_search_space_attribute &reduced_s_s_a);
	double directPointCalc( search_space_point point );
	void printDelayTime(double R, vector<unsigned> mode_numbers, vector<vector<double>> modal_group_velocities);

	search_space_point fromPointIndexesToPoint(vector<unsigned> cur_point_indexes, vector<double> depths);
	vector<unsigned> fromPointToPointIndexes(search_space_point point);
	search_space_point fromDoubleVecToPoint(vector<double> double_vec);
	search_space_point fromStrToPoint(string str); // BOINC client application
	void fromPointToFile(const search_space_point &point, ofstream &ofile); // BOINC client application
protected:
	unsigned long long N_total;
	search_space_point record_point;
	chrono::high_resolution_clock::time_point start_chrono_time;
	// hill climbing
	vector<search_space_point> checked_points;
};

#endif