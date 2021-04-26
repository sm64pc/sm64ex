#include <math.h>

float clamp(float value, float min, float max) {
    if (value < min)
        value = min;
    else if (value > max)
        value = max;
    return value;
}

float repeat(float t, float length) {
    return clamp((float) (t - floor(t / length) * length), 0.0f, length);
}

float ping_pong(float t, float length) {
    t = repeat(t, length * 2.0);
    return length - abs(t - length);
}

float clamp_01(float value){
    if (value < 0.0)
        return 0.0;
    else if (value > 1.0)
        return 1.0;
    else
        return value;
}

// Interpolates between /a/ and /b/ by /t/. /t/ is clamped between 0 and 1.
float lerp(float a, float b, float t) {
    return a + (b - a) * clamp_01(t);
}