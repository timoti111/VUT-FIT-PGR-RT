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
        ~Chess();

        void drawGui();
    private:
        void saveChessState();
        bool loadChessState();
        void setDefaultChessState();
        Board board;
        std::map<Geometry::MeshInstance*, Piece> pieces;
        void drawSelectedPieceSettings();
        int row = 0, col = 0;
        std::map<std::string, std::shared_ptr<Geometry::Shape>> pieceShapes;
        int blackPiecesMaterialID;
        int whitePiecesMaterialID;
    };
}