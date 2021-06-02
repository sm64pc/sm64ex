#ifndef AnimationAlgorithmAPI
#define AnimationAlgorithmAPI

#include <cmath>
#define M_PI 3.14159265358979323846

class AnimationAlgorithms {
public:
    static inline float linear(float delta){
        return delta;
    }

    static inline float easeInOut(float delta){
        if (delta <= 0.5) { // first half of the animation
            return quad(2 * delta) / 2;
        } else { // second half of the animation
            return (2 - quad(2 * (1 - delta))) / 2;
        }
    }

    static inline float quad(float delta){
        return quad(delta, 2);
    }

    static inline float quad(float delta, int power){
        return (float) pow(delta, power);
    }

    static inline float circ(float delta){
        return (float) (1 - sin(cos(delta)));
    }

    static inline float bounce(float delta){
        for (int i = 0, j = 1; true; i+=j, j /=2) {
            if (delta >= (7 - 4 * i) / 11) {
                return (float) (-pow((11 - 6 * i - 11 * delta) / 4, 2) + pow(j, 2));
            }
        }
    }

    static inline float back(float x, float delta){
        return (float) (pow(delta, 2) * ((x + 1) * delta - x));
    }

    static inline float elastic(float x, float delta){
        return (float) (pow(2, 10 * (delta - 1)) * cos(20 * M_PI * x / 3 * delta));
    }
};

#endif