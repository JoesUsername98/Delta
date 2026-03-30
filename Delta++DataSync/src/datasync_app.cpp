#include <Delta++DataSync/datasync_app.h>
#include <Delta++DataSync/job_factory.h>

#include <iostream>

namespace DPP::DataSync
{
    int runDataSyncApp(const ParsedArgs& args)
    {
        if (!args.job.has_value() || args.job->empty())
        {
            std::cerr << "Missing --job <name>\n";
            printUsage();
            return 2;
        }

        auto job = createJob(*args.job);
        if (!job)
        {
            std::cerr << "Unknown job: " << *args.job << "\n";
            return 2;
        }

        auto v = job->verifyArgs(args);
        if (!v.has_value())
        {
            std::cerr << v.error() << "\n";
            return 2;
        }

        return job->run(args);
    }
}
