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
		\brief Declares texture map handling and cacheing classes.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef TEXTUREMAP_H_INCLUDED
#define TEXTUREMAP_H_INCLUDED 1

#undef		min
#undef		max

#include	<vector>

#ifdef AQSIS_SYSTEM_MACOSX
#include	"macosx/valarray"
#else
#include	<valarray>
#endif

#include	"aqsis.h"

#include	"tiffio.h"

#include	"sstring.h"
#include	"color.h" 
#include	"matrix.h"
#include	"itexturemap.h"
#include	"ri.h"
#include	"ishaderexecenv.h"

START_NAMESPACE( Aqsis )

struct IqShader;
struct IqShaderData;

#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
#define	ZFILE_HEADER		"Aqsis ZFile" VERSION_STR
#else // AQSIS_SYSTEM_WIN32
#define ZFILE_HEADER "Aqsis ZFile" VERSION
#endif // !AQSIS_SYSTEM_WIN32

#define	LATLONG_HEADER	"Aqsis LatLong MIP MAP"
#define	CUBEENVMAP_HEADER	"Aqsis CubeFace Environment"
#define	SHADOWMAP_HEADER	"Shadow"
#define	MIPMAP_HEADER		"Aqsis MIP MAP"


enum EqBufferType
{
	BufferType_RGBA = 0,
	BufferType_Float,
};


//----------------------------------------------------------------------
/** \class CqTextureMapBuffer
 * Class referencing a buffer in the image map cache. 
 */

class CqTextureMapBuffer
{
	public:
		CqTextureMapBuffer() :
				m_pBufferData( 0 ),
				m_sOrigin( 0 ),
				m_tOrigin( 0 ),
				m_Width( 0 ),
				m_Height( 0 ),
				m_Samples( 0 ),
				m_Directory( 0 ),
				m_fProtected( TqFalse )
		{}
		virtual	~CqTextureMapBuffer()
		{
			Release();
		}

		/** Initialise the buffer reference to the specified format.
		 * \param xorigin Integer origin within the texture map.
		 * \param yorigin Integer origin within the texture map.
		 * \param width Width of the buffer segment.
		 * \param height Height of the buffer segment.
		 * \param samples Number of samples per pixel.
		 * \param directory The directory within the TIFF image map, used for multi image formats, i.e. cubeface environment map.
		 * \param fProt A flag indicating the buffer should be protected from removal by the cache management system.
		 */
		void	Init( TqUlong xorigin, TqUlong yorigin, TqUlong width, TqUlong height, TqInt samples, TqInt directory = 0, TqBool fProt = TqFalse )
		{
			Release();
			m_sOrigin = xorigin;
			m_tOrigin = yorigin;
			m_Width = width;
			m_Height = height;
			m_Samples = samples;
			m_Directory = directory;
			m_fProtected = fProt;

			m_pBufferData = AllocSegment( width, height, samples, m_fProtected );
		}
		/** Release this reference to the cache.
		 */
		void	Release()
		{
			if ( m_pBufferData != 0 ) FreeSegment( m_pBufferData, m_Width, m_Height, m_Samples );
			m_pBufferData = 0;
		}

		/** Determine if the specified sample point and directory index is within this buffer segment.
		 * \param s Horizontal sample position.
		 * \param t Vertical sample position.
		 * \param directory TIFF directory index.
		 * \return Boolean indicating sample is within this buffer.
		 */
		TqBool	IsValid( TqUlong s, TqUlong t, TqInt directory = 0 )
		{

			return (
			           s >= m_sOrigin && t >= m_tOrigin && s < m_sOrigin + m_Width && t < m_tOrigin + m_Height &&
			           directory == m_Directory );
		}

