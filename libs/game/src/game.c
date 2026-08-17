#include "game/game.h"
#include "game/placement_scene.h"
#include "game/battle_scene.h"

GameEC start_multiplayer_game(GameSettings settings) {
    BattleField* player1_bf = bf_init(&settings);
    if (player1_bf == NULL) return GAME_EC_AllocationError;

    GameEC ec = start_placement_screen(settings, player1_bf);
    if (ec != GAME_EC_PlacementFinished) {bf_free(player1_bf); return ec;}

    BattleField* player2_bf = bf_init(&settings);
    if (player2_bf == NULL) {bf_free(player1_bf); return GAME_EC_AllocationError;}
    ec = start_placement_screen(settings, player2_bf);
    if (ec != GAME_EC_PlacementFinished) {bf_free(player1_bf); bf_free(player2_bf); return ec;}

    ec = start_battle_scene(player1_bf, player2_bf, false);
    if (ec != GAME_EC_Ok) {bf_free(player1_bf); bf_free(player2_bf); return ec;}

    bf_free(player1_bf);
    bf_free(player2_bf);
    return GAME_EC_Ok;
}

GameEC start_singleplayer_game(GameSettings settings) {
    BattleField* player1_bf = bf_init(&settings);
    GameEC ec = 0;

    ec = start_placement_screen(settings, player1_bf);
    if (ec != GAME_EC_PlacementFinished) {bf_free(player1_bf); return ec;}

    BattleField* player2_bf = bf_init_random(&settings);

    ec = start_battle_scene(player1_bf, player2_bf, true);
    if (ec != GAME_EC_Ok) {bf_free(player1_bf); bf_free(player2_bf); return ec;}

    bf_free(player1_bf);
    bf_free(player2_bf);

    return ec;
}
