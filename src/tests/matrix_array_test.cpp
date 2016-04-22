/*!
 * \file mlnn_test.cpp
 * \brief Contains program for testing creation/serialization of multi-layer neural networks.
 * \author tkornut
 * \date Feb 17, 2016
 */

#include <iostream>

#include <types/MatrixArray.hpp>

#include <fstream>
// Include headers that implement a archive in simple text format
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>


int main() {
	// Default sizes of matrices.
	const size_t N = 2;
	const size_t M = 3;
	const size_t B = 4;

	// Test MatrixArray operations.
	mic::types::MatrixArray<double> ma1("test_array");

	ma1.add (
				{
					std::make_tuple ( "x", M, B ), 	// input
					std::make_tuple ( "y", N, B ) 		// output
				} );

	ma1.add (std::make_tuple ( "w", M, N ));

	ma1["x"]->randn(1, 0.00001);
	ma1["w"]->randn(1, 0.00001);


	mic::types::MatrixArray<double> ma2 = ma1;

	std::cout<<"Saved MatrixArray = " << ma1;

	const char* fileName = "saved.txt";
	// Save data
	{
		// Create an output archive
		std::ofstream ofs(fileName);
		boost::archive::text_oarchive ar(ofs);
		// Write data
		ar & ma1;
	}

	(*ma1["y"]) = (*ma1["w"]) * (*ma1["x"]);
	std::cout<<"Calculated MatrixArray = " << ma1;

	std::cout<<"Copied MatrixArray = " << ma2;

	// Restore data
	mic::types::MatrixArray<double> restored_ma;
	{
		// Create and input archive
		std::ifstream ifs(fileName);
		boost::archive::text_iarchive ar(ifs);
		// Load data
		ar & restored_ma;
		std::cout << "Restored MatrixArray = " << restored_ma << std::endl;
	}

}
