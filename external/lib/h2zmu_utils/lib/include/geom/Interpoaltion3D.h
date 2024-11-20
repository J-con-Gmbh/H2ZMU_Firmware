//
// Created by jriessner on 22.01.23.
//

#ifndef H2ZMU_2_INTERPOALTION3D_H
#define H2ZMU_2_INTERPOALTION3D_H

#include <cmath>
#include <iostream>

namespace utils {
    namespace geom {

        struct Point3D {
            float x;
            float y;
            float z;
        };

        typedef Point3D Vector3D;

        float toPowerOf(float base, int power);

        struct Point3D subtractPoints(struct Point3D a, struct Point3D b);

        struct Point3D addPoints(struct Point3D a, struct Point3D b);

        struct Point3D multiplyVector(struct Point3D vector, float factor);

        float vectorLength(Vector3D vector);

        class Line3D {
        private:
            Point3D _start;
            Point3D _end;
            Vector3D lineDesc;
            float length;
            float factor;
            Vector3D normal;

        public:
            Line3D(Point3D start, Point3D end);

            Point3D getPointForX(float x);

            Point3D getPointForY(float y);

        };

        float interpolateZ(Point3D A, Point3D B, Point3D C, Point3D D, Point3D XY);

        bool test();
    }
}

#endif //H2ZMU_2_INTERPOALTION3D_H