		/** Get a pointer to the data for this buffer segment.
		 */
		TqPuchar	pBufferData()
		{
			return ( m_pBufferData );
		}
		/** Get a pointer to the data for this buffer segment.
		 */
		void*	pVoidBufferData()
		{
			return ( m_pBufferData );
		}
		/** Get the size of a single element
		 */
		virtual TqInt	ElemSize()
		{
			return( m_Samples * sizeof(TqUchar) );
		}
		/** Get the type of the data in the buffer
		 */
		virtual EqBufferType	BufferType()
		{
			return( BufferType_RGBA );
		}

		/** Get the float value at the specified pixel/element (0.0 --> 1.0)
		 */
		virtual TqFloat	GetValue(TqInt x, TqInt y, TqInt sample)
		{
			TqInt iv = y * ( m_Width * ElemSize() );
			TqInt iu = x * ElemSize();
			return ( m_pBufferData[ iv + iu + sample ] / 256.0f );
		}
		/** Set the float value at the specified pixel/element (0.0 --> 1.0)
		 */
		virtual void	SetValue(TqInt x, TqInt y, TqInt sample, TqFloat value)
		{
			TqInt iv = y * ( m_Width * ElemSize() );
			TqInt iu = x * ElemSize();
			m_pBufferData[ iv + iu + sample ] = value * 256.0f;
		}
		/** Get the origin of this buffer segment.
		 */
		TqUlong sOrigin() const
		{
			return ( m_sOrigin );
		}
		/** Get the origin of this buffer segment.
		 */
		TqUlong tOrigin() const
		{
			return ( m_tOrigin );
		}
		/** Get the width of this buffer segment.
		 */
		TqUlong Width() const
		{
			return ( m_Width );
		}
		/** Get the height of this buffer segment.
		 */
		TqUlong Height() const
		{
			return ( m_Height );
		}
		/** Get the directory index of this buffer segment.
		 */
		TqInt	Directory() const
		{
			return ( m_Directory );
		}
		/** Get the number of samples per element.
		 */
		TqInt	Samples() const
		{
			return ( m_Samples );
		}
		/** Get the status of the protected flag
		 */
		TqBool	fProtected() const
		{
			return( m_fProtected );
		}
		/** Set this buffer as protected or not.
		 */
		void	SetfProtected( TqBool fProt = TqTrue )
		{
			m_fProtected = fProt;
		}


		TqPuchar AllocSegment( TqUlong width, TqUlong height, TqInt samples, TqBool fProt = TqFalse );
		void	FreeSegment( TqPuchar pBufferData, TqUlong width, TqUlong height, TqInt samples );

	protected:
		TqPuchar	m_pBufferData;	///< Pointer to the image data.
		TqUlong	m_sOrigin;		///< Horizontal segment origin.
		TqUlong	m_tOrigin;		///< Vertical segment origin.
		TqUlong	m_Width;		///< Width of segment.
		TqUlong	m_Height;		///< Height of segment.
		TqInt	m_Samples;		///< Number of samples per pixel.
		TqInt	m_Directory;	///< TIFF directory index. Used for multi image textures, i.e. cubeface environment.
		TqBool	m_fProtected;	///< Flag indicating if this buffer is protected from being automatically delted by the cache.

}
;


//----------------------------------------------------------------------
/** \class CqFloatTextureMapBuffer
 * Class referencing a buffer in the image map cache in floating point format. 
 */

class CqFloatTextureMapBuffer : public CqTextureMapBuffer
{
	public:
		CqFloatTextureMapBuffer() : CqTextureMapBuffer()
		{}
		virtual	~CqFloatTextureMapBuffer()
		{}

		virtual TqFloat	GetValue(TqInt x, TqInt y, TqInt sample)
		{
			TqInt iv = y * ( m_Width * ElemSize() );
			TqInt iu = x * ElemSize();
			return ( (reinterpret_cast<TqFloat*>(&m_pBufferData[ iv + iu ]))[sample] );
		}
		virtual void	SetValue(TqInt x, TqInt y, TqInt sample, TqFloat value)
		{
			TqInt iv = y * ( m_Width * ElemSize() );
			TqInt iu = x * ElemSize();
			(reinterpret_cast<TqFloat*>(&m_pBufferData[ iv + iu ]))[sample] = value;
		}
		virtual TqInt	ElemSize()
		{
			return( m_Samples * sizeof(TqFloat) );
		}
		/** Get the type of the data in the buffer
		 */
		virtual EqBufferType	BufferType()
		{
			return( BufferType_Float );
		}
}
;

