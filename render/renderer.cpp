// Aqsis
// Copyright � 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


/** \file
		\brief Implements the base CqRenderer class which is the central core of the rendering main loop.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<strstream>

#include	<time.h>

#include	"aqsis.h"
#include	"imagebuffer.h"
#include	"lights.h"
#include	"renderer.h"
#include	"shaders.h"
#include	"nurbs.h"
#include	"render.h"
#include	"transform.h"
#include	"rifile.h"
#include	"texturemap.h"

#include	"shadervm.h"

START_NAMESPACE( Aqsis )

extern IqDDManager* CreateDisplayDriverManager();

CqRenderer* pCurrRenderer = 0;


CqOptions	goptDefault;					///< Default options.

//---------------------------------------------------------------------
/** Default constructor for the main renderer class. Initialises current state.
 */

CqRenderer::CqRenderer() :
		m_pImageBuffer( 0 ),
		m_Mode( RenderMode_Image ),
		m_fSaveGPrims( TqFalse )
{
	m_pconCurrent = 0;
	m_pImageBuffer = new	CqImageBuffer();

	// Initialise the array of coordinate systems.
	m_aCoordSystems.resize( CoordSystem_Last );

	m_aCoordSystems[ CoordSystem_Camera ]	.m_strName = "camera";
	m_aCoordSystems[ CoordSystem_Current ].m_strName = "current";
	m_aCoordSystems[ CoordSystem_World ]	.m_strName = "world";
	m_aCoordSystems[ CoordSystem_Screen ]	.m_strName = "screen";
	m_aCoordSystems[ CoordSystem_NDC ]	.m_strName = "NDC";
	m_aCoordSystems[ CoordSystem_Raster ]	.m_strName = "raster";

	m_pDDManager = CreateDisplayDriverManager();
	m_pDDManager->Initialise();
}

//---------------------------------------------------------------------
/** Destructor
 */

CqRenderer::~CqRenderer()
{
	// Delete the current context, should be main, unless render has been aborted.
	while ( m_pconCurrent != 0 )
	{
		CqContext * pconParent = m_pconCurrent->pconParent();
		delete( m_pconCurrent );
		m_pconCurrent = pconParent;
	}
	if ( m_pImageBuffer )
	{
		m_pImageBuffer->Release();
		m_pImageBuffer = 0;
	}
	FlushShaders();

	// Close down the Display device manager.
	m_pDDManager->Shutdown();
}


//---------------------------------------------------------------------
/** Create a new main context, called from within RiBegin(), error if not first
 * context created.  If first, create with this as the parent.
 */

