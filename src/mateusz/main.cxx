#include <fjpythia/mateusz/fj_and_root.h>
#include <fjpythia/util/argparser.h>

int main ( int argc, char *argv[] )
{
	auto &args = FJPyUtil::ArgParser::Instance();
	args.addOpts(argc, argv);

	int rv = 0;

	if (args.isSet("--fjroot"))
		rv = fj_and_root();

	return rv;
}
