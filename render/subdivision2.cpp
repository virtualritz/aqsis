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
		\brief Implements the classes for subdivision surfaces.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"subdivision2.h"
#include	"patch.h"

#include	<fstream>
#include	<vector>

START_NAMESPACE( Aqsis )


//------------------------------------------------------------------------------
/**
 *	Constructor.
 */

CqSubdivision2::CqSubdivision2( CqPolygonPoints* pPoints ) : m_fFinalised(TqFalse), m_bInterpolateBoundary( TqFalse ),
	CqMotionSpec<CqPolygonPoints*>(pPoints)
{
	if( pPoints )
	{
		// Store the reference to our points.
		pPoints->AddRef();
		AddTimeSlot( 0, pPoints );
	}
}


//------------------------------------------------------------------------------
/**
 *	Destructor.
 */

CqSubdivision2::~CqSubdivision2()
{
	// Delete the array of laths generated during the facet adding phase.
	for(std::vector<CqLath*>::const_iterator iLath=apLaths().begin(); iLath!=apLaths().end(); iLath++)
		if(*iLath)	delete(*iLath);

	// Release the reference to our points.
	TqInt i;
	for ( i = 0; i < cTimes(); i++ )
		GetMotionObject( Time( i ) ) ->Release();
}


//------------------------------------------------------------------------------
/**
 *	Get a pointer to a lath referencing the specified facet index.
 *	The returned lath pointer can be any lath on the edge of the facet.
 *	Asserts if the facet index is invalid.
 *
 *	@param	iIndex	Index of the facet to query.
 *
 *	@return			Pointer to a lath on the facet.
 */
CqLath* CqSubdivision2::pFacet(TqInt iIndex)
{
	assert(iIndex < m_apFacets.size());
	return(m_apFacets[iIndex]);
}


//------------------------------------------------------------------------------
/**
 *	Get a pointer to a lath which references the specified vertex index.
 *	The returned lath pointer can be any lath which references the vertex.
 *	Asserts if the vertex index is invalid.
 *
 *	@param	iIndex	Index of the vertex to query.
 *
 *	@return			Pointer to a lath on the vertex.
 */
CqLath* CqSubdivision2::pVertex(TqInt iIndex)
{
	assert(iIndex < m_aapVertices.size() && m_aapVertices[iIndex].size() >= 1);
	return(m_aapVertices[iIndex][0]);
}


//------------------------------------------------------------------------------
/**
 *	Initialise the topology class to store the specified number of vertices.
 *	Use this function to prepare the topology structure to receive a number of
 *	vertices then use SetVertex to initialise them.
 *
 *	@param	cVerts	Then number of vertices that will be needed.
 */
void CqSubdivision2::Prepare(TqInt cVerts)
{
	// Initialise the array of vertex indexes to the appropriate size.
	m_aapVertices.resize(cVerts);

	m_fFinalised=TqFalse;
}


//------------------------------------------------------------------------------
/**
 *	Add a completely new vertex to the list.
 *	Appends a new vertex to the end of the list, updating the referencing
 *	table as well.
 *
 *	@return			The index of the new point.
 */
TqInt CqSubdivision2::AddVertex(CqLath* pVertex)
{
	TqInt iIndex=0;

	std::vector<CqParameter*>::iterator iUP;
	TqInt iTime;

	for( iTime = 0; iTime < cTimes(); iTime++ )
	{
		for( iUP = pPoints( iTime )->aUserParams().begin(); iUP != pPoints( iTime )->aUserParams().end(); iUP++ )
		{
			iIndex = ( *iUP )->Size();
			( *iUP )->SetSize( iIndex+1 );

			switch ( ( *iUP )->Type() )
			{
					case type_float:
					{
						CqParameterTyped<TqFloat, TqFloat>* pParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( ( *iUP ) );
						CreateVertex( pParam, pVertex, iIndex );
					}
					break;

					case type_integer:
					{
						CqParameterTyped<TqInt, TqFloat>* pParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( ( *iUP ) );
						CreateVertex( pParam, pVertex, iIndex );
					}
					break;

					case type_point:
					case type_normal:
					case type_vector:
					{
						CqParameterTyped<CqVector3D, CqVector3D>* pParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( ( *iUP ) );
						CreateVertex( pParam, pVertex, iIndex );
					}
					break;

					case type_color:
					{
						CqParameterTyped<CqColor, CqColor>* pParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( ( *iUP ) );
						CreateVertex( pParam, pVertex, iIndex );
					}
					break;

					case type_hpoint:
					{
						CqParameterTyped<CqVector4D, CqVector3D>* pParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( ( *iUP ) );
						CreateVertex( pParam, pVertex, iIndex );
					}
					break;

					case type_string:
					{
						//CqParameterTyped<CqString, CqString>* pParam = static_cast<CqParameterTyped<CqString, CqString>*>( ( *iUP ) );
						//CreateVertex( pParam, pVertex, iIndex );
					}
					break;

					case type_matrix:
					{
						//CqParameterTyped<CqMatrix, CqMatrix>* pParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( ( *iUP ) );
						//CreateVertex( pParam, pVertex, iIndex );
					}
					break;
			}

		}
	}

	// Resize the vertex lath 
	m_aapVertices.resize(iIndex+1);

	return(iIndex);
}


//------------------------------------------------------------------------------
/**
 *	Add a completely new vertex to the list.
 *	Appends a new vertex to the end of the list, updating the referencing
 *	table as well.
 *
 *	@return			The index of the new point.
 */
TqInt CqSubdivision2::AddEdgeVertex(CqLath* pVertex)
{
	TqInt iIndex;

	std::vector<CqParameter*>::iterator iUP;
	TqInt iTime;

	for( iTime = 0; iTime < cTimes(); iTime ++ )
	{
		for ( iUP = pPoints( iTime )->aUserParams().begin(); iUP != pPoints( iTime )->aUserParams().end(); iUP++ )
		{
			iIndex = ( *iUP )->Size();
			( *iUP )->SetSize( iIndex+1 );

			switch ( ( *iUP )->Type() )
			{
					case type_float:
					{
						CqParameterTyped<TqFloat, TqFloat>* pParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( ( *iUP ) );
						CreateEdgeVertex( pParam, pVertex, iIndex );
					}
					break;

					case type_integer:
					{
						CqParameterTyped<TqInt, TqFloat>* pParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( ( *iUP ) );
						CreateEdgeVertex( pParam, pVertex, iIndex );
					}
					break;

					case type_point:
					case type_normal:
					case type_vector:
					{
						CqParameterTyped<CqVector3D, CqVector3D>* pParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( ( *iUP ) );
						CreateEdgeVertex( pParam, pVertex, iIndex );
					}
					break;

					case type_color:
					{
						CqParameterTyped<CqColor, CqColor>* pParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( ( *iUP ) );
						CreateEdgeVertex( pParam, pVertex, iIndex );
					}
					break;

					case type_hpoint:
					{
						CqParameterTyped<CqVector4D, CqVector3D>* pParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( ( *iUP ) );
						CreateEdgeVertex( pParam, pVertex, iIndex );
					}
					break;

					case type_string:
					{
						//CqParameterTyped<CqString, CqString>* pParam = static_cast<CqParameterTyped<CqString, CqString>*>( ( *iUP ) );
						//CreateEdgeVertex( pParam, pVertex, iIndex );
					}
					break;

					case type_matrix:
					{
						//CqParameterTyped<CqMatrix, CqMatrix>* pParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( ( *iUP ) );
						//CreateEdgeVertex( pParam, pVertex, iIndex );
					}
					break;
			}

		}
	}

	// Resize the vertex lath 
	m_aapVertices.resize(iIndex+1);

	return(iIndex);
}


