//
//  board.cpp
//  pphint
//
//  Created by Albrecht Eckardt on 2018-07-23.
//  Copyright © 2018 Albrecht Eckardt. All rights reserved.
//

#include "board.h"

/////////////////////////////////////////////////////////////////////////////
//
// GameEngine
//
/////////////////////////////////////////////////////////////////////////////

void GameEngine::print() const
{
    int i, j;
    for (j = 0; j < GAME_ROWS; j++)
    {
        for (i = 0; i < GAME_COLUMNS; i++)
        {
            if (items[j * GAME_COLUMNS + i] != '?' && items[j * GAME_COLUMNS + i] != '*')
            {
                std::cout << items[j * GAME_COLUMNS + i] << levels[j * GAME_COLUMNS + i];
                if (protection[j * GAME_COLUMNS + i])
                {
                    std::cout << "#";
                }
                else
                {
                    std::cout << "_";
                }
            }
            else
            {
                std::cout << "   ";
            }
        }
        std::cout << std::endl;
    }
}

bool GameEngine::hasMoveH(uint8_t i, uint8_t j) const
{
    bool sym1h, sym2h;
    uint8_t first_sym1v, last_sym1v, first_sym2v, last_sym2v;
    
    simulateMoveH(i, j, sym1h, sym2h, first_sym1v, last_sym1v, first_sym2v, last_sym2v);
    
    int sym1v = last_sym1v - first_sym1v + 1;
    int sym2v = last_sym2v - first_sym2v + 1;

    // check feasibility
    if (!sym1h && !sym2h && (sym1v < 3) && (sym2v < 3))
    {
        // this is not a possible move
        return false;
    }
    
    return true;
}

void GameEngine::simulateMoveH(
        uint8_t i, uint8_t j,
        bool& sym1h, bool& sym2h,
        uint8_t& first_sym1v, uint8_t& last_sym1v,
        uint8_t& first_sym2v, uint8_t& last_sym2v) const
{
    // possible affected area
    //
    // --xx--
    // --xx--
    // xx12xx
    // --xx--
    // --xx--
    
    char sym1 = items[j * GAME_COLUMNS + i + 1];
    char sym2 = items[j * GAME_COLUMNS + i];
    
    // symbol 1 horizontal
    //
    // ------
    // ------
    // xx1---
    // ------
    // ------
    if (i > 1)
        sym1h = (items[j * GAME_COLUMNS + i - 2] == sym1) && (items[j * GAME_COLUMNS + i - 1] == sym1);
    else
        sym1h = false;
    
    // symbol 2 horizontal
    // ------
    // ------
    // ---2xx
    // ------
    // ------
    if (i < GAME_COLUMNS - 3)
        sym2h = (items[j * GAME_COLUMNS + i + 3] == sym2) && (items[j * GAME_COLUMNS + i + 2] == sym2);
    else
        sym2h = false;
    
    // symbol 1 vertical
    // --x---
    // --x---
    // --1---
    // --x---
    // --x---
    
    if ((j > 0) && (items[(j - 1) * GAME_COLUMNS + i] == sym1))
    {
        if ((j > 1) && (items[(j - 2) * GAME_COLUMNS + i] == sym1))
            first_sym1v = j - 2;
        else
            first_sym1v = j - 1;
    }
    else
        first_sym1v = j;
    
    if ((j < GAME_ROWS - 1) && (items[(j + 1) * GAME_COLUMNS + i] == sym1))
    {
        if ((j < GAME_ROWS - 2) && (items[(j + 2) * GAME_COLUMNS + i] == sym1))
            last_sym1v = j + 2;
        else
            last_sym1v = j + 1;
    }
    else
        last_sym1v = j;
    
    // symbol 2 vertical
    // ---x--
    // ---x--
    // ---2--
    // ---x--
    // ---x--
    if ((j > 0) && (items[(j - 1) * GAME_COLUMNS + (i + 1)] == sym2))
    {
        if ((j > 1) && (items[(j - 2) * GAME_COLUMNS + (i + 1)] == sym2))
            first_sym2v = j - 2;
        else
            first_sym2v = j - 1;
    }
    else
        first_sym2v = j;
    
    if ((j < GAME_ROWS - 1) && (items[(j + 1) * GAME_COLUMNS + (i + 1)] == sym2))
    {
        if ((j < GAME_ROWS - 2) && (items[(j + 2) * GAME_COLUMNS + (i + 1)] == sym2))
            last_sym2v = j + 2;
        else
            last_sym2v = j + 1;
    }
    else
        last_sym2v = j;
}

bool GameEngine::hasMoveV(uint8_t i, uint8_t j) const
{
    bool sym1v, sym2v;
    uint8_t first_sym1h, last_sym1h, first_sym2h, last_sym2h;

    simulateMoveV(i, j, sym1v, sym2v, first_sym1h, last_sym1h, first_sym2h, last_sym2h);
    
    int sym1h = last_sym1h - first_sym1h + 1;
    int sym2h = last_sym2h - first_sym2h + 1;
    
    // check feasibility
    if (!sym1v && !sym2v && (sym1h < 3) && (sym2h < 3))
    {
        // this is not a possible move
        return false;
    }
    
    return true;
}

void GameEngine::simulateMoveV(
        uint8_t i, uint8_t j,
        bool& sym1v, bool& sym2v,
        uint8_t& first_sym1h, uint8_t& last_sym1h,
        uint8_t& first_sym2h, uint8_t& last_sym2h) const
{
    // possible affected area
    //
    // --x--
    // --x--
    // xx1xx
    // xx2xx
    // --x--
    // --x--
    
    char sym1 = items[(j + 1) * GAME_COLUMNS + i];
    char sym2 = items[j * GAME_COLUMNS + i];
    
    // symbol 1 vertical
    //
    // --x--
    // --x--
    // --1--
    // -----
    // -----
    // -----
    if (j > 1)
        sym1v = (items[(j - 2) * GAME_COLUMNS + i] == sym1) && (items[(j - 1) * GAME_COLUMNS + i] == sym1);
    else
        sym1v = false;
    
    // symbol 2 vertical
    // -----
    // -----
    // -----
    // --2--
    // --x--
    // --x--
    if (j < GAME_ROWS - 3)
        sym2v = (items[(j + 3) * GAME_COLUMNS + i] == sym2) && (items[(j + 2) * GAME_COLUMNS + i] == sym2);
    else
        sym2v = false;
    
    // symbol 1 horizontal
    // -----
    // -----
    // xx1xx
    // -----
    // -----
    // -----
    
    if ((i > 0) && (items[j * GAME_COLUMNS + (i - 1)] == sym1))
    {
        if ((i > 1) && (items[j * GAME_COLUMNS + (i - 2)] == sym1))
            first_sym1h = i - 2;
        else
            first_sym1h = i - 1;
    }
    else
        first_sym1h = i;
    
    if ((i < GAME_COLUMNS - 1) && (items[j * GAME_COLUMNS + (i + 1)] == sym1))
    {
        if ((i < GAME_COLUMNS - 2) && (items[j * GAME_COLUMNS + (i + 2)] == sym1))
            last_sym1h = i + 2;
        else
            last_sym1h = i + 1;
    }
    else
        last_sym1h = i;
    
    // symbol 2 horizontal
    // -----
    // -----
    // -----
    // xx2xx
    // -----
    // -----
    if ((i > 0) && (items[(j + 1) * GAME_COLUMNS + (i - 1)] == sym2))
    {
        if ((i > 1) && (items[(j + 1) * GAME_COLUMNS + (i - 2)] == sym2))
            first_sym2h = i - 2;
        else
            first_sym2h = i - 1;
    }
    else
        first_sym2h = i;
    
    if ((i < GAME_COLUMNS - 1) && (items[(j + 1) * GAME_COLUMNS + (i + 1)] == sym2))
    {
        if ((i < GAME_COLUMNS - 2) && (items[(j + 1) * GAME_COLUMNS + (i + 2)] == sym2))
            last_sym2h = i + 2;
        else
            last_sym2h = i + 1;
    }
    else
        last_sym2h = i;
}

inline bool rowcmp(int pos1, int pos2)
{
    return ((pos1 % GAME_COLUMNS) * GAME_COLUMNS + pos1 / GAME_COLUMNS) < ((pos2 % GAME_COLUMNS) * GAME_COLUMNS + pos2 / GAME_COLUMNS);
}

