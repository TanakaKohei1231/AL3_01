#include "Ease.h"

float Ease::EaseIn(float t) {
	return t * t;
}

float Ease::EaseOut(float t) {
	return 1.0f - (1.0f - t) * (1.0f - t);
}

float Ease::EaseInOut(float t) {
	if (t < 0.5f) {
		return 2.0f * t * t;
	} else {
		return 1.0f - 2.0f * (1.0f - t) * (1.0f - t);
	}
}

float Ease::EaseInBack(float t) {
	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;

	return c3 * t * t * t - c1 * t * t;
}

float Ease::EaseOutBack(float t) {
	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;

	float x = t - 1.0f;

	return 1.0f + c3 * x * x * x + c1 * x * x;
}
