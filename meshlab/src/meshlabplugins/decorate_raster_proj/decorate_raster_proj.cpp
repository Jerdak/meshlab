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

#include "decorate_raster_proj.h"
#include <wrap/gl/shot.h>
#include <common/pluginmanager.h>
#include <meshlab/glarea.h>
#include <vcg/math/matrix44.h>




void DecorateRasterProjPlugin::MeshDrawer::drawShadow( glw::Context &context )
{
    if( !m_Mesh->visible )
        return;

    if( m_VBOVertices.isNull() )
        m_Mesh->render( vcg::GLW::DMFlat, vcg::GLW::CMNone, vcg::GLW::TMNone );
    else
    {
        glPushAttrib( GL_TRANSFORM_BIT );
        glMatrixMode( GL_MODELVIEW );
        glPushMatrix();
        glMultMatrix(m_Mesh->cm.Tr);
        glPushClientAttrib( GL_CLIENT_VERTEX_ARRAY_BIT );
		glEnableClientState( GL_VERTEX_ARRAY );

        context.bindVertexBuffer( m_VBOVertices );
		glVertexPointer( 3, GL_FLOAT, 2*sizeof(vcg::Point3f), 0 );

        context.bindIndexBuffer( m_VBOIndices );
		glDrawElements( GL_TRIANGLES, 3*m_Mesh->cm.fn, GL_UNSIGNED_INT, 0 );

        context.unbindIndexBuffer();
		context.unbindVertexBuffer();

        glPopClientAttrib();
        glPopMatrix();
        glPopAttrib();
    }
}


void DecorateRasterProjPlugin::MeshDrawer::draw( glw::Context &context )
{
    if( !m_Mesh->visible )
        return;

    if( m_VBOVertices.isNull() )
        m_Mesh->render( vcg::GLW::DMSmooth, vcg::GLW::CMNone, vcg::GLW::TMNone );
    else
    {
        glPushAttrib( GL_TRANSFORM_BIT );
        glMatrixMode( GL_MODELVIEW );
        glPushMatrix();
        glMultMatrix(m_Mesh->cm.Tr);
        glPushClientAttrib( GL_CLIENT_VERTEX_ARRAY_BIT );
		glEnableClientState( GL_VERTEX_ARRAY );
        glEnableClientState( GL_NORMAL_ARRAY );

        context.bindVertexBuffer( m_VBOVertices );
		glVertexPointer( 3, GL_FLOAT, 2*sizeof(vcg::Point3f), 0 );
        glNormalPointer(    GL_FLOAT, 2*sizeof(vcg::Point3f), (GLvoid*)sizeof(vcg::Point3f) );

        context.bindIndexBuffer( m_VBOIndices );
		glDrawElements( GL_TRIANGLES, 3*m_Mesh->cm.fn, GL_UNSIGNED_INT, 0 );

        context.unbindIndexBuffer();
		context.unbindVertexBuffer();

        glPopClientAttrib();
        glPopMatrix();
        glPopAttrib();
    }
}


void DecorateRasterProjPlugin::MeshDrawer::update( glw::Context &context, bool useVBO )
{
    // Initialize the VBO if required.
    if( useVBO && m_Mesh->visible )
    {
        if( m_VBOVertices.isNull() )
        {
            // Transfer of vertex positions on GPU.
            CMeshO &meshData = m_Mesh->cm;
            vcg::Point3f *vertBuffer = new vcg::Point3f [ 2*meshData.vn ];
            for( int i=0, n=0; i<meshData.vn; ++i )
            {
                vertBuffer[n++] = meshData.vert[i].P();
                vertBuffer[n++] = meshData.vert[i].N();
            }

            m_VBOVertices = glw::createBuffer( context, 2*meshData.vn*sizeof(vcg::Point3f), vertBuffer );
            delete [] vertBuffer;

            // Transfer of face indices on GPU.
            unsigned int *indexBuffer = new unsigned int [ 3*meshData.fn ];
            for( int i=0, n=0; i<meshData.fn; ++i )
            {
                indexBuffer[n++] = meshData.face[i].V(0) - &meshData.vert[0];
                indexBuffer[n++] = meshData.face[i].V(1) - &meshData.vert[0];
                indexBuffer[n++] = meshData.face[i].V(2) - &meshData.vert[0];
            }

            m_VBOIndices = glw::createBuffer( context, 3*meshData.fn*sizeof(unsigned int), indexBuffer );
            delete [] indexBuffer;
        }
    }
    else
    {
        m_VBOIndices.setNull();
        m_VBOVertices.setNull();
    }
}




