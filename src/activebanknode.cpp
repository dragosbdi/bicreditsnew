// Copyright (c) 2009-2012 The Darkcoin developers
// Copyright (c) 2009-2012 The Crave developers
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2009-2012 The Bitcredit developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "protocol.h"
#include "activebanknode.h"
#include "banknodeman.h"
#include <boost/lexical_cast.hpp>
#include "clientversion.h"

//
// Bootup the Banknode, look for a 50 000 BCR input and register on the network
//
void CActiveBanknode::ManageStatus()
{
    std::string errorMessage;

    if(!fBankNode) return;

    if (fDebug) LogPrintf("CActiveBanknode::ManageStatus() - Begin\n");

    //need correct adjusted time to send ping
    bool fIsInitialDownload = IsInitialBlockDownload();
    if(fIsInitialDownload) {
        status = BANKNODE_SYNC_IN_PROCESS;
        LogPrintf("CActiveBanknode::ManageStatus() - Sync in progress. Must wait until sync is complete to start Banknode.\n");
        return;
    }

    if(status == BANKNODE_INPUT_TOO_NEW || status == BANKNODE_NOT_CAPABLE || status == BANKNODE_SYNC_IN_PROCESS){
        status = BANKNODE_NOT_PROCESSED;
    }

    if(status == BANKNODE_NOT_PROCESSED) {
        if(strBankNodeAddr.empty()) {
            if(!GetLocal(service)) {
                notCapableReason = "Can't detect external address. Please use the Banknodeaddr configuration option.";
                status = BANKNODE_NOT_CAPABLE;
                LogPrintf("CActiveBanknode::ManageStatus() - not capable: %s\n", notCapableReason.c_str());
                return;
            }
        } else {
            service = CService(strBankNodeAddr);
        }

        LogPrintf("CActiveBanknode::ManageStatus() - Checking inbound connection to '%s'\n", service.ToString().c_str());
      
            if(!ConnectNode((CAddress)service, service.ToString().c_str())){
                notCapableReason = "Could not connect to " + service.ToString();
                status = BANKNODE_NOT_CAPABLE;
                LogPrintf("CActiveBanknode::ManageStatus() - not capable: %s\n", notCapableReason.c_str());
                return;
            }
        

        if(pwalletMain->IsLocked()){
            notCapableReason = "Wallet is locked.";
            status = BANKNODE_NOT_CAPABLE;
            LogPrintf("CActiveBanknode::ManageStatus() - not capable: %s\n", notCapableReason.c_str());
            return;
        }

        // Set defaults
        status = BANKNODE_NOT_CAPABLE;
        notCapableReason = "Unknown. Check debug.log for more information.\n";

        // Choose coins to use
        CPubKey pubKeyCollateralAddress;
        CKey keyCollateralAddress;

        if(GetBankNodeVin(vin, pubKeyCollateralAddress, keyCollateralAddress)) {

            if(GetInputAge(vin) < BANKNODE_MIN_CONFIRMATIONS){
                notCapableReason = "Input must have least " + boost::lexical_cast<string>(BANKNODE_MIN_CONFIRMATIONS) +
                        " confirmations - " + boost::lexical_cast<string>(GetInputAge(vin)) + " confirmations";
                LogPrintf("CActiveBanknode::ManageStatus() - %s\n", notCapableReason.c_str());
                status = BANKNODE_INPUT_TOO_NEW;
                return;
            }

            LogPrintf("CActiveBanknode::ManageStatus() - Is capable bank node!\n");

            status = BANKNODE_IS_CAPABLE;
            notCapableReason = "";

            pwalletMain->LockCoin(vin.prevout);

            // send to all nodes
            CPubKey pubKeyBanknode;
            CKey keyBanknode;

            if(!darkSendSigner.SetKey(strBankNodePrivKey, errorMessage, keyBanknode, pubKeyBanknode))
            {
                LogPrintf("Register::ManageStatus() - Error upon calling SetKey: %s\n", errorMessage.c_str());
                return;
            }

            if(!Register(vin, service, keyCollateralAddress, pubKeyCollateralAddress, keyBanknode, pubKeyBanknode, errorMessage)) {
            	LogPrintf("CActiveBanknode::ManageStatus() - Error on Register: %s\n", errorMessage.c_str());
            }

            return;
        } else {
            notCapableReason = "Could not find suitable coins!";
            LogPrintf("CActiveBanknode::ManageStatus() - %s\n", notCapableReason.c_str());
        }
    }

    //send to all peers
    if(!Dseep(errorMessage)) {
        LogPrintf("CActiveBanknode::ManageStatus() - Error on Ping: %s\n", errorMessage.c_str());
    }
}

