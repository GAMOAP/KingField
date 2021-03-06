//
//  SceneIntro.cpp
//  kingfield-mobile
//
//  Created by alexandre gimeno on 25/09/2019.
//

#include "SceneIntro.hpp"

#include "Constants.h"

#include "MainMultiPlayer.hpp"

#include "GameDirector.hpp"
#include "GameInfoLayer.hpp"
#include "GameBoxes.hpp"
#include "GameCharacters.hpp"
#include "GameCards.hpp"

#include <iostream>

USING_NS_CC;

SceneIntro* SceneIntro::setScene()
{
    if (!m_SharedSceneIntro)
    {
        m_SharedSceneIntro = new (std::nothrow) SceneIntro;
        m_SharedSceneIntro->init();
    }
    return m_SharedSceneIntro;
}

bool SceneIntro::init()
{
    setName("intro");;
    
    m_removeAuth = false;
    
    return true;
}

void SceneIntro::addToStage()
{
    GameInfoLayer::addIntroTitle();
    
    GameBoxes::setBoxes();
    GameCharacters::setCharacters(0);
    
    auto gameCards = GameCards::getInstance();
    gameCards->removeDeck();
    gameCards->removeSheet();
    gameCards->removeLibrary();
}

void SceneIntro::removeToStage()
{
    GameInfoLayer::removeIntroTitle();
    GameDirector::setScene("barrack");
}


bool SceneIntro::allNodeIsIn()
{
    if(!MULTI_PLAYER_ON)
        removeToStage();
    else
        MainMultiPlayer::connect();
    
    return true;
}

bool SceneIntro::appConnected()
{
    GameInfoLayer::removeConnectSprite();
    removeToStage();
    return true;
}