//------------------------------------------------------------------------------
/**
 *	Add a completely new vertex to the list.
 *	Appends a new vertex to the end of the list, updating the referencing
 *	table as well.
 *
 *	@return			The index of the new point.
 */
TqInt CqSubdivision2::AddFaceVertex(CqLath* pVertex)
{
	TqInt iIndex=0;

	std::vector<CqParameter*>::iterator iUP;
	TqInt iTime;

	for( iTime = 0; iTime < cTimes(); iTime++ )
	{
		for ( iUP = pPoints( iTime )->aUserParams().begin(); iUP != pPoints( iTime )->aUserParams().end(); iUP++ )
		{
			iIndex = ( *iUP )->Size();
			( *iUP )->SetSize( iIndex+1 );

			switch ( ( *iUP )->Type() )
			{
					case type_float:
					{
						CqParameterTyped<TqFloat, TqFloat>* pParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( ( *iUP ) );
						CreateFaceVertex( pParam, pVertex, iIndex );
					}
					break;

					case type_integer:
					{
						CqParameterTyped<TqInt, TqFloat>* pParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( ( *iUP ) );
						CreateFaceVertex( pParam, pVertex, iIndex );
					}
					break;

					case type_point:
					case type_normal:
					case type_vector:
					{
						CqParameterTyped<CqVector3D, CqVector3D>* pParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( ( *iUP ) );
						CreateFaceVertex( pParam, pVertex, iIndex );
					}
					break;

					case type_color:
					{
						CqParameterTyped<CqColor, CqColor>* pParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( ( *iUP ) );
						CreateFaceVertex( pParam, pVertex, iIndex );
					}
					break;

					case type_hpoint:
					{
						CqParameterTyped<CqVector4D, CqVector3D>* pParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( ( *iUP ) );
						CreateFaceVertex( pParam, pVertex, iIndex );
					}
					break;

					case type_string:
					{
						//CqParameterTyped<CqString, CqString>* pParam = static_cast<CqParameterTyped<CqString, CqString>*>( ( *iUP ) );
						//CreateFaceVertex( pParam, pVertex, iIndex );
					}
					break;

					case type_matrix:
					{
						//CqParameterTyped<CqMatrix, CqMatrix>* pParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( ( *iUP ) );
						//CreateFaceVertex( pParam, pVertex, iIndex );
					}
					break;
			}

		}
	}

	// Resize the vertex lath 
	m_aapVertices.resize(iIndex+1);

	return(iIndex);
}


//------------------------------------------------------------------------------
/**
 *	Add a new facet to the topology structure.
 *	Adds the facet by adding new laths for the specified vertex indices, and
 *	linking them to each other clockwise about the facet. By convention, as
 *	outside of the topology structure facets are stored counter clockwise, the
 *	vertex indices should be passed to this function as counter clockwise and
 *	they will be internally altered to specify the facet as clockwise.
 *
 *	@param	cVerts		The number of vertices in the facet.
 *	@param	pIndices	Pointer to an array of vertex indices.
 *
 *	@return				Pointer to one of the laths which represent this new
 *						facet in the topology structure.
 */
CqLath* CqSubdivision2::AddFacet(TqInt cVerts, TqInt* pIndices)
{
	CqLath* pLastLath=NULL;
	CqLath* pFirstLath=NULL;
	// Add the laths for this facet, referencing the appropriate vertexes as we go.
	for(TqInt iVert = 0; iVert < cVerts; iVert++)
	{
		CqLath* pNewLath = new CqLath();
		pNewLath->SetVertexIndex(pIndices[iVert]);

		if(NULL != pLastLath)
			pNewLath->SetpClockwiseFacet(pLastLath);

		m_apLaths.push_back(pNewLath);
		pLastLath = pNewLath;
		if(iVert == 0)	pFirstLath = pLastLath;

		// We also need to keep up to date a complete list of which laths refer to which 
		// vertices to aid us in finalising the topology structure later.
		m_aapVertices[pIndices[iVert]].push_back(pLastLath);
	}
	// complete the chain by linking the last one as the next clockwise one to the first.
	pFirstLath->SetpClockwiseFacet(pLastLath);

	// Add the start lath in as the one referring to this facet in the list.
	m_apFacets.push_back(pFirstLath);

	return(pFirstLath);
}


//------------------------------------------------------------------------------
/**
 *	Finalise the linkage of the laths.
 *	After adding vertices and facets, call this to complete the linkage of the
 *	laths. To overcome any non-manifold areas in the mesh, this function may
 *	change the topology in order to produce a manifold mesh, or series of
 *	manifold meshes. This also means that all facets in the mesh may no longer
 *	be joined in a complete loop, so care must be taken when traversing the
 *	topology to ensure that all facets are processed.
 */
