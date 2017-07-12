#include "sspemdd_parallel.h"
#include "sspemdd_sequential.h"
#include "sspemdd_utils.h"

#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <cmath>

sspemdd_parallel::sspemdd_parallel() :
	task_len ( 0 ),
	result_len ( 0 ),
	corecount ( 0 )
{}

sspemdd_parallel::~sspemdd_parallel()
{}

void sspemdd_parallel::MPI_main()
{
	if (rank == 0)
		control_process();
	else if (rank > 0)
		computing_process();
}

void sspemdd_parallel::control_process()
{
#ifdef _MPI
	MPI_Status status;
	stringstream sstream_out;
	mpi_start_time = MPI_Wtime();

	cout << endl << "control_process() started" << endl;

	vector<vector<double>> depths_vec;
	createDepthsArray(depths_vec);
	cout << "depths_vec.size() " << depths_vec.size() << endl;
	
	task = new double[TASK_LEN];
	result = new double[RESULT_LEN];
	
	unsigned send_task_count = 0;
	
	// sending first part of tasks
	for (int computing_process_index = 1; computing_process_index < corecount; computing_process_index++) {
		//sstream_out << "before filling task" << std::endl;
		//std::cout << sstream_out.str();
		unsigned elements_to_send = depths_vec[send_task_count].size() + 1;
		cout << "elements_to_send " << elements_to_send << endl;
		for (unsigned j = 0; j < elements_to_send - 1; j++)
			task[j] = depths_vec[send_task_count][j];
		task[elements_to_send] = (double)send_task_count;
		for (unsigned j = elements_to_send + 1; j < TASK_LEN; j++)
			task[j] = -1;
		MPI_Send(task, TASK_LEN, MPI_DOUBLE, computing_process_index, 0, MPI_COMM_WORLD);
		send_task_count++;
	}
	sstream_out << "send_task_count " << send_task_count << std::endl;

	std::ofstream ofile("mpi_out");
	ofile << sstream_out.rdbuf();
	sstream_out.clear(); sstream_out.str("");
	ofile.close(); ofile.clear();
	
	unsigned processed_task_count = 0;
	unsigned received_task_index;
	double received_residual;

	// get results and send new tasks on idle computing processes
	while (processed_task_count < depths_vec.size()) {
		MPI_Recv(result, RESULT_LEN, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		processed_task_count++;
		if (processed_task_count % 10 == 0)
			sstream_out << std::endl << "processed_task_count " << processed_task_count << std::endl;
		
		received_task_index = (unsigned)result[0];
		received_residual = result[1];
		//sstream_out << "received_residual " << received_residual << std::endl;
		//sstream_out << "received_task_index  " << received_task_index << std::endl;
		
		if (received_residual < record_point.residual) {
			record_point.residual = received_residual;
			record_point.R = result[2];
			record_point.tau = result[3];
			record_point.rhob = result[4];
			record_point.cb = result[5];
			unsigned cur_depths_number = depths_vec[received_task_index];
			record_point.cws.resize(cur_depths_number);
			record_point.depths.resize(cur_depths_number);
			for (unsigned j = 0; j < cur_depths_number; j++)
				record_point.cws[j] = result[6 + j];
			for (unsigned j = 0; j < cur_depths_number; j++)
				record_point.depths[j] = result[6 + cur_depths_number + j];
			
			std::cout << "Control process, new residual minimum : "  << received_residual << std::endl;
			sstream_out << std::endl << "Control process, new residual minimum:" << std::endl;
			sstream_out << "err = " << record_point.residual << ", parameters:" << std::endl;
			sstream_out << "c_b = " << record_point.cb << 
				           ", rho_b= " << record_point.rhob << 
						   ", tau = " << record_point.tau <<
				           ", R = " << record_point.R << std::endl;
			sstream_out << "cws :" << std::endl;
			for (auto &x : record_point.cws)
				sstream_out << x << " ";
			sstream_out << std::endl;
			sstream_out << "depths :" << std::endl;
			for (auto &x : record_point.depths)
				sstream_out << x << " ";
			sstream_out << std::endl;
			sstream_out << "time from start " << MPI_Wtime() - mpi_start_time << " s" << std::endl;
		}
		// if free tasks for sending
		if (send_task_count < point_values_vec.size()) {
			unsigned elements_to_send = depths_vec[send_task_count].size() + 1;
			cout << "elements_to_send " << elements_to_send << endl;
			for (unsigned j = 0; j < elements_to_send - 1; j++)
				task[j] = depths_vec[send_task_count][j];
			task[elements_to_send] = (double)send_task_count;
			for (unsigned j = elements_to_send + 1; j < TASK_LEN; j++)
				task[j] = -1;
			MPI_Send(task, TASK_LEN, MPI_DOUBLE, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
			send_task_count++;
		}
		else {
			// send stop-messages
			for (unsigned j = 0; j < TASK_LEN; j++)
				task[j] = STOP_MESSAGE;
			MPI_Send(task, TASK_LEN, MPI_DOUBLE, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
		}
		
		ofile.open("mpi_out", std::ios_base::app);
		ofile << sstream_out.rdbuf();
		sstream_out.clear(); sstream_out.str("");
		ofile.close(); ofile.clear();
	}
	
	sstream_out << std::endl << "SEARCH ENDED!" << std::endl;
	sstream_out << "err = " << record_point.residual << ", parameters:" << std::endl;
	sstream_out << "c_b = " << record_point.cb <<
		", rho_b = " << record_point.rhob <<
		", R = " << record_point.R <<
		", tau = " << record_point.tau << std::endl;
	sstream_out << "cws :" << std::endl;
	for (auto &x : record_point.cws)
		sstream_out << x << " ";
	sstream_out << std::endl;
	sstream_out << "final time " << MPI_Wtime() - mpi_start_time << " s" << std::endl;
	
	ofile.open("mpi_out", std::ios_base::app);
	ofile << sstream_out.rdbuf();
	sstream_out.clear(); sstream_out.str("");
	ofile.close(); ofile.clear();
	
	delete[] task;
	delete[] result;
	
	std::cout << "finilizing process " << rank << std::endl;
	MPI_Finalize();
#endif
}

void sspemdd_parallel::computing_process()
{
#ifdef _MPI
	MPI_Status status;
	task = new double[TASK_LEN];
	result_len = 2; // index of a task + calculated residual
	result = new double[result_len];
	vector<double> cur_point_values_vec;
	search_space_point cur_point;
	cur_point_values_vec.resize(task_len-1);
	double task_index;
	
	if (rank == 1) {
		std::cout << "computing process, rank " << rank << std::endl;
		std::cout << "task_len " << task_len << std::endl;
		std::cout << "result_len " << result_len << std::endl;
	}

	std::stringstream cur_process_points_sstream;
	
	for (;;) {
		MPI_Recv(task, TASK_LEN, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		if (rank == 1) {
			std::cout << "received task " << std::endl;
			for ( unsigned i=0; i < TASK_LEN; i++)
				std::cout << task[i] << " " << std::endl;
			std::cout << std::endl;
		}
		
		// if stop-message then finalize
		if (task[task_len-1] == -1) {
			std::cout << "rank " << rank << " received stop-message" << std::endl;
			break;
		}
		for ( unsigned i=0; i < task_len - 1; i++ )
			cur_point_values_vec[i] = task[i];
		task_index = task[task_len - 1];
		cur_point = fromDoubleVecToPoint(cur_point_values_vec);
		if (rank == 1) {
			cout << "received depths: ";
			for (auto &x : cur_point.depths)
				cout << x << " ";
			cout << endl;
		}
		cout << endl;
		
		if (rank == 1)
			std::cout << "received task_index " << task_index << std::endl;
		
		//fillDataComputeResidual(cur_point); // calculated residual is written to cur_point

		result[0] = task_index;
		result[1] = cur_point.residual;
		MPI_Send(result, result_len, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);

		/*cur_process_points_sstream << cur_point.cb << " " 
			                       << cur_point.rhob << " "
								   << cur_point.R << " "
								   << cur_point.tau << " ";
		for (unsigned i = 0; i < cur_point.cws.size(); i++)
			cur_process_points_sstream << cur_point.cws[i] << " ";
		cur_process_points_sstream << cur_point.residual;
		cur_process_points_sstream << std::endl;*/
	}
	delete[] task;
	delete[] result;

	/*std::stringstream cur_process_file_name_sstream;
	cur_process_file_name_sstream << "points_process" << rank;
	std::ofstream cur_process_file(cur_process_file_name_sstream.str().c_str());
	cur_process_file << cur_process_points_sstream.rdbuf();
	cur_process_file.close();*/

	MPI_Finalize();

#endif
}