#pragma once
#include <vector>
#include "osureader.h"

class PMod {
private:
public:
	std::vector<float> pmodValues;
};

class ConsecutiveDoubles : public PMod {
public:
	const float base = 0.99F;
	const float maxMod = 1.0F;
	const float minMod = 0.50F;
	void calcCD(Chart* c);
};

class Chaos : public PMod {
public:
	const int chaosWindow = 14;
	const float base = 0.7F;
	const float scaler = 0.9F;
	const float maxMod = 1.0F;
	const float minMod = 1.F;
	float calcCV(std::vector<float>* msVals);
	void calcChaos(Chart* c);
};

class Stamina : public PMod {
public:
	const float floor = 0.98F;
	const float magnitude = 1500.F;
	const float stamProp = 0.7F;
	const float coolingRate = 1.5F;
	const float ceil = 1.05F;
	void calcStam(Chart* c, float rating);
};

class GeneralPatternComplexity : public PMod {
public:

};

extern Chaos ChaosMod;
extern ConsecutiveDoubles CDMod;
extern Stamina StamMod;
extern GeneralPatternComplexity ComplexMod;