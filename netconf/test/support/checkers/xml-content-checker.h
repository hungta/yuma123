#ifndef __YUMA_XML_DATABASE_CONTENT_CHECKER_H
#define __YUMA_XML_DATABASE_CONTENT_CHECKER_H

// ---------------------------------------------------------------------------|
// Standard includes
// ---------------------------------------------------------------------------|
#include "test/support/misc-util/ptree-utils.h"

// ---------------------------------------------------------------------------|
// Standard includes
// ---------------------------------------------------------------------------|
#include <string>
#include <vector>
#include <memory>
#include <boost/xpressive/xpressive.hpp>

// ---------------------------------------------------------------------------|
// Boost includes
// ---------------------------------------------------------------------------|
#include <boost/test/unit_test.hpp>

// ---------------------------------------------------------------------------|
namespace YumaTest
{

/**
 * Check the results of a netconf query and ensure that all the
 * supplied strings are present.
 *
 * \tparam the type of data being checked. This type must provide a 
 * constructor that allows population from a boost::property tree.
 *
 * TODO: Add additional parameter to allow customisation of the check
 * function (currently this is hard-coded to 'checkEqual'
 */
template<class T> 
class XMLContentChecker
{
    typedef T type;
public:
    /**
     * Constructor.
     *
     * \param expData the contents to check. This is the test
     * harness's model of the remote database.
     */
    explicit XMLContentChecker( std::shared_ptr<T> expData )
    : expData_( expData )
    {
    }

    /** 
     * Destructor
     */
    ~XMLContentChecker()
    {
    }

    /**
     * Check the query results. This function extracts the <data>...</data> 
     * section from the returned query and uses it to construct an
     * object of 'type', containing the actual results returned by the
     * Netconf server. These results are then checked using the
     * function checkEqual.
     *
     * \param queryResult the query to check,
     */
    void operator()( const std::string& queryResult ) const
    {
        using namespace boost::xpressive;
        using boost::property_tree::ptree;

        // extract the <data> .... </data> part of the result
        sregex re = sregex::compile( "<data>(.*)</data>" );
        smatch what;
        BOOST_REQUIRE_MESSAGE( regex_search( queryResult, what, re ),
                "No <data> section in result!" );

        ptree pt = PTreeUtils::ParseXMlString( what[0] );
        checkEqual( *expData_, type( pt ) );
    }

private:
    /** the expected results */
    std::shared_ptr<type> expData_;
};

} // namespace YumaTest

#endif // __YUMA_XML_DATABASE_CONTENT_CHECKER_H

