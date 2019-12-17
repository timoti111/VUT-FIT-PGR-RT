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
            BLACK, WHITE
        };

        Piece(Board* board, int materialID, Color color, std::shared_ptr<Geometry::MeshInstance> instance);
        bool placeAt(std::string index);
        bool drawGui();
        Geometry::MeshInstance* getInstance();
        Color getColor();
        std::string getIndex();
        std::string getName();

    protected:
        Board* board;
        Color color;
        std::shared_ptr<Geometry::MeshInstance> instance;
        int row = 0, col = 0;
        std::string name;
    };
}