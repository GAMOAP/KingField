//
//  MainAction.cpp
//  kingfield-mobile
//
//  Created by alexandre gimeno on 23/08/2019.
//

#include "Constants.h"
#include "KFSpecCard.hpp"

#include "MainStuff.hpp"
#include "MainObject.hpp"

#include "MainAction.hpp"

#include <iostream>

USING_NS_CC;

std::vector<Vec3> MainAction::getActionBoxesTags(int charSelectNbr, int cardSelectNbr)
{
    std::vector<std::string> cardName = MainStuff::getStuffByName(charSelectNbr, cardSelectNbr);
    KFSpecCard* specCard = MainStuff::getCardSpec(CARD_TYPE[cardSelectNbr], cardName[0], cardName[1]);
    
    std::vector<Vec3> actionBoxesTags;
    std::vector<std::vector<int>> board = specCard->getBoard();
    
    const int charCrystal = MainStuff::getCharSpec(charSelectNbr)["crystal"];
    const int charCrystalRed = MainStuff::getCharSpec(charSelectNbr)["crystal_red"];
    const int cardCost = specCard->getMana(false);
    
    if(cardCost <= charCrystal && charCrystal > charCrystalRed )
    {
        std::vector<int>forbidenTagList = setForbiddenTagList(charSelectNbr, board);
        for(int b = 0; b < board.size(); b++)
        {
            std::vector<int> boardAction = board[b];
            const int startTag = getStartTag(charSelectNbr, boardAction[0], boardAction[1]);
            int moveType = boardAction[2];
            
            for(int m = 3; m < boardAction.size(); m++)
            {
                int tag = startTag + getBoardMap(charSelectNbr)[boardAction[m]];
                switch (moveType) {
                        // type move.
                    case 0:
                        if(isInBoardTag(tag) && !isCharTag(tag) && !isForbiddenTag(tag, forbidenTagList))
                            actionBoxesTags.push_back(Vec3(startTag, tag, moveType));
                        break;
                        // type strike.
                    case 1:
                        if(isInBoardTag(tag) && isCharTag(tag))
                        {
                            printf("startTag = %i, tag = %i\n",startTag,tag);
                            actionBoxesTags.push_back(Vec3(startTag, tag, moveType));
                        }
                        break;
                        // type spell.
                    case 2:
                        if(isInBoardTag(tag) && isCharTag(tag))
                            actionBoxesTags.push_back(Vec3(startTag, tag, moveType));
                        break;
                        
                    default:
                        break;
                }
            }
        }
    }
    
    return actionBoxesTags;
}

std::vector<KFAction*> MainAction::getActionSequence(int charSelectNbr, int cardSelectNbr, int touchedBoxTag)
{
    std::vector<std::string> cardName = MainStuff::getStuffByName(charSelectNbr, cardSelectNbr);
    KFSpecCard* specCard = MainStuff::getCardSpec(CARD_TYPE[cardSelectNbr], cardName[0], cardName[1]);
    int crystalCost = specCard->getMana(false);
    
    std::vector<KFAction*> actionSequence;
    std::vector<Vec3> actionBoxesTags = getActionBoxesTags(charSelectNbr, cardSelectNbr);
    
    Vec3 touchedBox;
    for(int t = 0; t < actionBoxesTags.size(); t++)
    {
        if(actionBoxesTags[t].y == touchedBoxTag)
            touchedBox = actionBoxesTags[t];
    }
    int actionType = touchedBox.z;
    
    //move.
    if(actionType == 0)
    {
        int max = 0;
        int testTag = touchedBox.y;
        while(testTag != charSelectNbr && max < 9)
        {
            for(int a = 0; a < actionBoxesTags.size(); a++)
            {
                int startTag = actionBoxesTags[a].x;
                int endTag = actionBoxesTags[a].y;
                
                if(testTag == endTag)
                {
                    auto action = KFAction::setAction(actionType, charSelectNbr);
                    action->setCost(crystalCost);
                    action->setStartTag(startTag);
                    action->setEndTag(endTag);
                    actionSequence.insert(actionSequence.begin(), action);
                    testTag = startTag;
                }
            }
            max++;
        }
    }
    
    //strike.
    if(actionType == 1)
    {
        auto action = KFAction::setAction(actionType, charSelectNbr);
        action->setCost(crystalCost);
        printf("strike->");
        for(int a = 0; a < actionBoxesTags.size(); a++)
        {
            printf("actionBoxTag: x = %f, y = %f, z = %f\n",actionBoxesTags[a].x,actionBoxesTags[a].y,actionBoxesTags[a].z);
        }
        actionSequence.push_back(action);
    }
    
    //spell.
    if(actionType == 2)
    {
        
    }
    return actionSequence;
}
std::vector<KFAction*> MainAction::getEnemyActionSequence(int charSelectNbr, int cardSelectNbr, int touchedBoxTag)
{
    int charEnemyNbr = charSelectNbr + 5;
    int l = touchedBoxTag / 10;
    int c = (touchedBoxTag - l * 10) * -1 + 6;
    int enemyTouchedBoxTag = l * 10 + c;
    
    return getActionSequence(charEnemyNbr, cardSelectNbr, enemyTouchedBoxTag);
}

bool MainAction::isInBoardTag(int tag)
{
    bool isInBoard = false;
    for(int b = 0; b < m_boardIn.size(); b++)
    {
        if(tag == m_boardIn[b])
            isInBoard = true;
    }
    return isInBoard;
}
bool MainAction::isCharTag(int tag)
{
    auto character = MainObject::getCharByTag(tag);
    if(!character)
        return false;
    
    return true;
}
std::vector<int> MainAction::setForbiddenTagList(int charNbr, std::vector<std::vector<int>> board)
{
    std::vector<int> forbiddenTagList;
    
    int charTag = MainObject::getCharByNumber(charNbr)->getTag();
    
    for(int b = 0; b < board.size(); b++)
    {
        std::vector<int> boardAction = board[b];
        const int startTag = getStartTag(charNbr, boardAction[0], boardAction[1]);
        
        if((isCharTag(startTag) && startTag != charTag) || isForbiddenTag(startTag, forbiddenTagList))
        {
            for(int m = 3; m < boardAction.size(); m++)
            {
                int tag = startTag + getBoardMap(charNbr)[boardAction[m]];
                forbiddenTagList.push_back(tag);
            }
        }
    }
    return forbiddenTagList;
}
bool MainAction::isForbiddenTag(int tag, std::vector<int> forbiddenTagList)
{
    bool isForbiddenTag = false;
    for(int f = 0; f < forbiddenTagList.size(); f++)
    {
        if(tag == forbiddenTagList[f])
            isForbiddenTag = true;
    }
    return isForbiddenTag;
}

//team invert box(right, left).
int MainAction::getStartTag(int charNbr, int boardActionC, int boardActionL)
{
    int startTag;
    int charTag = MainObject::getCharByNumber(charNbr)->getTag();
    if(charNbr < 5)
        startTag = charTag + boardActionC + boardActionL*10;
    else
        startTag = charTag + boardActionC - boardActionL*10;
    return startTag;
}

std::vector<int> MainAction::getBoardMap(int charNbr)
{
    std::vector<int> boardMap;
    
    if(charNbr < 5)
        boardMap = m_friendBoardMap;
    else
        boardMap = m_enemyBoardMap;
    
    return boardMap;
}
