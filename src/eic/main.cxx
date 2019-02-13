#include <fjpythia/eic/example.h>
#include <fjpythia/util/argparser.h>

int main ( int argc, char *argv[] )
{
	FJPyUtil::ArgParser::Instance().addOpts(argc, argv);

	int rv = example();

    return rv;
}
