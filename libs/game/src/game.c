#include "game/game.h"
#include "game/placement_scene.h"

GameEC start_multiplayer_game(GameSettings settings) {
    BattleField* player1_bf = bf_init(&settings);
    if (player1_bf == NULL) return GAME_EC_AllocationError;

    GameEC ec = start_placement_screen(settings, player1_bf);
    if (ec != GAME_EC_Ok) {bf_free(player1_bf); return ec;}

    BattleField* player2_bf = bf_init(&settings);
    if (player2_bf == NULL) {bf_free(player1_bf); return GAME_EC_AllocationError;}
    ec = start_placement_screen(settings, player2_bf);
    if (ec != GAME_EC_Ok) {bf_free(player1_bf); bf_free(player2_bf); return ec;}

    bf_free(player1_bf);
    bf_free(player2_bf);
    return GAME_EC_Ok;
}

GameEC start_singleplayer_game(GameSettings settings) {

}