void GameEngine::addCombo(std::vector<Combo>& combos, uint8_t first, uint8_t last) const
{
    combos.push_back(Combo());
    Combo& combo = *combos.rbegin();
    
    combo.color = items[first];
    combo.type = UNKNOWN;
    
    bool horizontal = last - first < GAME_COLUMNS;
    
    int max_level   = 0;
    uint8_t max_pos = 0xff;
    
    combo.protection = false;
    combo.modified = false;

    uint8_t k;
    for (k = first; k <= last; k += (horizontal ? 1 : GAME_COLUMNS))
    {
        combo.items.push_back(k);
        if (levels[k] > max_level)
        {
            max_pos = k;
            max_level = levels[k];
        }
        combo.protection |= protection[k];
    }
    
    if (max_pos == 0xff)
    {
        combo.center = static_cast<uint8_t>(last - ((combo.items.size() - 1) / 2) * (horizontal ? 1 : GAME_COLUMNS));
    }
    else
    {
        combo.center = max_pos;
    }
}

void GameEngine::addSwapCombos(std::vector<Combo>& combos, uint8_t pos1, uint8_t pos2) const
{
    bool horizontal = pos2 - pos1 == 1;
    
    uint8_t i = pos1 % GAME_COLUMNS;
    uint8_t j = pos1 / GAME_COLUMNS;
    uint8_t k;
    
    if (horizontal)
    {
        bool sym1h, sym2h;
        uint8_t first_sym1v, last_sym1v, first_sym2v, last_sym2v;
        
        simulateMoveH(i, j, sym1h, sym2h, first_sym1v, last_sym1v, first_sym2v, last_sym2v);
        
        if (sym1h || last_sym1v - first_sym1v + 1 >= 3)
        {
            combos.push_back(Combo());
            Combo& combo = *combos.rbegin();
            combo.color = items[pos2];
            combo.center = pos1;
            
            if (sym1h)
            {
                combo.items.push_back(pos1 - 2);
                combo.items.push_back(pos1 - 1);
            }
            
            if (last_sym1v - first_sym1v + 1 >= 3)
            {
                for (k = first_sym1v; k <= last_sym1v; k++)
                {
                    combo.items.push_back(k * GAME_COLUMNS + i);
                }
            }
            else
            {
                combo.items.push_back(pos1);
            }
            
            if (combo.items.size() == 3)
            {
                combo.type = THREE;
            }
            else if (combo.items.size() == 4)
            {
                combo.type = FOUR;
            }
            else if (combo.items.size() - (sym1h ? 2 : 0) == 5)
            {
                combo.type = FIVE;
            }
            else
            {
                combo.type = L_OR_T;
            }
            
            combo.protection = protection[pos2];
            for (k = 0; k < combo.items.size(); k++)
            {
                if (combo.items[k] != combo.center)
                {
                    combo.protection |= protection[combo.items[k]];
                }
            }
        }
        
        if (sym2h || last_sym2v - first_sym2v + 1 >= 3)
        {
            combos.push_back(Combo());
            Combo& combo = *combos.rbegin();
            combo.color = items[pos1];
            combo.center = pos2;
            
            if (sym2h)
            {
                combo.items.push_back(pos2 + 1);
                combo.items.push_back(pos2 + 2);
            }
            
            if (last_sym2v - first_sym2v + 1 >= 3)
            {
                for (k = first_sym2v; k <= last_sym2v; k++)
                {
                    combo.items.push_back(k * GAME_COLUMNS + (i + 1));
                }
            }
            else
            {
                combo.items.push_back(pos2);
            }
            
            if (combo.items.size() == 3)
            {
                combo.type = THREE;
            }
            else if (combo.items.size() == 4)
            {
                combo.type = FOUR;
            }
            else if (combo.items.size() - (sym2h ? 2 : 0) == 5)
            {
                combo.type = FIVE;
            }
            else
            {
                combo.type = L_OR_T;
            }

            combo.protection = protection[pos1];
            for (k = 0; k < combo.items.size(); k++)
            {
                if (combo.items[k] != combo.center)
                {
                    combo.protection |= protection[combo.items[k]];
                }
            }
        }
    }
    else
    {
        bool sym1v, sym2v;
        uint8_t first_sym1h, last_sym1h, first_sym2h, last_sym2h;
    
        simulateMoveV(i, j, sym1v, sym2v, first_sym1h, last_sym1h, first_sym2h, last_sym2h);
        
        if (sym1v || last_sym1h - first_sym1h + 1 >= 3)
        {
            combos.push_back(Combo());
            Combo& combo = *combos.rbegin();
            combo.color = items[pos2];
            combo.center = pos1;
            
            if (sym1v)
            {
                combo.items.push_back(pos1 - 2 * GAME_COLUMNS);
                combo.items.push_back(pos1 - GAME_COLUMNS);
            }
            
            if (last_sym1h - first_sym1h + 1 >= 3)
            {
                for (k = first_sym1h; k <= last_sym1h; k++)
                {
                    combo.items.push_back(j * GAME_COLUMNS + k);
                }
            }
            else
            {
                combo.items.push_back(pos1);
            }
            
            if (combo.items.size() == 3)
            {
                combo.type = THREE;
            }
            else if (combo.items.size() == 4)
            {
                combo.type = FOUR;
            }
            else if (combo.items.size() - (sym1v ? 2 : 0) == 5)
            {
                combo.type = FIVE;
            }
            else
            {
                combo.type = L_OR_T;
            }

            combo.protection = protection[pos2];
            for (k = 0; k < combo.items.size(); k++)
            {
                if (combo.items[k] != combo.center)
                {
                    combo.protection |= protection[combo.items[k]];
                }
            }
        }
        
        if (sym2v || last_sym2h - first_sym2h + 1 >= 3)
        {
            combos.push_back(Combo());
            Combo& combo = *combos.rbegin();
            combo.color = items[pos1];
            combo.center = pos2;
            
            if (sym2v)
            {
                combo.items.push_back(pos2 + GAME_COLUMNS);
                combo.items.push_back(pos2 + 2 * GAME_COLUMNS);
            }
            
            if (last_sym2h - first_sym2h + 1 >= 3)
            {
                for (k = first_sym2h; k <= last_sym2h; k++)
                {
                    combo.items.push_back((j + 1) * GAME_COLUMNS + k);
                }
            }
            else
            {
                combo.items.push_back(pos2);
            }
            
            if (combo.items.size() == 3)
            {
                combo.type = THREE;
            }
            else if (combo.items.size() == 4)
            {
                combo.type = FOUR;
            }
            else if (combo.items.size() - (sym2v ? 2 : 0) == 5)
            {
                combo.type = FIVE;
            }
            else
            {
                combo.type = L_OR_T;
            }
            
            combo.protection = protection[pos1];
            for (k = 0; k < combo.items.size(); k++)
            {
                if (combo.items[k] != combo.center)
                {
                    combo.protection |= protection[combo.items[k]];
                }
            }
        }
    }
}

void GameEngine::comboPrint(std::vector<Combo>& combos) const
{
    uint8_t i;
    for (i = 0; i < combos.size(); i++)
    {
        std::cout << "combo: " << i << " has color '" << combos[i].color << "', first: " << static_cast<int>(combos[i].items[0]) << ", last: " <<
            static_cast<int>(combos[i].items[combos[i].items.size() - 1]) << ", center: " << static_cast<int>(combos[i].center) << ", ";
        switch (combos[i].type)
        {
        case THREE:
            std::cout << "type: THREE";
            break;
        case FOUR:
            std::cout << "type: FOUR";
            break;
        case FIVE:
            std::cout << "type: FIVE";
            break;
        case L_OR_T:
            std::cout << "type: L_OR_T";
            break;
        case MORE_THAN_FIVE_STRAIGHT:
            std::cout << "type: MORE_THAN_FIVE_STRAIGHT";
            break;
        case BIG_L_OR_T:
            std::cout << "type: BIG_L_OR_T";
            break;
        default:
            std::cout << "type: UNKNOWN";
            break;
        }
        std::cout << std::endl;
    }
}

