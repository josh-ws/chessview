#pragma once

#include "../Bitboard.h"

int EvaluateMaterial(const Position &p, CColor color);
int EvaluationCCCP(Position &p, const Move &m);
int EvaluationRawMaterial(Position &p, const Move &m);
int EvaluationHuddle(Position &p, const Move &m);
int EvaluationSwarm(Position &p, const Move &m);

// Evaluation that assesses how close the pieces are to each other
int EvaluationGlue(Position &p, const Move &m);

// Assesses how many pieces are on white squares
int EvaluationWhiteSquares(Position &p, const Move &m);

int EvaluationMirrorX(Position &p, const Move &m);
int EvaluationMirrorY(Position &p, const Move &m);
int EvaluationMirrorXY(Position &p, const Move &m);