//----------------------------------------------------------------------
/** \class CqShadowMapBuffer
 * Class referencing a depth buffer in the image map cache. 
 */

class CqShadowMapBuffer : public CqTextureMapBuffer
{
	public:
		CqShadowMapBuffer() : CqTextureMapBuffer()
		{}
		virtual	~CqShadowMapBuffer()
		{}

		virtual TqFloat	GetValue(TqInt x, TqInt y, TqInt sample)
		{
			TqInt iv = y * ( m_Width * ElemSize() );
			TqInt iu = x * ElemSize();
			return ( (reinterpret_cast<TqFloat*>(&m_pBufferData[ iv + iu ]))[sample] );
		}
		virtual void	SetValue(TqInt x, TqInt y, TqInt sample, TqFloat value)
		{
			TqInt iv = y * ( m_Width * ElemSize() );
			TqInt iu = x * ElemSize();
			(reinterpret_cast<TqFloat*>(&m_pBufferData[ iv + iu ]))[sample] = value;
		}
		virtual TqInt	ElemSize()
		{
			return( m_Samples * sizeof(TqFloat) );
		}
		/** Get the type of the data in the buffer
		 */
		virtual EqBufferType	BufferType()
		{
			return( BufferType_Float );
		}
}
;


//----------------------------------------------------------------------
/** \class CqTextureMap
 * Base class from which all texture maps are derived.
 */

class CqTextureMap : public IqTextureMap
{
	public:
		CqTextureMap( const CqString& strName ) :
				m_Compression( COMPRESSION_NONE ),
				m_Quality( 70 ),
				m_MinZ( RI_FLOATMAX ),
				m_XRes( 0 ),
				m_YRes( 0 ),
				m_PlanarConfig( PLANARCONFIG_CONTIG ),
				m_SamplesPerPixel( 3 ),
				m_Format( TexFormat_Plain ),
				m_strName( strName ),
				m_pImage( 0 ),
				m_IsValid( TqTrue ),
				m_smode( WrapMode_Black ),
				m_tmode( WrapMode_Black ),
				m_FilterFunc( RiBoxFilter ),
				m_swidth( 1.0 ), m_twidth( 1.0 )

		{
			m_tempval1.resize( m_SamplesPerPixel );
			m_tempval2.resize( m_SamplesPerPixel );
			m_tempval3.resize( m_SamplesPerPixel );
			m_tempval4.resize( m_SamplesPerPixel );
			m_low_color.resize( m_SamplesPerPixel );
			m_high_color.resize( m_SamplesPerPixel );
		}
		virtual	~CqTextureMap();

		/** Get/Set the mininum depth this texture (for any surfaces using it)
		 */
		TqFloat	MinZ() const
		{
			return ( m_MinZ );
		}
		void	SetMinZ( TqFloat minz )
		{
			if ( minz <= m_MinZ ) m_MinZ = minz;
		}

		/** Get the horizontal resolution of this image.
		 */
		virtual	TqUint	XRes() const
		{
			return ( m_XRes );
		}
		/** Get the vertical resolution of this image.
		 */
		virtual	TqUint	YRes() const
		{
			return ( m_YRes );
		}
		/** Get the number of samples per pixel.
		 */
		virtual	TqInt	SamplesPerPixel() const
		{
			return ( m_SamplesPerPixel );
		}
		/** Get the storage format of this image.
		 */
		virtual	EqTexFormat	Format() const
		{
			return ( m_Format );
		}