void GameEngine::findCombos(std::vector<Combo>& combos) const
{
    uint8_t i, j;
    
    char color = '?';
    char item_color;
    uint8_t first = 0;
        
    // go vertical lines and check for combos
    for (i = 0; i < GAME_COLUMNS; i++)
    {
        for (j = 0; j < GAME_ROWS; j++)
        {
            if (levels[j * GAME_COLUMNS + i] < 5 && items[j * GAME_COLUMNS + i] != '*')
            {
                item_color = items[j * GAME_COLUMNS + i];
            }
            else
            {
                item_color = '?';
            }
        
            if (j == 0)
            {
                first = 0;
                color = item_color;
            }
            else
            {
                if (j == (GAME_ROWS - 1) && first < (GAME_ROWS - 2) && item_color == color && color != '?')
                {
                    // vertical combo found
                    addCombo(combos, first * GAME_COLUMNS + i, (GAME_ROWS - 1) * GAME_COLUMNS + i);
                }
                else if (item_color != color)
                {
                    if (j - first > 2 && color != '?')
                    {
                        // vertical combo found
                        addCombo(combos, first * GAME_COLUMNS + i, (j - 1) * GAME_COLUMNS + i);
                    }
                    
                    if (j != (GAME_ROWS))
                    {
                        first = j;
                        color = item_color;
                    }
                }
            }
        }
    }

    // go horizontal lines and check for combos
    for (j = 0; j < GAME_ROWS; j++)
    {
        for (i = 0; i < GAME_COLUMNS; i++)
        {
            if (levels[j * GAME_COLUMNS + i] < 5 && items[j * GAME_COLUMNS + i] != '*')
            {
                item_color = items[j * GAME_COLUMNS + i];
            }
            else
            {
                item_color = '?';
            }
        
            if (i == 0)
            {
                first = 0;
                color = item_color;
            }
            else
            {
                if (i == (GAME_COLUMNS - 1) && first < (GAME_COLUMNS - 2) && item_color == color && color != '?')
                {
                    // horizontal combo found
                    addCombo(combos, j * GAME_COLUMNS + first, j * GAME_COLUMNS + (GAME_COLUMNS - 1));
                }
                else if (item_color != color)
                {
                    if (i - first > 2 && color != '?')
                    {
                        // horizontal combo found
                        addCombo(combos, j * GAME_COLUMNS + first, j * GAME_COLUMNS + (i - 1));
                    }
                    
                    if (i != (GAME_COLUMNS - 1))
                    {
                        first = i;
                        color = item_color;
                    }
                }
            }
        }
    }
}

void GameEngine::linkCombos(std::vector<Combo>& combos) const
{
    size_t i, j, k, l;
    bool intersect;
    
    for (i = 0; i < combos.size() - 1; i++)
    {
        Combo& combo1 = combos[i];
        for (j = i + 1; j < combos.size(); j++)
        {
            Combo& combo2 = combos[j];
            if (combo1.color == combo2.color)
            {
                intersect = false;
                for (k = 0; k < combo2.items.size(); k++)
                {
                    for (l = 0; l < combo1.items.size(); l++)
                    {
                        if (combo1.items[l] == combo2.items[k])
                        {
                            intersect = true;
                            break;
                        }
                    }
                    if (intersect)
                    {
                        break;
                    }
                }
                
                if (intersect)
                {
                    // combos have an intersection
                    // -> merge them into combo1
                    // and then erase combo2
                    for (k = 0; k < combo2.items.size(); k++)
                    {
                        intersect = false;
                        for (l = 0; l < combo1.items.size(); l++)
                        {
                            if (combo1.items[l] == combo2.items[k])
                            {
                                intersect = true;
                                break;
                            }
                        }
                        if (!intersect)
                        {
                            combo1.items.push_back(combo2.items[k]);
                        }
                        else
                        {
                            combo1.center = combo2.items[k];
                        }
                    }
                    
                    combo1.protection |= combo2.protection;
                    combo2.items.clear();
                    break;
                }
            }
        }
    }
    
    // delete empty combos from array
    int rev_i;
    for (rev_i = static_cast<int>(combos.size()); rev_i >= 0; --rev_i)
    {
        if (combos[static_cast<size_t>(rev_i)].items.empty())
        {
            combos.erase(combos.begin() + rev_i);
        }
    }
    
    handleIntersections(combos);
    redefineCombos(combos);
}

void GameEngine::handleIntersections(std::vector<Combo>& combos) const
{
    size_t i, j, k;
    int rev_l;
    for (i = 0; i < combos.size() - 1; i++)
    {
        const Combo& combo1 = combos[i];
        for (j = i + 1; j < combos.size(); j++)
        {
            Combo& combo2 = combos[j];
            for (k = 0; k < combo1.items.size(); k++)
            {
                for (rev_l = static_cast<int>(combo2.items.size() - 1); rev_l >= 0; --rev_l)
                {
                    if (combo1.items[k] == combo2.items[static_cast<size_t>(rev_l)])
                    {
                        combo2.items.erase(combo2.items.begin() + rev_l);
                        combo2.modified = true;
                    }
                }
            }
        }
    }
}

bool isIn(uint8_t n, std::vector<uint8_t>& array)
{
    size_t k;
    for (k = 0; k < array.size(); k++)
    {
        if (n == array[k])
        {
            return true;
        }
    }
    
    return false;
}

void GameEngine::redefineCombos(std::vector<Combo>& combos) const
{
    size_t i, j;
    int rev_k, rev_l;
    for (rev_k = static_cast<int>(combos.size() - 1); rev_k >= 0; --rev_k)
    {
        Combo& combo = combos[static_cast<size_t>(rev_k)];
        if (combo.modified)
        {
            i = combo.center % GAME_COLUMNS;
            j = combo.center / GAME_COLUMNS;
        
            std::vector<uint8_t> horizontal;
            std::vector<uint8_t> vertical;
            
            for (rev_l = 0; static_cast<size_t>(rev_l) < combo.items.size(); ++rev_l)
            {
                if ((combo.items[static_cast<size_t>(rev_l)] % GAME_COLUMNS) == i)
                {
                    horizontal.push_back(combo.items[static_cast<size_t>(rev_l)]);
                }
                if ((combo.items[static_cast<size_t>(rev_l)] / GAME_COLUMNS) == j)
                {
                    vertical.push_back(combo.items[static_cast<size_t>(rev_l)]);
                }
            }
            
            combo.items.clear();
            
            sort(horizontal.begin(), horizontal.end());
            sort(vertical.begin(), vertical.end());
            
            std::vector<std::vector<uint8_t>> horizontal_combos;
            std::vector<std::vector<uint8_t>> vertical_combos;

            if (horizontal.size() >= 3)
            {
                horizontal_combos.push_back(std::vector<uint8_t>());
            
                for (rev_l = 0; static_cast<size_t>(rev_l) < horizontal.size(); ++rev_l)
                {
                    if (rev_l > 0)
                    {
                        if (horizontal[static_cast<size_t>(rev_l)] - horizontal[static_cast<size_t>(rev_l - 1)] > 1)
                        {
                            horizontal_combos.push_back(std::vector<uint8_t>());
                        }
                    }
                    
                    horizontal_combos.rbegin()->push_back(horizontal[static_cast<size_t>(rev_l)]);
                }
                
                for (rev_l = static_cast<int>(horizontal_combos.size() - 1); rev_l >= 0; --rev_l)
                {
                    if (horizontal_combos[static_cast<size_t>(rev_l)].size() < 3)
                    {
                        horizontal_combos.erase(horizontal_combos.begin() + rev_l);
                    }
                }
            }
            
            if (vertical.size() >= 3)
            {
                vertical_combos.push_back(std::vector<uint8_t>());
            
                for (rev_l = 0; static_cast<size_t>(rev_l) < vertical.size(); ++rev_l)
                {
                    if (rev_l > 0)
                    {
                        if (vertical[static_cast<size_t>(rev_l)] - vertical[static_cast<size_t>(rev_l - 1)] > 1)
                        {
                            vertical_combos.push_back(std::vector<uint8_t>());
                        }
                    }
                    
                    vertical_combos.rbegin()->push_back(vertical[static_cast<size_t>(rev_l)]);
                }

                for (rev_l = static_cast<int>(vertical_combos.size() - 1); rev_l >= 0; --rev_l)
                {
                    if (vertical_combos[static_cast<size_t>(rev_l)].size() < 3)
                    {
                        vertical_combos.erase(vertical_combos.begin() + rev_l);
                    }
                }
            }
            
            combo.items.clear();
            if (horizontal_combos.empty() && vertical_combos.empty())
            {
                combos.erase(combos.begin() + rev_k);
            }
            else if (horizontal_combos.size() == 1 && vertical_combos.empty())
            {
                combo.items = horizontal_combos[0];
                if (!isIn(combo.center, combo.items))
                {
                    combo.center = combo.items[combo.items.size() / 2];
                }
            }
            else if (horizontal_combos.empty() && vertical_combos.size() == 1)
            {
                combo.items = vertical_combos[0];
                if (!isIn(combo.center, combo.items))
                {
                    combo.center = combo.items[combo.items.size() / 2];
                }
            }
            else if (horizontal_combos.size() == 1 && vertical_combos.size() == 1)
            {
                combo.items = horizontal_combos[0];
                for (rev_l = 0; static_cast<size_t>(rev_l) < vertical_combos[0].size(); ++rev_l)
                {
                    if (!isIn(vertical_combos[0][static_cast<size_t>(rev_l)], combo.items))
                    {
                        combo.items.push_back(vertical_combos[0][static_cast<size_t>(rev_l)]);
                    }
                }
            }
            else
            {
                // other cases will not be handled so far....
            }
        }
    }
}

