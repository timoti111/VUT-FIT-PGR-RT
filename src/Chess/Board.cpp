#include "Board.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Utils/tinyxml2.h"

Chess::Board::Board(std::string boardDirectory)
{
    boardModel = Geometry::Shape::fromObjFile(boardDirectory + "Board.obj", "Board");
    boardModel->instantiate(glm::mat4x4(1.0f), 0, true);

    tinyxml2::XMLDocument doc;
    doc.LoadFile((boardDirectory + "Board.xml").c_str());
    auto a1PositionElm = doc.FirstChildElement("board")->FirstChildElement("a1")->FirstChildElement("position");
    auto h8PositionElm = doc.FirstChildElement("board")->FirstChildElement("h8")->FirstChildElement("position");
    piecesScale = std::stof(doc.FirstChildElement("board")->FirstChildElement("pieceScale")->GetText());
    a1Position = glm::vec3(
        std::stof(a1PositionElm->FirstChildElement("x")->GetText()),
        std::stof(a1PositionElm->FirstChildElement("y")->GetText()),
        std::stof(a1PositionElm->FirstChildElement("z")->GetText())
    );
    h8Position = glm::vec3(
        std::stof(h8PositionElm->FirstChildElement("x")->GetText()),
        std::stof(h8PositionElm->FirstChildElement("y")->GetText()),
        std::stof(h8PositionElm->FirstChildElement("z")->GetText())
    );

    initialize();
}

void Chess::Board::placePiece(Piece* piece, std::string index)
{
    static glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    auto& square = boardIndexPositions.at(index);
    if (square.first == nullptr || overlapping)
    {
        if (piece != nullptr)
        {
            int color = static_cast<int>(piece->getColor());
            auto transform = glm::translate(glm::mat4x4(1.0f), square.second);
            transform = glm::rotate(transform, glm::radians(180.0f) * color, up);
            transform = glm::scale(transform, glm::vec3(piecesScale));
            piece->getInstance()->setObjectToWorld(transform);
            square.first = piece;
        }
        return;
    }
    throw SquareOccupied(square.first);
}

void Chess::Board::free(std::string index)
{
    auto& square = boardIndexPositions.at(index);
    square.first = nullptr;
}

std::shared_ptr<Geometry::Shape> Chess::Board::getBoardModel()
{
    return boardModel;
}

void Chess::Board::setAllowOverlapping(bool enabled)
{
    overlapping = enabled;
}

std::string Chess::Board::indexToString(int col, int row)
{
    static std::vector<std::string> cols = { "A", "B", "C", "D", "E", "F", "G", "H" };
    static std::vector<std::string> rows = { "1", "2", "3", "4", "5", "6", "7", "8" };
    return cols[col] + rows[row];
}

std::pair<int, int> Chess::Board::stringToIndex(std::string string)
{
    static std::vector<char> cols = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H' };
    static std::vector<char> rows = { '1', '2', '3', '4', '5', '6', '7', '8' };
    int col = 0, row = 0;
    for (int i = 0; i < cols.size(); i++)
    {
        if (cols[i] == string[0])
            col = i;
        if (rows[i] == string[1])
            row = i;
    }
    return std::make_pair(col, row);
}

void Chess::Board::initialize()
{
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 a1h8 = h8Position - a1Position;
    glm::vec3 mid = (a1Position + h8Position) * 0.5f;
    glm::vec3 midH1 = glm::normalize(glm::cross(a1h8, up));
    float a1h8length = glm::length(a1h8);
    glm::vec3 h1Position = mid + (a1h8length * 0.5f) * midH1;
    glm::vec3 a8Position = mid - (a1h8length * 0.5f) * midH1;
    float t = 1.0f / 7.0f;

    for (int i = 0; i < 8; i++)
    {
        float v = t * i;
        for (int j = 0; j < 8; j++)
        {
            float u = t * j;
            auto col1Position = glm::mix(a1Position, h1Position, u);
            auto col8Position = glm::mix(a8Position, h8Position, u);
            auto position = glm::mix(col1Position, col8Position, v);
            boardIndexPositions.emplace(std::make_pair(indexToString(j, i), std::make_pair(nullptr, position)));
        }
    }

}

Chess::Board::SquareOccupied::SquareOccupied(Piece* by) : by(by)
{}

const char* Chess::Board::SquareOccupied::what() const throw()
{
    return "Square Occupied";
}
