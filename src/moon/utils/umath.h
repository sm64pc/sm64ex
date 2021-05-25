#ifndef UMATH_H
#define UMATH_H

#include <cmath>
#include <limits>

class MathUtil{
    public:
        inline static float Sin(float f) { return (float)sin(f); }

        // Returns the cosine of angle /f/ in radians.
        inline static float Cos(float f) { return (float)cos(f); }

        // Returns the tangent of angle /f/ in radians.
        inline static float Tan(float f) { return (float)tan(f); }

        // Returns the arc-sine of /f/ - the angle in radians whose sine is /f/.
        inline static float Asin(float f) { return (float)asin(f); }

        // Returns the arc-cosine of /f/ - the angle in radians whose cosine is /f/.
        inline static float Acos(float f) { return (float)acos(f); }

        // Returns the arc-tangent of /f/ - the angle in radians whose tangent is /f/.
        inline static float Atan(float f) { return (float)atan(f); }

        // Returns the angle in radians whose ::ref::Tan is @@y/x@@.
        inline static float Atan2(float y, float x) { return (float)atan2(y, x); }

        // Returns square root of /f/.
        inline static float Sqrt(float f) { return (float)sqrt(f); }

        // Returns the absolute value of /f/.
        inline static float Abs(float f) { return (float)abs(f); }

        // Returns the absolute value of /value/.
        inline static int Abs(int value) { return abs(value); }

        /// *listonly*
        inline static float Min(float a, float b) { return a < b ? a : b; }
        // Returns the smallest of two or more values.
        inline static float Min(float values[]){
            int len = sizeof(values);
            if (len == 0)
                return 0;
            float m = values[0];
            for (int i = 1; i < len; i++)
            {
                if (values[i] < m)
                    m = values[i];
            }
            return m;
        }

        /// *listonly*
        inline static int Min(int a, int b) { return a < b ? a : b; }
        // Returns the smallest of two or more values.
        inline static int Min(int values[])
        {
            int len = sizeof(values);
            if (len == 0)
                return 0;
            int m = values[0];
            for (int i = 1; i < len; i++)
            {
                if (values[i] < m)
                    m = values[i];
            }
            return m;
        }

        /// *listonly*
        inline static float Max(float a, float b) { return a > b ? a : b; }
        // Returns largest of two or more values.
        inline static float Max(float values[]){
            int len = sizeof(values);
            if (len == 0)
                return 0;
            float m = values[0];
            for (int i = 1; i < len; i++)
            {
                if (values[i] > m)
                    m = values[i];
            }
            return m;
        }

        /// *listonly*
        inline static int Max(int a, int b) { return a > b ? a : b; }
        // Returns the largest of two or more values.
        inline static int Max(int values[])
        {
            int len = sizeof(values);
            if (len == 0)
                return 0;
            int m = values[0];
            for (int i = 1; i < len; i++)
            {
                if (values[i] > m)
                    m = values[i];
            }
            return m;
        }

        // Returns /f/ raised to power /p/.
        inline static float Pow(float f, float p) { return (float)pow(f, p); }

        // Returns e raised to the specified power.
        inline static float Exp(float power) { return (float)exp(power); }

        // Returns the natural (base e) logarithm of a specified number.
        inline static float Log(float f) { return (float)log(f); }

        // Returns the base 10 logarithm of a specified number.
        inline static float Log10(float f) { return (float)log10(f); }

        // Returns the smallest integer greater to or equal to /f/.
        inline static float Ceil(float f) { return (float)ceil(f); }

        // Returns the largest integer smaller to or equal to /f/.
        inline static float Floor(float f) { return (float)floor(f); }

        // Returns /f/ rounded to the nearest integer.
        inline static float Round(float f) { return (float)round(f); }

        // Returns the smallest integer greater to or equal to /f/.
        inline static int CeilToInt(float f) { return (int)ceil(f); }

        // Returns the largest integer smaller to or equal to /f/.
        inline static int FloorToInt(float f) { return (int)floor(f); }

        // Returns /f/ rounded to the nearest integer.
        inline static int RoundToInt(float f) { return (int)round(f); }

        // Returns the sign of /f/.
        inline static float Sign(float f) { return f >= 0.0 ? 1.0 : -1.0; }

        // The infamous ''3.14159265358979...'' value (RO).
        const float PI = (float)PI;

        // Degrees-to-radians conversion constant (RO).
        const float Deg2Rad = PI * 2.0 / 360.0;

        // Radians-to-degrees conversion constant (RO).
        const float Rad2Deg = 1.0 / Deg2Rad;

        // Clamps a value between a minimum float and maximum float value.
        inline static float Clamp(float value, float min, float max)
        {
            if (value < min)
                value = min;
            else if (value > max)
                value = max;
            return value;
        }

        // Clamps value between min and max and returns value.
        // Set the position of the transform to be that of the time
        // but never less than 1 or more than 3
        //
        inline static int Clamp(int value, int min, int max)
        {
            if (value < min)
                value = min;
            else if (value > max)
                value = max;
            return value;
        }