void GameEngine::determineComboTypes(std::vector<Combo>& combos) const
{
    size_t k;
    for (k = 0; k < combos.size(); k++)
    {
        Combo& combo = combos[k];
        if (combo.items.size() == 3)
        {
            combo.type = THREE;
        }
        else if (combo.items.size() == 4)
        {
            combo.type = FOUR;
        }
        else if (combo.items.size() == 5 && (combo.items[4] - combo.items[0] == 4 || combo.items[4] - combo.items[0] == 4 * GAME_COLUMNS))
        {
            combo.type = FIVE;
        }
        else if (combo.items.size() == 5)
        {
            combo.type = L_OR_T;
        }
        else if (combo.items.size() > 5 && 
            (combo.items[combo.items.size() - 1] - combo.items[0] == combo.items.size() - 1 ||
            combo.items[combo.items.size() - 1] - combo.items[0] == (combo.items.size() - 1) * GAME_COLUMNS))
        {
            combo.type = MORE_THAN_FIVE_STRAIGHT;
        }
        else
        {
            combo.type = BIG_L_OR_T;
        }
        // for other cases there is no implementation yet
    }
}

void GameEngine::waveGrow(int center, bool horizontal)
{
    int i;
    int j;
    if (horizontal)
    {
        j = center / GAME_COLUMNS;
        for (i = 0; i < GAME_COLUMNS; i++)
        {
            if (levels[j * GAME_COLUMNS + i] > 0)
            {
                levels[j * GAME_COLUMNS + i]++;
            }
        }
    }
    else
    {
        i = center % GAME_COLUMNS;
        for (j = 0; j < GAME_ROWS; j++)
        {
            if (levels[j * GAME_COLUMNS + i] > 0)
            {
                levels[j * GAME_COLUMNS + i]++;
            }
        }
    }
}

void GameEngine::growAll(char color)
{
    int k;
    for (k = 0; k < BOARD_ITEMS; k++)
    {
        if (items[k] == color && levels[k] != 5)
        {
            levels[k]++;
        }
    }
    
    switch (color)
    {
    case 'y':
        growth[0]++;
        break;
    case 'r':
        growth[1]++;
        break;
    case 'g':
        growth[2]++;
        break;
    case 'p':
        growth[3]++;
        break;
    case 'b':
    default:
        growth[4]++;
        break;
    }
}

void GameEngine::protectAll(char color)
{
    int k;
    for (k = 0; k < BOARD_ITEMS; k++)
    {
        if (items[k] == color)
        {
            protection[k] = true;
        }
    }
}

void GameEngine::reduce(std::vector<Combo>& combos)
{
    size_t i, k;
    short int lvl;
    for (k = 0; k < combos.size(); k++)
    {
        const Combo& combo = combos[k];
    
        lvl = 0;
        for (i = 0; i < combo.items.size(); i++)
        {
            lvl += levels[combo.items[i]];
            if (combo.items[i] != combo.center)
            {
                items[combo.items[i]] = '?';
            }
        }
        
        protection[combo.center] |= combo.protection;

        switch (combo.type)
        {
        case THREE:
            lvl += 1;
            score += static_cast<int>(static_cast<double>(200) * multiplier);
            break;
        case FOUR:
            waveGrow(combo.center, combo.items[1] - combo.items[0] == 1);
            lvl += 2;
            score += static_cast<int>(static_cast<double>(400) * multiplier);
            break;
        case L_OR_T:
            protectAll(combo.color);
            lvl += 3;
            score += static_cast<int>(static_cast<double>(600) * multiplier);
            break;
        case BIG_L_OR_T:
            protectAll(combo.color);
            lvl += combo.items.size() - 2;
            score += static_cast<int>(static_cast<double>(600) * multiplier);
            break;
        case FIVE:
            growAll(combo.color);
            protectAll(combo.color);
            lvl += 3;
            score += static_cast<int>(static_cast<double>(600) * multiplier);
            break;
        case MORE_THAN_FIVE_STRAIGHT:
            growAll(combo.color);
            protectAll(combo.color);
            lvl += combo.items.size() - 2;
            score += static_cast<int>(static_cast<double>(600) * multiplier);
            break;
        case UNKNOWN:
            std::cout << "ERROR: combo type is unknown" << std::endl;
            break;
        }
        
        if (lvl > 5)
        {
            lvl = 5;
        }
        
        levels[combo.center] = lvl;
    }
}

void GameEngine::drop()
{
    uint8_t i, j, k;
    for (i = 0; i < GAME_COLUMNS; i++)
    {
        for (k = 0; k < GAME_ROWS - 1; k++)
        {
            for (j = GAME_ROWS - 1; j >= 1; j--)
            {
                if (items[j * GAME_COLUMNS + i] == '?')
                {
                    swap(j * GAME_COLUMNS + i, (j - 1) * GAME_COLUMNS + i);
                }
            }
        }
    }
}

void GameEngine::fill()
{
    if (falling_items.empty())
    {
        // error: no more falling items available! 
        return;
    }

    int i, j;
    int depth;
    for (i = 0; i < GAME_COLUMNS; i++)
    {
        depth = -1;
        for (j = 0; j < GAME_ROWS; j++)
        {
            if (items[j * GAME_COLUMNS + i] != '?')
            {
                break;
            }
            depth = j;
        }
        
        for (j = depth; j >= 0; j--)
        {
            items[j * GAME_COLUMNS + i] = falling_items[0];
            levels[j * GAME_COLUMNS + i] = 0;
            protection[j * GAME_COLUMNS + i] = false;
            burning_order[j * GAME_COLUMNS + i] = -1;
            
            falling_items.erase(falling_items.begin());
            if (falling_items.empty())
            {
                return;
            }
        }
    }
}

void GameEngine::runFarmer()
{
    if (farmer_ko > 0 || farmer_pos == 0)
    {
        farmer_ko--;
        return;
    }

    int j;
    for (j = 0; j < GAME_ROWS; j++)
    {
        if (levels[j * GAME_COLUMNS + farmer_pos - 1] > 0 && !protection[j * GAME_COLUMNS + farmer_pos - 1])
        {
            score += getLevelScore(levels[j * GAME_COLUMNS + farmer_pos - 1]);
            items[j * GAME_COLUMNS + farmer_pos - 1] = '?';
        }
        else
        {
            protection[j * GAME_COLUMNS + farmer_pos - 1] = false; 
        }
    }
    
    if (farmer_pos == 1)
    {
        farmer_dir = 1;
    }
    else if (farmer_pos == GAME_COLUMNS)
    {
        farmer_dir = -1;
    }

    farmer_pos += farmer_dir;
}

void GameEngine::resetBurning()
{
    size_t k;
    for (k = 0; k < BOARD_ITEMS; k++)
    {
        burning_order[k] = -1;
    }
    total_burning = 0;
}

