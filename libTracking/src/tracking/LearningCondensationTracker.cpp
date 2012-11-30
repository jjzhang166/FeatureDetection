/*
 * LearningCondensationTracker.cpp
 *
 *  Created on: 20.09.2012
 *      Author: poschmann
 */

#include "tracking/Rectangle.h"
#include "tracking/Sample.h"
#include "tracking/LearningCondensationTracker.h"
#include "tracking/Sampler.h"
#include "tracking/LearningMeasurementModel.h"
#include "tracking/PositionExtractor.h"
#include "tracking/LearningStrategy.h"

namespace tracking {

LearningCondensationTracker::LearningCondensationTracker(shared_ptr<Sampler> sampler,
		shared_ptr<LearningMeasurementModel> measurementModel, shared_ptr<PositionExtractor> extractor,
		shared_ptr<LearningStrategy> learningStrategy) :
				samples(),
				oldSamples(),
				oldPosition(),
				offset(3),
				learningActive(true),
				sampler(sampler),
				measurementModel(measurementModel),
				extractor(extractor),
				learningStrategy(learningStrategy) {
	offset.push_back(0);
	offset.push_back(0);
	offset.push_back(0);
}

LearningCondensationTracker::~LearningCondensationTracker() {}

optional<Rectangle> LearningCondensationTracker::process(Mat& image) {
	oldSamples = samples;
	sampler->sample(oldSamples, offset, image, samples);
	// evaluate samples and extract position
	measurementModel->evaluate(image, samples);
	optional<Sample> position = extractor->extract(samples);
	// update offset
	if (oldPosition && position) {
		offset[0] = position->getX() - oldPosition->getX();
		offset[1] = position->getY() - oldPosition->getY();
		offset[2] = position->getSize() - oldPosition->getSize();
	} else {
		offset[0] = 0;
		offset[1] = 0;
		offset[2] = 0;
	}
	oldPosition = position;
	// update model
	if (learningActive) {
		if (position)
			learningStrategy->update(*measurementModel, samples, position.get());
		else
			learningStrategy->update(*measurementModel, samples);
	}
	// return position
	if (position)
		return optional<Rectangle>(position->getBounds());
	return optional<Rectangle>();
}

void LearningCondensationTracker::setLearningActive(bool active) {
	learningActive = active;
	if (!active)
		measurementModel->reset();
}

} /* namespace tracking */
