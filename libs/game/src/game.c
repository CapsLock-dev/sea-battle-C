#include "game/game.h"
#include "game/placement_scene.h"

void start_game(GameSettings settings) {
    BattleField* bf = bf_init(&settings);

    start_placement_screen(settings, bf);

    bf_free(bf);
}