// Send stop dseep to network for remote Banknode
bool CActiveBanknode::StopBankNode(std::string strService, std::string strKeyBanknode, std::string& errorMessage) {
    CTxIn vin;
    CKey keyBanknode;
    CPubKey pubKeyBanknode;

    if(!darkSendSigner.SetKey(strKeyBanknode, errorMessage, keyBanknode, pubKeyBanknode)) {
        LogPrintf("CActiveBanknode::StopBankNode() - Error: %s\n", errorMessage.c_str());
        return false;
    }

    return StopBankNode(vin, CService(strService), keyBanknode, pubKeyBanknode, errorMessage);
}

// Send stop dseep to network for main Banknode
bool CActiveBanknode::StopBankNode(std::string& errorMessage) {
    if(status != BANKNODE_IS_CAPABLE && status != BANKNODE_REMOTELY_ENABLED) {
        errorMessage = "Banknode is not in a running status";
        LogPrintf("CActiveBanknode::StopBankNode() - Error: %s\n", errorMessage.c_str());
        return false;
    }

    status = BANKNODE_STOPPED;

    CPubKey pubKeyBanknode;
    CKey keyBanknode;

    if(!darkSendSigner.SetKey(strBankNodePrivKey, errorMessage, keyBanknode, pubKeyBanknode))
    {
        LogPrintf("Register::ManageStatus() - Error upon calling SetKey: %s\n", errorMessage.c_str());
        return false;
    }

    return StopBankNode(vin, service, keyBanknode, pubKeyBanknode, errorMessage);
}

// Send stop dseep to network for any Banknode
bool CActiveBanknode::StopBankNode(CTxIn vin, CService service, CKey keyBanknode, CPubKey pubKeyBanknode, std::string& errorMessage) {
    pwalletMain->UnlockCoin(vin.prevout);
    return Dseep(vin, service, keyBanknode, pubKeyBanknode, errorMessage, true);
}

bool CActiveBanknode::Dseep(std::string& errorMessage) {
    if(status != BANKNODE_IS_CAPABLE && status != BANKNODE_REMOTELY_ENABLED) {
        errorMessage = "Banknode is not in a running status";
        LogPrintf("CActiveBanknode::Dseep() - Error: %s\n", errorMessage.c_str());
        return false;
    }

    CPubKey pubKeyBanknode;
    CKey keyBanknode;

    if(!darkSendSigner.SetKey(strBankNodePrivKey, errorMessage, keyBanknode, pubKeyBanknode))
    {
        LogPrintf("CActiveBanknode::Dseep() - Error upon calling SetKey: %s\n", errorMessage.c_str());
        return false;
    }

    return Dseep(vin, service, keyBanknode, pubKeyBanknode, errorMessage, false);
}

