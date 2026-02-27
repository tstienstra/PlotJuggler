/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <QDoubleSpinBox>
#include "PlotJuggler/transform_function.h"
#include "ui_time_window_transform.h"

using namespace PJ;

namespace Ui
{
class TimeWindowTransform;
}

/**
 * @brief TimeWindowTransform shows only the portion of a timeseries within
 * [tracker_time - prev_seconds, tracker_time + next_seconds].
 *
 * The output series is recomputed every time the tracker position changes,
 * making it useful for "follow" views in XY plots or time plots.
 */
class TimeWindowTransform : public TransformFunction_SISO
{
public:
  explicit TimeWindowTransform();

  ~TimeWindowTransform() override;

  static const char* transformName()
  {
    return "Time Window";
  }

  const char* name() const override
  {
    return transformName();
  }

  QWidget* optionsWidget() override;

  bool xmlSaveState(QDomDocument& doc, QDomElement& parent_element) const override;

  bool xmlLoadState(const QDomElement& parent_element) override;

  /** Called by MainWindow::updateReactivePlots() each time the tracker moves. */
  void setTimeTracker(double t)
  {
    _tracker_time = t;
  }

  // Accessors for the current spinbox values so the dialog can read/write them
  double prevSec() const;
  double nextSec() const;
  void setValues(double prev_sec, double next_sec);

  /**
   * Overrides the SISO calculate() to clear the destination and recompute
   * the window from scratch each call.
   */
  void calculate() override;

private:
  Ui::TimeWindowTransform* ui;
  QWidget* _widget;
  double _tracker_time = 0.0;

  /** Required by TransformFunction_SISO but not used (calculate() is fully overridden). */
  std::optional<PlotData::Point> calculateNextPoint(size_t index) override;
};
