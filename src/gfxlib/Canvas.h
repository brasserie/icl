/*=============================================================================
 * TarotClub - Canvas.h
 *=============================================================================
 * Visual game contents
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

#ifndef CANVAS_H
#define CANVAS_H

// Qt includes
#include <QtSvg>
#include <QtGui>
#include <QVector>
#include <QList>
#include <QTemporaryFile>
#include <QFlags>

// Game includes
#include "Common.h"
#include "Deck.h"
#include "Identity.h"
#include "TextBox.h"
#include "PlayerBox.h"
#include "GfxCard.h"
#include "MenuItem.h"
#include "PopupItem.h"
#include "MessageBoxItem.h"
#include "IButtonEvent.h"
#include "ICardEvent.h"
#include "Score.h"


/*****************************************************************************/
class Canvas : public QGraphicsView, public IButtonEvent, public ICardEvent
{
    Q_OBJECT

public:
    enum Filter
    {
        BLOCK_ALL   = 0x00,
        MENU        = 0x01,
        CARDS       = 0x02,
        BOARD       = 0x04
    };

    enum CursorType
    {
        FORBIDDEN,
        ARROW
    };

    Canvas(QWidget *parent);

    // Helpers
    bool Initialize();
    void ShowTaker(Place taker, Place myPlace);
    void ShowSelection(Place p, Place myPlace);
    void DrawCard(std::uint8_t index, Place p, Place myPlace);
    void DrawSouthCards(const Deck &cards);
    void DrawCardsInPopup(const QList<Card *> &cards);
    void HidePopup();
    void ShowBidsChoice(Contract contract);
    void ShowBid(Place p, Contract contract, Place myPlace);
    void HideBidsChoice();
    void ShowAvatars(bool b);
    void InitBoard();
    void ResetCards();
    Place SwapPlace(Place my_place, Place absolute);
    void DisplayDiscardMenu(bool visible);
    void DisplayHandleMenu(bool visible);
    void DisplayMainMenu(bool visible);
    void ShowCard(std::uint8_t index, bool visible);
    void ToggleCardSelection(std::uint8_t index);

    // Getters
    bool GetSlamOption();

    // Setters
    void SetCursorType(CursorType t);
    void SetAvatar(Place p, const QString &file);
    void SetFilter(quint8 f);
    void SetBackground(const std::string &code);
    void SetPlayerIdentity(QMap<Place, Identity> &players, Place myPlace);
    void SetResult(const Score &score, const Tarot::Bid &bid);

    // From IButtonEvent
    void ButtonClicked(std::uint8_t id, std::uint8_t menu);

    // From ICardEvent
    void CardClicked(std::uint8_t card, bool selected);
    void CardHoverEnter(std::uint8_t card);
    void CardHoverLeave(std::uint8_t card);

signals:
    void sigViewportClicked();
    void sigClickCard(std::uint8_t index, bool selected);
    void sigCursorOverCard(std::uint8_t index);
    void sigContract(Contract c);
    void sigAcceptDiscard();
    void sigAcceptHandle();
    void sigStartGame();

protected:

    void  mousePressEvent(QMouseEvent *e);
/*    void  mouseMoveEvent(QMouseEvent *e);
    */
    void  resizeEvent(QResizeEvent *event);

private:
    void DrawCardShadows();
    bool TestFilter(quint8 mask);

    quint8 mFilter;
    bool mShowAvatars;
    QVector<GfxCard *> cardsPics;
    QGraphicsScene scene;

    QMap<Place, PlayerBox *> playerBox;
    MenuItem    menuItem;
    PopupItem   popupItem;
    MessageBoxItem mMsgBoxItem;
};

#endif // CANVAS_H

//=============================================================================
// End of file Canvas.h
//=============================================================================
