#include "Viewer.h"

#include "Bitboard.h"
#include "Player.h"
#include "Types.h"
#include "raylib.h"
#include <cassert>
#include <format>
#include <memory>
#include <optional>
#include <unordered_map>

const Color moveColor = Color(205, 210, 106, 100);

std::unordered_map<u8, Texture2D> CreatePieceTextures()
{
    static std::string pieces[] = {"pawn", "king", "queen", "bishop", "knight", "rook"};

    auto textures = std::unordered_map<u8, Texture2D>();
    for (auto color : {CBLACK, CWHITE})
        for (auto piece : {PAWN, KING, QUEEN, BISHOP, KNIGHT, ROOK}) {
            const auto colorStr = color == CBLACK ? "black" : "white";
            const auto path = std::format("Img/{}_{}.png", pieces[piece], colorStr);
            textures[MakePiece(color, piece)] = LoadTexture(path.c_str());
        }
    return textures;
}

struct Transition {
    Move move = {};
    bool active = false;
    int ticks = 0;
    int maxTicks = 0;
    u8 capturedPiece = NONE;

    void Start(const Move &m, int newTicks, u8 captured)
    {
        move = m;
        active = true;
        ticks = 0;
        maxTicks = newTicks;
        capturedPiece = captured;
    }

    void Update()
    {
        if (!active)
            return;
        if (ticks++ >= maxTicks) {
            active = false;
            ticks = 0;
        }
    }

    float Interp() const
    {
        const auto t = static_cast<float>(ticks) / static_cast<float>(maxTicks);
        return t * t * (3.0f - 2.0f * t);
    }
};

struct Viewer {
    ViewerOptions options;
    Position position;
    Transition transition;
    std::optional<Move> lastMove;
    std::vector<Player> players;

    Texture2D boardTexture;
    std::unordered_map<u8, Texture2D> pieceTextures;

    Viewer(ViewerOptions v)
        : options(std::move(v)),
          position(CreateDefaultPosition()),
          transition(),
          lastMove(std::nullopt),
          players(),
          boardTexture(),
          pieceTextures()
    {
        boardTexture = CreateBoardTexture();
        pieceTextures = CreatePieceTextures();
        assert(options.players.size() == 2);
        for (const auto &player : options.players) {
            players.emplace_back(MakePlayer(player));
        }
    }

    inline constexpr int TileSize() const { return options.width / 8; }

    void Update()
    {
        // const auto state = board.getBoardState(0);
        // if (state != STATE_NORMAL)
        //     return;

        transition.Update();
        DoNextMove();
    }

    void DoNextMove()
    {
        if (transition.active)
            return;
        const auto idx = position.whoseturn == CWHITE ? 0 : 1;
        const auto move = players[idx].GetMove(position);
        const auto captured = GetPiece(position, ColOf(move.to), RowOf(move.to));
        MakeMove(position, move);
        transition.Start(move, 15, captured);
        lastMove = move;
    }

    void Draw() const
    {
        BeginDrawing();
        DrawBoard();
        DrawSpecialTiles();
        DrawPieces();
        DrawTransitionPieces();
        EndDrawing();
    }

    void DrawBoard() const
    {
        DrawTexture(boardTexture, 0, 0, WHITE);
    }

    void DrawSpecialTiles() const
    {
        if (lastMove) {
            DrawRectangleF(ColOf(lastMove->from) * TileSize(), RowOf(lastMove->from) * TileSize(), TileSize(), TileSize(), moveColor);
            DrawRectangleF(ColOf(lastMove->to) * TileSize(), RowOf(lastMove->to) * TileSize(), TileSize(), TileSize(), moveColor);
        }
    }

    void DrawPieces() const
    {
        for (int col = 0; col < GRID_LENGTH; col++) {
            for (int row = 0; row < GRID_LENGTH; row++) {
                const auto piece = GetPiece(position, col, row);
                if (transition.active && ColOf(transition.move.to) == col && RowOf(transition.move.to) == row)
                    continue;
                if (piece == NONE)
                    continue;
                const auto texture = pieceTextures.at(piece);
                DrawTextureF(texture, col * TileSize(), row * TileSize());
            }
        }
    }

    void DrawTransitionPieces() const
    {
        if (!transition.active)
            return;
        const auto fromX = ColOf(transition.move.from) * TileSize();
        const auto fromY = RowOf(transition.move.from) * TileSize();
        const auto toX = ColOf(transition.move.to) * TileSize();
        const auto toY = RowOf(transition.move.to) * TileSize();

        const auto piece = GetPiece(position, ColOf(transition.move.to), RowOf(transition.move.to));
        if (piece == NONE)
            return;
        const auto texture = pieceTextures.at(piece);
        const auto x = fromX + (toX - fromX) * transition.Interp();
        const auto y = fromY + (toY - fromY) * transition.Interp();

        if (transition.capturedPiece != NONE && transition.Interp() < 0.9f)
            DrawTextureF(pieceTextures.at(transition.capturedPiece), toX, toY);
        DrawTextureF(texture, x, y);
    }

    void DrawRectangleF(int posX, int posY, int w, int h, Color color) const
    {
        DrawRectangle(posX, options.height - posY - TileSize(), w, h, color);
    }

    void DrawTextureF(Texture2D texture, int posX, int posY) const
    {
        DrawTexture(texture, posX, options.height - posY - TileSize(), WHITE);
    }

    Texture2D CreateBoardTexture()
    {
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

void RunViewer(const ViewerOptions &options)
{

    InitWindow(options.width, options.height, options.title.c_str());
    SetTargetFPS(60);
    auto vw = Viewer(options);
    while (!WindowShouldClose()) {
        vw.Update();
        vw.Draw();
    }
}