bool DecorateRasterProjPlugin::s_AreVBOSupported;

    
DecorateRasterProjPlugin::DecorateRasterProjPlugin() :
    m_CurrentRaster(NULL),
    m_CurrentMesh(NULL)
{
    typeList << DP_PROJECT_RASTER;

    foreach( FilterIDType tt, types() )
        actionList << new QAction(decorationName(tt), this);

    foreach( QAction *ap, actionList )
        ap->setCheckable(true);
}

        
DecorateRasterProjPlugin::~DecorateRasterProjPlugin()
{
}


QString DecorateRasterProjPlugin::decorationInfo( FilterIDType id ) const
{
    switch( id )
    {
        case DP_PROJECT_RASTER: return tr("Project the current raster onto the 3D mesh");
        default: assert(0); return QString();
    }  
}


QString DecorateRasterProjPlugin::decorationName( FilterIDType id ) const
{
    switch( id )
    {
        case DP_PROJECT_RASTER: return tr("Rasters-to-geometry reprojection");
        default: assert(0); return QString();
    }
}


int DecorateRasterProjPlugin::getDecorationClass( QAction *act ) const
{
    switch( ID(act) )
    {
        case DP_PROJECT_RASTER: return PerDocument;
        default: assert(0); return Generic;
    }
}


void DecorateRasterProjPlugin::initGlobalParameterSet( QAction *act, RichParameterSet &par )
{
    switch( ID(act) )
    {
        case DP_PROJECT_RASTER:
        {
            par.addParam( new RichDynamicFloat( "MeshLab::Decoration::ProjRasterAlpha",
                                                1.0f,
                                                0.0f,
                                                1.0f,
                                                "Transparency",
                                                "Transparency" ) );

            par.addParam( new RichBool( "MeshLab::Decoration::ProjRasterLighting",
                                        true,
                                        "Apply lighting",
                                        "Apply lighting" ) );

            par.addParam( new RichBool( "MeshLab::Decoration::ProjRasterUseVBO",
                                        false,
                                        "Use VBO",
                                        "Use VBO" ) );

            par.addParam( new RichBool( "MeshLab::Decoration::ProjRasterOnAllMeshes",
                                        false,
                                        "Project on all meshes",
                                        "Project the current raster on all meshes instead of only on the current one" ) );
            break;
        }
        default: assert(0);
    }
}		


void DecorateRasterProjPlugin::updateCurrentMesh( MeshDocument &m,
                                                  RichParameterSet &par )
{
    if( par.getBool("MeshLab::Decoration::ProjRasterOnAllMeshes") )
    {
        QMap<int,MeshDrawer> tmpScene = m_Scene;
        m_Scene.clear();

        foreach( MeshModel *md, m.meshList )
        {
            QMap<int,MeshDrawer>::iterator t = tmpScene.find( md->id() );
            if( t != tmpScene.end() )
                m_Scene[ t.key() ] = t.value();
            else
                m_Scene[ md->id() ] = MeshDrawer( md );
        }
    }
    else
    {
        if( m_CurrentMesh && m.mm()==m_CurrentMesh->mm() )
            return;

        m_Scene.clear();
        m_CurrentMesh = &( m_Scene[m.mm()->id()] = MeshDrawer(m.mm()) );
    }


    bool areVBORequired = par.getBool( "MeshLab::Decoration::ProjRasterUseVBO" );
    if( areVBORequired && !s_AreVBOSupported )
    {
        par.setValue( "MeshLab::Decoration::ProjRasterUseVBO", BoolValue(false) );
        areVBORequired = false;
    }

    m_SceneBox.SetNull();
    for( QMap<int,MeshDrawer>::iterator m=m_Scene.begin(); m!=m_Scene.end(); ++m )
    {
        m_SceneBox.Add( m->mm()->cm.Tr, m->mm()->cm.bbox );
        m->update( m_Context, areVBORequired );
    }
}


