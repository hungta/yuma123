// ---------------------------------------------------------------------------|
// Test Harness includes
// ---------------------------------------------------------------------------|
#include "test/support/db-models/device-test-db.h"
#include "test/support/misc-util/ptree-utils.h"

// ---------------------------------------------------------------------------|
// Standard includes
// ---------------------------------------------------------------------------|
#include <iostream>

// ---------------------------------------------------------------------------|
// Boost includes
// ---------------------------------------------------------------------------|
#include <boost/foreach.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/phoenix/core.hpp>
#include <boost/phoenix/bind.hpp>
#include <boost/phoenix/operator.hpp> 
#include <boost/phoenix/fusion/at.hpp> 
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/tuple.hpp>
#include <boost/algorithm/string/predicate.hpp>

// ---------------------------------------------------------------------------|
// Boost includes
// ---------------------------------------------------------------------------|
#include <boost/test/unit_test.hpp>

// ---------------------------------------------------------------------------|
// File wide namespace use
// ---------------------------------------------------------------------------|
using namespace std;
using boost::property_tree::ptree;

namespace ph = boost::phoenix;
namespace ph_arg = boost::phoenix::arg_names;

// ---------------------------------------------------------------------------|
namespace 
{

// ---------------------------------------------------------------------------|
// TODO: Move to utility and make checkEqual function configurable
// TODO: possible by making this a functor...
template <class T>
void MapZipContainerHelper( const boost::tuple< const T&, const T& >& zIter )
{
    BOOST_CHECK_EQUAL( zIter.get<0>().first, zIter.get<1>().first );
    checkEqual( zIter.get<0>().second, zIter.get<1>().second );
}

// ---------------------------------------------------------------------------|
template <class T>
void CheckMaps( const T& lhs, const T& rhs )
{
    BOOST_CHECK_EQUAL( lhs.size(), rhs.size() );

    auto begZip = boost::make_zip_iterator( boost::make_tuple( lhs.begin(), rhs.begin() ) );
    auto endZip = boost::make_zip_iterator( boost::make_tuple( lhs.end(), rhs.end() ) );

    typedef typename T::value_type value_type;
    for_each( begZip, endZip, 
            ph::bind( &MapZipContainerHelper<value_type>, ph_arg::arg1 ) );
    
}

// ---------------------------------------------------------------------------|
void UnpackConnectionItem( const ptree::value_type& connItem,
                           std::map<uint32_t, YumaTest::ConnectionItem>& connMap ) 
{
    YumaTest::ConnectionItem conn( connItem.second );
    
    uint16_t connId = connItem.second.get<uint16_t>( "id" );
    BOOST_REQUIRE_MESSAGE( 
            connMap.end() == 
            connMap.find( connId ),
            "ERROR Duplicate connection item found! ID: " << connId );
    
    connMap.insert( make_pair( connId, conn ) );
}

} // anonymous namespace

