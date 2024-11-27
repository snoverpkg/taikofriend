#pragma once
#include "osureader.h"
#include "pmod.h"

const float defaultMS = 5000.F;
const float minMS = 10.F;

const int smoothWindow = 5;
const float curNoteWeight = 0.F;

const float baseScaler = 1.F;
const float ppscaler = 2.0F;

const float accCap = 0.98F;

enum Mods {
	EZ = 1,
	HT = 2,
	HR = 4,
	DT = 8
};

void calcMS(Chart* c, Mods mods);
void smoothMS(Chart* c);
void baseDiffCalc(Chart* c);

float ptLoss(float x, float y);

double calcEffOD(Chart* c, Mods mods);

float diffIteration(std::vector<float>* diffs, float goal);

float calcMain(Chart* c, float goal, Mods mods);

float odAdjust(double od);

float accLinearExtrapolation(float acc);

const int calcver = 12;