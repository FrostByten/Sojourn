#include "ServerEnemyController.h"

#include "../NetworkEntityPairs.h"
#include "../Artificial Intelligence/Behaviour.h"

#include <cstdio>

ServerEnemyController::ServerEnemyController(Behaviour *behaviour)
    : ServerNetworkController((int)NetworkEntityPair::SERVERCONTROLLER_NETCONTROLLER)
     ,behaviour(behaviour)
{
}

ServerEnemyController::~ServerEnemyController()
{
}

void ServerEnemyController::init()
{
    if (behaviour)
        behaviour->init();
}

void ServerEnemyController::updateBehaviour(float deltaTime)
{
    if (behaviour)
        behaviour->update(deltaTime);
}