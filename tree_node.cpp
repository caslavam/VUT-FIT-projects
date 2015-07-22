//predmet: Bakalarska prace
//autor: xcasla03@stud.fit.vbutbr.cz, Martin Caslava
//projekt: Bakalarska prace - graficka podoba uzlu stavoveho prostoru (pouze dilci modul celeho programu)
//prog: jazyk: C++
//hodnoceni: doufejme ze kladne:-)

#include "tree_node.h",
#include "game_board.h"
#include "square.h"
#include "qdebug.h"
#include <iostream>


extern game current_game;
tree_node::tree_node( int id )
{
    this->id = id;
}

QRectF tree_node::boundingRect() const
{
    // outer most edges
    return QRectF(0,0,30,30);
}

void tree_node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF rect = boundingRect();
    QPen pen(Qt::black, 2);
    painter->setPen(pen);
    painter->drawRect(rect);

    //vykresli mrizku
    QLineF line1(0,10,30,10);
    painter->drawLine(line1);
    QLineF line2(0,20,30,20);
    painter->drawLine(line2);
    QLineF line3(10,0,10,30);
    painter->drawLine(line3);
    QLineF line4(20,0,20,30);
    painter->drawLine(line4);

    QPen pen2(Qt::red, 1.4);
    painter->setPen(pen2);

    /**
      * KOLECKA
      */
    for ( int i = 1; i <= 9; i++ )
    {
        if (current_game.tree[this->id]->node_game_desk[i] == 'O')
            painter->drawEllipse(10 * ((i - 1) % 3), 10 * ((i - 1) / 3), 10, 10);
    }

    /**
      * KRIZKY
      */
    QPen pen3(Qt::blue, 1.4);
    painter->setPen(pen3);

    int xx, yy;
    for ( int i = 1; i <= 9; i++ )
    {
        if (current_game.tree[this->id]->node_game_desk[i] == 'X')
        {
            xx = 10 * ((i - 1) % 3); yy = 10 * ((i - 1) / 3);
            QLineF cross_line1(xx, yy, xx + 10, yy + 10);
            painter->drawLine(cross_line1);

            QLineF cross_line2(xx + 10, yy, xx, yy + 10);
            painter->drawLine(cross_line2);
        }
    }
}