void GameEngine::checkBurning()
{
    uint8_t i, j;
    for (i = 0; i < GAME_COLUMNS; i++)
    {
        for (j = 0; j < GAME_ROWS; j++)
        {
            const uint8_t k = j * GAME_COLUMNS + i;
            if (levels[k] == 5 && burning_order[k] == -1)
            {
                burning_order[k] = total_burning;
                total_burning++;
            }
            else if (levels[k] < 5)
            {
                burning_order[k] = -1;
            }
        }
    }
}

void GameEngine::burnNext()
{
    size_t k;
    int next = -1;
    for (k = 0; k < BOARD_ITEMS; k++)
    {
        if (burning_order[k] == 0)
        {
            next = static_cast<int>(k);
        }
        
        if (burning_order[k] != -1)
        {
            burning_order[k]--;
        }
    }
    
    if (next == -1)
    {
        // error: no burning pepper found!
        return;
    }
    
    score += getLevelScore(5);
    
    growAll(items[static_cast<size_t>(next)]);
    items[static_cast<size_t>(next)] = '?';
    levels[static_cast<size_t>(next)] = 0;
    burning_order[static_cast<size_t>(next)] = -1;
    total_burning--;
}

void GameEngine::pepperPanic()
{
    int k;
    for (k = 0; k < BOARD_ITEMS; k++)
    {
        if (levels[k] > 0)
        {
            score += getLevelScore(levels[k]) / 2;
            
            items[k] = '?';
            levels[k] = 0;
        }
    }
    score += 100000;
}

int GameEngine::getLevelScore(short level)
{
    switch (level)
    {
    case 1:
        return 1000;
    case 2:
        return 2000;
    case 3:
        return 3000;
    case 4:
        return 5000;
    case 5:
        return 7500;
    }
    
    return 0;
}

//#define PRINT_LOG

void GameEngine::makeCombos()
{
    std::vector<Combo> combos;
    
#ifdef PRINT_LOG
    std::cout << std::endl;
    print();
    std::cout << std::endl;
#endif

    multiplier = 1.00;
    resetBurning();

    findCombos(combos);
    
    if (combos.empty())
    {
        checkBurning();
    }
    
    int burned = 0;
    
    while (!combos.empty() || (total_burning > 0))
    {
        if (!combos.empty())
        {
            linkCombos(combos);
            determineComboTypes(combos);
#ifdef PRINT_LOG
    comboPrint(combos);
#endif
            reduce(combos);
            
            multiplier *= 1.1;
        }
        else // total_burning > 0
        {
            if (burned < 10)
            {
#ifdef PRINT_LOG
    std::cout << "burning..." << std::endl;
#endif
                burnNext();
                burned++;
            }
            else
            {
#ifdef PRINT_LOG
    std::cout << "PEPPER PANIC!!" << std::endl;
#endif
                pepperPanic();
                burned = 0;
                total_burning = 0;
                farmer_ko = 3; // will be reduced further down
            }
        }
        
#ifdef PRINT_LOG
    std::cout << std::endl;
    print();
    std::cout << std::endl;
#endif
    
        drop();
        fill();
        
#ifdef PRINT_LOG
    print();
    std::cout << std::endl;
#endif
    
        combos.clear();
        findCombos(combos);
        
        if (combos.empty())
        {
            checkBurning();
        }
    }

#ifdef PRINT_LOG
    std::cout << "the farmer is coming..." << std::endl;
#endif

    if (farmer_ko == 0)
    {
        runFarmer();
    }
    else
    {
#ifdef PRINT_LOG
    std::cout << std::endl;
#endif
        farmer_ko--;
        return;
    }
    
    multiplier = 1.00;
    resetBurning();
    
#ifdef PRINT_LOG
    std::cout << std::endl;
    print();
    std::cout << std::endl;
#endif
    
    drop();
    fill();
    
#ifdef PRINT_LOG
    print();
    std::cout << std::endl;
#endif
    
    combos.clear();
    findCombos(combos);
    
    if (combos.empty())
    {
        checkBurning();
    }
    
    burned = 0;
    
    while (!combos.empty() || (total_burning > 0))
    {
        if (!combos.empty())
        {
            linkCombos(combos);
            determineComboTypes(combos);
#ifdef PRINT_LOG
    comboPrint(combos);
#endif
            reduce(combos);

            multiplier *= 1.1;
        }
        else
        {
            if (burned < 10)
            {
#ifdef PRINT_LOG
    std::cout << "burning..." << std::endl;
#endif
                burnNext();
                burned++;
            }
            else
            {
#ifdef PRINT_LOG
    std::cout << "PEPPER PANIC!!" << std::endl;
#endif
                pepperPanic();
                burned = 0;
                total_burning = 0;
                farmer_ko = 2;
            }
        }
        
#ifdef PRINT_LOG
    std::cout << std::endl;
    print();
    std::cout << std::endl;
#endif
    
        drop();
        fill();
        
#ifdef PRINT_LOG
    std::cout << std::endl;
    print();
    std::cout << std::endl;
#endif
    
        combos.clear();
        findCombos(combos);
        
        if (combos.empty())
        {
            checkBurning();
        }
    }
}

void GameEngine::executeMove(uint8_t pos1, uint8_t pos2)
{
    std::vector<Combo> combos;
    
    int k;
    for (k = 0; k < 5; k++)
    {
        growth[k] = 0;
    }
    
    multiplier = 1.00;

    // it is assumed that feasibility has already been proven!
    addSwapCombos(combos, pos1, pos2);
    swap(pos1, pos2);
    
    reduce(combos);
    
#ifdef PRINT_LOG
    comboPrint(combos);
    
    std::cout << std::endl;
    print();
    std::cout << std::endl;
#endif
    
    drop();    
    fill();

#ifdef PRINT_LOG
    std::cout << std::endl;
    print();
    std::cout << std::endl;
#endif

    makeCombos();
}

bool intpairsort(const std::pair<int, int>& pair1, const std::pair<int, int>& pair2)
{
    return pair1.second > pair2.second;
}

void GameEngine::calculateBestMove(int& pos1, int& pos2) const
{
    std::vector<std::pair<int, int>> possible_moves;

    uint8_t i, j;
    for (j = 0; j < GAME_ROWS; j++)
    {
        for (i = 0; i < GAME_COLUMNS - 1; i++)
        {
            if (hasMoveH(i, j))
            {
                possible_moves.push_back({j * GAME_COLUMNS + i, j * GAME_COLUMNS + i + 1});
            }
        }
    }

    for (i = 0; i < GAME_COLUMNS; i++)
    {
        for (j = 0; j < GAME_ROWS - 1; j++)
        {
            if (hasMoveV(i, j))
            {
                possible_moves.push_back({j * GAME_COLUMNS + i, (j + 1) * GAME_COLUMNS + i});
            }
        }
    }

    if (possible_moves.size() == 0)
    {
        // error: no moves found on board
        pos1 = -1;
        pos2 = -1;
        
        return;
    }
    
//    std::cout << possible_moves.size() << " possible moves found" << std::endl << std::endl;
//    std::cout << "move| score |  2nd  |  3rd  |  growth   |     flow     | value" << std::endl;
//    std::cout << "---------------------------------------------------------------" << std::endl;
    
    MoveValue move_value;
    std::vector<std::pair<size_t, int>> move_values;
    move_values.resize(possible_moves.size());
    
    static const char colors[5] = {'y', 'r', 'g', 'p', 'b'};
    
    int color_count[5];
    
    size_t k, l, c, n;
    for (k = 0; k < possible_moves.size(); k++)
    {
        determineMoveValue(0, possible_moves[k], move_value);
        
        move_values[k].first = k;

        move_values[k].second = 0;
        
        for (c = 0; c < 5; c++)
        {
            color_count[c] = 0;
            for (n = 0; n < BOARD_ITEMS; n++)
            {
                if (items[n] == colors[c])
                {
                    color_count[c]++;
                }
            }
            
            if (color_count[c] >= 10)
            {
                move_values[k].second += 10000 * move_value.growth[c];
            }
        }
        
        move_values[k].second += 100 * move_value.color_value[0];
        move_values[k].second += 100 * move_value.color_value[1];
        move_values[k].second += 100 * move_value.color_value[2];
        move_values[k].second += 100 * move_value.color_value[3];
        move_values[k].second += 100 * move_value.color_value[4];
        move_values[k].second += 10 * move_value.total_flow;
        
        move_values[k].second += 5000 * move_value.max_total_growth_2nd_it;
        move_values[k].second += 50 * move_value.max_color_value_2nd_it[0];
        move_values[k].second += 50 * move_value.max_color_value_2nd_it[1];
        move_values[k].second += 50 * move_value.max_color_value_2nd_it[2];
        move_values[k].second += 50 * move_value.max_color_value_2nd_it[3];
        move_values[k].second += 50 * move_value.max_color_value_2nd_it[4];
        move_values[k].second += 5 * move_value.max_total_flow_2nd_it;
        
        move_values[k].second += 2000 * move_value.max_total_growth_3rd_it;
        move_values[k].second += 20 * move_value.max_color_value_3rd_it[0];
        move_values[k].second += 20 * move_value.max_color_value_3rd_it[1];
        move_values[k].second += 20 * move_value.max_color_value_3rd_it[2];
        move_values[k].second += 20 * move_value.max_color_value_3rd_it[3];
        move_values[k].second += 20 * move_value.max_color_value_3rd_it[4];
        move_values[k].second += 2 * move_value.max_total_flow_3rd_it;
        
        int growth_gz = 0;
        
        for (l = 0; l < 5; l++)
        {
            growth_gz += (move_value.growth[l] > 0);
            if (move_value.color_value[l] > 50 && move_value.growth[l] > 0)
            {
                move_values[k].second += 10000;
            }
        }
        
        if (growth_gz > 1)
        {
            move_values[k].second += 5000 * (growth_gz - 1); 
        }
    }
    
    std::sort(move_values.begin(), move_values.end(), intpairsort);
    
    pos1 = possible_moves[move_values[0].first].first;
    pos2 = possible_moves[move_values[0].first].second;
}

