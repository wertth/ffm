//
// Created by 缪浩楚 on 2024/6/16.
//
#include "fmt/core.h"
#include "player.h"

int main () {
    Player* player = new Player("./assets/test.avi");
    player->main_event_loop();
}
