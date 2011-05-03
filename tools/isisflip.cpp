#include "DataStorage/io_application.hpp"
#include "DataStorage/io_factory.hpp"

#include <map>
#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <boost/numeric/ublas/io.hpp>

using namespace isis;


int main( int argc, char **argv )
{	
	class : public data::Image::ChunkOp
	{
	public:
		data::dimensions dim;
		bool operator()( data::Chunk &ch, util::FixedVector<size_t, 4> posInImage ) {
			ch.swapAlong( dim );
		}
	}flifu;
	
	ENABLE_LOG( data::Runtime, util::DefaultMsgPrint, error );
	const size_t getBiggestVecElem( const util::fvector4 & vec );
	std::map<std::string, unsigned int> alongMap = boost::assign::map_list_of
			( "row", 0 ) ( "column", 1 ) ( "slice", 2 ) ( "x", 3 ) ( "y", 4 ) ( "z", 5 );
	data::IOApplication app( "isisflip", true, true );
	util::Selection along( "row,column,slice,x,y,z" );
	util::Selection flip( "image,space,both" );
	along.set( "x" );
	flip.set( "both" );
	app.parameters["image_center"] = bool();
	app.parameters["image_center"] = false;
	app.parameters["image_center"].needed() = false;
	app.parameters["image_center"].setDescription( "If activated the center of the image will be translated to the of the scanner space and after flipping back to its initial position" );
	app.parameters["along"] = along;
	app.parameters["along"].needed() = true;
	app.parameters["along"].setDescription( "Flip along the specified axis" );
	app.parameters["flip"] = flip;
	app.parameters["flip"].needed() = true;
	app.parameters["flip"].setDescription( "What has to be flipped" );
	app.init( argc, argv );
	std::list<data::Image> finImageList;
	unsigned int dim = alongMap[app.parameters["along"].toString()];
	//go through every image
	BOOST_FOREACH( data::Image & refImage, app.images ) {
		boost::numeric::ublas::matrix<float> T = boost::numeric::ublas::identity_matrix<float>( 3, 3 );
		if( dim > 2 ) {
			dim = refImage.mapScannerAxesToImageDimension( static_cast<data::scannerAxis>( dim - 3 ) );
		}
		T( dim, dim ) *= -1;
		data::Image newImage = refImage;

		if ( app.parameters["flip"].toString() == "image" || app.parameters["flip"].toString() == "both" ) {
			flifu.dim=static_cast<data::dimensions>( dim );
			refImage.foreachChunk( flifu );
		}

		if ( app.parameters["flip"].toString() == "both" || app.parameters["flip"].toString() == "space" ) {
			refImage.transformCoords( T, app.parameters["image_center"] );
		}

		finImageList.push_back( refImage );
	}
	app.autowrite( finImageList );
	return 0;
};
