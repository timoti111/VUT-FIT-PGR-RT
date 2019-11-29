#pragma once
#include "Scene/Geometry/Shape.h"

namespace Chess
{
    class Board;
    class Piece
    {
    public:
        enum class Color
        {
            WHITE, BLACK
        };

        Piece(Board* board, int color, std::shared_ptr<Geometry::MeshInstance> instance);
        bool placeAt(std::string index);
        bool drawGui();
        Geometry::MeshInstance* getInstance();
        Color getColor();
        std::string getIndex();

    protected:
        Board* board;
        Color color;
        std::shared_ptr<Geometry::MeshInstance> instance;
        int row = 0, col = 0;
    };
}