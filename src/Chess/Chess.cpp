#include <imgui.h>
#include "Chess.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Utils/tinyxml2.h"

Chess::Chess::Chess(std::string boardDirectory, std::string setDirectory) :
    Scene(),
    board(boardDirectory)
{
    addShape(board.getBoardModel());
    //drawableMaterials.emplace(std::make_pair(std::string("Board Material"), board.getMaterial()));
    pieceShapes.emplace("Bishop", Geometry::Shape::fromObjFile(setDirectory + "Bishop.obj", "Bishop"));
    pieceShapes.emplace("King", Geometry::Shape::fromObjFile(setDirectory + "King.obj", "King"));
    pieceShapes.emplace("Knight", Geometry::Shape::fromObjFile(setDirectory + "Knight.obj", "Knight"));
    pieceShapes.emplace("Pawn", Geometry::Shape::fromObjFile(setDirectory + "Pawn.obj", "Pawn"));
    pieceShapes.emplace("Queen", Geometry::Shape::fromObjFile(setDirectory + "Queen.obj", "Queen"));
    pieceShapes.emplace("Rook", Geometry::Shape::fromObjFile(setDirectory + "Rook.obj", "Rook"));
    addShape(pieceShapes.at("Bishop"));
    addShape(pieceShapes.at("King"));
    addShape(pieceShapes.at("Knight"));
    addShape(pieceShapes.at("Pawn"));
    addShape(pieceShapes.at("Queen"));
    addShape(pieceShapes.at("Rook"));

    auto blackPiecesMaterial = Material::generateNewMaterial(blackPiecesMaterialID);
    blackPiecesMaterial->Kd = glm::vec4(0.1f);
    blackPiecesMaterial->type = DIFFUSE;
    drawableMaterials.emplace(std::make_pair(std::string("Black Pieces Material"), blackPiecesMaterialID));

    auto whitePiecesMaterial = Material::generateNewMaterial(whitePiecesMaterialID);
    whitePiecesMaterial->Kd = glm::vec4(0.9f);
    whitePiecesMaterial->type = DIFFUSE;
    drawableMaterials.emplace(std::make_pair(std::string("White Pieces Material"), whitePiecesMaterialID));

    if (!loadChessState())
        setDefaultChessState();

    updateSceneBVH = true;
}

Chess::Chess::~Chess()
{
    saveChessState();
}

void Chess::Chess::drawSelectedPieceSettings()
{
    if (actualSelectedMesh == nullptr)
        return;

    if (!pieces.count(actualSelectedMesh))
        return;

    if (pieces.at(actualSelectedMesh).drawGui())
    {
        updateSelectedMesh = true;
        updateBVHs();
    }
}

void Chess::Chess::drawGui()
{
    drawSelectedPieceSettings();
    drawMaterialSettings();
    drawLightSettings();
}

void Chess::Chess::saveChessState()
{
    tinyxml2::XMLDocument doc;
    auto blackPiecesElem = doc.NewElement("BlackPieces");
    auto whitePiecesElem = doc.NewElement("WhitePieces");
    for (auto& piece : pieces)
    {
        if (piece.second.getColor() == Piece::Color::BLACK)
        {
            auto pieceElem = doc.NewElement(piece.second.getName().c_str());
            pieceElem->SetText(piece.second.getIndex().c_str());
            blackPiecesElem->InsertEndChild(pieceElem);
        }
        if (piece.second.getColor() == Piece::Color::WHITE)
        {
            auto pieceElem = doc.NewElement(piece.second.getName().c_str());
            pieceElem->SetText(piece.second.getIndex().c_str());
            whitePiecesElem->InsertEndChild(pieceElem);
        }
    }
    doc.InsertEndChild(blackPiecesElem);
    doc.InsertEndChild(whitePiecesElem);
    doc.SaveFile("ChessConfiguration.xml");
}