        // Clamps value between 0 and 1 and returns value
        inline static float Clamp01(float value)
        {
            if (value < 0.0)
                return 0.0;
            else if (value > 1.0)
                return 1.0;
            else
                return value;
        }

        // Interpolates between /a/ and /b/ by /t/. /t/ is clamped between 0 and 1.
        inline static float Lerp(float a, float b, float t)
        {
            return a + (b - a) * Clamp01(t);
        }

        // Interpolates between /a/ and /b/ by /t/ without clamping the interpolant.
        inline static float LerpUnclamped(float a, float b, float t)
        {
            return a + (b - a) * t;
        }

        // Same as ::ref::Lerp but makes sure the values interpolate correctly when they wrap around 360 degrees.
        inline static float LerpAngle(float a, float b, float t)
        {
            float delta = Repeat((b - a), 360);
            if (delta > 180)
                delta -= 360;
            return a + delta * Clamp01(t);
        }

        // Moves a value /current/ towards /target/.
        inline static  float MoveTowards(float current, float target, float maxDelta)
        {
            if (abs(target - current) <= maxDelta)
                return target;
            return current + Sign(target - current) * maxDelta;
        }

        // Same as ::ref::MoveTowards but makes sure the values interpolate correctly when they wrap around 360 degrees.
        inline static  float MoveTowardsAngle(float current, float target, float maxDelta)
        {
            float deltaAngle = DeltaAngle(current, target);
            if (-maxDelta < deltaAngle && deltaAngle < maxDelta)
                return target;
            target = current + deltaAngle;
            return MoveTowards(current, target, maxDelta);
        }

        // Interpolates between /min/ and /max/ with smoothing at the limits.
        inline static float SmoothStep(float from, float to, float t)
        {
            t = Clamp01(t);
            t = -2.0F * t * t * t + 3.0F * t * t;
            return to * t + from * (1.0 - t);
        }

        //*undocumented
        inline static float Gamma(float value, float absmax, float gamma)
        {
            bool negative = value < 0.0;
            float absval = Abs(value);
            if (absval > absmax)
                return negative ? -absval : absval;

            float result = Pow(absval / absmax, gamma) * absmax;
            return negative ? -result : result;
        }

        // Gradually changes a value towards a desired goal over time.
        inline static float SmoothDamp(float current, float target, float currentVelocity, float smoothTime, float maxSpeed, float deltaTime)
        {
            // Based on Game Programming Gems 4 Chapter 1.10
            smoothTime = Max(0.0001F, smoothTime);
            float omega = 2.0 / smoothTime;

            float x = omega * deltaTime;
            float exp = 1.0 / (1.0 + x + 0.48F * x * x + 0.235F * x * x * x);
            float change = current - target;
            float originalTo = target;

            // Clamp maximum speed
            float maxChange = maxSpeed * smoothTime;
            change = Clamp(change, -maxChange, maxChange);
            target = current - change;

            float temp = (currentVelocity + omega * change) * deltaTime;
            currentVelocity = (currentVelocity - omega * temp) * exp;
            float output = target + (change + temp) * exp;

            // Prevent overshooting
            if (originalTo - current > 0.0F == output > originalTo)
            {
                output = originalTo;
                currentVelocity = (output - originalTo) / deltaTime;
            }

            return output;
        }


        inline static float SmoothDampAngle(float current, float target, float currentVelocity, float smoothTime, float maxSpeed, float deltaTime)
        {
            return SmoothDampAngle2(current, target, currentVelocity, smoothTime, maxSpeed, deltaTime);
        }


        inline static float SmoothDampAngle1(float current, float target, float currentVelocity, float smoothTime, float deltaTime)
        {
            float maxSpeed = (float) std::numeric_limits<double>::infinity();
            return SmoothDampAngle(current, target, currentVelocity, smoothTime, maxSpeed, deltaTime);
        }

        // Gradually changes an angle given in degrees towards a desired goal angle over time.
        inline static float SmoothDampAngle2(float current, float target, float currentVelocity, float smoothTime, float maxSpeed,  float deltaTime)
        {
            target = current + DeltaAngle(current, target);
            return SmoothDamp(current, target, currentVelocity, smoothTime, maxSpeed, deltaTime);
        }

        // Loops the value t, so that it is never larger than length and never smaller than 0.
        inline static float Repeat(float t, float length)
        {
            return Clamp((float) (t - floor(t / length) * length), 0.0f, length);
        }

        // PingPongs the value t, so that it is never larger than length and never smaller than 0.
        inline static float PingPong(float t, float length)
        {
            t = Repeat(t, length * 2.0);
            return length - Abs(t - length);
        }

        // Calculates the ::ref::Lerp parameter between of two values.
        inline static float InverseLerp(float a, float b, float value)
        {
            if (a != b)
                return Clamp01((value - a) / (b - a));
            else
                return 0.0f;
        }

        // Calculates the shortest difference between two given angles.
        inline static float DeltaAngle(float current, float target)
        {
            float delta = Repeat((target - current), 360.0F);
            if (delta > 180.0F)
                delta -= 360.0F;
            return delta;
        }
};

#endif