void DecorateRasterProjPlugin::updateShadowProjectionMatrix()
{
    // Recover the near and far clipping planes by considering the bounding box of the current mesh
    // in the camera space of the current raster.
    float zNear, zFar;
    GlShot< vcg::Shot<float> >::GetNearFarPlanes( m_CurrentRaster->shot, m_SceneBox, zNear, zFar );
    if( zNear < 0.0001f )
        zNear = 0.1f;
    if( zFar < zNear )
        zFar = zNear + 1000.0f;


    // Recover the view frustum of the current raster.
    float l, r, b, t, focal;
    m_CurrentRaster->shot.Intrinsics.GetFrustum( l, r, b, t, focal );


    // Compute from the frustum values the camera projection matrix.
    const float normFactor = zNear / focal;
    l *= normFactor;
    r *= normFactor;
    b *= normFactor;
    t *= normFactor;

    m_RasterProj.SetZero();
    m_RasterProj[0][0] = 2.0f*zNear / (r-l);
    m_RasterProj[2][0] = (r+l) / (r-l);
    m_RasterProj[1][1] = 2.0f*zNear / (t-b);
    m_RasterProj[2][1] = (t+b) / (t-b);
    m_RasterProj[2][2] = (zNear+zFar) / (zNear-zFar);
    m_RasterProj[3][2] = 2.0f*zNear*zFar / (zNear-zFar);
    m_RasterProj[2][3] = -1.0f;


    // Extract the pose matrix from the current raster.
    m_RasterPose =  m_CurrentRaster->shot.GetWorldToExtrinsicsMatrix().transpose() ;


    // Define the bias matrix that will enable to go from clipping space to texture space.
    const float biasMatData[16] = { 0.5f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 0.5f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.5f, 0.0f,
                                    0.5f, 0.5f, 0.5f, 1.0f };
    vcg::Matrix44f biasMat( biasMatData );


    // Update the shadow map projection matrix.
    m_ShadowProj = m_RasterPose * m_RasterProj * biasMat;
}


void DecorateRasterProjPlugin::updateColorTexture()
{
    glPushAttrib( GL_TEXTURE_BIT );

    const int w = m_CurrentRaster->currentPlane->image.width();
    const int h = m_CurrentRaster->currentPlane->image.height();


    // Recover image data and convert pixels to the adequate format for transfer onto the GPU.
    GLubyte *texData = new GLubyte [ 3*w*h ];
    for( int y=h-1, n=0; y>=0; --y )
        for( int x=0; x<w; ++x )
        {
            QRgb pixel = m_CurrentRaster->currentPlane->image.pixel(x,y);
            texData[n++] = (GLubyte) qRed  ( pixel );
            texData[n++] = (GLubyte) qGreen( pixel );
            texData[n++] = (GLubyte) qBlue ( pixel );
        }


    // Create and initialize the OpenGL texture object.
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    m_ColorTexture = glw::createTexture2D( m_Context, GL_RGB, w, h, GL_RGB, GL_UNSIGNED_BYTE, texData );
    delete [] texData;

    glw::BoundTexture2DHandle t = m_Context.bindTexture2D( m_ColorTexture, 0 );
    t->setSampleMode( glw::TextureSampleMode(GL_LINEAR,GL_LINEAR,GL_REPEAT,GL_REPEAT,GL_REPEAT) );
    m_Context.unbindTexture2D( 0 );

    glPopAttrib();
}


