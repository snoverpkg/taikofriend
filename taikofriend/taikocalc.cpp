#include "taikocalc.h"
#include <math.h>

void calcMS(Chart* c, Mods mods) {
    c->noteData.NoteMS.push_back(defaultMS);
    float rateModifier = 1;
    if (mods & Mods::HT) rateModifier *= 0.75F;
    if (mods & Mods::DT) rateModifier *= 1.5F;
    for (int note = 1; note < c->noteData.NoteInfo.size(); note++) {
        int curMS = c->noteData.NoteInfo[note].first;
        int prevMS = c->noteData.NoteInfo[note - 1].first;
        c->noteData.NoteMS.push_back(std::max((float)(curMS - prevMS) / rateModifier, minMS));
    }
}

void smoothMS(Chart* c) {
    if (smoothWindow > c->noteData.NoteMS.size()) return;

    std::vector<float> smoothie;
    for (int i = 0; i < smoothWindow; i++) {
        smoothie.push_back(defaultMS);
    }

    for (int note = 0; note < c->noteData.NoteMS.size(); note++) {
        smoothie.erase(smoothie.begin());
        float curMS = c->noteData.NoteMS[note];
        smoothie.push_back(curMS);
        float totalMS = 1;
        for (int i = 0; i < smoothWindow; i++) {
            totalMS *= std::pow(smoothie[i], (1.F / smoothWindow));
        }
        //totalMS /= smoothWindow;
        totalMS = (1 - curNoteWeight) * totalMS + curNoteWeight * curMS;
        c->noteData.adjMS.push_back(totalMS);
    }
}

void baseDiffCalc(Chart* c) {
    for (int i = 0; i < c->noteData.adjMS.size(); i++) {
        c->noteData.baseDiffs.push_back(1000 / (c->noteData.adjMS[i] / baseScaler));
    }
}

float ptLoss(float x, float y) {
    /*
    // a = horizontal stretch factor
    // b = vertical stretch factor
    // c = horizontal shift
    float a = 5.8F;
    float b = 0.9F;
    float c = 1.2F;
    return std::max((1 / (1 - (1 / b) / 2)) * (std::erf((y - x) / a - c) + b), 0.F);
    */
    //return std::min(pow(std::max(y - x, 0.0F) / 10.F, 1.F), 1.F);
    return std::clamp(std::erf((y - x) / 10.F), 0.F, 1.F);
}

double calcEffOD(Chart* c, Mods mods) {
    double od = c->metaData.od;
    if (mods & Mods::HR) od *= 1.4F;
    if (mods & Mods::EZ) od *= 0.5F;
    od = std::clamp(od, (double)0, (double)10);
    if (mods & Mods::HT) {
        od /= 0.75F;
        od -= 5.5F;
    }
    if (mods & Mods::DT) {
        od /= 1.5F;
        od += 5.5F;
    }
    return od;
}

//nerf files under 90 seconds
float grindScaler(float length, Mods mods) {
    float timeMult = 1.F;
    if (mods & Mods::DT) timeMult /= 1.5F;
    if (mods & Mods::HT) timeMult /= 0.75F;
    return std::clamp(-1.F / sqrt(length * timeMult / 1000.F) + 1.10541F, 0.F, 1.F);
}

float diffIteration(std::vector<float>* diffs, float goal) {
    float ratingLow = 0.F;
    float ratingHigh = 40.F;
    float rating;

    if (goal > accCap) {
        goal = accCap;
    }

    int maxPts = diffs->size();
    float goalPts = maxPts * goal;
    float totalPtLoss;

    for (int i = 0; i < 15; i++) {
        rating = (ratingHigh + ratingLow) / 2;
        totalPtLoss = 0;
        for (int j = 0; j < maxPts; j++) {
            float curDiff = diffs->at(j);
            totalPtLoss += ptLoss(rating, curDiff);
        }
        if (maxPts - totalPtLoss < goalPts) { //rating too low
            ratingLow = (ratingHigh + ratingLow) / 2;
        }
        else {
            ratingHigh = (ratingHigh + ratingLow) / 2;
        }
    }
    rating = (ratingHigh + ratingLow) / 2;
    return rating;
}

float calcMain(Chart* c, float goal, Mods mods) {
    if (c->noteData.NoteInfo.size() == 0) {
        std::cout << "file not found: " << 
            c->metaData.artist << " - " <<
            c->metaData.title << " (" <<
            c->metaData.creator << ") [" <<
            c->metaData.diff << "]" << std::endl;
        return 0;
    }

    float accCapMult = accLinearExtrapolation(goal);

    calcMS(c, mods);
    smoothMS(c);
    baseDiffCalc(c);

    noteInterpreter(c);

    c->noteData.adj_diffs = c->noteData.baseDiffs;

    //main pmod calc part sex indian orgy desi feet
    ChaosMod.calcChaos(c);
    CDMod.calcCD(c);
    CD2Mod.calcCD(c);
    LengthMod.calcBonus(c);

    //run stammod on base diffs since that's a better representation of the actual physical difficulty
    StamMod.calcStam(c, diffIteration(&c->noteData.baseDiffs, goal));

    //pmod apply loop
    for (int i = 0; i < c->noteData.baseDiffs.size(); i++) {
        //c->NoteData.adj_diffs[i] *= ChaosMod.pmodValues[i];
        c->noteData.adj_diffs[i] *= CDMod.pmodValues[i];
        c->noteData.adj_diffs[i] *= CD2Mod.pmodValues[i];
        c->noteData.adj_diffs[i] *= StamMod.pmodValues[i];
        c->noteData.adj_diffs[i] *= LengthMod.pmodValues[i];
    }

    //remember to actually clear the pmodvalues so it doesn't break the whole calc...
    ChaosMod.pmodValues.clear();
    CDMod.pmodValues.clear();
    CD2Mod.pmodValues.clear();
    StamMod.pmodValues.clear();
    LengthMod.pmodValues.clear();

    float od = calcEffOD(c, mods);
    float odMult = odAdjust(od);
    float grindMult = grindScaler(c->noteData.NoteInfo[c->noteData.NoteInfo.size() - 1].first - c->noteData.NoteInfo[0].first, mods);
    return diffIteration(&c->noteData.adj_diffs, goal) * odMult * accCapMult * grindMult;
}

float odAdjust(double od) {
    return sqrt(pow(1.3F, od - 14) + 0.88F);
}

float accLinearExtrapolation(float acc) {
    acc = std::min(acc, 1.F);
    const float scaler = 3.F;
    return std::max(1.F, 1.F + (acc - accCap) * scaler);
}