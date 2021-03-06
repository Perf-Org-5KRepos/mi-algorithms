/*!
 * Copyright (C) tkornuta, IBM Corporation 2015-2019
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*!
 * \file RawTextImporter.cpp
 * \brief 
 * \author tkornut
 * \date Dec 22, 2015
 */

#include <importers/RawTextImporter.hpp>

#include <fstream>

namespace mic {
namespace importers {

RawTextImporter::RawTextImporter(std::string node_name_) : Importer (node_name_),
		data_filename("data_filename","data_filename")
{
	// Register properties - so their values can be overridden (read from the configuration file).
	registerProperty(data_filename);
}

void RawTextImporter::setDataFilename(std::string data_filename_) {
	data_filename = data_filename_;
}


bool RawTextImporter::importData(){
	char character;
	// Open file.
	std::ifstream data_file(data_filename, std::ios::in | std::ios::binary);

	LOG(LSTATUS) << "Importing raw data from file: " << data_filename;

	// Check if file is open.
	if (data_file.is_open()) {
		// Iterate through the characters.
		while(!data_file.eof()) {
			// Read the character.
			data_file.get(character);
			LOG(LDEBUG) << character;

			// Add character to both data and labels.
			sample_data.push_back(std::make_shared <char> (character) );
			sample_labels.push_back(std::make_shared <char> (character) );

		}//: while ! eof

		// Close the file.
		data_file.close();

	} else {
		LOG(LFATAL) << "Oops! Couldn't find file: " << data_filename;
		return false;
	}//: else

	LOG(LINFO) << "Imported " << sample_data.size() << " characters";

	// Fill the indices table(!)
	for (size_t i=0; i < sample_data.size(); i++ )
		sample_indices.push_back(i);

	// Count the classes.
	countClasses();

	LOG(LINFO) << "Data import finished";

	return true;
}

} /* namespace importers */
} /* namespace mic */