CqContext*	CqRenderer::CreateMainContext()
{
	if ( m_pconCurrent == 0 )
	{
		m_pconCurrent = new CqMainContext();
		return ( m_pconCurrent );
	}
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Create a new Frame context, should only be called when the current
 * context is a Main context, but the internal context handling deals
 * with it so I don't need to worry.
 */

CqContext*	CqRenderer::CreateFrameContext()
{
	if ( m_pconCurrent != 0 )
	{
		CqContext * pconNew = m_pconCurrent->CreateFrameContext();
		if ( pconNew != 0 )
		{
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return ( 0 );
	}
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Create a new world context, again the internal context handling deals
 * with invalid calls.
 */

CqContext*	CqRenderer::CreateWorldContext()
{
	if ( m_pconCurrent != 0 )
	{
		CqContext * pconNew = m_pconCurrent->CreateWorldContext();
		if ( pconNew != 0 )
		{
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return ( 0 );
	}
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Create a new attribute context.
 */

CqContext*	CqRenderer::CreateAttributeContext()
{
	if ( m_pconCurrent != 0 )
	{
		CqContext * pconNew = m_pconCurrent->CreateAttributeContext();
		if ( pconNew != 0 )
		{
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return ( 0 );
	}
	else
		return ( 0 );
}



//---------------------------------------------------------------------
/** Create a new transform context.
 */

CqContext*	CqRenderer::CreateTransformContext()
{
	if ( m_pconCurrent != 0 )
	{
		CqContext * pconNew = m_pconCurrent->CreateTransformContext();
		if ( pconNew != 0 )
		{
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return ( 0 );
	}
	else
		return ( 0 );
}



//---------------------------------------------------------------------
/** Create a new solid context.
 */

CqContext*	CqRenderer::CreateSolidContext( CqString& type )
{
	if ( m_pconCurrent != 0 )
	{
		CqContext * pconNew = m_pconCurrent->CreateSolidContext( type );
		if ( pconNew != 0 )
		{
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return ( 0 );
	}
	else
		return ( 0 );
}



//---------------------------------------------------------------------
/** Create a new object context.
 */

CqContext*	CqRenderer::CreateObjectContext()
{
	if ( m_pconCurrent != 0 )
	{
		CqContext * pconNew = m_pconCurrent->CreateObjectContext();
		if ( pconNew != 0 )
		{
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return ( 0 );
	}
	else
		return ( 0 );
}



//---------------------------------------------------------------------
/** Create a new motion context.
 */

CqContext*	CqRenderer::CreateMotionContext( TqInt N, TqFloat times[] )
{
	if ( m_pconCurrent != 0 )
	{
		CqContext * pconNew = m_pconCurrent->CreateMotionContext( N, times );
		if ( pconNew != 0 )
		{
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return ( 0 );
	}
	else
		return ( 0 );
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a main context.
 */

void	CqRenderer::DeleteMainContext()
{
	if ( m_pconCurrent != 0 )
	{
		CqContext * pconParent = m_pconCurrent->pconParent();
		m_pconCurrent->DeleteMainContext();
		m_pconCurrent = pconParent;
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a frame context.
 */

void	CqRenderer::DeleteFrameContext()
{
	if ( m_pconCurrent != 0 )
	{
		CqContext * pconParent = m_pconCurrent->pconParent();
		m_pconCurrent->DeleteFrameContext();
		m_pconCurrent = pconParent;
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a world context.
 */

void	CqRenderer::DeleteWorldContext()
{
	if ( m_pconCurrent != 0 )
	{
		CqContext * pconParent = m_pconCurrent->pconParent();
		m_pconCurrent->DeleteWorldContext();
		m_pconCurrent = pconParent;
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a attribute context.
 */

void	CqRenderer::DeleteAttributeContext()
{
	if ( m_pconCurrent != 0 )
	{
		CqContext * pconParent = m_pconCurrent->pconParent();
		m_pconCurrent->DeleteAttributeContext();
		m_pconCurrent = pconParent;
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a transform context.
 */

void	CqRenderer::DeleteTransformContext()
{
	if ( m_pconCurrent != 0 )
	{
		CqContext * pconParent = m_pconCurrent->pconParent();
		// Copy the current state of the attributes UP the stack as a TransformBegin/End doesn't store them
		pconParent->m_pattrCurrent = m_pconCurrent->m_pattrCurrent;
		m_pconCurrent->DeleteTransformContext();
		m_pconCurrent = pconParent;
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a solid context.
 */

void	CqRenderer::DeleteSolidContext()
{
	if ( m_pconCurrent != 0 )
	{
		CqContext * pconParent = m_pconCurrent->pconParent();
		m_pconCurrent->DeleteSolidContext();
		m_pconCurrent = pconParent;
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a object context.
 */

void	CqRenderer::DeleteObjectContext()
{
	if ( m_pconCurrent != 0 )
	{
		CqContext * pconParent = m_pconCurrent->pconParent();
		m_pconCurrent->DeleteObjectContext();
		m_pconCurrent = pconParent;
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a motion context.
 */

void	CqRenderer::DeleteMotionContext()
{
	if ( m_pconCurrent != 0 )
	{
		CqContext * pconParent = m_pconCurrent->pconParent();
		// Copy the current state of the attributes UP the stack as a TransformBegin/End doesn't store them
		pconParent->m_pattrCurrent = m_pconCurrent->m_pattrCurrent;
		pconParent->m_ptransCurrent = m_pconCurrent->m_ptransCurrent;
		m_pconCurrent->DeleteMotionContext();
		m_pconCurrent = pconParent;
	}
}


//----------------------------------------------------------------------
/** Get the current shutter time, always returns 0.0 unless within a motion block,
 * when it returns the appropriate shutter time.
 */

TqFloat	CqRenderer::Time() const
{
	if ( m_pconCurrent != 0 )
		return ( m_pconCurrent->Time() );
	else
		return ( 0 );
}

//----------------------------------------------------------------------
/** Advance the current shutter time, only valid within motion blocks.
 */

void CqRenderer::AdvanceTime()
{
	if ( m_pconCurrent != 0 )
		m_pconCurrent->AdvanceTime();
}


//----------------------------------------------------------------------
/** Return a reference to the current options.
 */

CqOptions& CqRenderer::optCurrent() const
{
	if ( m_pconCurrent != 0 )
		return ( m_pconCurrent->optCurrent() );
	else
		return ( goptDefault );
}


//----------------------------------------------------------------------
/** Return a pointer to the current attributes.
 */

const CqAttributes* CqRenderer::pattrCurrent()
{
	if ( m_pconCurrent != 0 )
		return ( m_pconCurrent->pattrCurrent() );
	else
		return ( &m_attrDefault );
}


//----------------------------------------------------------------------
/** Return a writable pointer to the current attributes.
 */

CqAttributes* CqRenderer::pattrWriteCurrent()
{
	if ( m_pconCurrent != 0 )
		return ( m_pconCurrent->pattrWriteCurrent() );
	else
		return ( &m_attrDefault );
}


//----------------------------------------------------------------------
/** Return a pointer to the current transform.
 */

const CqTransform* CqRenderer::ptransCurrent()
{
	if ( m_pconCurrent != 0 )
		return ( m_pconCurrent->ptransCurrent() );
	else
		return ( &m_transDefault );
}


//----------------------------------------------------------------------
/** Return a writable pointer to the current transform.
 */

CqTransform* CqRenderer::ptransWriteCurrent()
{
	if ( m_pconCurrent != 0 )
		return ( m_pconCurrent->ptransWriteCurrent() );
	else
		return ( &m_transDefault );
}


//----------------------------------------------------------------------
/** Render all surface in the current list to the image buffer.
 */

void CqRenderer::RenderWorld()
{
	// Check we have a valid Image buffer
	if ( pImage() == 0 )
		SetImage( new CqImageBuffer );

	// Store the time at start.
	//	m_timeTaken=time(0);

	// Print to the defined output what we are rendering.
	//	CqString strMsg("Rendering - ");
	//	strMsg+=optCurrent().strDisplayName();
	//	CqBasicError(0,Severity_Normal,strMsg.c_str());

	m_pDDManager->OpenDisplays();

	pImage() ->RenderImage();

	// Calculate the time taken.
	//	m_timeTaken=time(0)-m_timeTaken;

	/* moved the following code into RiWorldEnd()
		TqInt verbosity=0;
		const TqInt* poptEndofframe=optCurrent().GetIntegerOption("statistics","endofframe");
		if(poptEndofframe!=0)
			verbosity=poptEndofframe[0];
	 
		Stats().PrintStats(verbosity);
		*/

	m_pDDManager->CloseDisplays();
}



//----------------------------------------------------------------------
/** Quit rendering at the next opportunity.
 */

void CqRenderer::Quit()
{
	if ( m_pImageBuffer )
	{
		// Ask the image buffer to quit.
		m_pImageBuffer->Quit();
	}
}


//----------------------------------------------------------------------
/** Initialise the renderer.
 */

void CqRenderer::Initialise()
{
	ClearSymbolTable();
	FlushShaders();

	// Initialise the matrices for this camera according to the
	// status of the camera attributes.
	optCurrent().InitialiseCamera();

	// Truncate the array of named coordinate systems to just the standard ones.
	m_aCoordSystems.resize( CoordSystem_Last );
}


//----------------------------------------------------------------------
/** Get the matrix to convert between the specified coordinate systems.
 */

CqMatrix	CqRenderer::matSpaceToSpace( const char* strFrom, const char* strTo, const CqMatrix& matShaderToWorld, const CqMatrix& matObjectToWorld )
{
	CqMatrix	matResult, matA, matB;
	// Get the two component matrices.
	// First check for special cases.
	if ( strcmp( strFrom, "object" ) == 0 ) matA = matObjectToWorld;
	else if ( strcmp( strFrom, "shader" ) == 0 ) matA = matShaderToWorld;
	if ( strcmp( strTo, "object" ) == 0 ) matB = matObjectToWorld.Inverse();
	else if ( strcmp( strTo, "shader" ) == 0 ) matB = matShaderToWorld.Inverse();
	TqInt i;
	for ( i = m_aCoordSystems.size() - 1; i >= 0; i-- )
	{
		if ( m_aCoordSystems[ i ].m_strName == strFrom ) matA = m_aCoordSystems[ i ].m_matToWorld;
		if ( m_aCoordSystems[ i ].m_strName == strTo ) matB = m_aCoordSystems[ i ].m_matWorldTo;
	}
	matResult = matB * matA;

	return ( matResult );
}


//----------------------------------------------------------------------
/** Get the matrix to convert vectors between the specified coordinate systems.
 */

CqMatrix	CqRenderer::matVSpaceToSpace( const char* strFrom, const char* strTo, const CqMatrix& matShaderToWorld, const CqMatrix& matObjectToWorld )
{
	CqMatrix	matResult, matA, matB;
	// Get the two component matrices.
	// First check for special cases.
	if ( strcmp( strFrom, "object" ) == 0 ) matA = matObjectToWorld;
	else if ( strcmp( strFrom, "shader" ) == 0 ) matA = matShaderToWorld;
	if ( strcmp( strTo, "object" ) == 0 ) matB = matObjectToWorld.Inverse();
	else if ( strcmp( strTo, "shader" ) == 0 ) matB = matShaderToWorld.Inverse();
	TqInt i;
	for ( i = m_aCoordSystems.size() - 1; i >= 0; i-- )
	{
		if ( m_aCoordSystems[ i ].m_strName == strFrom ) matA = m_aCoordSystems[ i ].m_matToWorld;
		if ( m_aCoordSystems[ i ].m_strName == strTo ) matB = m_aCoordSystems[ i ].m_matWorldTo;
	}
	matResult = matB * matA;

	matResult[ 3 ][ 0 ] = matResult[ 3 ][ 1 ] = matResult[ 3 ][ 2 ] = matResult[ 0 ][ 3 ] = matResult[ 1 ][ 3 ] = matResult[ 2 ][ 3 ] = 0.0;
	matResult[ 3 ][ 3 ] = 1.0;

	return ( matResult );
}


//----------------------------------------------------------------------
/** Get the matrix to convert normals between the specified coordinate systems.
 */

CqMatrix	CqRenderer::matNSpaceToSpace( const char* strFrom, const char* strTo, const CqMatrix& matShaderToWorld, const CqMatrix& matObjectToWorld )
{
	CqMatrix	matResult, matA, matB;
	// Get the two component matrices.
	// First check for special cases.
	if ( strcmp( strFrom, "object" ) == 0 ) matA = matObjectToWorld;
	else if ( strcmp( strFrom, "shader" ) == 0 ) matA = matShaderToWorld;
	if ( strcmp( strTo, "object" ) == 0 ) matB = matObjectToWorld.Inverse();
	else if ( strcmp( strTo, "shader" ) == 0 ) matB = matShaderToWorld.Inverse();
	TqInt i;
	for ( i = m_aCoordSystems.size() - 1; i >= 0; i-- )
	{
		if ( m_aCoordSystems[ i ].m_strName == strFrom ) matA = m_aCoordSystems[ i ].m_matToWorld;
		if ( m_aCoordSystems[ i ].m_strName == strTo ) matB = m_aCoordSystems[ i ].m_matWorldTo;
	}
	matResult = matB * matA;

	matResult[ 3 ][ 0 ] = matResult[ 3 ][ 1 ] = matResult[ 3 ][ 2 ] = matResult[ 0 ][ 3 ] = matResult[ 1 ][ 3 ] = matResult[ 2 ][ 3 ] = 0.0;
	matResult[ 3 ][ 3 ] = 1.0;
	matResult.Inverse();
	matResult.Transpose();

	return ( matResult );
}


const	TqFloat*	CqRenderer::GetFloatOption( const char* strName, const char* strParam ) const
{
	return(optCurrent().GetFloatOption( strName, strParam ) );
}

const	TqInt*		CqRenderer::GetIntegerOption( const char* strName, const char* strParam ) const
{
	return(optCurrent().GetIntegerOption( strName, strParam ) );
}

const	CqString*	CqRenderer::GetStringOption( const char* strName, const char* strParam ) const
{
	return(optCurrent().GetStringOption( strName, strParam ) );
}

const	CqVector3D*	CqRenderer::GetPointOption( const char* strName, const char* strParam ) const
{
	return(optCurrent().GetPointOption( strName, strParam ) );
}

const	CqColor*	CqRenderer::GetColorOption( const char* strName, const char* strParam ) const
{
	return(optCurrent().GetColorOption( strName, strParam ) );
}


TqFloat*			CqRenderer::GetFloatOptionWrite( const char* strName, const char* strParam )
{
	return(optCurrent().GetFloatOptionWrite( strName, strParam ) );
}

TqInt*				CqRenderer::GetIntegerOptionWrite( const char* strName, const char* strParam )
{
	return(optCurrent().GetIntegerOptionWrite( strName, strParam ) );
}

CqString*			CqRenderer::GetStringOptionWrite( const char* strName, const char* strParam )
{
	return(optCurrent().GetStringOptionWrite( strName, strParam ) );
}

CqVector3D*			CqRenderer::GetPointOptionWrite( const char* strName, const char* strParam )
{
	return(optCurrent().GetPointOptionWrite( strName, strParam ) );
}

CqColor*			CqRenderer::GetColorOptionWrite( const char* strName, const char* strParam )
{
	return(optCurrent().GetColorOptionWrite( strName, strParam ) );
}


//----------------------------------------------------------------------
/** Store the named coordinate system in the array of named coordinate systems, overwrite any existing
 * with the same name. Returns TqTrue if system already exists.
 */

TqBool	CqRenderer::SetCoordSystem( const char* strName, const CqMatrix& matToWorld )
{
	// Search for the same named system in the current list.
	for ( TqUint i = 0; i < m_aCoordSystems.size(); i++ )
	{
		if ( m_aCoordSystems[ i ].m_strName == strName )
		{
			m_aCoordSystems[ i ].m_matToWorld = matToWorld;
			m_aCoordSystems[ i ].m_matWorldTo = matToWorld.Inverse();
			return ( TqTrue );
		}
	}

	// If we got here, it didn't exists.
	m_aCoordSystems.push_back( SqCoordSys( strName, matToWorld, matToWorld.Inverse() ) );
	return ( TqFalse );
}


//----------------------------------------------------------------------
/** Find a parameter type declaration and return it.
 * \param strDecl Character pointer to the name of the declaration to find.
 */

SqParameterDeclaration CqRenderer::FindParameterDecl( const char* strDecl )
{
	TqInt Count = 1;
	CqString strName( "" );
	EqVariableType ILType = type_invalid;
	EqVariableClass ILClass = class_invalid;
	TqBool bArray = TqFalse;

	// First check if the declaration has embedded type information.
	CqString strLocalDecl( strDecl );
	TqInt i;
	for ( i = 0; i < gcVariableClassNames; i++ )
	{
		if ( strLocalDecl.find( gVariableClassNames[ i ] ) != CqString::npos )
		{
			ILClass = static_cast< EqVariableClass > ( i );
			break;
		}
	}

	/// \note Go backwards through the type names to make sure hpoint is matched before point.
	for ( i = gcVariableTypeNames-1; i >= 0; i-- )
	{
		if ( strLocalDecl.find( gVariableTypeNames[ i ] ) != CqString::npos )
		{
			ILType = static_cast< EqVariableType > ( i );
			break;
		}
	}

	// Now search for an array specifier.
	TqUint s, e;
	if ( ( s = strLocalDecl.find( '[' ) ) != CqString::npos )
	{
		if ( ( e = strLocalDecl.find( ']' ) ) != CqString::npos && e > s )
		{
			Count = static_cast<TqInt>( atoi( strLocalDecl.substr( s + 1, e - ( s + 1 ) ).c_str() ) );
			bArray = TqTrue ;
		}
	}

	// Copy the token to the name.
	s = strLocalDecl.find_last_of( ' ' );
	if ( s != CqString::npos ) strName = strLocalDecl.substr( s + 1 );
	else	strName = strLocalDecl;

	if ( ILType != type_invalid )
	{
		// Default to uniform if no class specified
		if ( ILClass == class_invalid )
			ILClass = class_uniform ;

		SqParameterDeclaration Decl;
		Decl.m_strName = strName;
		Decl.m_Count = Count;
		Decl.m_Type = ILType;
		Decl.m_Class = ILClass;
		Decl.m_strSpace = "";

		// Get the creation function.
		switch ( ILClass )
		{
				case class_constant:
				{
					if ( bArray )
						Decl.m_pCreate = gVariableCreateFuncsConstantArray[ ILType ];
					else
						Decl.m_pCreate = gVariableCreateFuncsConstant[ ILType ];
				}
				break;

				case class_uniform:
				{
					if ( bArray )
						Decl.m_pCreate = gVariableCreateFuncsUniformArray[ ILType ];
					else
						Decl.m_pCreate = gVariableCreateFuncsUniform[ ILType ];
				}
				break;

				case class_varying:
				{
					if ( bArray )
						Decl.m_pCreate = gVariableCreateFuncsVaryingArray[ ILType ];
					else
						Decl.m_pCreate = gVariableCreateFuncsVarying[ ILType ];
				}
				break;

				case class_vertex:
				{
					if ( bArray )
						Decl.m_pCreate = gVariableCreateFuncsVertexArray[ ILType ];
					else
						Decl.m_pCreate = gVariableCreateFuncsVertex[ ILType ];
				}
				break;

				case class_facevarying:
				{
					if ( bArray )
						Decl.m_pCreate = gVariableCreateFuncsFaceVaryingArray[ ILType ];
					else
						Decl.m_pCreate = gVariableCreateFuncsFaceVarying[ ILType ];
				}
				break;
		}
		return ( Decl );
	}

	strName = strDecl;
	// Search the local parameter declaration list.
	std::vector<SqParameterDeclaration>::const_iterator is;
	for ( is = m_Symbols.begin(); is != m_Symbols.end(); is++ )
	{
		if ( strName == is->m_strName )
			return ( *is );
	}
	return ( SqParameterDeclaration( "", type_invalid, class_invalid, 0, 0, "" ) );
}


//----------------------------------------------------------------------
/** Add a parameter type declaration to the local declarations.
 * \param strName Character pointer to parameter name.
 * \param strType Character pointer to string containing the type identifier.
 */

void CqRenderer::AddParameterDecl( const char* strName, const char* strType )
{
	CqString strDecl( strType );
	strDecl += " ";
	strDecl += strName;
	SqParameterDeclaration Decl = FindParameterDecl( strDecl.c_str() );

	// Put new declaration at the top to make it take priority over pervious
	m_Symbols.insert( m_Symbols.begin(), Decl );
}


//---------------------------------------------------------------------
/** Register a shader of the specified type with the specified name.
 */

void CqRenderer::RegisterShader( const char* strName, EqShaderType type, IqShader* pShader )
{
	assert( pShader );
	m_Shaders.LinkLast( new CqShaderRegister( strName, type, pShader ) );
}


//---------------------------------------------------------------------
/** Find a shader of the specified type with the specified name.
 */

CqShaderRegister* CqRenderer::FindShader( const char* strName, EqShaderType type )
{
	// Search the register list.
	CqShaderRegister * pShaderRegister = m_Shaders.pFirst();
	while ( pShaderRegister )
	{
		if ( pShaderRegister->strName() == strName && pShaderRegister->Type() == type )
			return ( pShaderRegister );

		pShaderRegister = pShaderRegister->pNext();
	}
	return ( 0 );
}


//---------------------------------------------------------------------
/** Find a shader of the specified type with the specified name.
 * If not found, try and load one.
 */

IqShader* CqRenderer::CreateShader( const char* strName, EqShaderType type )
{
	CqShaderRegister * pReg = FindShader( strName, type );
	if ( pReg != 0 )
	{
		IqShader * pShader = pReg->Create();
		RegisterShader( strName, type, pShader );
		return ( pShader );
	}
	else
	{
		// Search in the current directory first.
		CqString strFilename( strName );
		strFilename += RI_SHADER_EXTENSION;
		CqRiFile SLXFile( strFilename.c_str(), "shader" );
		if ( SLXFile.IsValid() )
		{
			CqShaderVM * pShader = new CqShaderVM();
			pShader->LoadProgram( SLXFile );
			pShader->SetstrName( strName );
			RegisterShader( strName, type, pShader );
			return ( pShader );
		}
		else
		{
			if( strcmp(strName, "null" ) != 0 )
			{
				CqString strError( "Shader \"" );
				strError += strName;
				strError += "\" not found";
				//strError.Format("Shader \"%s\" not found",strName.String());
				CqBasicError( ErrorID_FileNotFound, Severity_Normal, strError.c_str() );
			}
			CqShaderVM * pShader = new CqShaderVM();
			pShader->SetstrName( "null" );
			RegisterShader( strName, type, pShader );
			return ( pShader );
		}
	}
}



//---------------------------------------------------------------------
/** Add a new requested display driver to the list.
 */

void CqRenderer::AddDisplayRequest( const TqChar* name, const TqChar* type, const TqChar* mode, TqInt compression, TqInt quality )
{
	m_pDDManager->AddDisplay( name, type, mode, compression, quality);
}



//---------------------------------------------------------------------
/** Clear the list of requested display drivers.
 */

void CqRenderer::ClearDisplayRequests()
{
	m_pDDManager->ClearDisplays();
}


void QSetRenderContext( CqRenderer* pRend )
{
	pCurrRenderer = pRend;
}

IqRenderer* QGetRenderContextI()
{
	return ( pCurrRenderer );
}


IqTextureMap* CqRenderer::GetTextureMap(const CqString& strFileName)
{
	return( CqTextureMap::GetTextureMap( strFileName ) );
}

IqTextureMap* CqRenderer::GetEnvironmentMap(const CqString& strFileName)
{
	return( CqTextureMap::GetEnvironmentMap( strFileName ) );
}

IqTextureMap* CqRenderer::GetShadowMap(const CqString& strFileName)
{
	return( CqTextureMap::GetShadowMap( strFileName ) );
}

IqTextureMap* CqRenderer::GetLatLongMap(const CqString& strFileName)
{
	return( CqTextureMap::GetLatLongMap( strFileName ) );
}


TqBool	CqRenderer::GetBasisMatrix( CqMatrix& matBasis, const CqString& name)
{
	RtBasis basis;
	if( BasisFromName( &basis, name.c_str() ) )
	{
		matBasis = basis;
		return(TqTrue);
	}
	else
		return(TqFalse);
} 

//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )
