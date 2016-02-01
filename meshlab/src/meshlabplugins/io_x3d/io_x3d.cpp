/****************************************************************************
* MeshLab                                                           o o     *
* An extendible mesh processor                                    o     o   *
*                                                                _   O  _   *
* Copyright(C) 2005, 2006                                          \/)\/    *
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

#include <Qt>
#include <QtXml>


#include "io_x3d.h"

#include "import_x3d.h"
#include "export_x3d.h"


#include <QMessageBox>
#include <QFileDialog>

using namespace std;
using namespace vcg;

bool IoX3DPlugin::open(const QString &formatName, const QString &fileName, MeshModel &m, int& mask, const RichParameterSet &, CallBackPos *cb, QWidget *parent)
{
	// initializing mask
	mask = 0;
	
	// initializing progress bar status
	if (cb != NULL)		(*cb)(0, "Loading...");

	QString errorMsgFormat = "Error encountered while loading file:\n\"%1\"\n\nFile: %2\nError details: %3";
	string filename = QFile::encodeName(fileName).constData ();
	bool normalsUpdated = false;
	vcg::tri::io::AdditionalInfoX3D* info = NULL;
	if(formatName.toUpper() == tr("X3D") || formatName.toUpper() == tr("X3DV") || formatName.toUpper() == tr("WRL"))
	{
		int result;
		if (formatName.toUpper() == tr("X3D"))
			result = vcg::tri::io::ImporterX3D<CMeshO>::LoadMask(filename.c_str(), info); 
		else
			result = vcg::tri::io::ImporterX3D<CMeshO>::LoadMaskVrml(filename.c_str(), info);
		if ( result != vcg::tri::io::ImporterX3D<CMeshO>::E_NOERROR)
		{
			QMessageBox::critical(parent, tr("X3D Opening Error"), errorMsgFormat.arg(fileName, info->filenameStack[info->filenameStack.size()-1], vcg::tri::io::ImporterX3D<CMeshO>::ErrorMsg(result)));
			delete info;
			return false;
		}
		if (info->mask & vcg::tri::io::Mask::IOM_VERTTEXCOORD)
		{
			info->mask |= vcg::tri::io::Mask::IOM_WEDGTEXCOORD;
			info->mask &=(~vcg::tri::io::Mask::IOM_VERTTEXCOORD);
		}
		if (info->mask & vcg::tri::io::Mask::IOM_WEDGCOLOR)
			info->mask &=(~vcg::tri::io::Mask::IOM_WEDGCOLOR);
		if (info->mask & vcg::tri::io::Mask::IOM_WEDGNORMAL)
			info->mask &=(~vcg::tri::io::Mask::IOM_WEDGNORMAL);
		m.Enable(info->mask);
				
		errorMsgFormat = "Error encountered while loading file:\n\"%1\"\n\nFile: %2\nLine number: %3\nError details: %4";
		result = vcg::tri::io::ImporterX3D<CMeshO>::Open(m.cm, filename.c_str(), info, cb);
		if (result != vcg::tri::io::ImporterX3D<CMeshO>::E_NOERROR)
		{
			QString fileError = info->filenameStack[info->filenameStack.size()-1];
			QString lineError;
			lineError.setNum(info->lineNumberError);
			QMessageBox::critical(parent, tr("X3D Opening Error"), errorMsgFormat.arg(fileName, fileError, lineError, vcg::tri::io::ImporterX3D<CMeshO>::ErrorMsg(result)));
			delete info;
			return false;
		}
		if (m.cm.vert.size() == 0)
		{
			errorMsgFormat = "Error encountered while loading file:\n\"%1\"\n\nError details: File without a geometry";
			QMessageBox::critical(parent, tr("X3D Opening Error"), errorMsgFormat.arg(fileName));
			delete info;
			return false;
		}
		if(info->mask & vcg::tri::io::Mask::IOM_WEDGNORMAL)
			normalsUpdated = true;
		mask = info->mask;

		QString missingTextureFilesMsg = "The following texture files were not found:\n";
		bool someTextureNotFound = false;
		for(unsigned int tx = 0; tx < info->textureFile.size(); ++tx)
		{
			FILE* pFile = fopen (info->textureFile[tx].toStdString().c_str(), "r");
			if (pFile == NULL)
			{
				missingTextureFilesMsg.append("\n");
				missingTextureFilesMsg.append(info->textureFile[tx].toStdString().c_str());
				someTextureNotFound = true;
			}
			else
			{
				m.cm.textures.push_back(info->textureFile[tx].toStdString());
				fclose (pFile);
			}
		}	
		
		
		if (someTextureNotFound)
			QMessageBox::warning(parent, tr("Missing texture files"), missingTextureFilesMsg);
			
		vcg::tri::UpdateBounding<CMeshO>::Box(m.cm);					// updates bounding box
		if (!normalsUpdated) 
			vcg::tri::UpdateNormal<CMeshO>::PerVertexPerFace(m.cm);		// updates normals

		delete info;
	}
	// verify if texture files are present
	
	if (cb != NULL)	(*cb)(99, "Done");
	return true;
}


bool IoX3DPlugin::save(const QString &formatName, const QString &fileName, MeshModel &m, const int mask, const RichParameterSet &, vcg::CallBackPos *cb, QWidget *parent)
{
	QString errorMsgFormat = "Error encountered while exportering file:\n%1\n\nError details: %2";
	string filename = QFile::encodeName(fileName).constData ();
	if(formatName.toUpper() == tr("X3D"))
	{
		int result = vcg::tri::io::ExporterX3D<CMeshO>::Save(m.cm, filename.c_str(), mask, cb);
		if(result!=0)
		{
			QMessageBox::warning(parent, tr("Saving Error"), errorMsgFormat.arg(fileName, vcg::tri::io::ExporterX3D<CMeshO>::ErrorMsg(result)));
			return false;
		}
		if (cb !=NULL) (*cb)(99, "Saving X3D File...");
		return true;
	}
	assert(0);
	return false;
}

/*
	returns the list of the file's type which can be imported
*/
QList<MeshIOInterface::Format> IoX3DPlugin::importFormats() const
{
	QList<Format> formatList;
	formatList << Format("X3D File Format - XML encoding", tr("X3D"));
	formatList << Format("X3D File Format - VRML encoding", tr("X3DV"));
	formatList << Format("VRML 2.0 File Format", tr("WRL"));
	return formatList;
}

/*
	returns the list of the file's type which can be exported
*/
QList<MeshIOInterface::Format> IoX3DPlugin::exportFormats() const
{
	QList<Format> formatList;
	formatList << Format("X3D File Format", tr("X3D"));
	return formatList;
}

/*
	returns the mask on the basis of the file's type. 
	otherwise it returns 0 if the file format is unknown
*/
void IoX3DPlugin::GetExportMaskCapability(QString &format, int &capability, int &defaultBits) const
{
	if(format.toUpper() == tr("X3D")){
		capability = defaultBits = vcg::tri::io::ExporterX3D<CMeshO>::GetExportMaskCapability();
		return; 
	}
	assert(0);
}
 
MESHLAB_PLUGIN_NAME_EXPORTER(IoX3DPlugin)