void DecorateRasterProjPlugin::updateDepthTexture()
{
    glPushAttrib( GL_TEXTURE_BIT   |
                  GL_ENABLE_BIT    |
                  GL_POLYGON_BIT   |
                  GL_CURRENT_BIT   |
                  GL_TRANSFORM_BIT |
                  GL_VIEWPORT_BIT  );

    const int w = m_CurrentRaster->currentPlane->image.width();
    const int h = m_CurrentRaster->currentPlane->image.height();


    // Create and initialize the OpenGL texture object used to store the shadow map.
    m_DepthTexture = glw::createTexture2D( m_Context, GL_DEPTH_COMPONENT, w, h, GL_DEPTH_COMPONENT, GL_INT, NULL );
    glw::BoundTexture2DHandle t = m_Context.bindTexture2D( m_DepthTexture, 0 );
    t->setSampleMode( glw::TextureSampleMode(GL_LINEAR,GL_LINEAR,GL_REPEAT,GL_REPEAT,GL_REPEAT) );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL );
    glTexParameteri( GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_INTENSITY );
    m_Context.unbindTexture2D( 0 );


    // Perform an off-screen rendering pass so as to generate the a depth map of the model
    // from the viewpoint of the current raster's camera.
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadMatrixf( (GLfloat*) m_RasterProj.V() );

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadMatrixf( (GLfloat*) m_RasterPose.V() );



    glw::FramebufferHandle fbuffer = glw::createFramebuffer( m_Context, glw::texture2DTarget(m_DepthTexture) );
    m_Context.bindReadDrawFramebuffer( fbuffer );

    glViewport( 0, 0, m_DepthTexture->width(), m_DepthTexture->height() );

    glEnable( GL_DEPTH_TEST );
    glEnable( GL_POLYGON_OFFSET_FILL );
    glPolygonOffset( 2.0f, 2.0f );

    glClear( GL_DEPTH_BUFFER_BIT );
    for( QMap<int,MeshDrawer>::iterator m=m_Scene.begin(); m!=m_Scene.end(); ++m )
        m->drawShadow( m_Context );

    m_Context.unbindReadDrawFramebuffer();



    glPopMatrix();
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();

    glPopAttrib();
}


void DecorateRasterProjPlugin::updateCurrentRaster( MeshDocument &m )
{
    // Update the stored raster with the one provided by the mesh document.
    // If both are identical, the update is simply skiped.
    if( m.rm() == m_CurrentRaster )
        return;

    m_CurrentRaster = m.rm();

    updateColorTexture();
    updateShadowProjectionMatrix();
    updateDepthTexture();
}


bool DecorateRasterProjPlugin::initShaders( std::string &logs )
{
    const std::string vertSrc = GLW_STRINGIFY
    (
        varying vec4    v_ProjVert;
        varying vec3    v_Normal;
        varying vec3    v_RasterView;
        varying vec3    v_Light;

        uniform mat4    u_ProjMat;
        uniform vec3    u_Viewpoint;
        uniform mat4    u_LightToObj;
        uniform mat4    u_ModelXf;

        void main()
        {
            gl_Position  = ftransform();
            v_ProjVert = u_ProjMat * u_ModelXf * gl_Vertex;
            v_Normal = (u_ModelXf*vec4(gl_Normal,1.0)).xyz;
            v_RasterView = u_Viewpoint - (u_ModelXf*gl_Vertex).xyz;
            v_Light = u_LightToObj[2].xyz;
    
            float d = length( gl_ModelViewMatrix * gl_Vertex );
            float distAtten = 1.0 / (gl_Point.distanceConstantAttenuation      +
                                     gl_Point.distanceLinearAttenuation*d      +
                                     gl_Point.distanceQuadraticAttenuation*d*d );
            gl_PointSize = clamp( gl_Point.size*sqrt(distAtten) + 0.5, gl_Point.sizeMin, gl_Point.sizeMax );
        }
    );

    const std::string fragSrc = GLW_STRINGIFY
    (
        varying vec4            v_ProjVert;
        varying vec3            v_Normal;
        varying vec3            v_RasterView;
        varying vec3            v_Light;

        uniform sampler2DShadow u_ColorMap;
        uniform sampler2DShadow u_DepthMap;
        uniform bool            u_IsLightActivated;
        uniform float           u_AlphaValue;

        void main()
        {
            if( dot(v_Normal,v_RasterView) <= 0.0 )
                discard;

            vec2 clipCoord = v_ProjVert.xy / v_ProjVert.w;
            if( clipCoord.x<0.0 || clipCoord.x>1.0 ||
                clipCoord.y<0.0 || clipCoord.y>1.0 )
                discard;
        
            float visibility = shadow2DProj( u_DepthMap, v_ProjVert ).r;
            if( visibility <= 0.001 )
                discard;

            vec4 color = shadow2DProj( u_ColorMap, v_ProjVert );

            if( u_IsLightActivated )
            {
                vec4 Ka = gl_LightModel.ambient * gl_FrontLightProduct[0].ambient;

                vec3 L = normalize( v_Light );
                vec3 N = normalize( v_Normal );
                float Kd = max( dot(L,N), 0.0 );
        
                color = Ka + gl_FrontMaterial.emission + Kd*gl_FrontLightProduct[0].diffuse*color;
            }

            gl_FragColor = vec4( color.xyz, u_AlphaValue );
        }
    );

	m_ShadowMapShader = glw::createProgram( m_Context, "", vertSrc, fragSrc );
    return m_ShadowMapShader->isLinked();
}