TqBool CqSubdivision2::Finalise()
{
	for(std::vector<std::vector<CqLath*> >::const_iterator ivert=m_aapVertices.begin(); ivert!=m_aapVertices.end(); ivert++)
	{
		TqInt cLaths = (*ivert).size();

		// If there is only one lath, it can't be connected to anything.
		if(cLaths<=1)	continue;

		// Create an array for the laths on this vertex that have been visited.
		std::vector<TqBool>  aVisited;
		aVisited.resize(cLaths);
		TqInt cVisited = 0;

		// Initialise it to all false.
		aVisited.assign(cLaths, TqFalse);
		
		CqLath* pCurrent = (*ivert)[0];
		CqLath* pStart = pCurrent;
		TqInt iCurrent = 0;
		TqInt iStart = 0;
		
		TqBool fDone = TqFalse;
		while(!fDone)
		{
			// Find a clockwise vertex match for the counterclockwise vertex index of this lath.
			TqInt ccwVertex = pCurrent->ccf()->VertexIndex();
			TqInt iLath = 0;
			for(iLath = 0; iLath < cLaths; iLath++)
			{
				// Only check non-visited laths.
				if(!aVisited[iLath] && (*ivert)[iLath]->cf()->VertexIndex() == ccwVertex)
				{
					pCurrent->SetpClockwiseVertex((*ivert)[iLath]);
					pCurrent = (*ivert)[iLath];
					iCurrent = iLath;
					// Mark the linked to lath as visited.
					aVisited[iLath] = TqTrue;
					cVisited++;

					break;
				}
			}
			// If we didn't find a match then we are done.
			fDone = iLath==cLaths;
		}

		// If the last lath wasn't linked, then we have a boundary condition, so
		// start again from the initial lath and process backwards.
		if(NULL == pCurrent->cv())
		{
			fDone = TqFalse;
			while(!fDone)
			{
				// Find a counterclockwise vertex match for the clockwise vertex index of this lath.
				TqInt cwVertex = pStart->cf()->VertexIndex();
				TqInt iLath = 0;
				for(iLath = 0; iLath < cLaths; iLath++)
				{
					// Only check non-visited laths.
					if(!aVisited[iLath] && (*ivert)[iLath]->ccf()->VertexIndex() == cwVertex)
					{
						// Link the current to the match.
						(*ivert)[iLath]->SetpClockwiseVertex(pStart);
						// Mark the linked to lath as visited.
						aVisited[iStart] = TqTrue;
						cVisited++;
						pStart = (*ivert)[iLath];
						iStart = iLath;

						break;
					}
				}
				// If we didn't find a match then we are done.
				fDone = iLath==cLaths;
			}
		}
		aVisited[iStart] = TqTrue;
		cVisited++;
		// If we have not visited all the laths referencing this vertex, then we have a non-manifold situation.
		if(cVisited < cLaths)
		{
			return( TqFalse );
		}
	}
	
	m_fFinalised = TqTrue;
	return( TqTrue );
}


#define modulo(a, b) (a * b >= 0 ? a % b : (a % b) + b)
struct SqFaceLathList {
	CqLath* pA, *pB, *pC, *pD;
};

void CqSubdivision2::SubdivideFace(CqLath* pFace, std::vector<CqLath*>& apSubFaces)
{
	assert(NULL != pFace);

	// If this has already beed subdivided then skip it.
	if( NULL != pFace->pFaceVertex() )
	{
		apSubFaces.clear();
		std::vector<CqLath*> aQvf;
		pFace->pFaceVertex()->Qvf(aQvf);
		// Fill in the lath pointers to the same laths that reference the faces in the topology list. This ensures that
		// the dicing routine will still get the lath it expects in the corner for reading data out.
		std::vector<CqLath*>::iterator iVF;
		for( iVF = aQvf.begin(); iVF != aQvf.end(); iVF++ )
			apSubFaces.push_back( (*iVF)->ccf()->ccf() );
		return;
	}

	// First make sure that the appropriate neighbour facets have been subdivided if this is >0 level face.
	if( NULL != pFace->pParentFacet() )
	{
		std::vector<CqLath*> aQff;
		std::vector<CqLath*> apSubFaces2;
		pFace->pParentFacet()->Qff( aQff );
		std::vector<CqLath*>::iterator iF;
		for( iF = aQff.begin(); iF != aQff.end(); iF++ )
			SubdivideFace(*iF, apSubFaces2);
	}

	std::vector<CqLath*> aQfv;
	std::vector<TqInt> aVertices;

	pFace->Qfv(aQfv);
	TqInt n = aQfv.size();

	aVertices.resize((2*n)+1);

	// Clear the return array for subdface indices.
	apSubFaces.clear();

	// First of all setup the points.
	TqInt i;

	// Create new point for the face midpoint.
	TqInt iVert = AddFaceVertex(pFace);

	// Create new points for the edge midpoints.
	for(i = 0; i < n; i++)
	{
		TqInt iVert;
		// Create new vertices for the edge mid points.
		if( NULL != aQfv[i]->ec() && NULL != aQfv[i]->ec()->pMidVertex() )
			// There is already a next level vertex for this, so just setup a lath to it.
			iVert = aQfv[i]->ec()->pMidVertex()->VertexIndex();
		else
			// Create new vertex for the edge midpoint.
			iVert = AddEdgeVertex(aQfv[i]);

		// Store the index, for later lath creation
		aVertices[i+n] = iVert;
	}

	// Create new points for the existing vertices
	for(i = 0; i < n; i++)
	{
		TqInt iVert;
		// Create new vertices for the original points.
		if( NULL == aQfv[i]->pChildVertex() )
			// Create a new vertex for the next level
			iVert = AddVertex(aQfv[i]);
		else
			// There is already a next level vertex for this, so just setup a lath to it.
			iVert = aQfv[i]->pChildVertex()->VertexIndex();

		// Store the index, for later lath creation
		aVertices[i] = iVert;
	}

	// Store the index, for later lath creation
	aVertices[2*n] = iVert;

	// Now create new laths for the new facets
	std::vector<SqFaceLathList>	apFaceLaths;
	apFaceLaths.resize(n);

	for( i = 0; i < n; i++ )
	{
		// For each facet, create 4 laths and join them in the order of the facet
		CqLath* pLathA = apFaceLaths[i].pA = new CqLath( aVertices[i] );
		m_apLaths.push_back(pLathA);
		CqLath* pLathB = apFaceLaths[i].pB = new CqLath( aVertices[(modulo((i+1),n))+n] );
		m_apLaths.push_back(pLathB);
		CqLath* pLathC = apFaceLaths[i].pC = new CqLath( aVertices[2*n] );
		m_apLaths.push_back(pLathC);
		CqLath* pLathD = apFaceLaths[i].pD = new CqLath( aVertices[i+n] );
		m_apLaths.push_back(pLathD);
		pLathA->SetpClockwiseFacet(pLathB);
		pLathB->SetpClockwiseFacet(pLathC);
		pLathC->SetpClockwiseFacet(pLathD);
		pLathD->SetpClockwiseFacet(pLathA);
		pLathA->SetpParentFacet(pFace);
		pLathB->SetpParentFacet(pFace);
		pLathC->SetpParentFacet(pFace);
		pLathD->SetpParentFacet(pFace);

		// Fill in the vertex references table for these vertices.
		m_aapVertices[pLathA->VertexIndex()].push_back(pLathA);
		m_aapVertices[pLathB->VertexIndex()].push_back(pLathB);
		m_aapVertices[pLathC->VertexIndex()].push_back(pLathC);
		m_aapVertices[pLathD->VertexIndex()].push_back(pLathD);

		// Set the child vertex pointer for all laths which reference the A vertex of this facet
		// so that we can use them when subdividing other faces.
		CqLath* pNextV = aQfv[i];
		do
		{
			pNextV->SetpChildVertex(pLathA);
			pNextV = pNextV->cv();
		}while( pNextV && pNextV != aQfv[i]);
		// Make sure that if we have hit a boundary, we go backwards from the start point until we hit the boundary that
		// way as well.
		if(NULL == pNextV)
		{
			pNextV = aQfv[i]->ccv();
			// We know we are going to hit a boundary in this direction as well so we can just look for that
			// case as a terminator.
			while( pNextV )
			{
				assert( pNextV != aQfv[i] );
				pNextV->SetpChildVertex(pLathA);
				pNextV = pNextV->ccv();
			}
		}
		
		// For this edge of the original face, set a ponter to the new midpoint lath, so that we can
		// use it when subdividing neighbour facets.
		aQfv[i]->SetpMidVertex(pLathD);

		// Transfer sharpness information
		float sharpness = EdgeSharpness( aQfv[ i ] );
		if( sharpness > 0.0f )
			AddSharpEdge( pLathA, sharpness * sharpness );

		sharpness = EdgeSharpness( aQfv[ modulo( (i+1),n ) ] );
		if( sharpness > 0.0f )
			AddSharpEdge( pLathB, sharpness * sharpness );

		if( CornerSharpness( aQfv[ i ] ) > 0.0f )
			AddSharpCorner( pLathA, CornerSharpness( aQfv[ i ] ) );

		// Store a lath reference for the facet.
		apSubFaces.push_back( pLathA );
		m_apFacets.push_back( pLathA );
	}

	// Now connect up the laths we have created.
	// The clcckwise face connections will have already been made, we need to fixup and clockwise
	// vertex connections we can.
	for( i = 0; i < n; i++ )
	{
		// Set the facet point reference for all laths representing this facet.
		aQfv[i]->SetpFaceVertex(apFaceLaths[i].pC);
		// Connect midpoints clockwise vertex pointers.
		apFaceLaths[((i+1)%n)].pD->SetpClockwiseVertex( apFaceLaths[i].pB );
		// Connect all laths around the new face point.
		apFaceLaths[i].pC->SetpClockwiseVertex( apFaceLaths[ ((i+1)%n) ].pC );

		// Connect the new corner vertices, this is only possible if neighbouring facets have previously been
		// subdivided.
		std::vector<CqLath*>::iterator iVertLath;
		for( iVertLath = m_aapVertices[apFaceLaths[i].pA->VertexIndex()].begin(); iVertLath != m_aapVertices[apFaceLaths[i].pA->VertexIndex()].end(); iVertLath++ )
		{
			if( (*iVertLath)->cf()->VertexIndex() == apFaceLaths[i].pD->VertexIndex() )
				apFaceLaths[i].pA->SetpClockwiseVertex( (*iVertLath ) );
			if( (*iVertLath)->ccf()->VertexIndex() == apFaceLaths[i].pB->VertexIndex() )
				(*iVertLath)->SetpClockwiseVertex( apFaceLaths[i].pA );
		}
	}

	for( i = 0; i < n; i++ )
	{
		// Connect the new edge midpoint vertices to any neighbours, this is only possible if neighbouring facets have previously been
		// subdivided.
		std::vector<CqLath*>::iterator iVertLath;
		for( iVertLath = m_aapVertices[apFaceLaths[i].pB->VertexIndex()].begin(); iVertLath != m_aapVertices[apFaceLaths[i].pB->VertexIndex()].end(); iVertLath++ )
		{
			if( (*iVertLath)->cf()->VertexIndex() == apFaceLaths[i].pA->VertexIndex() )
				apFaceLaths[i].pB->SetpClockwiseVertex( (*iVertLath ) );
		}
		for( iVertLath = m_aapVertices[apFaceLaths[i].pD->VertexIndex()].begin(); iVertLath != m_aapVertices[apFaceLaths[i].pD->VertexIndex()].end(); iVertLath++ )
		{
			if( (*iVertLath)->ccf()->VertexIndex() == apFaceLaths[i].pA->VertexIndex() )
				(*iVertLath )->SetpClockwiseVertex( apFaceLaths[i].pD );
		}
	}
}