bool CActiveBanknode::Dseep(CTxIn vin, CService service, CKey keyBanknode, CPubKey pubKeyBanknode, std::string &retErrorMessage, bool stop) {
    std::string errorMessage;
    std::vector<unsigned char> vchBankNodeSignature;
    std::string strBankNodeSignMessage;
    int64_t bankNodeSignatureTime = GetAdjustedTime();

    std::string strMessage = service.ToString() + boost::lexical_cast<std::string>(bankNodeSignatureTime) + boost::lexical_cast<std::string>(stop);

    if(!darkSendSigner.SignMessage(strMessage, errorMessage, vchBankNodeSignature, keyBanknode)) {
        retErrorMessage = "sign message failed: " + errorMessage;
        LogPrintf("CActiveBanknode::Dseep() - Error: %s\n", retErrorMessage.c_str());
        return false;
    }

    if(!darkSendSigner.VerifyMessage(pubKeyBanknode, vchBankNodeSignature, strMessage, errorMessage)) {
        retErrorMessage = "Verify message failed: " + errorMessage;
        LogPrintf("CActiveBanknode::Dseep() - Error: %s\n", retErrorMessage.c_str());
        return false;
    }

    // Update Last Seen timestamp in Banknode list
    CBanknode* pmn = mnodeman.Find(vin);
    if(pmn != NULL)
    {
        if(stop)
            mnodeman.Remove(pmn->vin);
        else
            pmn->UpdateLastSeen();
    }
    else
    {
        // Seems like we are trying to send a ping while the Banknode is not registered in the network
        retErrorMessage = "Darksend Banknode List doesn't include our Banknode, Shutting down Banknode pinging service! " + vin.ToString();
        LogPrintf("CActiveBanknode::Dseep() - Error: %s\n", retErrorMessage.c_str());
        status = BANKNODE_NOT_CAPABLE;
        notCapableReason = retErrorMessage;
        return false;
    }

    //send to all peers
    LogPrintf("CActiveBanknode::Dseep() - RelayBanknodeEntryPing vin = %s\n", vin.ToString().c_str());
    mnodeman.RelayBanknodeEntryPing(vin, vchBankNodeSignature, bankNodeSignatureTime, stop);

    return true;
}

bool CActiveBanknode::Register(std::string strService, std::string strKeyBanknode, std::string txHash, std::string strOutputIndex, std::string& errorMessage) {
	CTxIn vin;
    CPubKey pubKeyCollateralAddress;
    CKey keyCollateralAddress;
    CPubKey pubKeyBanknode;
    CKey keyBanknode;

    if(!darkSendSigner.SetKey(strKeyBanknode, errorMessage, keyBanknode, pubKeyBanknode))
    {
        LogPrintf("CActiveBanknode::Register() - Error upon calling SetKey: %s\n", errorMessage.c_str());
        return false;
    }

    if(!GetBankNodeVin(vin, pubKeyCollateralAddress, keyCollateralAddress, txHash, strOutputIndex)) {
		errorMessage = "could not allocate vin";
    	LogPrintf("CActiveBanknode::Register() - Error: %s\n", errorMessage.c_str());
		return false;
	}
	return Register(vin, CService(strService), keyCollateralAddress, pubKeyCollateralAddress, keyBanknode, pubKeyBanknode, errorMessage);
}

bool CActiveBanknode::RegisterByPubKey(std::string strService, std::string strKeyBanknode, std::string collateralAddress, std::string& errorMessage) {
	CTxIn vin;
    CPubKey pubKeyCollateralAddress;
    CKey keyCollateralAddress;
    CPubKey pubKeyBanknode;
    CKey keyBanknode;

    if(!darkSendSigner.SetKey(strKeyBanknode, errorMessage, keyBanknode, pubKeyBanknode))
    {
    	LogPrintf("CActiveBanknode::RegisterByPubKey() - Error upon calling SetKey: %s\n", errorMessage.c_str());
    	return false;
    }

    if(!GetBankNodeVinForPubKey(collateralAddress, vin, pubKeyCollateralAddress, keyCollateralAddress)) {
		errorMessage = "could not allocate vin for collateralAddress";
    	LogPrintf("Register::Register() - Error: %s\n", errorMessage.c_str());
		return false;
	}
	return Register(vin, CService(strService), keyCollateralAddress, pubKeyCollateralAddress, keyBanknode, pubKeyBanknode, errorMessage);
}