		virtual	TqInt Compression() const
		{
			return ( m_Compression );
		}

		virtual	void SetCompression( TqInt Compression )
		{
			m_Compression = Compression;
		}

		virtual	TqInt Quality() const
		{
			return ( m_Quality );
		}

		virtual	void SetQuality( TqInt Quality )
		{
			m_Quality = Quality;
		}

		CqMatrix& GetMatrix( TqInt which )
		{
			return ( m_matWorldToScreen );
		}

		/** Get the image type.
		 */
		virtual	EqMapType	Type() const
		{
			return ( IsValid() ? MapType_Texture : MapType_Invalid );
		}
		/** Open this image ready for reading.
		                         */

		/** Use the plugin to convert to a tif file any texture file provided
		**/
		virtual TqInt Convert	( CqString &strName );

		virtual	void	Open();
		/** Close this image file.
		 */
		virtual	void	Close();

		/** Determine if this image file is valid, i.e. has been found and opened successfully.
		 */
		bool	IsValid() const
		{
			return ( m_IsValid );
		}
		/** Set the flag indicating that this image has not been successfully opened.
		 */
		void	SetInvalid()
		{
			m_IsValid = TqFalse;
		}

		virtual	CqTextureMapBuffer*	GetBuffer( TqUlong s, TqUlong t, TqInt directory = 0, TqBool fProt = TqFalse );
		void	CreateMIPMAP( TqBool fProtectBuffers = TqFalse );
		virtual	CqTextureMapBuffer* CreateBuffer( TqUlong xorigin, TqUlong yorigin, TqUlong width, TqUlong height, TqInt directory = 0, TqBool fProt = TqFalse )
		{
			CqTextureMapBuffer* pRes;
			switch( m_SampleFormat )
			{
				case SAMPLEFORMAT_IEEEFP:
					pRes = new CqFloatTextureMapBuffer();
					break;
				
				case SAMPLEFORMAT_UINT:
				default:
					pRes = new CqTextureMapBuffer();
					break;
			}
			pRes->Init( xorigin, yorigin, width, height, m_SamplesPerPixel, directory, fProt );
			return( pRes );
		}

		virtual	void	SampleMap( TqFloat s1, TqFloat t1, TqFloat swidth, TqFloat twidth, std::valarray<TqFloat>& val, std::map<std::string, IqShaderData*>& paramMap );
		virtual	void	SampleMap( TqFloat s1, TqFloat t1, TqFloat s2, TqFloat t2, TqFloat s3, TqFloat t3, TqFloat s4, TqFloat t4,
		                                 std::valarray<TqFloat>& val, std::map<std::string, IqShaderData*>& paramMap );
		virtual	void	SampleMap( CqVector3D& R, CqVector3D& swidth, CqVector3D& twidth,
		                                 std::valarray<TqFloat>& val, std::map<std::string, IqShaderData*>& paramMap )
		{}
		virtual	void	SampleMap( CqVector3D& R1, CqVector3D& R2, CqVector3D& R3, CqVector3D& R4,
		                                 std::valarray<TqFloat>& val, std::map<std::string, IqShaderData*>& paramMap )
		{}

		virtual	void	GetSample( TqFloat ss1, TqFloat tt1, TqFloat ss2, TqFloat tt2, std::valarray<TqFloat>& val );

		static	CqTextureMap* GetTextureMap( const CqString& strName );
		static	CqTextureMap* GetEnvironmentMap( const CqString& strName );
		static	CqTextureMap* GetShadowMap( const CqString& strName );
		static	CqTextureMap* GetLatLongMap( const CqString& strName );

		void	ImageFilterVal( CqTextureMapBuffer* pData, TqInt x, TqInt y, TqInt directory, std::vector<TqFloat>& accum );

		void Interpreted( TqPchar mode );

