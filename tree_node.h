//predmet: Bakalarska prace
//autor: xcasla03@stud.fit.vutbr.cz, Martin Caslava
//projekt: Bakalarska prace - graficka podoba uzlu stavoveho prostoru (pouze dilci modul celeho programu)
//prog: jazyk: C++
//hodnoceni: doufejme ze kladne:-)

#ifndef TREE_NODE_H
#define TREE_NODE_H

#include <Qpainter>
#include <QGraphicsItem>
#include <QGridLayout>


/**
 * Trida reprezentujici grafickou nadstavbu uzlu stromu
 */

class tree_node : public QGraphicsItem
{
public:
    int type;//1 - root, 2- normalni uzel
    tree_node( int id );
    QRectF boundingRect() const;

    // overriding paint()
    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);

protected:
    int id;
    // overriding mouse events
    void drawCross(QPainter * painter);
    void drawCircle(QPainter * painter);
    void showCurrentGame();
    void printBoard();
};

#endif // TREE_NODE_H