void CqSubdivision2::OutputMesh(const char* fname, std::vector<CqLath*>* paFaces)
{
	std::ofstream file(fname);
	std::vector<CqLath*> aQfv;

	TqUint i;
	for( i = 0; i < cVertices(); i++ )
	{
		CqVector3D vec = pPoints()->P()->pValue()[ pVertex( i )->VertexIndex() ];
		file << "v " << vec.x() << " " << vec.y() << " " << vec.z() << std::endl;
	}
	

	for(i = 0; i < cFacets(); i++)
	{
		if( NULL == pFacet(i)->pFaceVertex())
		{
			pFacet(i)->Qfv(aQfv);
			TqUint j;
			file << "f ";
			for( j = 0; j < aQfv.size(); j++ )
				file << aQfv[j]->VertexIndex()+1 << " ";
			file << std::endl;
		}
	}

	if( NULL != paFaces)
	{
		file << "g CurrentFace" << std::endl;
		for(i = 0; i < paFaces->size(); i++)
		{
			(*paFaces)[i]->Qfv(aQfv);
			TqUint j;
			file << "f ";
			for( j = 0; j < aQfv.size(); j++ )
				file << aQfv[j]->VertexIndex()+1 << " ";
			file << std::endl;
		}
	}

	file.close();
}


void CqSubdivision2::OutputInfo(const char* fname, std::vector<CqLath*>* paFaces)
{
	std::ofstream file(fname);
	std::vector<CqLath*> aQfv;

	std::vector<CqLath*>* paLaths = paFaces;
	
	if( NULL == paLaths )
		paLaths = &m_apFacets;
	
	for(TqUint i = 0; i < paLaths->size(); i++)
	{
		CqLath* pL = (*paLaths)[i];
		file << i << " - 0x" << pL << " - "	<<
			pL->VertexIndex() << " - (cf) ";
		if( pL->cf() )
			file << "0x" << pL->cf();
		else
			file << "***";
		file << " - (cv) ";
		
		if(pL->cv())
			file << "0x" << pL->cv();
		else
			file << "***";
			
		file << std::endl;
	}

	file.close();
}


