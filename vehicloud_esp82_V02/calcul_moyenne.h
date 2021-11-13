#pragma once

#ifndef _CALCUL_MOYENNE_H_
#define _CALCUL_MOYENNE_H_


class CalculMoyenne {

protected:

	float moyenne_;
	int n_;

public:

	CalculMoyenne();
	void UpdateMoyenne(float);
	void initialize();
	float get_moyenne();
};

#endif
