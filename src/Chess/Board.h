#pragma once
#include <memory>
#include <map>
#include <exception>
#include <glm/vec3.hpp>
#include "Piece.h"

namespace Chess
{
    class Board
    {
    public:
        Board(std::string boardDirectory); // Vytvori shape, nacita informacie o ploche zo suboru board.xml
        void placePiece(Piece* piece, std::string index);
        void free(std::string index);
        std::shared_ptr<Geometry::Shape> getBoardModel();
        void setAllowOverlapping(bool enabled);

        struct SquareOccupied : public std::exception
        {
            SquareOccupied(Piece* by);
            Piece* by;
            const char* what() const throw ();
        };
        static std::string indexToString(int col, int row);
        static std::pair<int, int> stringToIndex(std::string string);

    private:
        void initialize();
        bool overlapping = false;
        glm::vec3 a1Position;
        glm::vec3 h8Position;
        std::shared_ptr<Geometry::Shape> boardModel;
        std::map<std::string, std::pair<Piece*, glm::vec3>> boardIndexPositions;
    };
}