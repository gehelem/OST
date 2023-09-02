#ifndef COMMON_VALUE_h_
#define COMMON_VALUE_h_

#include <QtCore>

namespace  OST
{

typedef enum
{
    Idle = 0, /*!< State is idle */
    Ok,       /*!< State is ok */
    Busy,     /*!< State is busy */
    Error     /*!< State is error */
} State;
inline State IntToState(int val )
{
    if (val == 0) return Idle;
    if (val == 1) return Ok;
    if (val == 2) return Busy;
    if (val == 3) return Error;
    qDebug() << "Cant convert " << val << " to OST::State (0-3) - defaults to Idle";
    return Idle;
}
inline int StateToInt(State val )
{
    if (val == Idle) return 0;
    if (val == Ok) return 1;
    if (val == Busy) return 2;
    if (val == Error) return 3;
    qDebug() << "StateToInt Cant convert state - return State::Error";
    return 3;
}

template <typename U>
class ListOfValues
{
    public:
        QMap<U, QString> getList()
        {
            return list;
        }
        bool add(U val, QString label)
        {
            if (list.contains(val))
            {
                qDebug() << "ListOfValues - add - key " << val << " already exists (" << list[val] << ").";
                return false;
            }
            list[val] = label;
            return true;
        }
        bool update(U val, QString label)
        {
            if (!list.contains(val))
            {
                qDebug() << "ListOfValues - update - key " << val << " doesn't exist (" << list[val] << ").";
                return false;
            }
            list[val] = label;
            return true;
        }
        bool del(U val)
        {
            if (!list.contains(val))
            {
                qDebug() << "ListOfValues - del - key " << val << " does not exist.";
                return false;
            }
            list.remove(val);
            return true;
        }
        bool clear()
        {
            list.clear();
            return true;
        }

    private:
        QMap<U, QString> list;
};

template <typename U>
class Grid
{
    public:
        QList<U> getGrid()
        {
            return mGridValues;
        }
        bool add(U val)
        {
            mGridValues.append(val);
            return true;
        }
        bool del(int i)
        {
            if (i >= mGridValues.size())
            {
                qDebug() << "Grid - del - trying to delete item " << i << " while grid size is " << mGridValues.size();
                return false;
            }
            mGridValues.removeAt(i);
            return true;
        }
        bool update(U val, int i)
        {
            if (i >= mGridValues.size())
            {
                qDebug() << "Grid - update - trying to update item " << i << " while grid size is " << mGridValues.size();
                return false;
            }
            mGridValues[i] = val;
            return true;
        }
        bool clear()
        {
            mGridValues.clear();
            return true;
        }
        void setGridValues(QList<U> vals )
        {
            mGridValues.clear();
            mGridValues = vals;
        }
    private:
        QList<U> mGridValues;

};


}

#endif


