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
    auto bishop = Geometry::Shape::fromObjFile(setDirectory + "Bishop.obj", "Bishop");
    auto king = Geometry::Shape::fromObjFile(setDirectory + "King.obj", "King");
    auto knight = Geometry::Shape::fromObjFile(setDirectory + "Knight.obj", "Knight");
    auto pawn = Geometry::Shape::fromObjFile(setDirectory + "Pawn.obj", "Pawn");
    auto queen = Geometry::Shape::fromObjFile(setDirectory + "Queen.obj", "Queen");
    auto rook = Geometry::Shape::fromObjFile(setDirectory + "Rook.obj", "Rook");
    addShape(bishop);
    addShape(king);
    addShape(knight);
    addShape(pawn);
    addShape(queen);
    addShape(rook);

    std::string bishopIndices[] = { "C1", "F1", "C8", "F8" };
    std::string kingIndices[] = { "E1", "E8" };
    std::string knightIndices[] = { "B1", "G1", "B8", "G8" };
    std::string pawnIndices[] = {
        "A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2",
        "A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7"
    };
    std::string queenIndices[] = { "D1", "D8" };
    std::string rookIndices[] = { "A1", "H1", "A8", "H8" };

    for (int color = 0; color < 2; color++)
    {
        int materialID;
        if (color == 0)
        {
            auto blackPiecesMaterial = Material::generateNewMaterial(materialID);
            blackPiecesMaterial->Kd = glm::vec4(0.1f);
            blackPiecesMaterial->type = DIFFUSE;
            drawableMaterials.emplace(std::make_pair(std::string("Black Pieces Material"), materialID));
        }
        else
        {
            auto whitePiecesMaterial = Material::generateNewMaterial(materialID);
            whitePiecesMaterial->Kd = glm::vec4(0.9f);
            whitePiecesMaterial->type = DIFFUSE;
            drawableMaterials.emplace(std::make_pair(std::string("White Pieces Material"), materialID));
        }
        for (int i = 0; i < 2; i++)
        {
            instantiateShape("Bishop", 0, true);
            auto instance = bishop->meshes[0].instances.back();
            pieces.emplace(std::make_pair(instance.get(), Piece(&board, materialID, instance)));
            pieces.at(instance.get()).placeAt(bishopIndices[i + color * 2]);
        }
        for (int i = 0; i < 1; i++)
        {
            instantiateShape("King", 0, true);
            auto instance = king->meshes[0].instances.back();
            pieces.emplace(std::make_pair(instance.get(), Piece(&board, materialID, instance)));
            pieces.at(instance.get()).placeAt(kingIndices[i + color * 1]);
        }
        for (int i = 0; i < 2; i++)
        {
            instantiateShape("Knight", 0, true);
            auto instance = knight->meshes[0].instances.back();
            pieces.emplace(std::make_pair(instance.get(), Piece(&board, materialID, instance)));
            pieces.at(instance.get()).placeAt(knightIndices[i + color * 2]);
        }
        for (int i = 0; i < 8; i++)
        {
            instantiateShape("Pawn", 0, true);
            auto instance = pawn->meshes[0].instances.back();
            pieces.emplace(std::make_pair(instance.get(), Piece(&board, materialID, instance)));
            pieces.at(instance.get()).placeAt(pawnIndices[i + color * 8]);
        }
        for (int i = 0; i < 1; i++)
        {
            instantiateShape("Queen", 0, true);
            auto instance = queen->meshes[0].instances.back();
            pieces.emplace(std::make_pair(instance.get(), Piece(&board, materialID, instance)));
            pieces.at(instance.get()).placeAt(queenIndices[i + color * 1]);
        }
        for (int i = 0; i < 2; i++)
        {
            instantiateShape("Rook", 0, true);
            auto instance = rook->meshes[0].instances.back();
            pieces.emplace(std::make_pair(instance.get(), Piece(&board, materialID, instance)));
            pieces.at(instance.get()).placeAt(rookIndices[i + color * 2]);
        }
    }
    updateSceneBVH = true;
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
}