		/** Clear the cache of texture maps.
		 */
		static	void	FlushCache()
		{
			std::vector<CqTextureMap*>::iterator i;
			while ( ( i = m_TextureMap_Cache.begin() ) != m_TextureMap_Cache.end() )
				delete( *i );

			m_TextureMap_Cache.clear();

		}
		void CriticalMeasure();

		static void WriteTileImage( TIFF* ptex, CqTextureMapBuffer* pBuffer, TqUlong twidth, TqUlong theight, TqInt compression, TqInt quality );
		static void WriteImage( TIFF* ptex, CqTextureMapBuffer* pBuffer, TqInt compression, TqInt quality );
		static void WriteImage( TIFF* ptex, TqFloat *raster, TqUlong width, TqUlong length, TqInt samples, TqInt compression, TqInt quality );
		static void WriteTileImage( TIFF* ptex, TqFloat *raster, TqUlong width, TqUlong length, TqUlong twidth, TqUlong tlength, TqInt samples, TqInt compression, TqInt quality );
		static void WriteImage( TIFF* ptex, TqPuchar raster, TqUlong width, TqUlong length, TqInt samples, TqInt compression, TqInt quality );
		static void WriteTileImage( TIFF* ptex, TqPuchar raster, TqUlong width, TqUlong length, TqUlong twidth, TqUlong tlength, TqInt samples, TqInt compression, TqInt quality );



	protected:
		static	std::vector<CqTextureMap*>	m_TextureMap_Cache;	///< Static array of loaded textures.
		static std::vector<CqString*>	m_ConvertString_Cache; ///< Static array of filename (after conversion)

		TqInt m_Compression;            ///< TIFF Compression model
		TqInt m_Quality;                ///< If Jpeg compression is used than its overall quality

		TqFloat m_MinZ;                 ///< Minimum Depth
		TqUint	m_XRes;					///< Horizontal resolution.
		TqUint	m_YRes;					///< Vertical resolution.
		TqInt	m_PlanarConfig;			///< TIFF planar configuration type.
		TqInt	m_SamplesPerPixel;		///< Number of samples per pixel.
		TqInt	m_SampleFormat;			///< Format of the sample elements, i.e. RGBA, or IEEE

		EqTexFormat	m_Format;			///< Image storage format type.

		CqString	m_strName;			///< Name of the image.
		TIFF*	m_pImage;			///< Pointer to an opened TIFF image.
		TqBool	m_IsValid;			///< Indicate whether this image has been successfully opened.
		enum EqWrapMode m_smode;        ///< Periodic, black, clamp
		enum EqWrapMode m_tmode;        ///< Periodic, black, clamp
		RtFilterFunc m_FilterFunc;       ///< Catmull-Rom, sinc, disk, ... pixelfilter
		TqFloat m_swidth, m_twidth;   ///< for the pixel's filter
		std::vector<CqTextureMapBuffer*>	m_apSegments;	///< Array of cache segments related to this image.

		CqMatrix	m_matWorldToScreen;		///< Matrix to convert points from world space to screen space.

		// Temporary values used during sampling.
		std::valarray<TqFloat>	m_tempval1;
		std::valarray<TqFloat>	m_tempval2;
		std::valarray<TqFloat>	m_tempval3;
		std::valarray<TqFloat>	m_tempval4;
		std::valarray<TqFloat>	m_low_color;
		std::valarray<TqFloat>	m_high_color;
}
;


//----------------------------------------------------------------------
/** \class CqEnvironmentMap
 * Environment map, derives from texture map and handles converting reflection
 * vector to s,t coordinates.
 */

class CqEnvironmentMap : public CqTextureMap
{
	public:
		CqEnvironmentMap( const CqString& strName ) :
				CqTextureMap( strName )
		{}
		virtual	~CqEnvironmentMap()
		{}

		virtual	EqMapType	Type() const
		{
			return ( IsValid() ? MapType_Environment : MapType_Invalid );
		}

