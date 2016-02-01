/****************************************************************************
* MeshLab                                                           o o     *
* A versatile mesh processing toolbox                             o     o   *
*                                                                _   O  _   *
* Copyright(C) 2005                                                \/)\/    *
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

#ifndef FILTER_FUNC_PLUGIN_H
#define FILTER_FUNC_PLUGIN_H

#include <QObject>

#include <common/interfaces.h>

#include "muParser.h"
#include "filter_refine.h"

class FilterFunctionPlugin : public QObject, public MeshFilterInterface
{
	Q_OBJECT
	MESHLAB_PLUGIN_IID_EXPORTER(MESH_FILTER_INTERFACE_IID)
	Q_INTERFACES(MeshFilterInterface)

protected:
  double x,y,z,nx,ny,nz,r,g,b,q,fq,rad,vtu,vtv;
  double x0,y0,z0,x1,y1,z1,x2,y2,z2,nx0,ny0,nz0,nx1,ny1,nz1,nx2,ny2,nz2,r0,g0,b0,r1,g1,b1,r2,g2,b2,q0,q1,q2,wtu0,wtv0,wtu1,wtv1,wtu2,wtv2;
	double v,f,v0i,v1i,v2i;
  std::vector<std::string> v_attrNames;  // names of the <float> per vertex attributes
  std::vector<double>      v_attrValue;  // values of the <float> per vertex attributes
  std::vector<std::string> v3_attrNames;  // names of the <Point3f> per vertex attributes There are 3x (one foreach coord _x, _y, _z)
  std::vector<double>      v3_attrValue;  // values of the <Point3f> per vertex attributes. There are 3x (one foreach coord _x, _y, _z)
  std::vector<std::string> f_attrNames;
	std::vector<double> f_attrValue;
  std::vector<CMeshO::PerVertexAttributeHandle<float> > v_handlers;
  std::vector<CMeshO::PerVertexAttributeHandle<Point3f> > v3_handlers;
  std::vector<CMeshO::PerFaceAttributeHandle<float> > f_handlers;

public:
	enum {
	  FF_VERT_SELECTION,
	  FF_FACE_SELECTION,
	  FF_GEOM_FUNC,
	  FF_VERT_TEXTURE_FUNC,
	  FF_WEDGE_TEXTURE_FUNC,
	  FF_FACE_COLOR,
	  FF_VERT_COLOR,
	  FF_VERT_NORMAL,
	  FF_VERT_QUALITY,
	  FF_FACE_QUALITY,
	  FF_DEF_VERT_ATTRIB,
	  FF_DEF_FACE_ATTRIB,
	  FF_GRID,
	  FF_ISOSURFACE,
	  FF_REFINE
	} ;

	FilterFunctionPlugin();
	~FilterFunctionPlugin();
	
	virtual QString filterName(FilterIDType filter) const;
	virtual QString filterInfo(FilterIDType filter) const;
	virtual FilterClass getClass(QAction *);
  virtual int postCondition(QAction *action) const;
  virtual void initParameterSet(QAction *,MeshModel &/*m*/, RichParameterSet & /*parent*/);
	virtual int getRequirements(QAction *);
  virtual bool applyFilter(QAction *filter, MeshDocument &md, RichParameterSet & /*parent*/, vcg::CallBackPos * cb) ;

	void showParserError(const QString &s, mu::Parser::exception_type &e);
  void setAttributes(CMeshO::VertexIterator &vi,CMeshO &m);
	void setAttributes(CMeshO::FaceIterator &fi,CMeshO &m);
  void setPerVertexVariables(mu::Parser &p, CMeshO &m);
  void setPerFaceVariables(mu::Parser &p, CMeshO &m);

};

#endif