bool CActiveBanknode::Register(CTxIn vin, CService service, CKey keyCollateralAddress, CPubKey pubKeyCollateralAddress, CKey keyBanknode, CPubKey pubKeyBanknode, std::string &retErrorMessage) {
    std::string errorMessage;
    std::vector<unsigned char> vchBankNodeSignature;
    std::string strBankNodeSignMessage;
    int64_t bankNodeSignatureTime = GetAdjustedTime();

    std::string vchPubKey(pubKeyCollateralAddress.begin(), pubKeyCollateralAddress.end());
    std::string vchPubKey2(pubKeyBanknode.begin(), pubKeyBanknode.end());

    std::string strMessage = service.ToString() + boost::lexical_cast<std::string>(bankNodeSignatureTime) + vchPubKey + vchPubKey2 + boost::lexical_cast<std::string>(PROTOCOL_VERSION);

    if(!darkSendSigner.SignMessage(strMessage, errorMessage, vchBankNodeSignature, keyCollateralAddress)) {
        retErrorMessage = "sign message failed: " + errorMessage;
        LogPrintf("CActiveBanknode::Register() - Error: %s\n", retErrorMessage.c_str());
        return false;
    }

    if(!darkSendSigner.VerifyMessage(pubKeyCollateralAddress, vchBankNodeSignature, strMessage, errorMessage)) {
        retErrorMessage = "Verify message failed: " + errorMessage;
        LogPrintf("CActiveBanknode::Register() - Error: %s\n", retErrorMessage.c_str());
        return false;
    }

    CBanknode* pmn = mnodeman.Find(vin);
    if(pmn == NULL)
    {
        LogPrintf("CActiveBanknode::Register() - Adding to Banknode list service: %s - vin: %s\n", service.ToString().c_str(), vin.ToString().c_str());
        CBanknode mn(service, vin, pubKeyCollateralAddress, vchBankNodeSignature, bankNodeSignatureTime, pubKeyBanknode, PROTOCOL_VERSION);
        mn.UpdateLastSeen(bankNodeSignatureTime);
        mnodeman.Add(mn);
    }

    //send to all peers
    LogPrintf("CActiveBanknode::Register() - RelayElectionEntry vin = %s\n", vin.ToString().c_str());
    mnodeman.RelayBanknodeEntry(vin, service, vchBankNodeSignature, bankNodeSignatureTime, pubKeyCollateralAddress, pubKeyBanknode, -1, -1, bankNodeSignatureTime, PROTOCOL_VERSION);

    return true;
}

bool CActiveBanknode::GetBankNodeVin(CTxIn& vin, CPubKey& pubkey, CKey& secretKey) {
	return GetBankNodeVin(vin, pubkey, secretKey, "", "");
}

bool CActiveBanknode::GetBankNodeVin(CTxIn& vin, CPubKey& pubkey, CKey& secretKey, std::string strTxHash, std::string strOutputIndex) {
    CScript pubScript;

    // Find possible candidates
    vector<COutput> possibleCoins = SelectCoinsBanknode();
    COutput *selectedOutput;

    // Find the vin
    if(!strTxHash.empty()) {
        // Let's find it
        uint256 txHash(strTxHash);
        int outputIndex = boost::lexical_cast<int>(strOutputIndex);
        bool found = false;
        BOOST_FOREACH(COutput& out, possibleCoins) {
            if(out.tx->GetHash() == txHash && out.i == outputIndex)
            {
                selectedOutput = &out;
                found = true;
                break;
            }
        }
        if(!found) {
            LogPrintf("CActiveBanknode::GetBankNodeVin - Could not locate valid vin\n");
            return false;
        }
    } else {
        // No output specified,  Select the first one
        if(possibleCoins.size() > 0) {
            selectedOutput = &possibleCoins[0];
        } else {
            LogPrintf("CActiveBanknode::GetBankNodeVin - Could not locate specified vin from possible list\n");
            return false;
        }
    }

    // At this point we have a selected output, retrieve the associated info
    return GetVinFromOutput(*selectedOutput, vin, pubkey, secretKey);
}


// Extract Banknode vin information from output
bool CActiveBanknode::GetVinFromOutput(COutput out, CTxIn& vin, CPubKey& pubkey, CKey& secretKey) {

    CScript pubScript;

    vin = CTxIn(out.tx->GetHash(),out.i);
    pubScript = out.tx->vout[out.i].scriptPubKey; // the inputs PubKey

    CTxDestination address1;
    ExtractDestination(pubScript, address1);
    CBitcreditAddress address2(address1);

    CKeyID keyID;
    if (!address2.GetKeyID(keyID)) {
        LogPrintf("CActiveBanknode::GetBankNodeVin - Address does not refer to a key\n");
        return false;
    }

    if (!pwalletMain->GetKey(keyID, secretKey)) {
        LogPrintf ("CActiveBanknode::GetBankNodeVin - Private key for address is not known\n");
        return false;
    }

    pubkey = secretKey.GetPubKey();
    return true;
}