// ---------------------------------------------------------------------------|
namespace YumaTest
{

// ===========================================================================|
ConnectionItem::ConnectionItem() : sourceId_(0)
                                 , sourcePinId_(0)
                                 , destinationId_(0)
                                 , destinationPinId_(0)
                                 , bitrate_(0)
{}

// ---------------------------------------------------------------------------|
ConnectionItem::ConnectionItem( const boost::property_tree::ptree& pt )
    : sourceId_(0)
    , sourcePinId_(0)
    , destinationId_(0)
    , destinationPinId_(0)
    , bitrate_(0)
{
    BOOST_FOREACH( const ptree::value_type& v, pt )
    {
        if ( v.first == "sourceId" )
        {
            sourceId_ = v.second.get_value<uint32_t>();
        }
        else if ( v.first == "sourcePinId" )
        {
            sourcePinId_ = v.second.get_value<uint32_t>();
        }
        else if ( v.first == "destinationId" )
        {
            destinationId_ = v.second.get_value<uint32_t>();
        }
        else if ( v.first == "destinationPinId" )
        {
            destinationPinId_ = v.second.get_value<uint32_t>();
        }
        else if ( v.first == "bitrate" )
        {
            bitrate_ = v.second.get_value<uint32_t>();
        }
        else if ( v.first != "id" )
        {
            BOOST_FAIL( "Unsupported child for ResourceNode: " << v.first );
        }
    }
}

// ---------------------------------------------------------------------------|
void checkEqual( const ConnectionItem& lhs, const ConnectionItem& rhs )
{
    BOOST_CHECK_EQUAL( lhs.sourceId_, rhs.sourceId_ );
    BOOST_CHECK_EQUAL( lhs.sourcePinId_, rhs.sourcePinId_ );
    BOOST_CHECK_EQUAL( lhs.destinationId_, rhs.destinationId_ );
    BOOST_CHECK_EQUAL( lhs.destinationPinId_, rhs.destinationPinId_ );
    BOOST_CHECK_EQUAL( lhs.bitrate_, rhs.bitrate_ );
}

// ===========================================================================|
ResourceNode::ResourceNode() : id_(0)
                             , resourceType_(0)
                             , physicalPath_()
{}

// ---------------------------------------------------------------------------|
ResourceNode::ResourceNode( const boost::property_tree::ptree& pt )
    : id_(0)
    , resourceType_()
    , physicalPath_()
{
    BOOST_FOREACH( const ptree::value_type& v, pt )
    {
        if ( v.first == "id" )
        {
            id_ = v.second.get_value<uint16_t>();
        }
        else if ( v.first == "resourceType" )
        {
            resourceType_ = v.second.get_value<uint32_t>();
        }
        else if ( v.first == "physicalPath" )
        {
            physicalPath_ = v.second.get_value<string>();
        }
        else
        {
            BOOST_FAIL( "Unsupported child for ResourceNode: " << v.first );
        }
    }
}

// ---------------------------------------------------------------------------|
void checkEqual( const ResourceNode& lhs, const ResourceNode& rhs )
{
    BOOST_CHECK_EQUAL( lhs.id_, rhs.id_ );
    BOOST_CHECK_EQUAL( lhs.resourceType_, rhs.resourceType_ );
    BOOST_CHECK_EQUAL( lhs.physicalPath_, rhs.physicalPath_ );
}

// ===========================================================================|
StreamItem::StreamItem() : id_(0)
                         , resourceDescription_()
                         , virtualResourceConnections_()
{}

// ---------------------------------------------------------------------------|
StreamItem::StreamItem( const boost::property_tree::ptree& pt )
    : id_(0)
    , resourceDescription_()
    , virtualResourceConnections_()
{

    BOOST_FOREACH( const ptree::value_type& v, pt )
    {
        if ( v.first == "id" )
        {
            id_ = v.second.get_value<uint16_t>();
        }
        else if ( v.first == "resourceNode" )
        {
            ResourceNode res( v.second );

            BOOST_REQUIRE_MESSAGE( 
                    resourceDescription_.end() 
                    == resourceDescription_.find( res.id_ ),
                    "ERROR Duplicate Resource found! stream ( " << id_ 
                    << ") resource ( " << res.id_ << " )" );

            resourceDescription_.insert( make_pair( res.id_, res ) );
        }
        else if ( v.first == "connection" )
        {
            UnpackConnectionItem( v, virtualResourceConnections_ );
        }
        else
        {
            BOOST_FAIL( "Unsupported child for stream: " << v.first );
        }
    }
}

// ---------------------------------------------------------------------------|
void checkEqual( const StreamItem& lhs, const StreamItem& rhs )
{
    BOOST_TEST_MESSAGE( "Checking StreamItem match..." );

    BOOST_TEST_MESSAGE( "Comparing Stream Id ... " << lhs.id_ );
    BOOST_CHECK_EQUAL( lhs.id_,  rhs.id_ );

    BOOST_TEST_MESSAGE( "Comparing Stream Resource Descriptions ..." );
    CheckMaps( lhs.resourceDescription_, rhs.resourceDescription_ );

    BOOST_TEST_MESSAGE( "Comparing Stream Virtual Connection Resources ..." );
    CheckMaps( lhs.virtualResourceConnections_, rhs.virtualResourceConnections_ );
}

// ===========================================================================|
Profile::Profile() 
    : id_(0)
    , streams_()
    , streamConnections_()
{}


// ===========================================================================|
Profile::Profile( const boost::property_tree::ptree& pt)
    : id_(0)
    , streams_()
    , streamConnections_()
{
    BOOST_FOREACH( const ptree::value_type& v, pt )
    {
        if ( v.first == "id" )
        {
            id_ = v.second.get_value<uint16_t>();
        }
        else if ( v.first == "stream" )
        {
            StreamItem str( v.second );

            BOOST_REQUIRE_MESSAGE( streams_.end() == streams_.find( str.id_ ),
                    "ERROR Duplicate Stream found! profile ( " << id_ 
                    << ") stream ( " << str.id_ << " )" );
            streams_.insert( make_pair( str.id_, str ) );
        }
        else if ( v.first == "streamConnection" )
        {
            UnpackConnectionItem( v, streamConnections_ );
        }
        else
        {
            BOOST_FAIL( "Unsupported child for profile: " << v.first );
        }
    }
}

// ---------------------------------------------------------------------------|
void checkEqual( const Profile& lhs, const Profile& rhs )
{
    BOOST_TEST_MESSAGE( "Checking Profile match..." );

    BOOST_TEST_MESSAGE( "Comparing Profile Id ..." << lhs.id_ );
    BOOST_CHECK_EQUAL( lhs.id_,  rhs.id_ );

    BOOST_TEST_MESSAGE( "Comparing Streams List ..." );
    CheckMaps( lhs.streams_, rhs.streams_ );

    BOOST_TEST_MESSAGE( "Comparing Stream Connections List ..." );
    CheckMaps( lhs.streamConnections_, rhs.streamConnections_ );
}

// ===========================================================================|
XPO3Container::XPO3Container() : activeProfile_(0)
                               , profiles_()
{}

// ---------------------------------------------------------------------------|
XPO3Container::XPO3Container( const ptree& pt) : activeProfile_(0)
                                               , profiles_()
{
    const ptree& xpo = pt.get_child( "data.xpo", ptree() );

    BOOST_FOREACH( const ptree::value_type& v, xpo )
    {
        if ( v.first == "profile" )
        {
            Profile prof( v.second ); 

            BOOST_REQUIRE_MESSAGE( profiles_.end() == profiles_.find( prof.id_ ),
                    "ERROR Duplicate Profile with id( " << prof.id_ << " )\n" );
            profiles_.insert( std::make_pair( prof.id_, prof ) );
        }
        else if ( v.first == "activeProfile" )
        {
            activeProfile_ = v.second.get_value<uint16_t>();
        }
        else if ( !boost::starts_with( v.first, "<xmlattr" ) )
        {
            BOOST_FAIL( "Unsupported child for XPO3:  " << v.first );
        }
    }
}

// ---------------------------------------------------------------------------|
void XPO3Container::clear()
{
    activeProfile_ = 0;
    profiles_.clear();
}

// ---------------------------------------------------------------------------|
void checkEqual( const XPO3Container& lhs, const XPO3Container& rhs )
{
    BOOST_TEST_MESSAGE( "Checking XPO3Container match..." );
    BOOST_TEST_MESSAGE( "Comparing Active Profile Id ..." );
    BOOST_CHECK_EQUAL( lhs.activeProfile_,  rhs.activeProfile_ );

    BOOST_TEST_MESSAGE( "Comparing Profile List ..." );
    CheckMaps( lhs.profiles_, rhs.profiles_ );
}

} // namespace XPO3NetconfIntegrationTest
