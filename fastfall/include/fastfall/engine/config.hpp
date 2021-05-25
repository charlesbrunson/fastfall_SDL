#pragma once

constexpr unsigned int TILESIZE = 16u;
constexpr float TILESIZE_F = 16.f;

constexpr unsigned int GAME_TILE_W = 20u;
constexpr unsigned int GAME_TILE_H = 15u;

constexpr float GAME_TILE_W_F = 20.f;
constexpr float GAME_TILE_H_F = 15.f;

constexpr unsigned int GAME_W = (GAME_TILE_W * TILESIZE); // 320u
constexpr unsigned int GAME_H = (GAME_TILE_H * TILESIZE); // 240u

constexpr float GAME_W_F = (GAME_TILE_W_F * TILESIZE_F); // 320.f
constexpr float GAME_H_F = (GAME_TILE_H_F * TILESIZE_F); // 240.f