bool Chess::Chess::loadChessState()
{
    tinyxml2::XMLDocument doc;

    if (doc.LoadFile("ChessConfiguration.xml") != tinyxml2::XML_SUCCESS)
        return false;
    auto firstBlackPiece = doc.FirstChildElement("BlackPieces")->FirstChild();
    while (firstBlackPiece != nullptr)
    {
        auto name = std::string(firstBlackPiece->ToElement()->Value());
        auto index = std::string(firstBlackPiece->ToElement()->GetText());
        instantiateShape(name, 0, true);
        auto instance = pieceShapes.at(name)->meshes[0].instances.back();
        pieces.emplace(std::make_pair(instance.get(), Piece(&board, blackPiecesMaterialID, Piece::Color::BLACK, instance)));
        pieces.at(instance.get()).placeAt(index);
        firstBlackPiece = firstBlackPiece->NextSibling();
    }
    auto firstWhitePiece = doc.FirstChildElement("WhitePieces")->FirstChild();
    while (firstWhitePiece != nullptr)
    {
        auto name = std::string(firstWhitePiece->ToElement()->Value());
        auto index = std::string(firstWhitePiece->ToElement()->GetText());
        instantiateShape(name, 0, true);
        auto instance = pieceShapes.at(name)->meshes[0].instances.back();
        pieces.emplace(std::make_pair(instance.get(), Piece(&board, whitePiecesMaterialID, Piece::Color::WHITE, instance)));
        pieces.at(instance.get()).placeAt(index);
        firstWhitePiece = firstWhitePiece->NextSibling();
    }
    return true;
}

void Chess::Chess::setDefaultChessState()
{
    std::string bishopIndices[] = { "C1", "F1", "C8", "F8" };
    std::string kingIndices[] = { "E1", "E8" };
    std::string knightIndices[] = { "B1", "G1", "B8", "G8" };
    std::string pawnIndices[] = {
        "A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2",
        "A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7"
    };
    std::string queenIndices[] = { "D1", "D8" };
    std::string rookIndices[] = { "A1", "H1", "A8", "H8" };

    for (int team = 0; team < 2; team++)
    {
        int materialID;
        Piece::Color color;
        if (team == 0)
        {
            materialID = blackPiecesMaterialID;
            color = Piece::Color::BLACK;
        }
        else
        {
            materialID = whitePiecesMaterialID;
            color = Piece::Color::WHITE;
        }
        for (int i = 0; i < 2; i++)
        {
            instantiateShape("Bishop", 0, true);
            auto instance = pieceShapes.at("Bishop")->meshes[0].instances.back();
            pieces.emplace(std::make_pair(instance.get(), Piece(&board, materialID, color, instance)));
            pieces.at(instance.get()).placeAt(bishopIndices[i + team * 2]);
        }
        for (int i = 0; i < 1; i++)
        {
            instantiateShape("King", 0, true);
            auto instance = pieceShapes.at("King")->meshes[0].instances.back();
            pieces.emplace(std::make_pair(instance.get(), Piece(&board, materialID, color, instance)));
            pieces.at(instance.get()).placeAt(kingIndices[i + team * 1]);
        }
        for (int i = 0; i < 2; i++)
        {
            instantiateShape("Knight", 0, true);
            auto instance = pieceShapes.at("Knight")->meshes[0].instances.back();
            pieces.emplace(std::make_pair(instance.get(), Piece(&board, materialID, color, instance)));
            pieces.at(instance.get()).placeAt(knightIndices[i + team * 2]);
        }
        for (int i = 0; i < 8; i++)
        {
            instantiateShape("Pawn", 0, true);
            auto instance = pieceShapes.at("Pawn")->meshes[0].instances.back();
            pieces.emplace(std::make_pair(instance.get(), Piece(&board, materialID, color, instance)));
            pieces.at(instance.get()).placeAt(pawnIndices[i + team * 8]);
        }
        for (int i = 0; i < 1; i++)
        {
            instantiateShape("Queen", 0, true);
            auto instance = pieceShapes.at("Queen")->meshes[0].instances.back();
            pieces.emplace(std::make_pair(instance.get(), Piece(&board, materialID, color, instance)));
            pieces.at(instance.get()).placeAt(queenIndices[i + team * 1]);
        }
        for (int i = 0; i < 2; i++)
        {
            instantiateShape("Rook", 0, true);
            auto instance = pieceShapes.at("Rook")->meshes[0].instances.back();
            pieces.emplace(std::make_pair(instance.get(), Piece(&board, materialID, color, instance)));
            pieces.at(instance.get()).placeAt(rookIndices[i + team * 2]);
        }
    }
}
