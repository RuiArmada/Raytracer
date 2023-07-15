#pragma once
#include "geometry.hpp"

class Primitive {
    public:
        Primitive() = default;
        Primitive(Geometry geometry, int material_index = 0);
        
        Geometry geometry;
        int material_index;
};