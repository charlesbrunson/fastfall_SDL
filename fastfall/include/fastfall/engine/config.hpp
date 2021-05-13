#pragma once

constexpr unsigned int TILESIZE = 16u;
constexpr float TILESIZE_F = 16.f;

constexpr unsigned int GAME_TILE_W = 24u;
constexpr unsigned int GAME_TILE_H = 18u;

constexpr float GAME_TILE_W_F = 24.f;
constexpr float GAME_TILE_H_F = 18.f;

constexpr unsigned int GAME_W = (GAME_TILE_W * TILESIZE); // 240u
constexpr unsigned int GAME_H = (GAME_TILE_H * TILESIZE); // 160u

constexpr float GAME_W_F = (GAME_TILE_W_F * TILESIZE_F); // 240.f
constexpr float GAME_H_F = (GAME_TILE_H_F * TILESIZE_F); // 160.f
