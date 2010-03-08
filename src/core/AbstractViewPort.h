#ifndef ABSTRACTVIEWPORT_H
#define ABSTRACTVIEWPORT_H

#include <QPointF>
#include <QList>

class ContextItem;

class AbstractViewPort
{
public:
        AbstractViewPort() {};
        ~AbstractViewPort() {};

        virtual void grab_mouse() = 0;
        virtual void release_mouse() = 0;
        virtual QPointF map_to_scene(int x, int y) const = 0;
        virtual int get_current_mode() const = 0;
        virtual void set_cursor(const QString& cursor) = 0;
        virtual void reset_cursor() = 0;
        virtual void set_holdcursor(const QString& cursorName) = 0;
        virtual void set_holdcursor_text(const QString& text) = 0;
        virtual void set_holdcursor_pos(QPointF pos) = 0;
        virtual void get_pointed_context_items(QList<ContextItem* > &list) = 0;
};

#endif // ABSTRACTVIEWPORT_H
