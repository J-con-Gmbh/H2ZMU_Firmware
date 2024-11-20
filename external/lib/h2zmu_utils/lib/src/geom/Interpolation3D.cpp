//
// Created by jriessner on 13.06.23.
//

#include "../../include/geom/Interpoaltion3D.h"


namespace utils {
    namespace geom {
        float toPowerOf(float base, int power) {
            for (int i = 0; i < power - 1; ++i) {
                base *= base;
            }

            return base;
        }

        struct Point3D subtractPoints(struct Point3D a, struct Point3D b) {
            float x = a.x - b.x;
            float y = a.y - b.y;
            float z = a.z - b.z;

            return {x, y, z};
        }

        struct Point3D addPoints(struct Point3D a, struct Point3D b) {
            float x = b.x + a.x;
            float y = b.y + a.y;
            float z = b.z + a.z;

            return {x, y, z};
        }

        struct Point3D multiplyVector(struct Point3D vector, float factor) {
            float x = vector.x * factor;
            float y = vector.y * factor;
            float z = vector.z * factor;

            return {x, y, z};
        }

        float vectorLength(Vector3D vector) {
            return sqrt(toPowerOf(vector.x, 2) + toPowerOf(vector.y, 2) + toPowerOf(vector.z, 2));
        }

        Line3D::Line3D(Point3D start, Point3D end) {
            this->_start = start;
            this->_end = end;
            this->lineDesc = subtractPoints(this->_end, this->_start);
            this->length = vectorLength(this->lineDesc);
            this->factor = 1 / this->length;
            this->normal = multiplyVector(this->lineDesc, this->factor);
        }

        Point3D Line3D::getPointForX(float x) {
            if (this->_start.x <= x > this->_end.x) {
                return {0, 0, 0};
            }
            float delta = x - this->_start.x;
            Vector3D vector = multiplyVector(this->normal, delta);

            return addPoints(this->_start, vector);
        }

        Point3D Line3D::getPointForY(float y) {
            if (this->_start.y <= y > this->_end.y) {
                return {0, 0, 0};
            }
            float delta = y - this->_start.y;
            Vector3D vector = multiplyVector(this->normal, delta);

            return addPoints(this->_start, vector);
        }


        float interpolateZ(Point3D A, Point3D B, Point3D C, Point3D D, Point3D XY) {

            float x = XY.x;
            float y = XY.y;

            Line3D AB = Line3D(A, B);
            Line3D CD = Line3D(D, C);

            Point3D E = AB.getPointForX(x);
            Point3D F = CD.getPointForX(x);

            Line3D EF = Line3D(E, F);

            Point3D X = EF.getPointForY(y);

            return X.z;
        }


        bool test() {
            float x = 2;
            float y = 2;

            Point3D A = {2, 1, 1.5};
            Point3D B = {4, 1, 2};
            Point3D C = {4, 2, 2.5};
            Point3D D = {2, 2, 2};

            Line3D AB = Line3D(A, B);
            Line3D CD = Line3D(D, C);

            Point3D E = AB.getPointForX(x);
            Point3D F = CD.getPointForX(x);

            Line3D EF = Line3D(E, F);

            Point3D X = EF.getPointForY(y);

            return std::to_string(X.z).find("1.9472") != -1;

        }
    }
}
