CPMAddPackage(
    NAME fmt
    GIT_TAG 10.1.1
    GITHUB_REPOSITORY fmtlib/fmt
)

CPMAddPackage(
    NAME spdlog
    GIT_TAG v1.12.0
    GITHUB_REPOSITORY gabime/spdlog
    OPTIONS
    "SPDLOG_FMT_EXTERNAL ON"
)