CqBound	CqSurfaceSubdivisionPatch::Bound() const
{
	assert( NULL != pTopology() );
	assert( NULL != pTopology()->pPoints() );
	assert( NULL != pFace() );

	// First make sure that the appropriate neighbour facets have been subdivided if this is >0 level face.
	if( NULL != pFace()->pParentFacet() )
	{
		std::vector<CqLath*> aQff;
		std::vector<CqLath*> apSubFaces2;
		pFace()->pParentFacet()->Qff( aQff );
		std::vector<CqLath*>::iterator iF;
		for( iF = aQff.begin(); iF != aQff.end(); iF++ )
			pTopology()->SubdivideFace(*iF, apSubFaces2);
	}

	CqBound B;
	
	// Get the laths of the surrounding faces.
	std::vector<CqLath*> aQff;
	pFace()->Qff(aQff);
	std::vector<CqLath*>::iterator iFF;
	for( iFF = aQff.begin(); iFF != aQff.end(); iFF++ )
	{
		// Get the laths that reference the vertices of this face
		std::vector<CqLath*> aQfv;
		(*iFF)->Qfv(aQfv);

		// Now get the vertices, and form the bound.
		std::vector<CqLath*>::iterator iQfv;
		for( iQfv = aQfv.begin(); iQfv != aQfv.end(); iQfv++ )
		{
			TqInt iTime;
			for( iTime = 0; iTime < pTopology()->cTimes(); iTime++ )
				B.Encapsulate(pTopology()->pPoints( iTime )->P()->pValue((*iQfv)->VertexIndex())[0]);
		}
	}

	return( AdjustBoundForTransformationMotion( B ) );
}


/** Dice the patch this primitive represents. 
 * Subdivide recursively the appropriate number of times, then extract the information into 
 * a MPG structure.
 */

CqMicroPolyGridBase* CqSurfaceSubdivisionPatch::Dice()
{
	assert( NULL != pTopology() );
	assert( NULL != pTopology()->pPoints() );
	assert( NULL != pFace() );

	TqInt dicesize = MAX(m_uDiceSize, m_vDiceSize);
	TqInt sdcount = ( dicesize == 16 ) ? 4 : ( dicesize == 8 ) ? 3 : ( dicesize == 4 ) ? 2 : ( dicesize == 2 ) ? 1 : 1;
	dicesize = 1 << sdcount;
	TqInt lUses = Uses();

	std::vector<CqMicroPolyGrid*> apGrids;

	TqInt iTime;
	for( iTime = 0; iTime < pTopology()->cTimes(); iTime++ )
	{
		CqMicroPolyGrid* pGrid = new CqMicroPolyGrid( dicesize, dicesize, pTopology()->pPoints() );

		CqPolygonPoints* pMotionPoints = pTopology()->pPoints( iTime );

		TqInt isd;
		std::vector<CqLath*> apSubFace1, apSubFace2;
		apSubFace1.push_back(pFace());
		for( isd = 0; isd < sdcount; isd++ )
		{
			apSubFace2.clear();
			std::vector<CqLath*>::iterator iSF;
			for( iSF = apSubFace1.begin(); iSF != apSubFace1.end(); iSF++ )
			{
				// Subdivide this face, storing the resulting new face indices.
				std::vector<CqLath*> apSubFaceTemp;
				pTopology()->SubdivideFace( (*iSF), apSubFaceTemp );
				// Now combine these into the new face indices for this subdivision level.
				apSubFace2.insert(apSubFace2.end(), apSubFaceTemp.begin(), apSubFaceTemp.end());
			}
			// Now swap the new level's indices for the old before repeating at the next level, if appropriate.
			apSubFace1.swap(apSubFace2);
		}

		// Now we use the first face index to start our extraction
		TqInt nc, nr, c, r;
		nc = nr = MAX(m_uDiceSize, m_vDiceSize);
		r = 0;

		CqLath* pLath, *pTemp;
		pLath = apSubFace1[0];
		pTemp = pLath;

		// Get data from pLath
		TqInt ivA = pLath->VertexIndex();
		TqInt indexA = 0;
		
		StoreDice( pGrid, pMotionPoints, ivA, indexA );

		indexA++;		
		pLath = pLath->ccf();
		c = 0;
		while( c < nc )
		{
			TqInt ivA = pLath->VertexIndex();
			StoreDice( pGrid, pMotionPoints, ivA, indexA );

			if( c < ( nc - 1 ) )	
				pLath = pLath->cv()->ccf();
			
			indexA++;
			c++;
		}
		r++;

		while( r <= nr )
		{
			pLath = pTemp->cf();
			if( r < nr )
				pTemp = pLath->ccv();

			// Get data from pLath
			TqInt ivA = pLath->VertexIndex();
			TqInt indexA = ( r * ( nc + 1 ) );
			StoreDice( pGrid, pMotionPoints, ivA, indexA );

			indexA++;		
			pLath = pLath->cf();
			c = 0;
			while( c < nc )
			{
				TqInt ivA = pLath->VertexIndex();
				StoreDice( pGrid, pMotionPoints, ivA, indexA );

				if( c < ( nc - 1 ) )	
					pLath = pLath->ccv()->cf();
				
				indexA++;
				c++;
			}

			r++;
		}


		// If the color and opacity are not defined, use the system values.
		if ( USES( lUses, EnvVars_Cs ) && !pTopology()->pPoints()->bHasCs() )
		{
			if ( NULL != pAttributes() ->GetColorAttribute( "System", "Color" ) )
				pGrid->Cs() ->SetColor( pAttributes() ->GetColorAttribute( "System", "Color" ) [ 0 ] );
			else
				pGrid->Cs() ->SetColor( CqColor( 1, 1, 1 ) );
		}

		if ( USES( lUses, EnvVars_Os ) && !pTopology()->pPoints()->bHasOs() )
		{
			if ( NULL != pAttributes() ->GetColorAttribute( "System", "Opacity" ) )
				pGrid->Os() ->SetColor( pAttributes() ->GetColorAttribute( "System", "Opacity" ) [ 0 ] );
			else
				pGrid->Os() ->SetColor( CqColor( 1, 1, 1 ) );
		}
		apGrids.push_back( pGrid );

		// Fill in u/v if required.
		if ( USES( lUses, EnvVars_u ) && !pTopology()->pPoints()->bHasu() )
		{
			TqInt iv, iu;
			for ( iv = 0; iv <= dicesize; iv++ )
			{
				for ( iu = 0; iu <= dicesize; iu++ )
				{
					TqInt igrid = ( iv * ( dicesize + 1 ) ) + iu;
					pGrid->u()->SetFloat( BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, iu, iv ), igrid );
				}
			}
		}

		if ( USES( lUses, EnvVars_v ) && !pTopology()->pPoints()->bHasv() )
		{
			TqInt iv, iu;
			for ( iv = 0; iv <= dicesize; iv++ )
			{
				for ( iu = 0; iu <= dicesize; iu++ )
				{
					TqInt igrid = ( iv * ( dicesize + 1 ) ) + iu;
					pGrid->v()->SetFloat( BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, iu, iv ), igrid );
				}
			}
		}
	}

	if( apGrids.size() == 1 )
		return( apGrids[ 0 ] );
	else
	{
		CqMotionMicroPolyGrid * pGrid = new CqMotionMicroPolyGrid;
		TqInt i;
		for ( i = 0; i < pTopology()->cTimes(); i++ )
			pGrid->AddTimeSlot( pTopology()->Time( i ), apGrids[ i ] );
		return( pGrid );
	}
}



