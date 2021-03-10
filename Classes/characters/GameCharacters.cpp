//
//  GameCharacters.cpp
//  kingfield-mobile
//
//  Created by alexandre gimeno on 05/07/2019.
//
#include "Constants.h"

#include "MainObject.hpp"
#include "MainStuff.hpp"
#include "MainAction.hpp"

#include "MainDirector.hpp"

#include "GameBoxes.hpp"
#include "GameCards.hpp"

#include "GameCharacters.hpp"

#include <iostream>

USING_NS_CC;

GameCharacters* GameCharacters::getInstance()
{
    if (!m_SharedGameCharacters)
    {
        m_SharedGameCharacters = new (std::nothrow) GameCharacters;
        m_SharedGameCharacters->init();
    }
    return m_SharedGameCharacters;
}

bool GameCharacters::init()
{
    return true;
}

void GameCharacters::setCharacters(int charNbr)
{
    if(!m_SharedGameCharacters){ getInstance();}
    
    for(int c = 0; c < c_charNumber ; c++)
    {
        auto character = MainObject::getCharByNumber(c);
        if(character && c >= charNbr)
        {
            character->removeToStage();
        }
        if(!character && c < charNbr)
        {
            auto character = Character::setCharacter(c);
            if(c == 2){m_SharedGameCharacters->m_kingFriend = character;}
            if(c == 7){m_SharedGameCharacters->m_kingEnemy = character;}
            
            MainStuff::initCharSpec(c);
        }
    }
}
void GameCharacters::removeAllCharacters()
{
    auto nodeList = MainObject::getMainLayer()->getChildren();
    Vector<Node*>::iterator nlIt;
    for(nlIt = nodeList.begin(); nlIt != nodeList.end(); nlIt++)
    {
        auto character = dynamic_cast<Character*>(*nlIt);
        if(character)
        {
            GameBoxes::setBoxUnselect(character->getTag());
            character->removeToStage();
        }
    }
    m_SharedGameCharacters->m_charSelected = nullptr;
    if(m_SharedGameCharacters->m_characterUI)
    {
        m_SharedGameCharacters->m_characterUI->removeCharacterUI();
    }
}

//----------------------------CHARACTER SELECT------------------------------------
void GameCharacters::setCharSelect(int number)
{
    if(number < 0)
    {
        if(m_SharedGameCharacters->m_charNumberMemory >= 0)
            number = m_SharedGameCharacters->m_charNumberMemory;
        else
            number = 2;
    }
    
    auto charSelected = m_SharedGameCharacters->m_charSelected;
    if(charSelected)
        m_SharedGameCharacters->m_charNumberMemory = charSelected->getNumber();
    
    if(!charSelected || charSelected->getNumber() != number)
    {
        auto sceneChildren = MainObject::getMainLayer()->getChildren();
        Vector<Node*>::iterator scIt;
        for(scIt = sceneChildren.begin(); scIt != sceneChildren.end(); scIt++)
        {
            auto character = dynamic_cast<Character*>(*scIt);
            if(character)
            {
                if(character->getNumber() == number)
                {
                    m_SharedGameCharacters->m_charSelected = character;
                    if(MainDirector::getScene()->getIsPlayerTurn())
                    {
                        GameBoxes::setBoxSelect(character->getTag());
                        character->setSelect();
                    }
                    m_SharedGameCharacters->m_characterUI = CharacterUI::setCharacterUI(number);
                    GameCards::resetCards();
                }
                else
                {
                    GameBoxes::setBoxUnselect(character->getTag());
                    character->setUnselect();
                }
            }
        }
    }
}
Character* GameCharacters::getCharSelect()
{
    Character* character = m_SharedGameCharacters->m_charSelected;
    if(!m_SharedGameCharacters->m_charSelected)
    {
        int charSelectNbrMem = m_SharedGameCharacters->m_charNumberMemory;
        if(charSelectNbrMem < 0)
            charSelectNbrMem = 2;
        
        character = MainObject::getCharByNumber(charSelectNbrMem);
    }
    return character;
}
bool GameCharacters::getCharIsSelected()
{
    if(!m_SharedGameCharacters->m_charSelected)
        return false;
    else
        return true;
}
void GameCharacters::unselectAll()
{
    auto charSelected = m_SharedGameCharacters->m_charSelected;
    if(charSelected)
    {
        m_SharedGameCharacters->m_charNumberMemory = charSelected->getNumber();
        GameBoxes::setBoxUnselect(m_SharedGameCharacters->m_charSelected->getTag());
    }
    
    if(m_SharedGameCharacters->m_charSelected)
    {
        m_SharedGameCharacters->m_charSelected->setUnselect();
        m_SharedGameCharacters->m_charSelected = nullptr;
    }
    
    if(m_SharedGameCharacters->m_characterUI)
    {
        m_SharedGameCharacters->m_characterUI->removeCharacterUI();
        m_SharedGameCharacters->m_characterUI = nullptr;
    }
}
//----------------------------KING-------------------------------
Character* GameCharacters::getKingFriend()
{
    return m_SharedGameCharacters->m_kingFriend;
}

