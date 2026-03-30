#pragma once

#include <Delta++DataSync/datasync_args.h>

#include <expected>
#include <string>

namespace DPP::DataSync
{
    class IJob
    {
    public:
        virtual ~IJob() = default;

        virtual std::expected<void, std::string> verifyArgs(const ParsedArgs& args) const = 0;
        virtual int run(const ParsedArgs& args) = 0;
    };
}
