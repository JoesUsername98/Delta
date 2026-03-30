#include <Delta++DataSync/job_factory.h>
#include <Delta++DataSync/option_contract_sync.h>
#include <Delta++DataSync/yield_curve_sync.h>

#include <unordered_map>

namespace DPP::DataSync
{
    std::unique_ptr<IJob> createJob(const std::string_view jobName)
    {
        using FactoryFn = std::unique_ptr<IJob> (*)();

        static const std::unordered_map<std::string_view, FactoryFn> kRegistry = {
            {"option_contract_sync", []() -> std::unique_ptr<IJob> { return std::make_unique<OptionContractSyncJob>(); }},
            {"yield_curve", []() -> std::unique_ptr<IJob> { return std::make_unique<YieldCurveJob>(); }},
        };

        const auto it = kRegistry.find(jobName);
        if (it != kRegistry.end())
            return it->second();
        return nullptr;
    }
}