static void StoreDiceAPVar( IqShader* pShader, CqParameter* pParam, TqUint ivA, TqUint indexA )
{
	// Find the argument
	IqShaderData * pArg = pShader->FindArgument( pParam->strName() );
	if ( NULL != pArg )
	{
		switch ( pParam->Type() )
		{
			case type_float:
			{
				CqParameterTyped<TqFloat, TqFloat>* pNParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParam );
				pArg->SetValue( *pNParam->pValue( ivA ), indexA );
			}
			break;

			case type_integer:
			{
				CqParameterTyped<TqInt, TqFloat>* pNParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParam );
				pArg->SetValue( *pNParam->pValue( ivA ), indexA );
			}
			break;

			case type_point:
			case type_vector:
			case type_normal:
			{
				CqParameterTyped<CqVector3D, CqVector3D>* pNParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam );
				pArg->SetValue( *pNParam->pValue( ivA ), indexA );
			}
			break;

			case type_hpoint:
			{
				CqParameterTyped<CqVector4D, CqVector3D>* pNParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParam );
				pArg->SetValue( *pNParam->pValue( ivA ), indexA );
			}
			break;

			case type_string:
			{
				CqParameterTyped<CqString, CqString>* pNParam = static_cast<CqParameterTyped<CqString, CqString>*>( pParam );
				pArg->SetValue( *pNParam->pValue( ivA ), indexA );
			}
			break;

			case type_color:
			{
				CqParameterTyped<CqColor, CqColor>* pNParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParam );
				pArg->SetValue( *pNParam->pValue( ivA ), indexA );
			}
			break;

			case type_matrix:
			{
				CqParameterTyped<CqMatrix, CqMatrix>* pNParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam );
				pArg->SetValue( *pNParam->pValue( ivA ), indexA );
			}
			break;
		}
	}
}


void CqSurfaceSubdivisionPatch::StoreDice( CqMicroPolyGrid* pGrid, CqPolygonPoints* pPoints, TqInt iParam, TqInt iData )
{
	TqInt lUses = Uses();
	
	if ( USES( lUses, EnvVars_P ) )
		pGrid->P() ->SetPoint( ( *pPoints->P() ) [ iParam ], iData );

	if ( USES( lUses, EnvVars_s ) && ( NULL != pGrid->s() ) )
	{
		if( pPoints->bHass() )
			pGrid->s() ->SetFloat( ( *pPoints->s() ) [ iParam ], iData );
		else
			pGrid->s() ->SetFloat( ( *pPoints->P() ) [ iParam ].x(), iData );
	}

	if ( USES( lUses, EnvVars_t ) && ( NULL != pGrid->t() ) )
	{
		if( pPoints->bHast() )
			pGrid->t() ->SetFloat( ( *pPoints->t() ) [ iParam ], iData );
		else
			pGrid->t() ->SetFloat( ( *pPoints->P() ) [ iParam ].y(), iData );
	}

/*	if ( USES( lUses, EnvVars_u ) && ( NULL != pGrid->u() ) )
	{
		if( pPoints->bHasu() )
			pGrid->u() ->SetFloat( ( *pPoints->u() ) [ iParam ], iData );
		else
			pGrid->u() ->SetFloat( ( *pPoints->P() ) [ iParam ].x(), iData );
	}

	if ( USES( lUses, EnvVars_v ) && ( NULL != pGrid->v() ) )
	{
		if( pPoints->bHasv() )
			pGrid->v() ->SetFloat( ( *pPoints->v() ) [ iParam ], iData );
		else
			pGrid->v() ->SetFloat( ( *pPoints->P() ) [ iParam ].y(), iData );
	}
*/
	if ( USES( lUses, EnvVars_Cs ) && ( NULL != pGrid->Cs() ) && ( pPoints->bHasCs() ) )
		pGrid->Cs() ->SetColor( ( *pPoints->Cs() ) [ iParam ], iData );

	if ( USES( lUses, EnvVars_Os ) && ( NULL != pGrid->Os() ) && ( pPoints->bHasOs() ) )
		pGrid->Os() ->SetColor( ( *pPoints->Os() ) [ iParam ], iData );

	// Now lets store the diced user specified primitive variables.
	std::vector<CqParameter*>::iterator iUP;
	for ( iUP = pPoints->aUserParams().begin(); iUP != pPoints->aUserParams().end(); iUP++ )
	{
		/// \todo: Must transform point/vector/normal/matrix parameter variables from 'object' space to current before setting.
		if ( NULL != pGrid->pAttributes() ->pshadSurface() )
			StoreDiceAPVar( pGrid->pAttributes()->pshadSurface(), ( *iUP ), iParam, iData );

		if ( NULL != pGrid->pAttributes() ->pshadDisplacement() )
			StoreDiceAPVar( pGrid->pAttributes()->pshadDisplacement(), ( *iUP ), iParam, iData );

		if ( NULL != pGrid->pAttributes() ->pshadAtmosphere() )
			StoreDiceAPVar( pGrid->pAttributes()->pshadAtmosphere(), ( *iUP ), iParam, iData );
	}
}


