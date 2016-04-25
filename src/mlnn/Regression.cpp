/*!
 * \file Regression.cpp
 * \brief 
 * \author tkornut
 * \date Apr 22, 2016
 */

#include <mlnn/Regression.hpp>

namespace mic {
namespace mlnn {

Regression::Regression(size_t inputs_, size_t outputs_, size_t batch_size_) :
	Layer(inputs_, outputs_, batch_size_, "regression") {

}

void Regression::forward(bool test_) {

	// Pass inputs to outputs.
	(*s['y']) = (*s['x']);

}

void Regression::backward() {

	// dx = 2*(dy - y);
	(*g['x']) = 2 *( (*g['y']) - (*s['y']));

}


} /* namespace mlnn */
} /* namespace mic */