#include "calcul_moyenne.h"

CalculMoyenne::CalculMoyenne() {
	this->moyenne_ = 0;
	this->n_ = 0;
}

void CalculMoyenne::UpdateMoyenne(float value){
	
	if (value != -1) {
		n_++;
		moyenne_ = moyenne_ + (value - moyenne_) / n_;
	}
	
}

void CalculMoyenne::initialize() {

	n_ = 0;
	moyenne_ = 0;

}

float CalculMoyenne::get_moyenne() {
	return moyenne_;
}