//-------------------------CHAR MEMEORY----------------------------
int GameCharacters::getCharNumberMemory()
{
    return m_SharedGameCharacters->m_charNumberMemory;
}

//---------------------------ACTION-------------------------------
void GameCharacters::setAction(std::vector<KFAction*> actionSequence)
{
    m_SharedGameCharacters->unselectAll();
    GameCards::unselectAll();
    GameBoxes::unselectAll();
    
    auto character = MainObject::getCharByNumber(actionSequence[0]->getCharNbr());
    auto charIsPlace = EventListenerCustom::create("NODE_char" + std::to_string(character->getTag())+"_IS_PLACE", [=](EventCustom* event)
    {
        MainStuff::setCharSpec(character->getNumber(), "crystal", -actionSequence[0]->getCost());
        
        m_SharedGameCharacters->m_sequenceState = 0;
        m_SharedGameCharacters->m_actionSequence = actionSequence;
        m_SharedGameCharacters->setActionSequence();
    });
    auto eventDispatcher = Director::getInstance()->getEventDispatcher();
    eventDispatcher->addEventListenerWithSceneGraphPriority(charIsPlace, character);
    
    if(character && character->getIsPlace())
    {
        eventDispatcher->dispatchCustomEvent("NODE_char" + std::to_string(character->getTag())+"_IS_PLACE");
        eventDispatcher->removeCustomEventListeners("NODE_char" + std::to_string(character->getTag())+"_IS_PLACE");
    }
}
void GameCharacters::setActionSequence()
{
    if(m_sequenceState >= m_actionSequence.size())
    {
        m_actionSequence.clear();
        MainDirector::endTurn();
    }
    else
    {
        auto action = m_actionSequence[m_sequenceState];
        auto character = MainObject::getCharByNumber(action->getCharNbr());
        character->setAction(action);
        auto charIsActionEnd = EventListenerCustom::create("NODE_char" + std::to_string(character->getNumber()) + "_END_ACTION", [=](EventCustom* event)
        {
            character->getEventDispatcher()->removeCustomEventListeners("NODE_char" + std::to_string(character->getNumber()) + "_END_ACTION");
            action->~KFAction();
            
            m_sequenceState++;
            setActionSequence();
        });
        auto eventDispatcher = Director::getInstance()->getEventDispatcher();
        eventDispatcher->addEventListenerWithSceneGraphPriority(charIsActionEnd, character);
    }
}

void GameCharacters::setActionAll(std::string actionName)
{
    auto sceneChildren = MainObject::getMainLayer()->getChildren();
    Vector<Node*>::iterator scIt;
    for(scIt = sceneChildren.begin(); scIt != sceneChildren.end(); scIt++)
    {
        auto character = dynamic_cast<Character*>(*scIt);
        if(character)
        {
            if(actionName == "give_crystals" &&
               MainDirector::getScene()->getIsTeamTurn(character->getNumber()))
            {
                MainStuff::setCharSpec(character->getNumber(), "crystal", +1);
            }
        }
    }
}
//---------------------------------CHARACTER UI------------------------------
CharacterUI* GameCharacters::getCharUI()
{
    return m_SharedGameCharacters->m_characterUI;
}