TqInt CqSurfaceSubdivisionPatch::Split( std::vector<CqBasicSurface*>& aSplits )
{
	assert( NULL != pTopology() );
	assert( NULL != pTopology()->pPoints() );
	assert( NULL != pFace() );

	if( pTopology()->CanUsePatch( pFace() ) )
	{
		// Find the point indices for the 16 patch vertices.
		CqLath* pPoint = pFace()->cv()->cv()->cf()->cf();
		CqLath* pRow = pPoint;

		std::vector<TqInt>	aiVertices;

		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->ccf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->cv()->ccf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->cv()->ccf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pRow = pPoint = pRow->cf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->cf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->ccv()->cf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->ccv()->cf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pRow = pPoint = pRow->ccv()->cf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->cf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->ccv()->cf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->ccv()->cf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pRow->ccv()->cf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->cf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->ccv()->cf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->ccv()->cf();
		aiVertices.push_back( pPoint->VertexIndex() );

		std::vector< CqSurfacePatchBicubic* > apSurfaces;

		TqInt iTime;

		for( iTime = 0; iTime < pTopology()->cTimes(); iTime++ )
		{
			// Create a surface patch
			CqSurfacePatchBicubic * pSurface = new CqSurfacePatchBicubic();
			pSurface->AddRef();
			// Fill in default values for all primitive variables not explicitly specified.
			pSurface->SetSurfaceParameters( *pTopology()->pPoints( iTime ) );

			std::vector<CqParameter*>::iterator iUP;
			std::vector<CqParameter*>::iterator end = pTopology()->pPoints( iTime)->aUserParams().end();
			for ( iUP = pTopology()->pPoints( iTime )->aUserParams().begin(); iUP != end; iUP++ )
			{
				if ( ( *iUP ) ->Class() == class_varying )
				{
					// Copy any 'varying' class primitive variables.
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
					pNewUP->SetSize( pSurface->cVarying() );
					pNewUP->SetValue( ( *iUP ), 0, aiVertices[5] );
					pNewUP->SetValue( ( *iUP ), 1, aiVertices[6] );
					pNewUP->SetValue( ( *iUP ), 2, aiVertices[9] );
					pNewUP->SetValue( ( *iUP ), 3, aiVertices[10] );
					pSurface->AddPrimitiveVariable( pNewUP );
				}
				else if ( ( *iUP ) ->Class() == class_vertex )
				{
					// Copy any 'vertex' class primitive variables.
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
					pNewUP->SetSize( pSurface->cVertex() );
					TqUint i;
					for( i = 0; i < pSurface->cVertex(); i++ )
						pNewUP->SetValue( ( *iUP ), i, aiVertices[i] );
					pSurface->AddPrimitiveVariable( pNewUP );
				}
				else if ( ( *iUP ) ->Class() == class_uniform )
				{
					// Copy any 'uniform' class primitive variables.
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
					pNewUP->SetSize( pSurface->cUniform() );
					pNewUP->SetValue( ( *iUP ), 0, 0 );
					pSurface->AddPrimitiveVariable( pNewUP );
				}
				else if ( ( *iUP ) ->Class() == class_constant )
				{
					// Copy any 'constant' class primitive variables.
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
					pNewUP->SetSize( 1 );
					pNewUP->SetValue( ( *iUP ), 0, 0 );
					pSurface->AddPrimitiveVariable( pNewUP );
				}
			}

			// Need to get rid of any 'h' values added to the "P" variables during multiplication.
			TqUint i;
			for( i = 0; i < pSurface->cVertex(); i++ )
				( *pSurface->P() ) [i] = static_cast<CqVector3D>( ( *pSurface->P() ) [i] );

			CqMatrix matuBasis( RiBSplineBasis );
			CqMatrix matvBasis( RiBSplineBasis );
			pSurface->ConvertToBezierBasis( matuBasis, matvBasis );

			TqInt iUses = Uses();

			// If the shader needs s/t or u/v, and s/t is not specified, then at this point store the object space x,y coordinates.
			if ( USES( iUses, EnvVars_s ) || USES( iUses, EnvVars_t ) || USES( iUses, EnvVars_u ) || USES( iUses, EnvVars_v ) )
			{
				CqVector3D PA, PB, PC, PD;
				CqMatrix matID;
				const CqMatrix& matCurrentToWorld = QGetRenderContext() ->matSpaceToSpace( "current", "object", matID, pTopology()->pPoints()->pTransform() ->matObjectToWorld() );
				PA = matCurrentToWorld * pSurface->P() ->pValue() [ 0 ];
				PB = matCurrentToWorld * pSurface->P() ->pValue() [ 3 ];
				PC = matCurrentToWorld * pSurface->P() ->pValue() [ 15 ];
				PD = matCurrentToWorld * pSurface->P() ->pValue() [ 12 ];

				if ( USES( iUses, EnvVars_s ) && !pTopology()->pPoints()->bHass() )
				{
					CqParameterTypedVarying<TqFloat, type_float, TqFloat>* pNewUP = new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "s" );
					pNewUP->SetSize( pSurface->cVarying() );

					pNewUP->pValue() [ 0 ] = PA.x();
					pNewUP->pValue() [ 1 ] = PB.x();
					pNewUP->pValue() [ 2 ] = PD.x();
					pNewUP->pValue() [ 3 ] = PC.x();

					pSurface->AddPrimitiveVariable( pNewUP );
				}

				if ( USES( iUses, EnvVars_t ) && !pTopology()->pPoints()->bHast() )
				{
					CqParameterTypedVarying<TqFloat, type_float, TqFloat>* pNewUP = new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "t" );
					pNewUP->SetSize( pSurface->cVarying() );

					pNewUP->pValue() [ 0 ] = PA.y();
					pNewUP->pValue() [ 1 ] = PB.y();
					pNewUP->pValue() [ 2 ] = PD.y();
					pNewUP->pValue() [ 3 ] = PC.y();

					pSurface->AddPrimitiveVariable( pNewUP );
				}

				if ( USES( iUses, EnvVars_u ) && !pTopology()->pPoints()->bHasu() )
				{
					CqParameterTypedVarying<TqFloat, type_float, TqFloat>* pNewUP = new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "u" );
					pNewUP->SetSize( pSurface->cVarying() );

					pNewUP->pValue() [ 0 ] = 0;//PA.x();
					pNewUP->pValue() [ 1 ] = 1;//PB.x();
					pNewUP->pValue() [ 2 ] = 0;//PD.x();
					pNewUP->pValue() [ 3 ] = 1;//PC.x();

					pSurface->AddPrimitiveVariable( pNewUP );
				}

				if ( USES( iUses, EnvVars_v ) && !pTopology()->pPoints()->bHasv() )
				{
					CqParameterTypedVarying<TqFloat, type_float, TqFloat>* pNewUP = new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "v" );
					pNewUP->SetSize( pSurface->cVarying() );

					pNewUP->pValue() [ 0 ] = 0;//PA.y();
					pNewUP->pValue() [ 1 ] = 0;//PB.y();
					pNewUP->pValue() [ 2 ] = 1;//PD.y();
					pNewUP->pValue() [ 3 ] = 1;//PC.y();

					pSurface->AddPrimitiveVariable( pNewUP );
				}
			}
			apSurfaces.push_back( pSurface );
		}
	
		if( apSurfaces.size() == 1 )
			aSplits.push_back(apSurfaces[ 0 ]);
		else
		{
			CqDeformingSurface* pMotionSurface = new CqDeformingSurface( 0 );
			for( iTime = 0; iTime < pTopology()->cTimes(); iTime++ )
			{
				RtFloat time = pTopology()->Time( iTime );
				pMotionSurface->AddTimeSlot( time, apSurfaces[ iTime ] );
			}
			aSplits.push_back(pMotionSurface);
		}
	}
	else
	{
		// Subdivide the face, and create new patches for the subfaces.
		std::vector<CqLath*> apSubFaces;
		pTopology()->SubdivideFace( pFace(), apSubFaces );

		// Now create new patch objects for each subface.
		std::vector<CqLath*>::iterator iSF;
		for( iSF = apSubFaces.begin(); iSF != apSubFaces.end(); iSF++ )
		{
			CqSurfaceSubdivisionPatch* pNew = new CqSurfaceSubdivisionPatch( pTopology(), (*iSF) );
			aSplits.push_back(pNew);
		}
	}

	return(aSplits.size());
}

