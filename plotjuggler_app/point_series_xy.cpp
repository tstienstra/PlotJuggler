/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "point_series_xy.h"
#include <cmath>
#include <cstdlib>

PointSeriesXY::PointSeriesXY(const PlotData* x_axis, const PlotData* y_axis)
  : QwtTimeseries(nullptr), _x_axis(x_axis), _y_axis(y_axis), _cached_curve("", x_axis->group())
{
  updateCache(true);
}

size_t PointSeriesXY::size() const
{
  return _cached_curve.size();
}

std::optional<QPointF> PointSeriesXY::sampleFromTime(double t)
{
  if (_cached_curve.size() == 0)
  {
    return {};
  }

  int index = _y_axis->getIndexFromX(t);
  if (index < 0)
  {
    return {};
  }
  const auto& p = _cached_curve.at(size_t(index));
  return QPointF(p.x, p.y);
}

RangeOpt PointSeriesXY::getVisualizationRangeY(Range)
{
  return _cached_curve.rangeY();
}

void PointSeriesXY::updateCache(bool reset_old_data)
{
  _cached_curve.clear();

  if (_x_axis == nullptr)
  {
    throw std::runtime_error("the X axis is null");
  }

  const size_t data_size = std::min(_x_axis->size(), _y_axis->size());

  if (data_size == 0)
  {
    return;
  }

  const double EPS = std::numeric_limits<double>::epsilon();

  const double t_low = _windowed ? (_tracker_time - _prev_sec) : std::numeric_limits<double>::lowest();
  const double t_high = _windowed ? (_tracker_time + _next_sec) : std::numeric_limits<double>::max();

  // When windowed, use binary search to skip points before the window
  size_t start_index = 0;
  if (_windowed)
  {
    int hint = _x_axis->getIndexFromX(t_low);
    start_index = (hint > 1) ? static_cast<size_t>(hint - 1) : 0;
  }

  for (size_t i = start_index; i < data_size; i++)
  {
    if (std::abs(_x_axis->at(i).x - _y_axis->at(i).x) > EPS)
    {
      throw std::runtime_error("X and Y axis don't share the same time axis");
    }

    const double t = _x_axis->at(i).x;
    if (t > t_high)
    {
      break;
    }
    if (t < t_low)
    {
      continue;
    }

    const QPointF p(_x_axis->at(i).y, _y_axis->at(i).y);
    _cached_curve.pushBack({ p.x(), p.y() });
  }
}

void PointSeriesXY::setTimeWindow(double prev_sec, double next_sec)
{
  _windowed = true;
  _prev_sec = prev_sec;
  _next_sec = next_sec;
}

void PointSeriesXY::clearTimeWindow()
{
  _windowed = false;
}

void PointSeriesXY::setTrackerTime(double t)
{
  _tracker_time = t;
}

RangeOpt PointSeriesXY::getVisualizationRangeX()
{
  return _cached_curve.rangeX();
}
