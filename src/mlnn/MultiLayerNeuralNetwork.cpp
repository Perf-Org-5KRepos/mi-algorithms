/*!
 * \file MultiLayerNeuralNetwork.cpp
 * \brief 
 * \author tkornut
 * \date Mar 31, 2016
 */

#include <mlnn/MultiLayerNeuralNetwork.hpp>

#include <iomanip>

namespace mic {
namespace mlnn {



MultiLayerNeuralNetwork::MultiLayerNeuralNetwork(std::string name_ ) :
		name(name_),
		loss_type(LossFunctionType::Undefined), // Initially the type of loss function is undefined.
		connected(false) // Initially the network is not connected.
{

}

MultiLayerNeuralNetwork::~MultiLayerNeuralNetwork() {
}

void MultiLayerNeuralNetwork::forward(mic::types::MatrixXf& input_data, bool skip_dropout) {
	// Make sure that there are some layers in the nn!
	assert(layers.size() != 0);

	LOG(LDEBUG) << "Inputs size: " << input_data.cols() << "x" << input_data.rows();
	LOG(LDEBUG) << "First layer input matrix size: " <<  layers[0]->s['x']->cols() << "x" << layers[0]->s['x']->rows();

	// Make sure that the dimensions are ok.
	assert((layers[0]->s['x'])->cols() == input_data.cols());
	assert((layers[0]->s['x'])->rows() == input_data.rows());

	LOG(LDEBUG) <<" input_data: " << input_data.transpose();

	// Copy inputs to the lowest point in the network.
	(*(layers[0]->s['x'])) = input_data;

	// Connect layers by setting the input matrices pointers to point the output matrices.
	// There will not need to be copy data between layers anymore.
	if (!connected) {
		// Set pointers.
		for (size_t i = 0; i < layers.size()-1; i++) {
			layers[i+1]->s['x'] = layers[i]->s['y'];
			layers[i]->g['y'] = layers[i+1]->g['x'];
		}//: for
		connected = true;
	}


	// Compute the forward activations.
	for (size_t i = 0; i < layers.size(); i++) {
		LOG(LDEBUG) << "Layer [" << i << "] " << layers[i]->name() << ": (" <<
				layers[i]->inputsSize() << "x" << layers[i]->batchSize() << ") -> (" <<
				layers[i]->outputsSize() << "x" << layers[i]->batchSize() << ")";

		// Perform the forward computation: y = f(x).
		layers[i]->forward(skip_dropout);

		/*// Pass result to the next layer: x(next layer) = y(current layer).
		if (i + 1 < layers.size()) {
			(*(layers[i+1]->s['x'])) = (*(layers[i]->s['y']));
			//layers[i + 1]->x = layers[i]->y;
			LOG(LDEBUG) << "Passing data from " << i << " to "<< i+1;
			LOG(LDEBUG) << layers[i]->s['y']->transpose();
			LOG(LDEBUG) << layers[i+1]->s['y']->transpose();
		}*/
	}
	LOG(LDEBUG) <<" predictions: " << getPredictions()->transpose();
}

void MultiLayerNeuralNetwork::backward(mic::types::MatrixXf& targets_) {
	// Make sure that there are some layers in the nn!
	assert(layers.size() != 0);

	LOG(LDEBUG) << "Last layer output gradient matrix size: " << layers.back()->g['y']->cols() << "x" << layers.back()->g['y']->rows();
	LOG(LDEBUG) << "Passed target matrix size: " <<  targets_.cols() << "x" << targets_.rows();

	// Make sure that the dimensions are ok.
	assert((layers.back()->g['y'])->cols() == targets_.cols());
	assert((layers.back()->g['y'])->rows() == targets_.rows());

	//set targets at the top
	//layers[layers.size() - 1]->dy = t;
	(*(layers.back()->g['y'])) = targets_;

	//propagate error backward
	for (int i = layers.size() - 1; i >= 0; i--) {
		//std::cout << "layer i =  " << i << " name = " << layers[i]->id() << std::endl;

		layers[i]->resetGrads();
		layers[i]->backward();

		//dy(previous layer) = dx(current layer)
/*		if (i > 0) {

//			layers[i - 1]->dy = layers[i]->dx;
			(*(layers[i - 1]->g['y'])) = (*(layers[i]->g['x']));

		}*/

	}

}

void MultiLayerNeuralNetwork::update(float alpha, float decay) {

	// update all layers according to gradients
	for (size_t i = 0; i < layers.size(); i++) {

		layers[i]->applyGrads(alpha, decay);

	}

}


float MultiLayerNeuralNetwork::train(mic::types::MatrixXfPtr encoded_batch_, mic::types::MatrixXfPtr encoded_targets_, float learning_rate_, float weight_decay_) {

	// Forward propagate the activations from first layer to the last.
	forward(*encoded_batch_);

	// Get predictions.
	mic::types::MatrixXfPtr encoded_predictions = getPredictions();

	// Backpropagate the gradients from last layer to the first.
	backward(*encoded_targets_);

	// Apply changes
	update(learning_rate_, weight_decay_);

	// Calculate the loss.
	float loss = calculateLossFunction(encoded_targets_, encoded_predictions);
	float correct = countCorrectPredictions(encoded_targets_, encoded_predictions);
	LOG(LDEBUG) << " Loss = " << std::setprecision(2) << std::setw(6) << loss << " | " << std::setprecision(1) << std::setw(4) << std::fixed << 100.0 * (float)correct / (float)encoded_batch_->cols() << "% batch correct";
	// Return loss.
	return loss;
}


float MultiLayerNeuralNetwork::test(mic::types::MatrixXfPtr encoded_batch_, mic::types::MatrixXfPtr encoded_targets_) {
	// skip dropout layers at test time
	bool skip_dropout = true;

	forward(*encoded_batch_, skip_dropout);

	// Get predictions.
	mic::types::MatrixXfPtr encoded_predictions = getPredictions();

	// Calculate the loss.
	return calculateLossFunction(encoded_targets_, encoded_predictions);
//	return countCorrectPredictions(*(getPredictions()), *encoded_targets_);

}

float MultiLayerNeuralNetwork::calculateLossFunction(mic::types::MatrixXfPtr encoded_targets_, mic::types::MatrixXfPtr encoded_predictions_) {
	mic::types::MatrixXf diff;
	// Calculate the loss.
	switch (loss_type) {
		case LossFunctionType::RegressionQuadratic:
			diff = (Eigen::MatrixXf)((*encoded_predictions_) - (*encoded_targets_));
			return (diff * diff.transpose()).sum();
			break;
		case LossFunctionType::ClassificationEntropy:
			return encoded_predictions_->calculateCrossEntropy(*encoded_targets_);
			 break;
		case LossFunctionType::Undefined:
		default:
			LOG(LERROR)<<"Loss function not set! Possible reason: the network lacks the regression/classification layer. This may cause problems with the convergence during learning!";
			return std::numeric_limits<float>::infinity();
	}//: switch

}


size_t MultiLayerNeuralNetwork::countCorrectPredictions(mic::types::MatrixXfPtr targets_, mic::types::MatrixXfPtr predictions_) {

	// Get vectors of indices denoting classes (type of 1-ouf-of-k dencoding).
	mic::types::VectorXf predicted_classes = predictions_->colwiseReturnMaxIndices();
	mic::types::VectorXf target_classes = targets_->colwiseReturnMaxIndices();

	// Get direct pointers to data.
	float *p = predicted_classes.data();
	float *t = target_classes.data();

	size_t correct=0;
	size_t i;
#pragma omp parallel for private(i) shared(correct)
	for(i=0; i< (size_t) predicted_classes.size(); i++) {
		if (p[i] == t[i]) {
#pragma omp atomic
			correct++;
		}// if
	}//: for

	return correct;
}



/* TODO: serialization */

void MultiLayerNeuralNetwork::save_to_files(std::string prefix) {

	for (size_t i = 0; i < layers.size(); i++) {

		std::ostringstream ss;
		ss << std::setw(3) << std::setfill('0') << i + 1;
		layers[i]->save_to_files(prefix + "_" + ss.str() + "_" + layers[i]->layer_name);
	}

}

size_t MultiLayerNeuralNetwork::lastLayerInputsSize() {
	return layers.back()->inputsSize();
}

size_t MultiLayerNeuralNetwork::lastLayerOutputsSize() {
	return layers.back()->outputsSize();
}

size_t MultiLayerNeuralNetwork::lastLayerBatchSize() {
	return layers.back()->batchSize();
}

mic::types::MatrixXfPtr MultiLayerNeuralNetwork::getPredictions() {
	return layers.back()->s['y'];
}



} /* namespace mlnn */
} /* namespace mic */
