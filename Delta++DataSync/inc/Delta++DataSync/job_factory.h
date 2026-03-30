#pragma once

#include <Delta++DataSync/job.h>

#include <memory>
#include <string_view>

namespace DPP::DataSync
{
    std::unique_ptr<IJob> createJob(std::string_view jobName);
}
