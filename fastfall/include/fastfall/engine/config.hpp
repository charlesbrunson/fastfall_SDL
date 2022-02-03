#pragma once

constexpr unsigned int TILESIZE = 16u;
constexpr float TILESIZE_F = (float)TILESIZE;

constexpr unsigned int GAME_TILE_W = 20u;
constexpr unsigned int GAME_TILE_H = 15u;

constexpr float GAME_TILE_W_F = (float)GAME_TILE_W;
constexpr float GAME_TILE_H_F = (float)GAME_TILE_H;

constexpr unsigned int GAME_W = (GAME_TILE_W * TILESIZE); // 320u
constexpr unsigned int GAME_H = (GAME_TILE_H * TILESIZE); // 240u

constexpr float GAME_W_F = (GAME_TILE_W_F * TILESIZE_F); // 320.f
constexpr float GAME_H_F = (GAME_TILE_H_F * TILESIZE_F); // 240.f
