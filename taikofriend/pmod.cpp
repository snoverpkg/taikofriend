#include "pmod.h"
#include "taikocalc.h"

#pragma region Chaos
float Chaos::calcCV(std::vector<float>* msVals) {
	float cv = 0.F;
	float stdev = 0.F;
	float mean = 0.F;
	for (int i = 0; i < msVals->size(); i++) {
		mean += msVals->at(i);
	}
	mean /= msVals->size();
	for (int i = 0; i < msVals->size(); i++) {
		stdev += std::pow(msVals->at(i) - mean, 2);
	}
	stdev /= msVals->size();
	stdev = std::sqrt(stdev);

	cv = stdev / mean;
	return cv;
}

void Chaos::calcChaos(Chart* c) {
	this->pmodValues.clear();
	bool chaosFlag = false;
	std::vector<bool> chaosFlags;
	chaosFlags.push_back(chaosFlag);
	for (int i = 1; i < c->NoteData.NoteMS.size(); i++) {
		float curNote = c->NoteData.NoteMS[i];
		float lastNote = c->NoteData.NoteMS[i - 1];
		if (std::abs(curNote - lastNote) > curNote * this->chaosThreshold) {
			chaosFlag = true;
		}
		else {
			chaosFlag = false;
		}
		chaosFlags.push_back(chaosFlag);
	}

	int chaosSum = 0;
	std::vector<int> windowSums;
	for (int i = 0; i < c->NoteData.NoteMS.size(); i++) {
		chaosSum = 0;
		for (int j = 0; (j < chaosWindow) && (i - j >= 0); j++) {
			chaosSum += chaosFlags[j];
		}
		windowSums.push_back(chaosSum);
	}

	for (int i = 0; i < windowSums.size(); i++) {
		this->pmodValues.push_back(std::clamp(windowSums[i] * scaler / (float)chaosWindow + base, this->minMod, this->maxMod));
	}
}
#pragma endregion

#pragma region ConsecutiveDoubles
void ConsecutiveDoubles::calcCD(Chart* c) {
	std::vector<int> even;
	std::vector<int> odd;

	bool curNoteColor = 0;
	bool lastNoteColor = 0;
	bool parity = 0;
	for (int i = 0; i < c->NoteData.NoteInfo.size(); i++) {
		if (i == 0) {
			even.push_back(0);
			continue;
		}
		lastNoteColor = curNoteColor;
		curNoteColor = c->NoteData.NoteInfo[i].second & 1;
		parity = i % 2;
		switch (parity) {
		case 0:
			if (curNoteColor == lastNoteColor) {
				even.push_back(1);
			}
			else {
				even.push_back(0);
			}
			break;
		case 1:
			if (curNoteColor == lastNoteColor) {
				odd.push_back(1);
			}
			else {
				odd.push_back(0);
			}
			break;
		default:
			break;
		}
	}

	std::vector<float> evenVals;
	std::vector<float> oddVals;
	int counter = 0;
	for (int i = 0; i < even.size(); i++) {
		if (even[i] == 0) {
			counter = 0;
		}
		else {
			counter++;
		}
		evenVals.push_back(pow(this->base, counter * counter));
	}

	counter = 0;
	for (int i = 0; i < odd.size(); i++) {
		if (odd[i] == 0) {
			counter = 0;
		}
		else {
			counter++;
		}
		oddVals.push_back(pow(this->base, counter * counter));
	}

	unsigned int index = 0;
	float min = 1.F;
	for (int i = 0; i < c->NoteData.NoteInfo.size(); i++) {
		index = std::floor((float)i / 2.F);
		min = evenVals[index];
		if (index < oddVals.size()) {
			min = std::min(min, oddVals[index]);
		}
		this->pmodValues.push_back(std::clamp(min, minMod, maxMod));
	}
}
#pragma endregion

#pragma region Stamina
void Stamina::calcStam(Chart* c, float rating) {
	float curMod = this->floor;
	for (int i = 0; i < c->NoteData.baseDiffs.size(); i++) {
		if ((c->NoteData.baseDiffs[i] / rating) >= this->stamProp) {
			curMod += c->NoteData.NoteMS[i] / (this->magnitude * 1000.F);
		}
		else {
			curMod -= this->coolingRate * c->NoteData.NoteMS[i] / (this->magnitude * 1000.F);
		}
		curMod = std::clamp(curMod, this->floor, this->ceil);
		this->pmodValues.push_back(curMod);
	}
}
#pragma endregion