bool CActiveBanknode::GetBankNodeVinForPubKey(std::string collateralAddress, CTxIn& vin, CPubKey& pubkey, CKey& secretKey) {
	return GetBankNodeVinForPubKey(collateralAddress, vin, pubkey, secretKey, "", "");
}

bool CActiveBanknode::GetBankNodeVinForPubKey(std::string collateralAddress, CTxIn& vin, CPubKey& pubkey, CKey& secretKey, std::string strTxHash, std::string strOutputIndex) {
    CScript pubScript;

    // Find possible candidates
    vector<COutput> possibleCoins = SelectCoinsBanknodeForPubKey(collateralAddress);
    COutput *selectedOutput;

    // Find the vin
	if(!strTxHash.empty()) {
		// Let's find it
		uint256 txHash(strTxHash);
        int outputIndex = boost::lexical_cast<int>(strOutputIndex);
		bool found = false;
		BOOST_FOREACH(COutput& out, possibleCoins) {
			if(out.tx->GetHash() == txHash && out.i == outputIndex)
			{
				selectedOutput = &out;
				found = true;
				break;
			}
		}
		if(!found) {
			LogPrintf("CActiveBanknode::GetBankNodeVinForPubKey - Could not locate valid vin\n");
			return false;
		}
	} else {
		// No output specified,  Select the first one
		if(possibleCoins.size() > 0) {
			selectedOutput = &possibleCoins[0];
		} else {
			LogPrintf("CActiveBanknode::GetBankNodeVinForPubKey - Could not locate specified vin from possible list\n");
			return false;
		}
    }

	// At this point we have a selected output, retrieve the associated info
	return GetVinFromOutput(*selectedOutput, vin, pubkey, secretKey);
}




// get all possible outputs for running banknode
vector<COutput> CActiveBanknode::SelectCoinsBanknode()
{
    vector<COutput> vCoins;
    vector<COutput> filteredCoins;

    // Retrieve all possible outputs
    pwalletMain->AvailableCoins(vCoins);

    // Filter
    
    if (chainActive.Tip()->nHeight<145000) {
    BOOST_FOREACH(const COutput& out, vCoins)
    {
        if(out.tx->vout[out.i].nValue == 250000*COIN) { //exactly
            filteredCoins.push_back(out);
        }
    }
	}
	else {
    BOOST_FOREACH(const COutput& out, vCoins)
    {
        if(out.tx->vout[out.i].nValue == 50000*COIN) { //exactly
            filteredCoins.push_back(out);
        }
    }
	}
    
    return filteredCoins;
}

// get all possible outputs for running banknode for a specific pubkey
vector<COutput> CActiveBanknode::SelectCoinsBanknodeForPubKey(std::string collateralAddress)
{
    CBitcreditAddress address(collateralAddress);
    CScript scriptPubKey;
    scriptPubKey= GetScriptForDestination(address.Get());
    vector<COutput> vCoins;
    vector<COutput> filteredCoins;

    // Retrieve all possible outputs
    pwalletMain->AvailableCoins(vCoins);

    // Filter
    if (chainActive.Tip()->nHeight<145000) {
    BOOST_FOREACH(const COutput& out, vCoins)
    {
        if(out.tx->vout[out.i].scriptPubKey == scriptPubKey && out.tx->vout[out.i].nValue == 250000*COIN) { //exactly
        	filteredCoins.push_back(out);
        }
    }
	}
	else {
    BOOST_FOREACH(const COutput& out, vCoins)
    {
        if(out.tx->vout[out.i].scriptPubKey == scriptPubKey && out.tx->vout[out.i].nValue == 50000*COIN) { //exactly
        	filteredCoins.push_back(out);
        }
    }
	} 
    return filteredCoins;
}


/* select coins with specified transaction hash and output index */


// when starting a banknode, this can enable to run as a hot wallet with no funds
bool CActiveBanknode::EnableHotColdBankNode(CTxIn& newVin, CService& newService)
{
    if(!fBankNode) return false;

    status = BANKNODE_REMOTELY_ENABLED;

    //The values below are needed for signing dseep messages going forward
    this->vin = newVin;
    this->service = newService;

    LogPrintf("CActiveBanknode::EnableHotColdBankNode() - Enabled! You may shut down the cold daemon.\n");

    return true;
}
