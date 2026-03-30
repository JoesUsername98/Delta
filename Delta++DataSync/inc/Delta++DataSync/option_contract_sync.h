#pragma once

#include <Delta++DataSync/job.h>

namespace DPP::DataSync
{
    class OptionContractSyncJob final : public IJob
    {
    public:
        std::expected<void, std::string> verifyArgs(const ParsedArgs& args) const override;
        int run(const ParsedArgs& args) override;
    };
}
