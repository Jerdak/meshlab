/****************************************************************************
 * MeshLab                                                           o o     *
 * A versatile mesh processing toolbox                             o     o   *
 *                                                                _   O  _   *
 * Copyright(C) 2008                                                \/)\/    *
 * Visual Computing Lab                                            /\/|      *
 * ISTI - Italian National Research Council                           |      *
 *                                                                    \      *
 * All rights reserved.                                                      *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
 * for more details.                                                         *
 *                                                                           *
 ****************************************************************************/
/****************************************************************************
  History
$Log: editmeasure.cpp,v $
****************************************************************************/

#include <meshlab/glarea.h>
#include "edit_measure.h"
#include <wrap/qt/gl_label.h>

using namespace vcg;

EditMeasurePlugin::EditMeasurePlugin()
 :rubberband(Color4b(255,170,85,255)),was_ready(false)
{}

const QString EditMeasurePlugin::Info()
{
    return tr("Allow to measure distances between points of a model");
}

void EditMeasurePlugin::mousePressEvent(QMouseEvent *, MeshModel &, GLArea * gla)
{
  if(was_ready||rubberband.IsReady()){
    rubberband.Reset();
    was_ready=false;
  }
  gla->update();
}

void EditMeasurePlugin::mouseMoveEvent(QMouseEvent * event, MeshModel &, GLArea * gla)
{
  rubberband.Drag(event->pos());
  gla->update();
}

void EditMeasurePlugin::mouseReleaseEvent(QMouseEvent * event, MeshModel &, GLArea * gla)
{
  rubberband.Pin(event->pos());
  gla->update();
}

void EditMeasurePlugin::Decorate(MeshModel &, GLArea * gla,QPainter* p)
{
  rubberband.Render(gla);
  if(rubberband.IsReady())
  {
    Point3f a,b;
    rubberband.GetPoints(a,b);
      vcg::glLabel::render(p,b,QString("%1").arg(Distance(a,b)));

    if(!was_ready)
    {
      suspendEditToggle();
      this->Log(GLLogStream::FILTER, "Distance: %f",Distance(a,b));
    }
    was_ready=true;
  }
  assert(!glGetError());
}

bool EditMeasurePlugin::StartEdit(MeshModel &, GLArea *gla )
{
  gla->setCursor(QCursor(QPixmap(":/images/cur_measure.png"),15,15));
  connect(this, SIGNAL(suspendEditToggle()),gla,SLOT(suspendEditToggle()) );
  was_ready = false;
  rubberband.Reset();
    return true;
}

void EditMeasurePlugin::EndEdit(MeshModel &, GLArea *)
{
  was_ready = false;
  rubberband.Reset();
}
