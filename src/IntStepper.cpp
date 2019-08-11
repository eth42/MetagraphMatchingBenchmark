#include "IntStepper.h"
#include <iostream>

namespace maxmatching {
	IntStepper::IntStepper()
		: IntStepper(0, 0, 1) {}
	IntStepper::IntStepper(unsigned int value)
		: IntStepper(value, value, 1) {}
	IntStepper::IntStepper(unsigned int min, unsigned int max, unsigned int step)
		: min(min)
		, max(max)
		, step(step) {}

	IntStepper::~IntStepper() {}

	IntStepper::Iterator::Iterator(IntStepper* stepper, unsigned int startValue)
		: stepper(stepper)
		, current(startValue) {}
	IntStepper::Iterator::~Iterator() {}
	unsigned int IntStepper::Iterator::operator*() {
		return this->current;
	}
	void IntStepper::Iterator::operator++() {
		this->current += stepper->step;
	}
	bool IntStepper::Iterator::operator==(Iterator& other) {
		return this->current == other.current
			|| ((this->current < stepper->min || this->current > stepper->max)
				&& (other.current < stepper->min || other.current > stepper->max));
	}
	bool IntStepper::Iterator::operator!=(Iterator & other) {
		return !(*this == other);
	}


	IntStepper::Iterator IntStepper::begin() {
		return Iterator(this, this->min);
	}
	IntStepper::Iterator IntStepper::end() {
		return Iterator(this, this->max + 1);
	}
}