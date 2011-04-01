#include "GLOrientationHandler.hpp"


namespace isis
{
namespace viewer
{

const unsigned short GLOrientationHandler::textureSize = 4;

util::fvector4 GLOrientationHandler::transformVectorWithImageOrientation( const isis::viewer::ImageHolder &image, util::fvector4 vector )
{
	boost::numeric::ublas::matrix<float> vectorAsMatrix = boost::numeric::ublas::zero_matrix<float>( textureSize, 1 );
	boost::numeric::ublas::matrix<float> retCoords = boost::numeric::ublas::zero_matrix<float>( textureSize, 1 );
	vectorAsMatrix( 0, 0 ) = vector[0];
	vectorAsMatrix( 1, 0 ) = vector[1];
	vectorAsMatrix( 2, 0 ) = vector[2];
	retCoords = boost::numeric::ublas::prod( image.getNormalizedImageOrientation(), vectorAsMatrix );
	util::fvector4 retVec;
	retVec[0] = retCoords( 0, 0 );
	retVec[1] = retCoords( 1, 0 );
	retVec[2] = retCoords( 2, 0 );
	return retVec;
}

util::fvector4 GLOrientationHandler::transformVectorWithImageAndPlaneOrientation( const isis::viewer::ImageHolder &image, util::fvector4 vector, GLOrientationHandler::PlaneOrientation orientation )
{
	//first we have to transform the voxels to our physical image space
	util::fvector4 imageOrientedVector = GLOrientationHandler::transformVectorWithImageOrientation( image, vector );
	GLOrientationHandler::MatrixType tmpVector = boost::numeric::ublas::zero_matrix<float>( textureSize, 1 );

	for ( size_t i = 0; i < 3; i++ ) {
		tmpVector( i, 0 ) = imageOrientedVector[i];
	}

	//now we have to transform these coords to our respective plane view
	GLOrientationHandler::MatrixType planeOrientationMatrix =
		GLOrientationHandler::transformToView( boost::numeric::ublas::identity_matrix<float>( textureSize, textureSize ), orientation );
	GLOrientationHandler::MatrixType transformedImageVoxels = boost::numeric::ublas::prod( planeOrientationMatrix, tmpVector );
	util::fvector4 retVec = util::fvector4( transformedImageVoxels( 0, 0 ), transformedImageVoxels( 1, 0 ), transformedImageVoxels( 2, 0 ) );
	return retVec;
}

util::FixedVector<float, 3> GLOrientationHandler::getNormalizedScaling( const ImageHolder &image )
{
	util::FixedVector<float, 3> retScaling;
	util::fvector4 size = image.getImageSize();
	util::fvector4 scaling = image.getPropMap().getPropertyAs<util::fvector4>( "voxelSize" );
	util::fvector4 voxelGap = image.getPropMap().getPropertyAs<util::fvector4>( "voxelGap" );
	util::fvector4 physicalSize;

	for ( size_t index = 0; index < 3; index++ ) {
		physicalSize[index] = size[index] * ( scaling[index] + voxelGap[index] );
	}

	size_t biggestExtent = getBiggestVecElem<float>( physicalSize );

	for ( size_t index = 0; index < 3; index++ ) {
		retScaling[index] = 1.0 / physicalSize[biggestExtent] * physicalSize[index];
	}

	return retScaling;
}

GLOrientationHandler::GLCoordinates GLOrientationHandler::transformImageCoords2GLCoords( const isis::util::ivector4 imageCoords, const ImageHolder &image, const ViewPortCoords viewport, PlaneOrientation orientation )
{
#warning function GLOrientationHandler::transformImageCoords2GLCoords has to be optimized
	using namespace boost::numeric::ublas;
	MatrixType orientationMatrix = getOrientationMatrix( image, orientation );
	MatrixType vector = zero_matrix<float>( 4, 1 );
	MatrixType oneVector = zero_matrix<float>( 4, 1 );
	oneVector( 0, 0 ) = 1;
	oneVector( 1, 0 ) = 1;
	oneVector( 2, 0 ) = 1;
	oneVector( 3, 0 ) = 1;
	vector( 0, 0 ) = imageCoords[0];
	vector( 1, 0 ) = imageCoords[1];
	vector( 2, 0 ) = imageCoords[2];
	MatrixType transformedVector = prod( getOrientationMatrix( image, orientation, false ), vector ) ;
	util::fvector4 transformedImageSize =
		transformVectorWithImageAndPlaneOrientation( image, image.getImageSize(), orientation );
	MatrixType transformedScaling = prod( orientationMatrix, oneVector );
	float vpos_x = transformedScaling( 0, 0 ) < 0 ? abs( transformedImageSize[0] ) + transformedVector( 0, 0 ) - 0.5 : transformedVector( 0, 0 ) + 0.5;
	float vpos_y = transformedScaling( 1, 0 ) < 0 ? abs( transformedImageSize[1] ) + transformedVector( 1, 0 ) - 0.5 : transformedVector( 1, 0 ) + 0.5;
	size_t vpos_z = transformedScaling( 2, 0 ) < 0 ? abs( transformedImageSize[2] ) + transformedVector( 2, 0 ) - 1 : transformedVector( 2, 0 );
	float offset_x = ( ( viewport.w - ( viewport.w * fabs( transformedScaling( 0, 0 ) ) ) ) / 2 ) + viewport.x;
	float offset_y = ( ( viewport.h - ( viewport.h * fabs( transformedScaling( 1, 0 ) ) ) ) / 2 ) + viewport.y;
	size_t pos_x = ( float )( vpos_x  * viewport.w * fabs( transformedScaling( 0, 0 ) ) ) / fabs( transformedImageSize[0] ) + offset_x;
	size_t pos_y = ( float )( vpos_y * viewport.h * fabs( transformedScaling( 1, 0 ) ) ) / fabs( transformedImageSize[1] ) + offset_y;
	float textureMatrix[16];
	boostMatrix2Pointer( orientation2TextureMatrix( orientationMatrix ), textureMatrix );

	//now we have to get the normalized slice
	util::fvector4 sliceScalingVector = util::fvector4( textureMatrix[8], textureMatrix[9], textureMatrix[10], 0 );
	util::fvector4 textureTranslationVec = util::fvector4( textureMatrix[12], textureMatrix[13], textureMatrix[14], 0 );
	size_t relevantSliceDim = getBiggestVecElem<float>( sliceScalingVector );
	float oneHalfSlice = 1.0 / transformedImageSize[2] / 2;
	float slice = ( 1.0 / transformedImageSize[2] ) * vpos_z;
	slice -= fabs( textureTranslationVec[relevantSliceDim] ) >= 1 ?  textureTranslationVec[relevantSliceDim] - 1 : textureTranslationVec[relevantSliceDim];
	slice *= 1.0 / sliceScalingVector[relevantSliceDim];
	slice += sliceScalingVector[relevantSliceDim] < 0 ? -oneHalfSlice : oneHalfSlice;

	GLCoordinates retCoords;
	retCoords.x = pos_x;
	retCoords.y = pos_y;
	retCoords.slice = slice;
	return retCoords;
}

util::ivector4 GLOrientationHandler::transformGLCoords2ImageCoords( isis::viewer::GLOrientationHandler::GLCoordinates glCoords, const ImageHolder &image, ViewPortCoords viewport, PlaneOrientation orientation )
{
#warning function GLOrientationHandler::transformGLCoords2ImageCoords has to be optimized
	using namespace boost::numeric::ublas;
	MatrixType imageSize = zero_matrix<float> ( 4, 1 );

	for ( size_t i = 0; i < 3; i++ ) {
		imageSize( i, 0 ) = image.getImageSize()[i];
	}

	MatrixType oneVector = zero_matrix<float>( 4, 1 );
	oneVector( 0, 0 ) = 1;
	oneVector( 1, 0 ) = 1;
	oneVector( 2, 0 ) = 1;
	oneVector( 3, 0 ) = 1;
	MatrixType transformedScaling = prod( getOrientationMatrix( image, orientation ), oneVector );
	//get the transposed imagesize vector for the current planeView
	MatrixType transformedImageSize = prod(  getOrientationMatrix( image, orientation, false ) , imageSize );
	float textureMatrix[16];
	boostMatrix2Pointer( orientation2TextureMatrix( getOrientationMatrix( image, orientation ) ), textureMatrix );
	util::fvector4 sliceScalingVector = util::fvector4( textureMatrix[8], textureMatrix[9], textureMatrix[10], 0 );
	util::fvector4 textureTranslationVec = util::fvector4( textureMatrix[12], textureMatrix[13], textureMatrix[14], 0 );
	size_t relevantSliceDim = getBiggestVecElem<float>( sliceScalingVector );
	float oneHalfSlice = 1.0 / transformedImageSize( 2, 0 ) / 2;
	glCoords.slice += oneHalfSlice;
	glCoords.slice *= sliceScalingVector[relevantSliceDim];
	glCoords.slice += fabs( textureTranslationVec[relevantSliceDim] ) >= 1 ?  textureTranslationVec[relevantSliceDim] - 1 : textureTranslationVec[relevantSliceDim];
	
	size_t vpos_z = glCoords.slice < 0 ? abs( transformedImageSize( 2, 0 ) ) - fabs( glCoords.slice ) * abs( transformedImageSize( 2, 0 ) )
					- 1 : glCoords.slice * abs( transformedImageSize( 2, 0 ) ) - 1;
					
	float border_x = ( viewport.w - ( viewport.w * fabs( transformedScaling( 0, 0 ) ) ) )  / 2;
	float border_y = ( viewport.h - ( viewport.h * fabs( transformedScaling( 1, 0 ) ) ) )  / 2;
	if ( ( glCoords.x < (viewport.x + border_x) ) ) {
		glCoords.x = viewport.x + border_x;
	} else if ( glCoords.x >= (viewport.x + viewport.w - border_x) ) {
		glCoords.x = viewport.x + viewport.w - border_x;
	}	
	size_t vpos_x = ( abs( transformedImageSize( 0, 0 ) ) / ( viewport.w * fabs( transformedScaling( 0, 0 ) ) ) ) * ( glCoords.x - viewport.x - border_x );
	if ( ( glCoords.y < (viewport.y + border_y) ) ) {
		glCoords.y = viewport.y + border_y;
	} else if ( glCoords.y >= (viewport.y + viewport.h - border_y) ) {
		glCoords.y = viewport.y + viewport.h - border_y;
	}	
	size_t vpos_y = ( abs( transformedImageSize( 1, 0 ) ) / ( viewport.w * fabs( transformedScaling( 1, 0 ) ) ) ) * ( glCoords.y - viewport.y - border_y );
	
	vpos_x = transformedImageSize( 0, 0 ) < 0 ? abs( transformedImageSize( 0, 0 ) ) - vpos_x - 1 : vpos_x;
	vpos_y = transformedImageSize( 1, 0 ) < 0 ? abs( transformedImageSize( 1, 0 ) ) - vpos_y - 1 : vpos_y;
	MatrixType retCoords = zero_matrix<float>( 4, 1 );
	retCoords( 0, 0 ) = vpos_x;
	retCoords( 1, 0 ) = vpos_y;
	retCoords( 2, 0 ) = vpos_z;
	MatrixType transposedRetCoords = prod( transposeMatrix( getOrientationMatrix( image, orientation, false ) ), retCoords );
	return util::ivector4( abs( transposedRetCoords( 0, 0 ) ), abs( transposedRetCoords( 1, 0 ) ), abs( transposedRetCoords( 2, 0 ) ) );
}


GLOrientationHandler::MatrixType GLOrientationHandler::getOrientationMatrix( const ImageHolder &image, PlaneOrientation orientation, bool scaling )
{
	MatrixType retMat = image.getNormalizedImageOrientation();

	if ( scaling ) {
		MatrixType scaleMatrix = boost::numeric::ublas::identity_matrix<float>( textureSize, textureSize );
		util::FixedVector<float, 3> scaling = getNormalizedScaling( image );
		scaleMatrix( 0, 0 ) = scaling[0];
		scaleMatrix( 1, 1 ) = scaling[1];
		scaleMatrix( 2, 2 ) = scaling[2];
		retMat = boost::numeric::ublas::prod( retMat, scaleMatrix );
	}

	retMat = transformToView( retMat, orientation );
	return retMat;
}


GLOrientationHandler::MatrixType GLOrientationHandler::transformToView( GLOrientationHandler::MatrixType origMatrix, PlaneOrientation orientation )
{
	MatrixType transformMatrix = boost::numeric::ublas::identity_matrix<float>( textureSize, textureSize );

	switch ( orientation ) {
	case axial:
		/*setup axial matrix
		*-1  0  0
		* 0 -1  0
		* 0  0  1
		*/
		transformMatrix( 0, 0 ) = -1;
		transformMatrix( 1, 1 ) = -1;
		return boost::numeric::ublas::prod( transformMatrix, origMatrix );
		break;
	case sagittal:
		/*setup sagittal matrix
		* 0  1  0
		* 0  0  1
		* 1  0  0
		*/
		transformMatrix( 0, 0 ) = 0;
		transformMatrix( 2, 0 ) = 1;
		transformMatrix( 0, 1 ) = 1;
		transformMatrix( 2, 2 ) = 0;
		transformMatrix( 1, 2 ) = 1;
		transformMatrix( 1, 1 ) = 0;
		return boost::numeric::ublas::prod( transformMatrix, origMatrix );
		break;
	case coronal:
		/*setup coronal matrix
		* -1  0  0
		*  0  0  1
		*  0  1  0
		*/

		transformMatrix( 0, 0 ) = -1;
		transformMatrix( 1, 1 ) = 0;
		transformMatrix( 2, 2 ) = 0;
		transformMatrix( 2, 1 ) = 1;
		transformMatrix( 1, 2 ) = 1;
		return boost::numeric::ublas::prod( transformMatrix, origMatrix );
		break;
	}
}

void GLOrientationHandler::boostMatrix2Pointer( MatrixType boostMatrix, float *ret )
{
	size_t index = 0;

	for ( size_t i = 0; i < textureSize; i++ ) {
		for ( size_t j = 0; j < textureSize; j++ ) {
			ret[index++] = boostMatrix( i, j );
		}
	}
}

GLOrientationHandler::MatrixType GLOrientationHandler::orientation2TextureMatrix( const GLOrientationHandler::MatrixType &origMatrix )
{
	MatrixType retMat = boost::numeric::ublas::zero_matrix<float>( 4, 4 );

	for ( size_t i = 0; i < textureSize; i++ ) {
		for ( size_t j = 0; j < textureSize; j++ ) {
			retMat( i, j ) = origMatrix( i, j ) ? 1.0 / origMatrix( i, j ) : 0;

		}
	}

	//add the offsets
	for ( size_t i = 0; i < 3; i++ ) {
		for ( size_t j = 0; j < 3; j++ ) {
			if( retMat( i, j ) < 0 ) {
				retMat( 3, j ) = 1 + ( fabs( retMat( i, j ) ) - 1 ) / 2;
			} else if ( retMat( i, j ) > 0 ) {
				retMat( 3, j ) = -1 * ( fabs( retMat( i, j ) ) - 1 ) / 2;
			}
		}
	}

	return retMat;
}



GLOrientationHandler::ViewPortCoords GLOrientationHandler::calculateViewPort( size_t w, size_t h )
{
	ViewPortCoords retCoords;
	float scaleH = ( w < h ) ? ( ( float )w / ( float )h ) : 1;
	float scaleW = ( h < w ) ? ( ( float )h / ( float )w ) : 1;

	retCoords.h = h * scaleH;
	retCoords.w = w * scaleW;
	retCoords.x = ( w - ( w * scaleW ) ) / 2;
	retCoords.y = ( h - ( h * scaleH ) ) / 2;
	return retCoords;
}


}
} // end namespace