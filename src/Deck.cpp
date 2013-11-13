/*=============================================================================
 * TarotClub - Deck.cpp
 *=============================================================================
 * A deck composed of a (dynamic) list of cards.
 *=============================================================================
 * TarotClub ( http://www.tarotclub.fr ) - This file is part of TarotClub
 * Copyright (C) 2003-2999 - Anthony Rabine
 * anthony@tarotclub.fr
 *
 * TarotClub is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TarotClub is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TarotClub.  If not, see <http://www.gnu.org/licenses/>.
 *
 *=============================================================================
 */

#include "Deck.h"
// Standard includes
#ifndef QT_NO_DEBUG
#include <iostream>
#include <fstream>
#endif // QT_NO_DEBUG

#include <QtAlgorithms>

#include <cstdlib>
#include <ctime>
using namespace std;


/*****************************************************************************/
Deck::Deck()
{
    owner = NO_TEAM;
}
/*****************************************************************************/
QString Deck::GetCardList()
{
    QString list;

    for (int i = 0; i < size(); i++)
    {
        list += at(i)->GetName();
        if (i < (size() - 1))
        {
            list += ";";
        }
    }
    return list;
}
/*****************************************************************************/
void Deck::Shuffle(int seed)
{
    for (int i = size(); i > 0; --i)
    {
        // pseudorandom number generation algorithm
        // taken from KDE game KPat, thanks !
        seed = 214013 * seed + 2531011;
        int rand = (seed >> 16) & 0x7fff;
        int z = rand % i;
        swap(z, i - 1);
    }
}
/*****************************************************************************/
Card *Deck::GetCardById(int id)
{
    for (int i = 0; i < this->size(); i++)
    {
        if (this->at(i)->GetId() == id)
        {
            return this->at(i);
        }
    }
    return NULL;
}
/*****************************************************************************/
Card *Deck::GetCardByName(const QString &i_name)
{
    for (int i = 0; i < this->size(); i++)
    {
        if (this->at(i)->GetName() == i_name)
        {
            return this->at(i);
        }
    }
    return NULL;
}
/*****************************************************************************/
bool Deck::HasCard(Card *c)
{
    if (this->indexOf(c) == -1)
    {
        return false;
    }
    else
    {
        return true;
    }
}
/*****************************************************************************/
bool Deck::HasOneOfTrump()
{
    for (int i = 0; i < this->count(); i++)
    {
        if ((this->at(i)->GetSuit() == Card::TRUMPS) &&
                (this->at(i)->GetValue() == 1))
        {
            return true;
        }
    }
    return false;
}
/*****************************************************************************/
bool Deck::HasFool()
{
    for (int i = 0; i < this->count(); i++)
    {
        if ((this->at(i)->GetSuit() == Card::TRUMPS) &&
                (this->at(i)->GetValue() == 0))
        {
            return true;
        }
    }
    return false;
}
/*****************************************************************************/
/**
 * @brief Deck::HighestTrump
 *
 * This algorithm volontary eliminates the fool, which as a value of zero.
 * It is not considered as the highest trump, even if it is alone in the deck.
 *
 * @return The highest trump in the deck
 */
