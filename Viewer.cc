#include "Viewer.h"
#include "Board.h"

#include "Player.h"
#include "Types.h"
#include "raylib.h"
#include <cassert>
#include <format>
#include <memory>
#include <optional>
#include <unordered_map>

const Color moveColor = Color(205, 210, 106, 100);

std::unordered_map<u8, Texture2D> CreatePieceTextures() {
    static std::string pieces[] = {"", "pawn", "bishop", "knight", "queen", "king", "rook", "rook"};

    auto textures = std::unordered_map<u8, Texture2D>();
    for (auto color : {PIECE_BLACK, PIECE_WHITE})
        for (auto piece : {PIECE_BISHOP, PIECE_KING, PIECE_KNIGHT, PIECE_PAWN, PIECE_QUEEN, PIECE_ROOK, PIECE_CASTLE}) {
            const auto colorStr = color == PIECE_BLACK ? "black" : "white";
            const auto path = std::format("Img/{}_{}.png", pieces[piece], colorStr);
            textures[piece | color] = LoadTexture(path.c_str());
        }
    return textures;
}

struct Transition {
    Move move = {};
    bool active = false;
    int ticks = 0;
    int maxTicks = 0;
    u8 capturedPiece = EMPTY;

    void Start(const Move &m, int newTicks, u8 captured) {
        move = m;
        active = true;
        ticks = 0;
        maxTicks = newTicks;
        capturedPiece = captured;
    }

    void Update() {
        if (!active)
            return;
        if (ticks++ >= maxTicks) {
            active = false;
            ticks = 0;
        }
    }

    float Interp() const {
        const auto t = static_cast<float>(ticks) / static_cast<float>(maxTicks);
        return t * t * (3.0f - 2.0f * t);
    }
};

struct Viewer {
    ViewerOptions options;
    Board board;
    Transition transition;
    std::optional<Move> lastMove;
    std::vector<Player> players;

    Texture2D boardTexture;
    std::unordered_map<u8, Texture2D> pieceTextures;

    Viewer(ViewerOptions v)
        : options(std::move(v)),
          board(CreateDefaultBoard()),
          transition(),
          lastMove(std::nullopt),
          players(),
          boardTexture(),
          pieceTextures() {
        boardTexture = CreateBoardTexture();
        pieceTextures = CreatePieceTextures();
        assert(options.players.size() == 2);
        for (const auto &player : options.players) {
            players.emplace_back(MakePlayer(player));
        }
    }

    inline constexpr int TileSize() const { return options.width / 8; }

    void Update() {
        const auto state = board.getBoardState(0);
        if (state != STATE_NORMAL)
            return;

        transition.Update();
        DoNextMove();
    }

    void DoNextMove() {
        if (transition.active)
            return;
        const auto idx = board.whiteMove() ? 0 : 1;
        const auto move = players[idx].GetMove(board);
        const auto captured = board.pieceAt(move.toCol, move.toRow);
        board.MakeNewMove(move);
        transition.Start(move, 15, captured);
        lastMove = move;
    }

    void Draw() const {
        BeginDrawing();
        DrawBoard();
        DrawSpecialTiles();
        DrawPieces();
        DrawTransitionPieces();
        EndDrawing();
    }

    void DrawBoard() const {
        DrawTexture(boardTexture, 0, 0, WHITE);
    }

    void DrawSpecialTiles() const {
        if (lastMove) {
            DrawRectangleF(lastMove->fromCol * TileSize(), lastMove->fromRow * TileSize(), TileSize(), TileSize(), moveColor);
            DrawRectangleF(lastMove->toCol * TileSize(), lastMove->toRow * TileSize(), TileSize(), TileSize(), moveColor);
        }
    }

    void DrawPieces() const {
        for (int col = 0; col < GRID_LENGTH; col++) {
            for (int row = 0; row < GRID_LENGTH; row++) {
                const auto piece = board.pieceAt(col, row);
                if (transition.active && transition.move.toCol == col && transition.move.toRow == row)
                    continue;
                if (piece == EMPTY)
                    continue;
                const auto texture = pieceTextures.at(piece);
                DrawTextureF(texture, col * TileSize(), row * TileSize());
            }
        }
    }

    void DrawTransitionPieces() const {
        if (!transition.active)
            return;
        const auto fromX = transition.move.fromCol * TileSize();
        const auto fromY = transition.move.fromRow * TileSize();
        const auto toX = transition.move.toCol * TileSize();
        const auto toY = transition.move.toRow * TileSize();

        const auto piece = board.pieceAt(transition.move.toCol, transition.move.toRow);
        if (piece == EMPTY)
            return;
        const auto texture = pieceTextures.at(piece);
        const auto x = fromX + (toX - fromX) * transition.Interp();
        const auto y = fromY + (toY - fromY) * transition.Interp();

        if (transition.capturedPiece != EMPTY && transition.Interp() < 0.9f)
            DrawTextureF(pieceTextures.at(transition.capturedPiece), toX, toY);
        DrawTextureF(texture, x, y);
    }

    void DrawRectangleF(int posX, int posY, int w, int h, Color color) const {
        DrawRectangle(posX, options.height - posY - TileSize(), w, h, color);
    }

    void DrawTextureF(Texture2D texture, int posX, int posY) const {
        DrawTexture(texture, posX, options.height - posY - TileSize(), WHITE);
    }

    Texture2D CreateBoardTexture() {
        auto target = LoadRenderTexture(options.width, options.height);
        BeginTextureMode(target);
        for (int col = 0; col < GRID_LENGTH; col++) {
            for (int row = 0; row < GRID_LENGTH; row++) {
                const auto color = (col + row) & 1 ? LIGHTGRAY : DARKGRAY;
                DrawRectangle(col * TileSize(), row * TileSize(), TileSize(), TileSize(), color);
            }
        }
        EndTextureMode();
        return target.texture;
    }
};

void RunViewer(const ViewerOptions &options) {

    InitWindow(options.width, options.height, options.title.c_str());
    SetTargetFPS(60);
    auto vw = Viewer(options);
    while (!WindowShouldClose()) {
        vw.Update();
        vw.Draw();
    }
}
