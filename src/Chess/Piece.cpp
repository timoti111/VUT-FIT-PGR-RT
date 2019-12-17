#include "Piece.h"
#include "Board.h"
#include <imgui.h>

Chess::Piece::Piece(Board* board, int materialID, Color color, std::shared_ptr<Geometry::MeshInstance> instance) :
    board(board), color(color), instance(instance)
{
    instance->setMaterialID(materialID);
    name = instance->parent->name;
}

bool Chess::Piece::placeAt(std::string index)
{
    try
    {
        board->placePiece(this, index);
        auto indices = Board::stringToIndex(index);
        col = indices.first;
        row = indices.second;
        return true;
    }
    catch (const Board::SquareOccupied & e)
    {
    }
    return false;
}

bool Chess::Piece::drawGui()
{
    static const char* cols[] = { "A", "B", "C", "D", "E", "F", "G", "H" };
    static const char* rows[] = { "1", "2", "3", "4", "5", "6", "7", "8" };
    bool ret = false;
    int rowLast = row;
    int colLast = col;
    ImGui::Begin((name + " Settings").c_str());
    ImGui::Text("Board position:");
    ImGui::Combo("Col", &col, cols, IM_ARRAYSIZE(cols));
    ImGui::Combo("Row", &row, rows, IM_ARRAYSIZE(rows));
    if (rowLast != row || colLast != col)
    {
        if (placeAt(Board::indexToString(col, row)))
        {
            board->free(Board::indexToString(colLast, rowLast));
            ret = true;
        }
        else
        {
            row = rowLast;
            col = colLast;
        }
    }
    ImGui::End();
    return ret;
}

Geometry::MeshInstance* Chess::Piece::getInstance()
{
    return instance.get();
}

Chess::Piece::Color Chess::Piece::getColor()
{
    return color;
}

std::string Chess::Piece::getIndex()
{
    return Board::indexToString(col, row);
}

std::string Chess::Piece::getName()
{
    return name;
}
