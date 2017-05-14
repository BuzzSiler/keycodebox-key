#ifndef TIMEZONEBOX_H
#define TIMEZONEBOX_H

#include <QDebug>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPushButton>

class TimeZoneBox : public QComboBox
{
  Q_COMBOBOX
public:
    explicit FocusWatcher(QObject* parent = nullptr) : QObject(parent)
  {
    if (parent)
      parent->installEventFilter(this);
#endif // TIMEZONEBOX_H