		virtual	void	SampleMap( CqVector3D& R, CqVector3D& swidth, CqVector3D& twidth,
		                                 std::valarray<TqFloat>& val, std::map<std::string, IqShaderData*>& paramMap );
		virtual	void	SampleMap( CqVector3D& R1, CqVector3D& R2, CqVector3D& R3, CqVector3D& R4,
		                                 std::valarray<TqFloat>& val, std::map<std::string, IqShaderData*>& paramMap );

		CqMatrix& CqEnvironmentMap::GetMatrix( TqInt which )
		{

			return ( m_matWorldToScreen );
		}


	private:
		void	Getst( CqVector3D& R, TqUlong fullwidth, TqUlong fulllength, TqFloat& s, TqFloat& t );
		CqMatrix	m_matWorldToScreen;		///< Matrix to convert points from world space to screen space.

}
;

//----------------------------------------------------------------------
/** \class CqLatLongMap
 * Environment map, derives from texture map and handles converting reflection
 * vector to s,t coordinates.
 */

class CqLatLongMap : public CqEnvironmentMap
{
	public:
		CqLatLongMap( const CqString& strName ) :
				CqEnvironmentMap( strName )
		{}
		virtual	~CqLatLongMap()
		{}

		virtual	EqMapType	Type() const
		{
			return ( IsValid() ? MapType_LatLong : MapType_Invalid );
		}


};


//----------------------------------------------------------------------
/** \class CqShadowMap
 * Shadow map, derives from texture map.
 */

class CqShadowMap : public CqTextureMap
{
	public:
		CqShadowMap( const CqString& strName ) :
				CqTextureMap( strName )
		{}
		virtual	~CqShadowMap()
		{}

		virtual	EqMapType	Type() const
		{
			return ( IsValid() ? MapType_Shadow : MapType_Invalid );
		}

		/** Get the matrix used to convert points from work into camera space.
		 */
		CqMatrix&	matWorldToCamera()
		{
			return ( m_matWorldToCamera );
		}
		/** Get the matrix used to convert points from work into screen space.
		 */
		CqMatrix&	matWorldToScreen()
		{
			return ( m_matWorldToScreen );
		}



		void	AllocateMap( TqInt XRes, TqInt YRes );
		TqFloat	Sample( const CqVector3D&	vecPoint );
		void	SaveZFile();
		void	LoadZFile();
		void	SaveShadowMap( const CqString& strShadowName );
		void	ReadMatrices();

		virtual	CqTextureMapBuffer* CreateBuffer( TqUlong xorigin, TqUlong yorigin, TqUlong width, TqUlong height, TqInt directory = 0, TqBool fProt = TqFalse )
		{
			CqTextureMapBuffer* pRes = new CqShadowMapBuffer();
			pRes->Init( xorigin, yorigin, width, height, m_SamplesPerPixel, directory, fProt );
			return( pRes );
		}

		virtual	void	SampleMap( CqVector3D& R, CqVector3D& swidth, CqVector3D& twidth, std::valarray<TqFloat>& val, std::map<std::string, IqShaderData*>& paramMap );
		virtual	void	SampleMap( CqVector3D& R1, CqVector3D& R2, CqVector3D& R3, CqVector3D& R4, std::valarray<TqFloat>& val, std::map<std::string, IqShaderData*>& paramMap );
		CqMatrix& CqShadowMap::GetMatrix( TqInt which )
		{
			if ( which == 0 ) return m_matWorldToCamera;
			else if ( which == 1 ) return m_matWorldToScreen;
			return ( m_matWorldToCamera );
		}


	private:
		static	TqInt	m_rand_index;			///< Static random number table index.
		static	TqFloat	m_aRand_no[ 256 ];		///< Random no. table used for jittering the shadow sampling.

		CqMatrix	m_matWorldToCamera;		///< Matrix to convert points from world space to light space.
		CqMatrix	m_matWorldToScreen;		///< Matrix to convert points from world space to screen space.
}
;


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !TEXTUREMAP_H_INCLUDED

