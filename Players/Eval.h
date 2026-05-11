#pragma once

#include "../Bitboard.h"

int EvaluateMaterial(const Position &p, CColor color);
int EvaluationCCCP(Position &p, const Move &m);
int EvaluationRawMaterial(Position &p, const Move &m);
int EvaluationHuddle(Position &p, const Move &m);
int EvaluationSwarm(Position &p, const Move &m);
int EvaluationGlue(Position &p, const Move &m);
int EvaluationWhiteSquares(Position &p, const Move &m);

int EvaluationMirrorX(Position &p, const Move &m);
int EvaluationMirrorY(Position &p, const Move &m);
int EvaluationMirrorXY(Position &p, const Move &m);

int EvaluationCenter(Position &p, const Move &m);
int EvaluationEdge(Position &p, const Move &m);

int EvaluationMinResponse(Position &p, const Move &m);
int EvaluationMinSelf(Position &p, const Move &m);