bool DecorateRasterProjPlugin::startDecorate( QAction          *act,
                                              MeshDocument     & /*m*/,
                                              RichParameterSet * /*par*/,
                                              GLArea           * /*gla*/ )
{
    switch( ID(act) )
    {
        case DP_PROJECT_RASTER:
        {
            glPushAttrib( GL_ALL_ATTRIB_BITS );

            GLenum err = glewInit();
            if( err != GLEW_OK )
            {
                qWarning( (std::string("Impossible to load GLEW library.")+(char*)glewGetErrorString(err)).c_str() );
                return false;
            }

            m_Context.acquire();

            std::string logs;
            if( !initShaders(logs) )
            {
                qWarning( ("Error while initializing shaders.\n"+logs).c_str() );
                return false;
            }

            s_AreVBOSupported = glewIsSupported( "GL_ARB_vertex_buffer_object" );

            m_Scene.clear();
            m_CurrentMesh = NULL;
            m_CurrentRaster = NULL;

            glPopAttrib();
            return true;
        }
        default: assert( 0 );
    }

    return false;
}


void DecorateRasterProjPlugin::endDecorate( QAction          *act,
                                            MeshDocument     & /*m*/,
                                            RichParameterSet * /*par*/,
                                            GLArea           * /*gla*/ )
{
    switch( ID(act) )
    {
        case DP_PROJECT_RASTER:
        {
            glPushAttrib( GL_ALL_ATTRIB_BITS );

            m_Scene.clear();
            m_CurrentMesh = NULL;
            m_CurrentRaster = NULL;
            m_ShadowMapShader.setNull();
            m_DepthTexture.setNull();
            m_ColorTexture.setNull();

            glPopAttrib();
            break;
        }
        default: assert( 0 );
    }
}


void DecorateRasterProjPlugin::setPointParameters( MeshDrawer &md,
                                                   RichParameterSet *par )
{
    if( par->getBool("MeshLab::Appearance::pointSmooth") )
        glEnable( GL_POINT_SMOOTH );
    else
        glDisable( GL_POINT_SMOOTH );
    
    glPointSize( par->getFloat("MeshLab::Appearance::pointSize") );

    if( glPointParameterfv )
    {
        if( par->getBool("MeshLab::Appearance::pointDistanceAttenuation") )
        {
            vcg::Matrix44f mvMat;
            glGetFloatv( GL_MODELVIEW_MATRIX, mvMat.V() );
            vcg::Transpose( mvMat );
            float camDist = vcg::Norm( mvMat * md.mm()->cm.Tr * md.mm()->cm.bbox.Center() );

            float quadratic[3] = { 0.0f, 0.0f, 1.0f/(camDist*camDist) };
            glPointParameterfv( GL_POINT_DISTANCE_ATTENUATION, quadratic );
            glPointParameterf( GL_POINT_SIZE_MAX, 16.0f );
            glPointParameterf( GL_POINT_SIZE_MIN, 1.0f );
        }
        else
        {
            float quadratic[3] = { 1.0f, 0.0f, 0.0f };
            glPointParameterfv( GL_POINT_DISTANCE_ATTENUATION, quadratic );
        }
    }
}


