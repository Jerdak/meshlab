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
#ifndef DECORATE_RASTER_PROJ_PLUGIN_H
#define DECORATE_RASTER_PROJ_PLUGIN_H




#include <QObject>
#include <common/interfaces.h>
#include <wrap/glw/glw.h>




class DecorateRasterProjPlugin : public QObject, public MeshDecorateInterface
{
    Q_OBJECT
		MESHLAB_PLUGIN_IID_EXPORTER(MESH_DECORATE_INTERFACE_IID)
    Q_INTERFACES( MeshDecorateInterface )


    // Types.
    enum { DP_PROJECT_RASTER };

    class MeshDrawer
    {
        glw::BufferHandle   m_VBOVertices;
        glw::BufferHandle   m_VBOIndices;
        MeshModel           *m_Mesh;

    public:
        inline              MeshDrawer() : m_Mesh(NULL)                 {}
        inline              MeshDrawer( MeshModel *mm ) : m_Mesh(mm)    {}

        void                update( glw::Context &context, bool useVBO );
        void                drawShadow( glw::Context &context );
        void                draw( glw::Context &context );

        inline MeshModel*   mm()                                        { return m_Mesh; }
    };


    // Class variables.
    static bool             s_AreVBOSupported;


    // Member variables.
    glw::Context            m_Context;
    bool                    m_ProjectOnAllMeshes;

    vcg::Box3f              m_SceneBox;
    QMap<int,MeshDrawer>    m_Scene;
    MeshDrawer              *m_CurrentMesh;

    RasterModel             *m_CurrentRaster;
    vcg::Matrix44f          m_RasterProj;
    vcg::Matrix44f          m_RasterPose;

    vcg::Matrix44f          m_ShadowProj;
    glw::Texture2DHandle    m_DepthTexture;
    glw::Texture2DHandle    m_ColorTexture;
    glw::ProgramHandle      m_ShadowMapShader;


    // Constructors / destructor.
public:
    DecorateRasterProjPlugin();
    ~DecorateRasterProjPlugin();


    // Member functions.
private:
    void                    updateCurrentMesh( MeshDocument &m,
                                               RichParameterSet &par );
    void                    updateCurrentRaster( MeshDocument &m );

    void                    setPointParameters( MeshDrawer &md,
                                                RichParameterSet *par );
    void                    drawScene();
    void                    drawSceneShadow();
    void                    updateShadowProjectionMatrix();
    void                    updateColorTexture();
    void                    updateDepthTexture();

    bool                    initShaders( std::string &logs );


    virtual QString         decorationInfo( FilterIDType filter ) const;
    virtual QString         decorationName( FilterIDType filter ) const;

public:
    inline QList<QAction*>  actions() const                             { return actionList; }

    bool  startDecorate(QAction  *act, MeshDocument &m, RichParameterSet *par, GLArea *gla );
    void  decorateMesh( QAction  *   , MeshModel    & , RichParameterSet *   , GLArea *   , QPainter * , GLLogStream &  ) {}
    void  decorateDoc(  QAction  *act, MeshDocument &m, RichParameterSet *par, GLArea *gla, QPainter *p, GLLogStream &  );
    void  endDecorate(  QAction  *act, MeshDocument &m, RichParameterSet *par, GLArea *gla );
    void  initGlobalParameterSet( QAction *act, RichParameterSet &par );
    int   getDecorationClass( QAction *act ) const;
};




#endif // DECORATE_RASTER_PROJ_PLUGIN_H
