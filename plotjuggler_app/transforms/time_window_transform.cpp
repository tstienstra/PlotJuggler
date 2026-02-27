/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "time_window_transform.h"
#include "ui_time_window_transform.h"

using namespace PJ;

TimeWindowTransform::TimeWindowTransform() : ui(new Ui::TimeWindowTransform), _widget(new QWidget())
{
  ui->setupUi(_widget);

  connect(ui->spinBoxPrev, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
          [=](double) { emit parametersChanged(); });

  connect(ui->spinBoxNext, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
          [=](double) { emit parametersChanged(); });
}

TimeWindowTransform::~TimeWindowTransform()
{
  delete ui;
  delete _widget;
}

QWidget* TimeWindowTransform::optionsWidget()
{
  return _widget;
}

void TimeWindowTransform::calculate()
{
  if (_src_vector.empty() || _dst_vector.empty())
  {
    return;
  }

  const PlotData* src = _src_vector.front();
  PlotData* dst = _dst_vector.front();

  // Always recompute from scratch: clear the destination first
  dst->clear();
  dst->setMaximumRangeX(src->maximumRangeX());

  if (src->size() == 0)
  {
    return;
  }

  const double prev_sec = ui->spinBoxPrev->value();
  const double next_sec = ui->spinBoxNext->value();
  const double lower = _tracker_time - prev_sec;
  const double upper = _tracker_time + next_sec;

  // Use getIndexFromX as a starting hint, then walk back one step to avoid
  // missing the first in-window point (getIndexFromX returns the nearest index,
  // not necessarily the first point >= lower).
  int hint = src->getIndexFromX(lower);
  size_t i = (hint > 1) ? static_cast<size_t>(hint - 1) : 0;

  for (; i < src->size(); i++)
  {
    const auto& p = src->at(i);
    if (p.x > upper)
    {
      break;
    }
    if (p.x >= lower)
    {
      dst->pushBack(p);
    }
  }
}

// Required by the SISO interface but never called (calculate() is fully overridden).
std::optional<PlotData::Point> TimeWindowTransform::calculateNextPoint(size_t index)
{
  return dataSource()->at(index);
}

bool TimeWindowTransform::xmlSaveState(QDomDocument& doc, QDomElement& parent_element) const
{
  QDomElement elem = doc.createElement("options");
  elem.setAttribute("prev_seconds", ui->spinBoxPrev->value());
  elem.setAttribute("next_seconds", ui->spinBoxNext->value());
  parent_element.appendChild(elem);
  return true;
}

bool TimeWindowTransform::xmlLoadState(const QDomElement& parent_element)
{
  QDomElement elem = parent_element.firstChildElement("options");
  if (elem.isNull())
  {
    return false;
  }
  ui->spinBoxPrev->setValue(elem.attribute("prev_seconds", "5.0").toDouble());
  ui->spinBoxNext->setValue(elem.attribute("next_seconds", "5.0").toDouble());
  return true;
}

double TimeWindowTransform::prevSec() const
{
  return ui->spinBoxPrev->value();
}

double TimeWindowTransform::nextSec() const
{
  return ui->spinBoxNext->value();
}

void TimeWindowTransform::setValues(double prev_sec, double next_sec)
{
  QSignalBlocker b1(ui->spinBoxPrev);
  QSignalBlocker b2(ui->spinBoxNext);
  ui->spinBoxPrev->setValue(prev_sec);
  ui->spinBoxNext->setValue(next_sec);
}