TqBool CqSurfaceSubdivisionPatch::Diceable()
{
	assert( NULL != pTopology() );
	assert( NULL != pTopology()->pPoints() );
	assert( NULL != pFace() );

	// If the cull check showed that the primitive cannot be diced due to crossing the e and hither planes,
	// then we can return immediately.
	if ( !m_fDiceable )
		return ( TqFalse );

	// If we can use a patch, don't dice, as dicing a patch is much quicker.
	if( pTopology()->CanUsePatch( pFace() ) )
		return(TqFalse);

	// Get the laths that reference the vertices of this face
	std::vector<CqLath*> aQfv;
	pFace()->Qfv(aQfv);

	// Cannot dice if not 4 points
	if ( aQfv.size() != 4 )
		return ( TqFalse );

	// Otherwise we should continue to try to find the most advantageous split direction, OR the dice size.
	const CqMatrix & matCtoR = QGetRenderContext() ->matSpaceToSpace( "camera", "raster" );

	// Convert the control hull to raster space.
	CqVector2D	avecHull[ 4 ];
	TqInt i;

	TqFloat ShadingRate = pAttributes() ->GetFloatAttribute( "System", "ShadingRate" ) [ 0 ];

	for ( i = 0; i < 4; i++ )
		avecHull[ i ] = matCtoR * pTopology()->pPoints()->P()->pValue() [ aQfv[ i ]->VertexIndex() ];

	TqFloat uLen = 0;
	TqFloat vLen = 0;

	CqVector2D	Vec1 = avecHull[ 1 ] - avecHull[ 0 ];
	CqVector2D	Vec2 = avecHull[ 2 ] - avecHull[ 3 ];
	uLen = ( Vec1.Magnitude2() > Vec2.Magnitude2() ) ? Vec1.Magnitude2() : Vec2.Magnitude2();

	Vec1 = avecHull[ 3 ] - avecHull[ 0 ];
	Vec2 = avecHull[ 1 ] - avecHull[ 1 ];
	vLen = ( Vec1.Magnitude2() > Vec2.Magnitude2() ) ? Vec1.Magnitude2() : Vec2.Magnitude2();

	uLen = sqrt( uLen / ShadingRate);
	vLen = sqrt( vLen / ShadingRate);

	m_SplitDir = ( uLen > vLen ) ? SplitDir_U : SplitDir_V;

	// TODO: Should ensure powers of half to prevent cracking.
	uLen = MAX( ROUND( uLen ), 1 );
	vLen = MAX( ROUND( vLen ), 1 );
	
	m_uDiceSize = static_cast<TqInt>( uLen );
	m_vDiceSize = static_cast<TqInt>( vLen );

	// Ensure power of 2 to avoid cracking
	m_uDiceSize = CEIL_POW2( m_uDiceSize );
	m_vDiceSize = CEIL_POW2( m_vDiceSize );

	if ( uLen < FLT_EPSILON || vLen < FLT_EPSILON )
	{
		m_fDiscard = TqTrue;
		return ( TqFalse );
	}

	TqFloat gs = 16.0f;
	const TqFloat* poptGridSize = QGetRenderContext() ->optCurrent().GetFloatOption( "System", "SqrtGridSize" );
	if( NULL != poptGridSize )
		gs = poptGridSize[0];

	if ( m_uDiceSize > gs) return TqFalse;
	if ( m_vDiceSize > gs) return TqFalse;

	return ( TqTrue );
}

/**
 * Determine if the topology surrounding the specified face is suitable for
 * conversion to a bicubic patch.
 */

TqBool CqSubdivision2::CanUsePatch( CqLath* pFace )
{
	// If the patch is a quad with each corner having valence 4, and no special features, 
	// we can just create a B-Spline patch.
	if( pFace->cQfv() != 4 )
		return( TqFalse );

	std::vector<CqLath*> aQff, aQfv;
	pFace->Qfv(aQfv);
	std::vector<CqLath*>::iterator iFV;
	for( iFV = aQfv.begin(); iFV!=aQfv.end(); iFV++ )
	{
		// Check if all vertices are valence 4.
		if( (*iFV)->cQvv() != 4 )
			return( TqFalse );

		// Check if all vertices smooth.
		if( EdgeSharpness((*iFV)) != 0.0f || 
		    CornerSharpness((*iFV)) != 0.0f )
			return( TqFalse );

		// Check if no internal boundaries.
		CqLath* pEnd = (*iFV)->cv();
		while( (*iFV) != pEnd )
		{
			if( NULL == pEnd )
				return( TqFalse );
			pEnd = pEnd->cv();
		}
	}

	// Check local neighbourhood of patch is 9 quads.
	pFace->Qff(aQff);
	if( aQff.size() != 9 )
		return( TqFalse );

	std::vector<CqLath*>::iterator iFF;
	for( iFF = aQff.begin(); iFF!=aQff.end(); iFF++ )
	{
		if( (*iFF)->cQfv() != 4 )
			return( TqFalse );

		std::vector<CqLath*> aQfv;
		(*iFF)->Qfv(aQfv);
		std::vector<CqLath*>::iterator iFV;
		for( iFV = aQfv.begin(); iFV!=aQfv.end(); iFV++ )
		{
			// Check if all vertices smooth.
			if( EdgeSharpness((*iFV)) != 0.0f ||
			    CornerSharpness((*iFV)) != 0.0f )
				return( TqFalse );
		}
	}

	return( TqTrue );
}


CqBound	CqSurfaceSubdivisionMesh::Bound() const
{
	CqBound B;
	if( NULL != m_pTopology && NULL != m_pTopology->pPoints() && NULL != m_pTopology->pPoints()->P() ) 
	{
		TqInt PointIndex;
		for( PointIndex = m_pTopology->pPoints()->P()->Size()-1; PointIndex >= 0; PointIndex-- )
			B.Encapsulate( m_pTopology->pPoints()->P()->pValue()[PointIndex] );
	}
	return(B);
}

TqInt CqSurfaceSubdivisionMesh::Split( std::vector<CqBasicSurface*>& aSplits )
{
	TqInt	CreatedPolys = 0;
	TqInt	face;
	// Guard
	AddRef();
	for ( face = 0; face < m_NumFaces; face++ )
	{
		// Don't add faces which are on the boundary, unless "interpolateboundary" is specified.
		if( ( !m_pTopology->pFacet( face )->isBoundaryFacet() ) || ( m_pTopology->isInterpolateBoundary() ) )
		{
			// Don't add "hole" faces
			if( !m_pTopology->isHoleFace( face ) )
			{
				// Add a patch surface to the bucket queue
				CqSurfaceSubdivisionPatch* pNew = new CqSurfaceSubdivisionPatch( m_pTopology, m_pTopology->pFacet( face ) );
				pNew->AddRef();
				aSplits.push_back( pNew );
				CreatedPolys++;
			}
		}
	}
	// !Guard
	Release();
	return( CreatedPolys );
}


END_NAMESPACE( Aqsis )