void GameEngine::determineColorValue(const char color, int& color_value) const
{
    color_value = 0;

    int k;
    for (k = 0; k < BOARD_ITEMS; k++)
    {
        if (items[k] == color)
        {
            switch (levels[k])
            {
            case 1:
                color_value += 3;
                break;
            case 2:
                color_value += 5;
                break;
            case 3:
                color_value += 7;
                break;
            case 4:
                color_value += 10;
                break;
            default:
                break;
            }
        }
    }
}

void GameEngine::determineMoveValue(int iteration_level, const std::pair<uint8_t, uint8_t>& move, MoveValue& move_value) const
{
    static const char colors[5] = {'y', 'r', 'g', 'p', 'b'};
    
    memset(&move_value, 0, sizeof(MoveValue));
    
    std::pair<int, int> color_value[5];
    
    GameEngine temp;
    temp = *this;
    
    int k;
    for (k = 0; k < 5; k++)
    {
        temp.determineColorValue(colors[k], color_value[k].first);
        temp.growth[k] = 0;
    }
    
    temp.falling_items.resize(80, '*');

    std::vector<Combo> combos;
    
    temp.multiplier = 1.00;
    
    temp.addSwapCombos(combos, move.first, move.second);
    temp.swap(move.first, move.second);
    
    temp.reduce(combos);
    
    temp.drop();    
    temp.fill();
    
    temp.makeCombos();
    
    for (k = 0; k < 5; k++)
    {
        temp.determineColorValue(colors[k], color_value[k].second);
        move_value.color_value[k] = color_value[k].second;
        move_value.growth[k] = temp.growth[k];
        move_value.total_growth += temp.growth[k];
    }
    
    move_value.total_flow = 80 - static_cast<int>(temp.falling_items.size());
    move_value.score = temp.score;
    
    if (iteration_level < 2)
    {
        std::vector<std::pair<int, int>> possible_moves;

        uint8_t i, j;
        for (j = 0; j < GAME_ROWS; j++)
        {
            for (i = 0; i < GAME_COLUMNS - 1; i++)
            {
                if (temp.hasMoveH(i, j))
                {
                    possible_moves.push_back({j * GAME_COLUMNS + i, j * GAME_COLUMNS + i + 1});
                }
            }
        }

        for (i = 0; i < GAME_COLUMNS; i++)
        {
            for (j = 0; j < GAME_ROWS - 1; j++)
            {
                if (temp.hasMoveV(i, j))
                {
                    possible_moves.push_back({j * GAME_COLUMNS + i, (j + 1) * GAME_COLUMNS + i});
                }
            }
        }
        
        MoveValue next_move;
        
        size_t c, k;
        for (k = 0; k < possible_moves.size(); k++)
        {
            temp.determineMoveValue(iteration_level + 1, possible_moves[k], next_move);
            
            for (c = 0; c < 5; c++)
            {
                if (next_move.color_value[c] > move_value.max_color_value_2nd_it[c])
                {
                    move_value.max_color_value_2nd_it[c] = next_move.color_value[c];
                }
                
                if (next_move.growth[c] > move_value.max_growth_2nd_it[c])
                {
                    move_value.max_growth_2nd_it[c] = next_move.growth[c];
                }
            }
            
            if (next_move.score > move_value.max_score_2nd_it)
            {
                move_value.max_score_2nd_it = next_move.score;
            }
            
            if (next_move.total_flow > move_value.max_total_flow_2nd_it - move_value.total_flow)
            {
                move_value.max_total_flow_2nd_it = move_value.total_flow + next_move.total_flow;
            }
            
            if (next_move.total_growth > move_value.max_total_growth_2nd_it)
            {
                move_value.max_total_growth_2nd_it = next_move.total_growth;
            }
        
            if (iteration_level == 0)
            {
                for (c = 0; c < 5; c++)
                {
                    if (next_move.max_color_value_2nd_it[c] > move_value.max_color_value_3rd_it[c])
                    {
                        move_value.max_color_value_3rd_it[c] = next_move.max_color_value_2nd_it[c];
                    }
                }
                
                if (next_move.max_score_2nd_it > move_value.max_score_3rd_it)
                {
                    move_value.max_score_3rd_it = next_move.max_score_2nd_it;
                }
                
                if (next_move.max_total_flow_2nd_it > move_value.max_total_flow_3rd_it - move_value.max_total_flow_2nd_it)
                {
                    move_value.max_total_flow_3rd_it = move_value.max_total_flow_2nd_it + next_move.max_total_flow_2nd_it;
                }
                
                if (next_move.max_total_growth_2nd_it > move_value.max_total_growth_3rd_it)
                {
                    move_value.max_total_growth_3rd_it = next_move.max_total_growth_2nd_it;
                }
            }
        }
    }
}

bool GameEngine::isPossibleMove(uint8_t pos1, uint8_t pos2) const
{
    if (pos2 - pos1 == 1)
    {
        // check for horizontal move
        return hasMoveH(pos1 % GAME_COLUMNS, pos1 / GAME_COLUMNS);
    }
    else if (pos2 - pos1 == GAME_COLUMNS)
    {
        // check for vertical move
        return hasMoveV(pos1 % GAME_COLUMNS, pos1 / GAME_COLUMNS);
    }
    
    return false;
}

void GameEngine::swap(uint8_t i1, uint8_t i2)
{
    char temp = items[i2];
    short int templvl = levels[i2];
    bool tempprot = protection[i2];
    char tempbo = burning_order[i2];
    
    items[i2] = items[i1];
    levels[i2] = levels[i1];
    protection[i2] = protection[i1];
    burning_order[i2] = burning_order[i1];
    
    items[i1] = temp;
    levels[i1] = templvl;
    protection[i1] = tempprot;
    burning_order[i1] = tempbo;
}

GameEngine& GameEngine::operator=(const GameEngine& ge)
{
    memcpy(&items, &ge.items, sizeof(char) * BOARD_ITEMS);
    memcpy(&levels, &ge.levels, sizeof(short) * BOARD_ITEMS);
    
    int i;
    for (i = 0; i < BOARD_ITEMS; i++)
    {
        protection[i] = ge.protection[i];
    }
    
    farmer_pos = ge.farmer_pos;
    farmer_ko = ge.farmer_ko;
    
    return *this;
}

