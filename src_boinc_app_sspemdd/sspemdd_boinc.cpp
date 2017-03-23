/*****************************************************************************************
// SSPEMDD-based BOINC client application for Acoustics@home -- Copyright(c) 2017
// SSPEMDD stands for Sound Speed Profile Estimator from Modal Delay Data
// Oleg Zaikin (Matrosov Institute for System Dynamics and Control Theory of SB RAS)
// Pavel Petrov (Il'ichev Pacific Oceanological Institute of FEB RAS),
*****************************************************************************************/

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <cstdio>
#include <cctype>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <unistd.h>
#endif

#include "str_util.h"
#include "util.h"
#include "filesys.h"
#include "boinc_api.h"
#include "mfile.h"

#include <sstream>

#include "sspemdd_sequential.h"
#include "sspemdd_utils.h"

#define CHECKPOINT_FILE "chpt"
#define INPUT_FILENAME "in"
#define OUTPUT_FILENAME "out"

const int MIN_CHECKPOINT_INTERVAL_SEC = 2;

search_space_point fromStrToPoint(std::string str)
bool do_work( const std::string &input_file_name,
			  search_space_point &chpt_record_point );
int do_checkpoint( const unsigned long long &processed_points, const search_space_point &chpt_record_point );
void fromPointToFile(const search_space_point &point, std::ofstream &temp_ofile);

int main(int argc, char **argv) 
{
    char buf[256];
	int retval = boinc_init();
    if ( retval ) {
        fprintf(stderr, "%s boinc_init returned %d\n",
            boinc_msg_prefix(buf, sizeof(buf)), retval
        );
        exit( retval );
    }
	
	std::string input_file_name, output_file_name, chpt_file_name;
	std::string str;
	std::ofstream output_file;
	std::fstream chpt_file;
	unsigned long long processed_points = 0;
	
	// open the input file (resolve logical name first)
	boinc_resolve_filename_s( INPUT_FILENAME, input_file_name);
	
	// read data from the checkpoint file if such exists
	search_space_point chpt_record_point;
    boinc_resolve_filename_s( CHECKPOINT_FILE, chpt_file_name);
	chpt_file.open( chpt_file_name.c_str(), ios_base :: in );
	if ( chpt_file.is_open() ) {
		chpt_file >> processed_points;
		getline(chpt_file, str);
		chpt_record_point = fromStrToPoint(str);
		chpt_file.close();
	}
	
	if ( !do_work( input_file_name, chpt_record_point ) ) {
		fprintf( stderr, "APP: do_work() failed:\n" );
		perror("do_work");
        exit(1);
	}

	// resolve, open and write answer to output file
    boinc_resolve_filename_s( OUTPUT_FILENAME, output_file_name);
	output_file.open( output_file_name.c_str() );
    if ( !output_file.is_open() ) {
        fprintf(stderr, "%s APP: app output open failed:\n",
            boinc_msg_prefix(buf, sizeof(buf))
        );
		exit(-1);
    }
	for (unsigned i = 0; i < current_results_vec.size(); i++)
		output_file << current_results_vec[i] << std::endl;
	output_file.close();
	
    boinc_finish(0);
}

search_space_point fromStrToPoint(std::string str)
{
	search_space_point point;
	std::stringstream sstream;
	sstream << str;
	sstream >> point.residual >> point.cb >> point.rhob >> point.R >> point.tau;
	double val;
	while (sstream >> val)
		point.cws.push_back(val);
}

bool do_work( const std::string &input_file_name,
			  search_space_point &chpt_record_point );
{
	sspemdd_sequential sspemdd_seq;

	sspemdd_seq.readScenario(input_file_name);
	sspemdd_seq.readInputDataFromFiles();
	sspemdd_seq.init();

	std::vector<std::vector<double>> point_vec;
	std::vector<double> cur_point;
	std::vector<int> index_arr;
	while (SSPEMDD_utils::next_cartesian(sspemdd_seq.search_space, index_arr, cur_point))
		point_values_vec.push_back( cur_point_values );

	int retval;
	
	for ( ;; ) 
	{
		// checkpoint current results
		//if ( ( boinc_is_standalone() ) || ( boinc_time_to_checkpoint() ) ) {
			retval = do_checkpoint( current_results_vec );
            if (retval) {
                fprintf(stderr, "APP: checkpoint failed %d\n", retval );
                exit(retval);
            }
			boinc_checkpoint_completed();
        //}
	}

	/*
	record_point.residual = point.residual;
	record_point.cb       = point.cb;
	record_point.rhob     = point.rhob;
	record_point.R        = point.R;
	record_point.tau      = point.tau;
	record_point.cws      = point.cws;
	*/
	
	return true;
}

int do_checkpoint( const unsigned long long &processed_points, const search_space_point &chpt_record_point )
{
	int retval;
    string resolved_name;
	
	ofstream temp_ofile( "temp" );
	if ( !temp_ofile.is_open() ) return 1;

	temp_ofile << processed_points << std::endl;
	fromPointToFile(chpt_record_point, temp_ofile);
    temp_ofile.close();
	
    boinc_resolve_filename_s(CHECKPOINT_FILE, resolved_name);
    retval = boinc_rename( "temp", resolved_name.c_str() );
    if ( retval ) return retval;

	boinc_fraction_done( ( double )current_solved / ( double )total_tasks );

    return 0;
}

void fromPointToFile(const search_space_point &point, std::ofstream &temp_ofile)
{
	temp_ofile << point.residual << " " << point.cb << " " << point.rhob << " " 
		       << point.R << " " << point.tau << " ";
	for (unsigned i = 0; i < point.cws.size(); i++)
		temp_ofile << point.cws[i] << " ";
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode) {
    LPSTR command_line;
    char* argv[100];
    int argc;

    command_line = GetCommandLine();
    argc = parse_command_line( command_line, argv );
    return main(argc, argv);
}
#endif