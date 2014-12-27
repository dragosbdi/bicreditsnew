// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcredit Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCREDIT_BITCREDITCONSENSUS_H
#define BITCREDIT_BITCREDITCONSENSUS_H

#if defined(BUILD_BITCREDIT_INTERNAL) && defined(HAVE_CONFIG_H)
#include "config/bitcredit-config.h"
  #if defined(_WIN32)
    #if defined(DLL_EXPORT)
      #if defined(HAVE_FUNC_ATTRIBUTE_DLLEXPORT)
        #define EXPORT_SYMBOL __declspec(dllexport)
      #else
        #define EXPORT_SYMBOL
      #endif
    #endif
  #elif defined(HAVE_FUNC_ATTRIBUTE_VISIBILITY)
    #define EXPORT_SYMBOL __attribute__ ((visibility ("default")))
  #endif
#elif defined(MSC_VER) && !defined(STATIC_LIBBITCREDITCONSENSUS)
  #define EXPORT_SYMBOL __declspec(dllimport)
#endif

#ifndef EXPORT_SYMBOL
  #define EXPORT_SYMBOL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define BITCREDITCONSENSUS_API_VER 0

typedef enum bitcreditconsensus_error_t
{
    bitcreditconsensus_ERR_OK = 0,
    bitcreditconsensus_ERR_TX_INDEX,
    bitcreditconsensus_ERR_TX_SIZE_MISMATCH,
    bitcreditconsensus_ERR_TX_DESERIALIZE,
} bitcreditconsensus_error;

/** Script verification flags */
enum
{
    bitcreditconsensus_SCRIPT_FLAGS_VERIFY_NONE      = 0,
    bitcreditconsensus_SCRIPT_FLAGS_VERIFY_P2SH      = (1U << 0), // evaluate P2SH (BIP16) subscripts
};

/// Returns 1 if the input nIn of the serialized transaction pointed to by
/// txTo correctly spends the scriptPubKey pointed to by scriptPubKey under
/// the additional constraints specified by flags.
/// If not NULL, err will contain an error/success code for the operation
EXPORT_SYMBOL int bitcreditconsensus_verify_script(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen,
                                    const unsigned char *txTo        , unsigned int txToLen,
                                    unsigned int nIn, unsigned int flags, bitcreditconsensus_error* err);

EXPORT_SYMBOL unsigned int bitcreditconsensus_version();

#ifdef __cplusplus
} // extern "C"
#endif

#undef EXPORT_SYMBOL

#endif // BITCREDIT_BITCREDITCONSENSUS_H