bool GameEngine::operator==(const GameEngine& ge)
{
    int i;
    for (i = 0; i < BOARD_ITEMS; i++)
    {
        if (items[i] != ge.items[i])
        {
            return false;
        }
        if (levels[i] != ge.levels[i])
        {
            return false;
        }
        if (protection[i] != ge.protection[i])
        {
            return false;
        }
    }
    
    if (farmer_pos == 0)
    {
        // this is solely for testing purposes
        // farmer_pos == 0 means farmer inactive
        return true;
    }
    
    if (farmer_pos != ge.farmer_pos)
    {
        return false;
    }
    
    if (farmer_ko != ge.farmer_ko)
    {
        return false;
    }
    
    return true;
}

std::string categoryToStr(uchar n_cat)
{
    static const char abbr[5] = {'y', 'r', 'g', 'p', 'b'};

    std::string str;
    str.push_back(abbr[n_cat / 10]);
    str.push_back(static_cast<char>('0' + static_cast<char>((n_cat % 10) / 2)));
    str.push_back(static_cast<char>(((n_cat % 2) == 0) ? '_' : '#'));
    return str;
}

uchar strToCategory(const std::string& str)
{
    uchar category = 0;

    switch (str[0])
    {
    case 'y':
        break;
    case 'r':
        category += 10;
        break;
    case 'g':
        category += 20;
        break;
    case 'p':
        category += 30;
        break;
    case 'b':
        category += 40;
        break;
    }
    
    category += (str[1] - '0') * 2;
    category += str[2] == '#' ? 1 : 0;
    
    return category;
}

/////////////////////////////////////////////////////////////////////////////
//
// RecognitionTree
//
/////////////////////////////////////////////////////////////////////////////

void readResource(const TiXmlNode* node, RecognitionTree::ItemCategory& val)
{
    memset(&val, 0, sizeof(val));
    TiXmlNode const* indexes;

    indexes = node->FirstChild("index");
    if (indexes != nullptr)
    {
        readResource(indexes, val.indexes);
    }
    
    TiXmlNode const* subcategories;
    
    subcategories = node->FirstChild("category");
    while (subcategories != nullptr)
    {
        val.subcategories.push_back(RecognitionTree::ItemCategory());
        RecognitionTree::ItemCategory& rtic = *val.subcategories.rbegin();
        readResource(subcategories, rtic);
        
        subcategories = subcategories->NextSiblingElement("category");
    }
    
    val.use_exclusion = node->FirstChild("use_exclusion") != nullptr;
    val.best_match = node->FirstChild("best_match") != nullptr;
    
    TiXmlNode const* min_threshold;
    min_threshold = node->FirstChild("min_threshold");
    if (min_threshold != nullptr)
    {
        readResource(min_threshold, val.min_threshold);
    }
    else
    {
        val.min_threshold = 0;
    }
}

void writeResource(TiXmlNode* node, const RecognitionTree::ItemCategory& val)
{
    TiXmlElement* elem;

    elem = new TiXmlElement("index");
    writeResource(elem, val.indexes);
    node->LinkEndChild(elem);
    
    if (val.subcategories.size() > 0)
    {
        size_t i;
        for (i = 0; i < val.subcategories.size(); i++)
        {
            TiXmlElement* category = new TiXmlElement("category");
            writeResource(category, val.subcategories[i]);
            node->LinkEndChild(category);
        }
    }
    
    if (val.use_exclusion)
    {
        node->LinkEndChild(new TiXmlElement("use_exclusion"));
    }
    
    if (val.best_match)
    {
        node->LinkEndChild(new TiXmlElement("best_match"));
    }
    
    if (abs(val.min_threshold) > 0)
    {
        TiXmlElement* min_threshold = new TiXmlElement("min_threshold");
        writeResource(min_threshold, val.min_threshold);
        node->LinkEndChild(min_threshold);
    }
}

/////////////////////////////////////////////////////////////////////////////
//
// DataContainer implementation
//
/////////////////////////////////////////////////////////////////////////////

DataContainer::DataContainer(const char* filename)
{
    load(filename);
}

bool DataContainer::findIdentifier(const Image& screen, cv::Point& id_pt) const
{
    if (identifier.empty())
    {
        std::cout << "identifier not stored" << std::endl;
        return false;
    }
    
    if (id_pt == cv::Point(-1, -1))
    {
        // identifier is yet unknown
        double max_val;
        screen.matchTemplate(identifier, id_pt, max_val);
        if (max_val < 0.50)
        {
            id_pt = cv::Point(-1, -1);
            return false;
        }
        else
        {
            Image id_rect;
            extractIdentifier(screen, id_pt, id_rect);

            if (identifiers.bestMatch(id_rect, 0.95) == -1)
            {
                id_pt = cv::Point(-1, -1);
                return false;
            }
        }
        
        return true;
    }
    else
    {
        // identifier position is known
        // checking if it is still there
        Image id_rect;
        extractIdentifier(screen, id_pt, id_rect);
    
        if (identifiers.bestMatch(id_rect, 0.95) == -1)
        {
            // identifier has disappeared!!
            id_pt = cv::Point(-1, -1);
            return findIdentifier(screen, id_pt);
        }
        
        // success !!
        return true;
    }
}

void DataContainer::translate(const cv::Point& id_pt, cv::Point& mod) const
{
    mod = id_pt + distance_to_topleft;
}

// captureGameRegion
//
// id_pt is handed over to make use of previous identifier searches
// if it is found at the same spot as before, the pattern matching
// time is significantly reduced
//
// rgn and mod are write only, output data if the identifier is found

bool DataContainer::captureGameRegion(cv::Point& id_pt, Image& rgn, cv::Point& mod) const
{
    if (id_pt == cv::Point(-1, -1))
    {
        rgn.capture();
        
        if (findIdentifier(rgn, id_pt))
        {
            // success !!
            translate(id_pt, mod);
            return true;
        }
        else
        {
            // identifier not found
            return false;
        }
    }
    else
    {
        Image id_rect;
        captureIdentifier(id_pt, id_rect);
    
        if (identifiers.bestMatch(id_rect, 0.95) == -1)
        {
            // identifier has been seen before,
            // but now it is disappeared!!
            id_pt = cv::Point(-1, -1);
            return captureGameRegion(id_pt, rgn, mod);
        }
        
        // success !!
        captureAll(id_pt, rgn, mod);
        return true;
    }
}

void DataContainer::captureAll(const cv::Point& id_pt, Image& rgn, cv::Point& mod) const
{
    mod = (id_pt + distance_to_topleft) % 2;
    
    rgn.captureRegion(
        (id_pt.x + distance_to_topleft.x) / 2, 
        (id_pt.y + distance_to_topleft.y) / 2, 
        (total_width / 2) + 1,
        (total_height / 2) + 1);
}

void DataContainer::captureIdentifier(const cv::Point& id_pt, Image& id_rect) const
{
    id_rect.captureRegion(
        id_pt.x / 2,
        id_pt.y / 2, 
        (identifier.width() / 2) + 1, 
        (identifier.height() / 2) + 1);
}

void DataContainer::extractIdentifier(const Image& screen, const cv::Point& id_pt, Image& id_rect) const
{
    screen.extract(id_rect, id_pt.x, id_pt.y, identifier.width(), identifier.height());
}

void DataContainer::extractCell(const Image& rgn, const cv::Point& mod, uint8_t i, uint8_t j, Image& cell) const
{
    rgn.extract(cell, 
        mod.x + distance_to_board.x + column_pos[i], 
        mod.y + distance_to_board.y + row_pos[j], 
        cell_width, cell_height);
}

void DataContainer::extractFarmer(const Image& rgn, const cv::Point& mod, uint8_t k, Image& farmer) const
{
    rgn.extract(farmer,
        mod.x + distance_to_farmer.x + column_pos[k],
        mod.y + distance_to_farmer.y,
        farmer_width, farmer_height);
}

void DataContainer::extractFarmerKO(const Image& rgn, const cv::Point& mod, uint8_t k, Image& farmer_ko) const
{
    rgn.extract(farmer_ko,
        mod.x + distance_to_farmer_ko.x + column_pos[k],
        mod.y + distance_to_farmer_ko.y,
        farmer_ko_width, farmer_ko_height);
}

