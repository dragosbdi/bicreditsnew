// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcredit Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCREDIT_WALLET_ISMINE_H
#define BITCREDIT_WALLET_ISMINE_H

#include "key.h"
#include "script/standard.h"

class CKeyStore;
class CScript;

/** IsMine() return codes */
enum isminetype
{
    ISMINE_NO = 0,
    ISMINE_WATCH_ONLY = 1,
    ISMINE_SPENDABLE = 2,
    ISMINE_ALL = ISMINE_WATCH_ONLY | ISMINE_SPENDABLE
};
/** used for bitflags of isminetype */
typedef uint8_t isminefilter;

isminetype IsMine(const CKeyStore& keystore, const CScript& scriptPubKey);
isminetype IsMine(const CKeyStore& keystore, const CTxDestination& dest);

#endif // BITCREDIT_WALLET_ISMINE_H