Card *Deck::HighestTrump()
{
    Card *c = NULL;
    int value = 0;

    for (int i = 0; i < this->count(); i++)
    {
        if ((this->at(i)->GetSuit() == Card::TRUMPS) &&
                (this->at(i)->GetValue() > value))
        {
            value = this->at(i)->GetValue();
            c = this->at(i);
        }
    }
    return c;
}
/*****************************************************************************/
Card *Deck::HighestSuit()
{
    Card *c = NULL;
    int value = 0;

    for (int i = 0; i < this->count(); i++)
    {
        if ((this->at(i)->GetSuit() != Card::TRUMPS) &&
                (this->at(i)->GetValue() > value))
        {
            value = this->at(i)->GetValue();
            c = this->at(i);
        }
    }
    return c;
}
/*****************************************************************************/
void Deck::Sort()
{
    if (this->size() != 0)
    {
        qSort(this->begin(), this->end(), LessThanCards);
    }
}
/*****************************************************************************/
bool Deck::LessThanCards(Card *c1, Card *c2)
{
    return c1->GetId() > c2->GetId();
}
/*****************************************************************************/
void Deck::SetOwner(Team team)
{
    owner = team;
}
/*****************************************************************************/
int Deck::SetCards(const QString &cards)
{
    QStringList list = cards.split(';', QString::SkipEmptyParts);
    int count = 0;

    // Clear this deck before setting new cards
    this->clear();
    for (int i=0; i<list.size(); i++)
    {
        Card *c = TarotDeck::GetCard(list[0]);
        if (c != NULL)
        {
            count++;
            this->append(c);
        }
    }
    return count;
}
/*****************************************************************************/
Team Deck::GetOwner()
{
    return owner;
}
/*****************************************************************************/
void Deck::Statistics::Reset()
{
    nbCards = 0;

    oudlers = 0;
    trumps = 0;
    majorTrumps = 0;

    kings = 0;
    queens = 0;
    knights = 0;
    jacks = 0;

    weddings = 0;
    longSuits = 0;
    cuts = 0;
    singletons = 0;
    sequences = 0;

    clubs = 0;
    spades = 0;
    hearts = 0;
    diamonds = 0;

    littleTrump = false;
    bigTrump = false;
    fool = false;

    points = 0.0;
}
/*****************************************************************************/
void Deck::AnalyzeTrumps(Statistics &stats)
{
    int i, val;
    Card *c;

    stats.nbCards = this->size();

    // looking for trumps
    for (i = 0; i < this->count(); i++)
    {
        c = this->at(i);
        if (c->GetSuit() == Card::TRUMPS)
        {
            stats.trumps++;
            val = c->GetValue();
            if (val >= 15)
            {
                stats.majorTrumps++;
            }
            if (val == 21)
            {
                stats.bigTrump = true;
                stats.oudlers++;
            }
            if (val == 1)
            {
                stats.littleTrump = true;
                stats.oudlers++;
            }
            if (val == 0)
            {
                stats.fool = true;
                stats.oudlers++;
            }
        }
        stats.points += c->GetPoints();
    }
}
/*****************************************************************************/
void Deck::AnalyzeSuits(Statistics &stats)
{
    Card::Suit suit;
    int longue;
    int count = 0;
    int flag = 0;
    Card *c;
    int i, k;

    int distr[14] = {0}; // test of a distribution

    // Normal suits
    for (i = 0; i < 4; i++)
    {
        if (i == 0)
        {
            suit = Card::SPADES;
        }
        else if (i == 1)
        {
            suit = Card::HEARTS;
        }
        else if (i == 2)
        {
            suit = Card::CLUBS;
        }
        else
        {
            suit = Card::DIAMONDS;
        }

        for (k = 0; k < 14; k++)
        {
            distr[k] = 0;
        }
        count = 0;

        for (k = 0; k < this->count(); k++)
        {
            c = this->at(k);
            if (c->GetSuit() == suit)
            {
                count++;
                int val = c->GetValue();
                distr[val - 1] = 1;
                if (val == 11)
                {
                    stats.jacks++;
                }
                if (val == 12)
                {
                    stats.knights++;
                }
            }
        }

        if (count == 1)
        {
            stats.singletons++;
        }
        if (count == 0)
        {
            stats.cuts++;
        }

        // Number of cards in each normal suit
        if (i == 0)
        {
            stats.spades = count;
        }
        else if (i == 1)
        {
            stats.hearts = count;
        }
        else if (i == 2)
        {
            stats.clubs = count;
        }
        else
        {
            stats.diamonds = count;
        }

        if ((distr[13] == 1) && (distr[12] == 1))
        {
            stats.weddings++; // mariage (king + queen)
        }
        if (distr[13] == 1)
        {
            stats.kings++;     // king without queen
        }
        if (distr[12] == 1)
        {
            stats.queens++;    // queen without kings
        }

        // test des séquences :
        count = 0;  // longueur de la séquence
        flag = 0;   // couleur trouvée : on est dans la séquence
        longue = 0;

        for (k = 0; k < 14; k++)
        {
            if (distr[k] == 1)
            {
                longue++;
                // début d'une séquence
                if (flag == 0)
                {
                    flag = 1;
                    count++;
                }
                else
                {
                    count++;
                }
            }
            else if (flag == 1)
            {
                if (count >= 5)
                {
                    stats.sequences++;
                }
                count = 0;
                flag = 0;
            }
        }
        if (longue >= 5)
        {
            stats.longSuits++;
        }
    }
}

//=============================================================================
// End of file Deck.cpp
//=============================================================================
