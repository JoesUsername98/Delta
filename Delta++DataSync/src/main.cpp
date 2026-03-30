#include <Delta++DataSync/datasync_app.h>
#include <Delta++DataSync/datasync_args.h>

int main(int argc, char** argv)
{
    auto parsed = DPP::DataSync::parseArgs(argc, argv);
    if (!parsed.has_value())
        return 2;
    return DPP::DataSync::runDataSyncApp(*parsed);
}
