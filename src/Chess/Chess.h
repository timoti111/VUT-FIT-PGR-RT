#pragma once
#include <string>
#include <map>
#include "Scene/Scene.h"
#include "Board.h"
#include <map>

namespace Chess
{
    class Chess : public Scene
    {
    public:
        Chess(std::string boardDirectory , std::string setDirectory);

        void drawSelectedPieceSettings();
    private:
        Board board;
        std::map<Geometry::MeshInstance*, Piece> pieces;
        int row = 0, col = 0;
    };
}