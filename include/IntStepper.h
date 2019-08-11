#pragma once

namespace maxmatching {
	/* Pretty much an iterator for an integer range */
	class IntStepper {
	private:
		unsigned int min;
		unsigned int max;
		unsigned int step;
	public:
		IntStepper();
		IntStepper(unsigned int value);
		IntStepper(unsigned int min, unsigned int max, unsigned int step);
		~IntStepper();

		struct Iterator {
		private:
			IntStepper* stepper;
			unsigned int current;
		public:
			Iterator(IntStepper* stepper, unsigned int startValue);
			~Iterator();
			unsigned int operator*();
			void operator++();
			bool operator==(Iterator& other);
			bool operator!=(Iterator& other);
		};

		Iterator begin();
		Iterator end();
	};
}