//
//  SceneFight.cpp
//  kingfield-mobile
//
//  Created by alexandre gimeno on 03/10/2019.
//

#include "SceneFight.hpp"

#include "Constants.h"

#include "MainObject.hpp"
#include "MainStuff.hpp"
#include "MainMultiPlayer.hpp"
#include "MainAction.hpp"

#include "MainDirector.hpp"
#include "MainInfoLayer.hpp"

#include "GameBoxes.hpp"
#include "GameCharacters.hpp"
#include "GameCards.hpp"

#include <iostream>

USING_NS_CC;

SceneFight* SceneFight::setScene()
{
    if (!m_SharedSceneFight)
    {
        m_SharedSceneFight = new (std::nothrow) SceneFight;
        m_SharedSceneFight->init();
    }
    return m_SharedSceneFight;
}

bool SceneFight::init()
{
    setName("fight");
    
    return true;
}

void SceneFight::addToStage()
{
    GameBoxes::setBoxes();
    
    auto gameCharacters = GameCharacters::getInstance();
    gameCharacters->setCharacters(5);
    gameCharacters->unselectAll();
    
    auto gameCards = GameCards::getInstance();
    gameCards->removeSheet();
    gameCards->removeLibrary();
    gameCards->unselectAll();
    
    if(!c_multiPlayerOn)
        startFight(0);
    else
    {
        gameCards->removeDeck();
        
        GameBoxes::startRumbleBox(m_fightFieldTag);
        MainMultiPlayer::joinRoom();
    }
}

void SceneFight::removeToStage()
{
    GameBoxes::setUnselectActionBoxes();
    
    MainDirector::setScene("barrack");
}

bool SceneFight::allNodeIsIn()
{
    if(!GameCharacters::getCharIsSelected())
    {
        GameCharacters::setCharSelect();
    }
        
    if(!GameCards::getCardSelect())
    {
        GameCards::setCardSelect(0, "deck");
    }
    else
        GameCards::CardsReseted();
    
    setActionBoxTags();
    
    return true;
}

//-----------------------FIGHT EVENT--------------------------------
bool SceneFight::startFight(int teamNumber)
{
    m_SharedSceneFight->m_teamNumber = teamNumber;
    m_SharedSceneFight->m_turnNumber = 0;
    
    GameBoxes::stopRumbleBox();
    
    GameBoxes::setBoxes();
    
    GameCharacters::setCharacters(10);
    GameCards::addDeck();
    
    return true;
}
bool SceneFight::stopFight(bool isWin)
{
    m_SharedSceneFight->removeToStage();
    
    return true;
}
bool SceneFight::endTurn()
{
    m_SharedSceneFight->m_turnNumber++;
    GameCharacters::setActionAll("give_crystals");
    
    MainDirector::setActionInProgress(false);
    
    //select char, card and actionBoxes.
    m_SharedSceneFight->allNodeIsIn();
    
    return true;
}

//----------------------------TOUCH---------------------------------
bool SceneFight::touchBox(int tag)
{
    if(getIsUsedBox(tag) || getIsBoardTag(tag))
    {
        setTouchObject(tag);
        m_touchedBox = tag;
    }
    else
        m_touchedBox = NAN;
    
    return true;
}

bool SceneFight::unTouchBox(int tag)
{
    if(tag == m_touchedBox)
    {
        auto character = MainObject::getCharByTag(tag);
        auto card = GameCards::getCardByTag(tag);
        auto box = MainObject::getBoxByTag(tag);
        
        //select character.
        if(character && !box->getIsActionUI())
        {
            GameCharacters::setCharSelect(character->getNumber());
            GameBoxes::setUnselectActionBoxes();
        }
        //select card.
        if(card && !card->getIsSelect())
        {
            GameCards::setCardSelect(card->getNumber(), card->getBoard());
            GameBoxes::setUnselectActionBoxes();
            setActionBoxTags();
        }
        //select actionBox.
        if(box && box->getIsActionUI() && m_SharedSceneFight->getIsPlayerTurn())
        {
            int charSelectNbr = GameCharacters::getCharSelect()->getNumber();
            int cardSelectNbr = GameCards::getCardSelect()->getNumber();
            
            if(c_multiPlayerOn)
                MainMultiPlayer::sendCharacterActionData(charSelectNbr, cardSelectNbr, tag);
            
            auto actionSequence = MainAction::getActionSequence(charSelectNbr, cardSelectNbr, tag);
            GameCharacters::setAction(actionSequence);
        }
    }
    cancelTouchObject();
    m_touchedBox = NAN;
    
    return true;
}

bool SceneFight::cancelTouchBox(int tag)
{
    cancelTouchObject();
    return true;
}

bool SceneFight::longTouchBox(int tag)
{
    
    
    return true;
}

bool SceneFight::getIsPlayerTurn()
{
    int charSelectNbr;
    auto character = GameCharacters::getCharSelect();
    if(character)
        charSelectNbr = character->getNumber();
    else
        return false;
    
    if(!c_multiPlayerOn)
    {
        if(charSelectNbr < 5 && m_turnNumber%2 != 0)
            return false;
        
        if(charSelectNbr >= 5 && m_turnNumber%2 == 0)
            return false;
    }
    else
    {
        if(charSelectNbr >= 5)
            return false;
        if(m_turnNumber%2 != m_teamNumber)
            return false;
    }
    return true;
           
}
bool SceneFight::getIsTeamTurn(int charNbr)
{
    if((m_turnNumber%2 == m_teamNumber && charNbr >= 5) ||
       (m_turnNumber%2 != m_teamNumber && charNbr < 5))
        return false;
    else
        return true;
}

bool SceneFight::setActionBoxTags()
{
    int charNbr = GameCharacters::getCharSelect()->getNumber();
    auto cardSelect = GameCards::getCardSelect();
    std::string className = cardSelect->getClassName();
    int tag = cardSelect->getTag();
    auto IsCardUp = EventListenerCustom::create("NODE_"+ className + std::to_string(tag)+"_IS_UP", [=](EventCustom* event)
    {
        int cardNbr = cardSelect->getNumber();
        std::vector<Vec3> actionBoxTags = MainAction::getActionBoxesTags(charNbr, cardNbr);
        GameBoxes::setSelectActionBoxes(actionBoxTags);
    });
    auto eventDispatcher = Director::getInstance()->getEventDispatcher();
    eventDispatcher->addEventListenerWithSceneGraphPriority(IsCardUp, cardSelect);
    
    return true;
}

bool SceneFight::getIsBoardTag(int tag)
{
    bool isBoardTag = false;
    for(int t = 0; t < m_boardTag.size(); t++)
    {
        if(tag == m_boardTag[t])
            isBoardTag = true;
    }
    return isBoardTag;
}