void DecorateRasterProjPlugin::decorateDoc( QAction           *act,
                                         MeshDocument      &m  ,
                                         RichParameterSet  *par,
                                         GLArea            *gla,
                                         QPainter          *,
                                            GLLogStream &)
{
    switch( ID(act) )
    {
        case DP_PROJECT_RASTER:
        {
            glPushAttrib( GL_ALL_ATTRIB_BITS );

            updateCurrentMesh( m, *par );
            updateCurrentRaster( m );

            glEnable( GL_DEPTH_TEST );

            RenderMode rm = gla->getCurrentRenderMode();
            bool notDrawn = false;
            switch( rm.drawMode )
            {
                case vcg::GLW::DMPoints:
                {
                    glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
                    glEnable( GL_POLYGON_OFFSET_POINT );
                    break;
                }
                case vcg::GLW::DMHidden:
                case vcg::GLW::DMWire:
                {
                    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
                    glEnable( GL_POLYGON_OFFSET_LINE );
                    break;
                }
                case vcg::GLW::DMFlat:
                case vcg::GLW::DMFlatWire:
                case vcg::GLW::DMSmooth:
                {
                    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
                    glEnable( GL_POLYGON_OFFSET_FILL );
                    break;
                }
                default: notDrawn = true;
            }

            if( !notDrawn )
            {
                glEnable( GL_BLEND );
                glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
                glPolygonOffset( -2.0f, 1.0f );
                glEnable( GL_COLOR_MATERIAL );
                glColor3ub( 255, 255, 255 );

                glEnable( GL_PROGRAM_POINT_SIZE );
                glw::BoundProgramHandle shader = m_Context.bindProgram( m_ShadowMapShader );
                m_Context.bindTexture2D( m_ColorTexture, 0 );
                m_Context.bindTexture2D( m_DepthTexture, 1 );
                shader->setUniform( "u_ColorMap", 0 );
                shader->setUniform( "u_DepthMap", 1 );
                shader->setUniform4x4( "u_ProjMat", m_ShadowProj.V(), false );
                shader->setUniform3( "u_Viewpoint", m_CurrentRaster->shot.GetViewPoint().V() );
                vcg::Matrix44f lightToObj = ( gla->trackball.InverseMatrix() * gla->trackball_light.Matrix() ).transpose();
                shader->setUniform4x4( "u_LightToObj", lightToObj.V(), false );
                shader->setUniform( "u_IsLightActivated", rm.lighting && par->getBool("MeshLab::Decoration::ProjRasterLighting") );
                shader->setUniform( "u_AlphaValue", par->getFloat("MeshLab::Decoration::ProjRasterAlpha") );

                for( QMap<int,MeshDrawer>::iterator m=m_Scene.begin(); m!=m_Scene.end(); ++m )
                {
                    if( rm.drawMode == vcg::GLW::DMPoints )
                        setPointParameters( m.value(), par );
                    shader->setUniform4x4( "u_ModelXf", vcg::Matrix44f(m->mm()->cm.Tr).transpose().V(), false );
                    m->draw( m_Context );
                }

                m_Context.unbindProgram();
                m_Context.unbindTexture2D( 0 );
                m_Context.unbindTexture2D( 1 );
            }

            glPopAttrib();
            break;
        }
        default: assert(0);
    }
}




MESHLAB_PLUGIN_NAME_EXPORTER(DecorateRasterProjPlugin)
