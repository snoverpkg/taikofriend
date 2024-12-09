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

class ConsecutiveDoublesTheSequel : public PMod {
public:
	const float base = 0.99F;
	const float maxMod = 1.0F;
	const float minMod = 0.50F;
	void calcCD(Chart* c);
};

class Chaos : public PMod {
public:
	const int chaosWindow = 4;
	const float base = 0.9F;
	const float scaler = 0.2F;
	const float maxMod = 1.2F;
	const float minMod = 1.F;
	const float chaosThreshold = 0.1F;
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

class StreamLengthBonus : public PMod {
public:
	//magic numbers created by a magic system of equations
	const float base = 0.25F;
	const float scaler = 0.460567F;
	const float magnitude = 0.379634F;
	const float minMod = 1.F;
	const float maxMod = 1.F + base;
	const float stringProp = 0.6F;
	void calcBonus(Chart* c);
};

extern Chaos ChaosMod;
extern ConsecutiveDoubles CDMod;
extern ConsecutiveDoublesTheSequel CD2Mod;
extern Stamina StamMod;
extern StreamLengthBonus LengthMod;