# Package version
ARTIFACT_NAME               = lsp-runtime-lib
ARTIFACT_VARS               = LSP_RUNTIME_LIB
ARTIFACT_HEADERS            = lsp-plug.in
ARTIFACT_EXPORT_ALL         = 1
VERSION                     = 0.5.0

# List of dependencies
TEST_DEPENDENCIES = \
  TEST_STDLIB \
  LSP_TEST_FW

DEPENDENCIES = \
  STDLIB \
  LSP_COMMON_LIB \
  LSP_LLTL_LIB

# For Linux-based systems, use libsndfile
ifeq ($(PLATFORM),Linux)
  DEPENDENCIES             += LIBSNDFILE
endif

# For BSD-based systems, use libsndfile
ifeq ($(PLATFORM),BSD)
  DEPENDENCIES             += LIBSNDFILE
endif