int DataContainer::findFarmer(const Image& rgn, const cv::Point& mod) const
{
    size_t j;
    uint8_t k;

    for (k = 0; k < 11; k++)
    {
        for (j = 0; j < farmer_rep.size(); j++)
        {
            if (rgn.matchAt(
                mod.x + distance_to_farmer.x + column_pos[k], 
                mod.y + distance_to_farmer.y,
                farmer_rep[j], 0.8, 100) >= 0.8)
            {
                return k;
            }
        }
        
        for (j = 0; j < farmer_ko_rep.size(); j++)
        {
            if (rgn.matchAt(
                mod.x + distance_to_farmer_ko.x + column_pos[k], 
                mod.y + distance_to_farmer_ko.y,
                farmer_ko_rep[j], 0.99, 100) >= 0.99)
            {
                return static_cast<int>(k + ((j + 1) << 4));
            }
        }
    }
    
    return -1;
}

int DataContainer::identifyItemCategory(const Image& rgn, const cv::Point& mod, uint8_t i, uint8_t j, const RecognitionTree::ItemCategory& rtic) const
{
    size_t k;
    double val;
    
    if (rtic.best_match)
    {
        size_t max_k = 0xfffffffful;
        double max_val = 0;
        
        for (k = 0; k < rtic.subcategories.size(); k++)
        {
            match_profiles[rtic.subcategories[k].mp_index].match(rgn, 
                mod + distance_to_board + cv::Point(column_pos[i], row_pos[j]), rtic.min_threshold, val);
                
            if (val > max_val)
            {
                max_k = k;
                max_val = val;
            }
        }
        
        if (max_val < rtic.min_threshold)
        {
            return -1;
        }
        
        if (rtic.subcategories[max_k].indexes.size() == 1)
        {
            return static_cast<int>(rtic.subcategories[max_k].indexes[0]);
        }
        
        return identifyItemCategory(rgn, mod, i, j, rtic.subcategories[max_k]);
    }
    else
    {
        for (k = 0; k < rtic.subcategories.size(); k++)
        {
            if (match_profiles[rtic.subcategories[k].mp_index].match(rgn, 
                mod + distance_to_board + cv::Point(column_pos[i], row_pos[j]), rtic.min_threshold, val))
            {
                if (rtic.subcategories[k].indexes.size() == 1)
                {
                    return static_cast<int>(rtic.subcategories[k].indexes[0]);
                }
                
                return identifyItemCategory(rgn, mod, i, j, rtic.subcategories[k]);
            }
        }

        return -1;
    }
}

int DataContainer::identifyItemCategory(const Image& rgn, const cv::Point& mod, uint8_t i, uint8_t j) const
{
    return identifyItemCategory(rgn, mod, i, j, recognition_tree.root);
}

int DataContainer::tryAgainIdentify(const Image& rgn, const cv::Point& mod, uint8_t i, uint8_t j) const
{
    cv::Point periphery1[8] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
    cv::Point periphery2[16] = {{-2, -2}, {-2, -1}, {-2, 0}, {-2, 1}, {-2, 2}, {-1, -2}, {-1, 2}, {0, -2},
         {0, 2}, {1, -2}, {1, 2}, {2, -2}, {2, -1}, {2, 0}, {2, 1}, {2, 2}};
    
    cv::Point _mod;
    int result;

    int k;
    for (k = 0; k < 8; k++)
    {
        _mod = mod + periphery1[k];
        result = identifyItemCategory(rgn, _mod, i, j, recognition_tree.root);
        if (result != -1)
        {
            return result;
        }
    }
    
    for (k = 0; k < 16; k++)
    {
        _mod = mod + periphery2[k];
        result = identifyItemCategory(rgn, _mod, i, j, recognition_tree.root);
        if (result != -1)
        {
            return result;
        }
    }
    
    return -1;
}

void DataContainer::loadMatchProfile(RecognitionTree::ItemCategory& rtic, uint16_t& mp_index)
{
    rtic.mp_index = mp_index;
    if (match_profile_images.size() < (mp_index + 1))
    {
        // error: number of match profile images not sufficient!
        return;
    }
    
    if (mp_index != 0xffff) // root does not have a match profile
    {
        MatchProfile& mp = match_profiles.add();
        mp.createFromImage(items_comp[rtic.indexes[0]], match_profile_images[mp_index]);
        mp.makeSparse(200);
    }

    mp_index++;
    
    size_t j;
    for (j = 0; j < rtic.subcategories.size(); j++)
    {
        loadMatchProfile(rtic.subcategories[j], mp_index);
    }
}

void DataContainer::loadMatchProfiles()
{
    uint16_t mp_index = 0xffff;
    loadMatchProfile(recognition_tree.root, mp_index);
}

int _min3(int i1, int i2, int i3)
{
    if (i1 > i2)
    {
        if (i2 > i3)
        {
            return i3;
        }
        return i2;
    }
    if (i1 > i3)
    {
        return i3;
    }
    return i1;
}

int _max3(int i1, int i2, int i3)
{
    if (i1 > i2)
    {
        if (i1 > i3)
        {
            return i1;
        }
        return i3;
    }
    if (i2 > i3)
    {
        return i2;
    }
    return i3;
}

cv::Point _min3xy(const cv::Point& pt1, const cv::Point& pt2, const cv::Point& pt3)
{
    return cv::Point(_min3(pt1.x, pt2.x, pt3.x), _min3(pt1.y, pt2.y, pt3.y));
}

void DataContainer::recalcDimensions()
{
    distance_to_topleft = _min3xy(distance_to_board, distance_to_farmer, distance_to_farmer_ko);
    distance_to_topleft += cv::Point(-2, -2); // for tryAgainIdentify

    int board_right = distance_to_board.x + column_pos[10] + cell_width;
    int farmer_right = distance_to_farmer.x + column_pos[10] + farmer_width;
    int farmer_ko_right = distance_to_farmer_ko.x + column_pos[10] + farmer_ko_width;
    int maxr = _max3(board_right, farmer_right, farmer_ko_right);

    int board_bottom = distance_to_board.y + row_pos[6] + cell_height;

    total_width = maxr - distance_to_topleft.x;
    total_height = board_bottom - distance_to_topleft.y;
    
    total_width += 2; // for tryAgainIdentify
    total_height += 2;  // for tryAgainIdentify
    
    distance_to_board -= distance_to_topleft;
    distance_to_farmer -= distance_to_topleft;
    distance_to_farmer_ko -= distance_to_topleft;
}

void DataContainer::members(Mode mode)
{
    member(distance_to_board.x, "distance_to_board_x", mode);
    member(distance_to_board.y, "distance_to_board_y", mode);
    member(cell_width, "cell_width", mode);
    member(cell_height, "cell_height", mode);
    member(column_pos, "column_pos", mode);
    member(row_pos, "row_pos", mode);
    member(distance_to_farmer.x, "distance_to_farmer_x", mode);
    member(distance_to_farmer.y, "distance_to_farmer_y", mode);
    member(farmer_width, "farmer_width", mode);
    member(farmer_height, "farmer_height", mode);
    member(distance_to_farmer_ko.x, "distance_to_farmer_ko_x", mode);
    member(distance_to_farmer_ko.y, "distance_to_farmer_ko_y", mode);
    member(farmer_ko_width, "farmer_ko_width", mode);
    member(farmer_ko_height, "farmer_ko_height", mode);
    member(names, "names", mode);
    member(recognition_tree.root, "recognition_tree", mode);
    member(identifier_filename, "identifier", mode);
    member(identifier_array_filename, "identifier_array", mode);
    member(items_comp_filename, "items_comp", mode);
    member(match_profiles_filename, "match_profiles", mode);
    member(farmer_representation_filename, "farmer_representation", mode);
    member(farmer_ko_representation_filename, "farmer_ko_representation", mode);
    
    if (mode == eIn)
    {
        board_width = *column_pos.rbegin() + cell_width - *column_pos.begin();
        board_height = *row_pos.rbegin() + cell_height - *row_pos.begin();

        identifier.load(identifier_filename.c_str());
        identifiers.load(identifier_array_filename.c_str(), identifier.width());
        match_profile_images.load(match_profiles_filename.c_str(), cell_width);
        items_comp.load(items_comp_filename.c_str(), cell_width);
        
        loadMatchProfiles();
        
        farmer_rep.load(farmer_representation_filename.c_str(), farmer_width);
        farmer_ko_rep.load(farmer_ko_representation_filename.c_str(), farmer_ko_width);
        
        recalcDimensions();
    }
}
